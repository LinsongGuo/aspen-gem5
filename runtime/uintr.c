#include <stdint.h>
#include <stdio.h>
// #include <stdlib.h>
#include <unistd.h>
#include <x86intrin.h>
#include <errno.h>
#include <pthread.h>

#include <base/log.h>
#include <base/assert.h>
#include <base/init.h>
#include <runtime/uintr.h>

#include "defs.h"
#include "sched.h"

#ifndef __NR_uintr_register_handler
#define __NR_uintr_register_handler	471
#define __NR_uintr_unregister_handler	472
#define __NR_uintr_create_fd		473
#define __NR_uintr_register_sender	474
#define __NR_uintr_unregister_sender	475
#define __NR_uintr_wait			476
#endif

#define uintr_register_handler(handler, flags)	syscall(__NR_uintr_register_handler, handler, flags)
#define uintr_unregister_handler(flags)		syscall(__NR_uintr_unregister_handler, flags)
#define uintr_create_fd(vector, flags)		syscall(__NR_uintr_create_fd, vector, flags)
#define uintr_register_sender(fd, flags)	syscall(__NR_uintr_register_sender, fd, flags)
#define uintr_unregister_sender(ipi_idx, flags)	syscall(__NR_uintr_unregister_sender, ipi_idx, flags)
#define uintr_wait(flags)			syscall(__NR_uintr_wait, flags)

#define TOKEN 0
#define MAX_KTHREADS 16


long long UINTR_TIMESLICE = 1000000;
int uintr_fd[MAX_KTHREADS], kthread_num = 0;
int uipi_index[MAX_KTHREADS];
long long start, end;
long long uintr_sent[MAX_KTHREADS], uintr_recv[MAX_KTHREADS];
volatile int uintr_timer_flag = 0;

void uintr_summary(void);

// void compute(int cycles) {
// 	unsigned long long c1 = __rdtsc(), c2 = c1;
// 	while (c2 - c1 <= cycles) {
// 		c2 = __rdtsc();
// 	}
// }

// void print_entry() {
// 	printf("entry\n");
// }

// void print_exit() {
// 	printf("exit\n");
// }

// void print_time(long long diff) {
// 	printf("%lld\n", diff);	
// }


void set_thread_affinity(int core) {
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(core, &mask);
	sched_setaffinity(0, sizeof(mask), &mask);
}


long long now() {
	struct timespec ts;
	timespec_get(&ts, TIME_UTC);
	return ts.tv_sec * 1e9 + ts.tv_nsec;
}

void __attribute__ ((interrupt))
     __attribute__((target("general-regs-only", "inline-all-stringops")))
     ui_handler(struct __uintr_frame *ui_frame,
		unsigned long long vector) {
	
	// print_entry();
	
	++uintr_recv[vector];
	thread_yield();
	
	_stui();	

	// print_exit();		
}

void uintr_timer_start() {
	uintr_timer_flag = 1;
	start = now();
}

void uintr_timer_end() {
	end = now();
	uintr_timer_flag = -1;
}

void* uintr_timer(void*) {
    base_init_thread();

    // set_thread_affinity(12);
    // uintr_timer_start();
    
    int i;
    for (i = 0; i < kthread_num; ++i) {
        uipi_index[i] = uintr_register_sender(uintr_fd[i], 0);
        if (uipi_index[i] < 0) {
            log_err("failure to register uintr sender");
        }
    }	    

    long long last = now(), current;
	while (uintr_timer_flag != -1) {
        current = now();
		
		if (!uintr_timer_flag) {
			last = current;
			continue;
		}

        if (current - last >= UINTR_TIMESLICE) {
			last = current;
            for (i = 0; i < kthread_num; ++i) {
                _senduipi(uipi_index[i]);
                ++uintr_sent[i];
            }
        }
    } 

    return NULL;
}

int uintr_init(void) {
    memset(uintr_fd, 0, sizeof(uintr_fd));
    memset(uintr_sent, 0, sizeof(uintr_sent));
    memset(uintr_recv, 0, sizeof(uintr_recv));

    UINTR_TIMESLICE = atoi(getenv("UINTR_TIMESLICE")) * 1000L;
	log_info("UINTR_TIMESLICE: %lld us", UINTR_TIMESLICE / 1000);
    return 0;
}

int uintr_init_thread(void) {
    int kth_id = myk()->kthread_idx;
    assert(kth_id >= 0 && kth_id < 64);

	if (uintr_register_handler(ui_handler, 0)) {
		log_err("failure to register uintr handler");
    }

	int uintr_fd_ = uintr_create_fd(kth_id, 0);
	if (uintr_fd_ < 0) {
		log_err("failure to create uintr fd");
    }
    uintr_fd[kth_id] = uintr_fd_; 

    return 0;
}

int uintr_init_late(void) {
    int i, flag = 0;
    for (i = 0; i < MAX_KTHREADS; ++i) {
        if (uintr_fd[i] > 0) {
            if (flag) {
                log_err("wrong uintr_fd");
                break;
            }
            kthread_num++;
        }
        else {
            flag = 1;
        }
    }
    log_info("kthread_num = %d", kthread_num);

    pthread_t timer_thread;
    int ret = pthread_create(&timer_thread, NULL, uintr_timer, NULL);
	BUG_ON(ret);

    log_info("pthread_create ends");
}

void uintr_timer_summary(void) {
	// printf("start = %lld, end = %lld, time = %lld\n", start, end, end - start);
    printf("Execution: %.9f\n", 1.*(end - start) / 1e9);
    
    long long uintr_sent_total = 0, uintr_recv_total = 0;
    int i;
    for (i = 0; i < kthread_num; ++i) {
        uintr_sent_total += uintr_sent[i];
        uintr_recv_total += uintr_recv[i];
    }
    printf("Uintrs_sent: %lld\n", uintr_sent_total);
    printf("Uintrs_received: %lld\n", uintr_recv_total);	
}

