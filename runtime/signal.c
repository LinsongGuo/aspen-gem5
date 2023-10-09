#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <x86intrin.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>

#include <base/log.h>
#include <base/assert.h>
#include <base/init.h>
#include <base/thread.h>
#include <runtime/uintr.h>
// #include <runtime/thread.h>

#include "defs.h"
#include "sched.h"

#define MAX_KTHREADS 32

long long UINTR_TIMESLICE = 1000000;
long long start, end;
bool kth_flag[MAX_KTHREADS];
int kthread_num = 0;
long long signal_sent[MAX_KTHREADS], signal_recv[MAX_KTHREADS];
volatile int signal_timer_flag = 0;

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

void signal_block() {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);
}

void signal_unblock(void) {
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
}

#ifdef SIGNAL_PREEMPT
// DEFINE_PERTHREAD(unsigned int, non_reentrance);
// DEFINE_PERTHREAD(unsigned int, uintr_pending);

void signal_handler(int signum) {
    ++signal_recv[0];
    // printf("singal handler\n");

#if defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    // if (perthread_read(non_reentrance)) {
    //     perthread_store(uintr_pending, 1);
    // }
    // else {
    //     // printf("reentrace\n");
    //     perthread_store(uintr_pending, 0);
    //     thread_yield();
    //     signal_unblock();	
    // }
    if (!preempt_enabled()) {
		set_upreempt_needed();
		return;
	}
    thread_yield();
#else
    thread_yield();
	// signal_unblock();
#endif
}

void* signal_timer(void*) {
    base_init_thread();
    signal_block();

    int i;
    long long last = now(), current;
	while (signal_timer_flag != -1) {
        current = now();
		
		if (!signal_timer_flag) {
			last = current;
			continue;
		}

        if (current - last >= UINTR_TIMESLICE) {
			last = current;
            // kill(0, SIGUSR1);
            // ++signal_sent[0];
            for (i = 0; i < kthread_num; ++i) {
                // printf("sent: %d", i);
                pthread_kill(kth_tid[i], SIGUSR1);
            }
        }

        long long x = now();
        printf("signal sent: %lld, %lld\n", x - current, (x - current) / kthread_num);
    } 

    return NULL;
}

void uintr_timer_start() {
    signal_timer_flag = 1;
	start = now();
}

void uintr_timer_end() {
	end = now();
	signal_timer_flag = -1;
}

int uintr_init(void) {
    memset(kth_flag, 0, sizeof(kth_flag));
    memset(signal_sent, 0, sizeof(signal_sent));
    memset(signal_recv, 0, sizeof(signal_recv));

    UINTR_TIMESLICE = atoi(getenv("UINTR_TIMESLICE")) * 1000L;
	log_info("UINTR_TIMESLICE: %lld us", UINTR_TIMESLICE / 1000);
    return 0;
}

int uintr_init_thread(void) {
    int kth_id = myk()->kthread_idx;
    assert(kth_id >= 0 && kth_id < MAX_KTHREADS);

    kth_flag[kth_id] = 1;
    
    // perthread_store(non_reentrance, 0);
    // perthread_store(uintr_pending, 0);

    return 0;
}

int uintr_init_late(void) {
    struct sigaction action;
    action.sa_handler = signal_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGUSR1, &action, NULL) != 0) {
        log_err("signal handler registeration failed");
    }

    int i, flag = 0;
    for (i = 0; i < MAX_KTHREADS; ++i) {
        if (kth_flag[i]) {
            if (flag) {
                log_err("wrong kth");
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
    int ret = pthread_create(&timer_thread, NULL, signal_timer, NULL);
	BUG_ON(ret);
    log_info("timer pthread creates");
}

void uintr_timer_summary(void) {
	// printf("start = %lld, end = %lld, time = %lld\n", start, end, end - start);
    printf("Execution: %.9f\n", 1.*(end - start) / 1e9);
    
    long long uintr_sent_total = 0, uintr_recv_total = 0;
    uintr_sent_total += signal_sent[0];
    uintr_recv_total += signal_recv[0];
    
    printf("Signals_sent: %lld\n", uintr_sent_total);
    printf("Signals_received: %lld\n", uintr_recv_total);	
}
#endif
