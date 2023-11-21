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
#include <base/thread.h>
#include <runtime/uintr.h>
#include <runtime/preempt.h>

#include "defs.h"
#include "sched.h"
#ifdef DIRECTPATH
#include "net/directpath/mlx5/mlx5.h"
#endif 

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
#define MAX_KTHREADS 32


int uintr_fd[MAX_KTHREADS];
int uipi_index[MAX_KTHREADS];
long long uintr_sent[MAX_KTHREADS], uintr_recv[MAX_KTHREADS];
volatile int uintr_timer_flag = 0;

volatile int *cpu_preempt_points[MAX_KTHREADS];
__thread int concord_preempt_now;

#if defined(UINTR_PREEMPT) || defined(CONCORD_PREEMPT)
long long TIMESLICE = 1000000;
long long start, end;
#endif

void concord_func() {
    // if(concord_lock_counter)
    //     return;
    concord_preempt_now = 0;
    uintr_recv[myk()->kthread_idx]++;
    
#if defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    if (likely(preempt_enabled())) {
     	thread_yield();
	}
    else {
        set_upreempt_needed();
    }
#else
    thread_yield();
#endif
}

void concord_disable() {
    preempt_disable();
}

void concord_enable() {
    preempt_enable();
}

void concord_set_preempt_flag(int flag) {
    concord_preempt_now = flag;
}

#if defined(UINTR_PREEMPT) || defined(CONCORD_PREEMPT)
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
    __attribute__((target("general-regs-only" /*, "inline-all-stringops"*/)))
     ui_handler(struct __uintr_frame *ui_frame,
		unsigned long long vector) {
		
	++uintr_recv[vector];

#if defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    if (likely(preempt_enabled())) {
     	thread_yield();
	}
    else {
        set_upreempt_needed();
    }
 
#else
    thread_yield();
#endif
}

bool pending_uthreads(int kidx) {
#ifdef DIRECTPATH
    return ACCESS_ONCE(ks[kidx]->rq_tail) != ACCESS_ONCE(ks[kidx]->rq_head);
#else
    return true;
#endif
}

bool pending_cqe(int kidx) {
#ifdef DIRECTPATH
    return mlx5_rxq_pending(&rxqs[kidx]);
#else
    return true;
#endif
}

volatile long long last[MAX_KTHREADS];
void uintr_timer_upd(int kidx) {
    ACCESS_ONCE(last[kidx]) = now();
}

void* uintr_timer(void*) {
    base_init_thread();

    set_thread_affinity(55);
    
    int i;
#ifdef UINTR_PREEMPT
    for (i = 0; i < maxks; ++i) {
        uipi_index[i] = uintr_register_sender(uintr_fd[i], 0);
        // log_info("uipi_index %d %d", i, uipi_index[i]);
        if (uipi_index[i] < 0) {
            log_err("failure to register uintr sender");
        }
    }	    
#endif
    // long long last = now(), current;
	// while (uintr_timer_flag != -1) {
    //     current = now();
		
	// 	if (!uintr_timer_flag) {
	// 		last = current;
	// 		continue;
	// 	}

    //     if (current - last >= TIMESLICE) {
	// 		last = current;
    //         for (i = 0; i < maxks; ++i) {
    //             // if (ACCESS_ONCE(ks[i]->rq_tail) != ACCESS_ONCE(ks[i]->rq_head)) {
    //             // if (pending_uthreads(i) || pending_cqe(i)) {
    //                 // log_info("uipi %d", i);
    //                 _senduipi(uipi_index[i]);
    //                 ++uintr_sent[i];
    //             // }
    //         }
    //     }
    // } 

    long long current;
    for (i = 0; i < maxks; ++i) {
        ACCESS_ONCE(last[i]) = now();
    }
    while (uintr_timer_flag != -1) {
        for (i = 0; i < maxks; ++i) {
            current = now();
		
            if (!uintr_timer_flag) {
                ACCESS_ONCE(last[i]) = current;
                continue;
            }   
            if (current - ACCESS_ONCE(last[i]) >= TIMESLICE) {
                if (pending_uthreads(i) || pending_cqe(i)) {
                    // printf("uipi %d: %lld %lld\n", i, uintr_sent[i], current - ACCESS_ONCE(last[i]));
#ifdef UINTR_PREEMPT
                    _senduipi(uipi_index[i]);
#elif defined(CONCORD_PREEMPT)
                    *(cpu_preempt_points[i]) = 1;
#endif
                    ++uintr_sent[i];
                    ACCESS_ONCE(last[i]) = current;
                }
            }   
        }
    } 

    return NULL;
}

void uintr_timer_start() {
	uintr_timer_flag = 1;
    start = now();
    _stui();
}

void uintr_timer_end() {
	end = now();
	uintr_timer_flag = -1;
}

int uintr_init(void) {
    memset(uintr_fd, 0, sizeof(uintr_fd));
    memset(uintr_sent, 0, sizeof(uintr_sent));
    memset(uintr_recv, 0, sizeof(uintr_recv));

    // TIMESLICE = atoi(getenv("TIMESLICE")) * 1000L;
    TIMESLICE = 10 * 1000L;
	log_info("TIMESLICE: %lld us", TIMESLICE / 1000);
    return 0;
}

int uintr_init_thread(void) {
    int kth_id = myk()->kthread_idx;
    assert(kth_id >= 0 && kth_id < MAX_KTHREADS);

    // For concord:
    concord_preempt_now = 0;
    cpu_preempt_points[kth_id] = &concord_preempt_now;

    // For uintr:
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
    pthread_t timer_thread;
    int ret = pthread_create(&timer_thread, NULL, uintr_timer, NULL);
	BUG_ON(ret);
    log_info("UINTR timer pthread creates");

    return 0;
}

void uintr_timer_summary(void) {
	printf("Execution: %.9f\n", 1.*(end - start) / 1e9);

    long long uintr_sent_total = 0, uintr_recv_total = 0;
    int i;
    for (i = 0; i < maxks; ++i) {
        uintr_sent_total += uintr_sent[i];
        uintr_recv_total += uintr_recv[i];
    }
    printf("Preemption_sent: %lld\n", uintr_sent_total);
    printf("Preemption_received: %lld\n", uintr_recv_total);	
}
#endif // UINTR or CONCORD