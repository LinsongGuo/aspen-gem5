#include <stdint.h>
#include <stdio.h>
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
pid_t uintr_timer_tid;
pthread_barrier_t uintr_timer_barrier;
pthread_barrier_t uintr_init_barrier;

volatile int *cpu_preempt_points[MAX_KTHREADS];
__thread int concord_preempt_now;

long long TIMESLICE = 1000000;
long long start, end;
long long tsc1, tsc2;

void concord_func() {
    // if(concord_lock_counter)
    //     return;
    concord_preempt_now = 0;
    // uintr_recv[myk()->kthread_idx]++;
#if defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    if (likely(preempt_enabled())) {
        // uintr_recv[myk()->kthread_idx]++;
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

long long last_uintr = 0;
int interval[1000000], icnt = 0;
void __attribute__ ((interrupt))
    __attribute__((target("general-regs-only" /*, "inline-all-stringops"*/)))
     ui_handler(struct __uintr_frame *ui_frame,
		unsigned long long vector) {
		
	//++uintr_recv[0];
    // long long current_uintr = rdtsc();
    // if (last_uintr > 0 && current_uintr - last_uintr > 50  * 2000) {
            //  log_info("d: %lld", (current_uintr - last_uintr) / 2000);
    // }
    // interval[icnt++] = (current_uintr - last_uintr) / 2000;
    // last_uintr = current_uintr;

#ifdef M5_UTIMER
    m5_reset_stats(8,0);
#endif
#if defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    if (likely(preempt_enabled())) {
        ++uintr_recv[0];
        thread_yield();
	}
    else {
        set_upreempt_needed();
    }
 
#else
    thread_yield();
#endif
}

void signal_handler(int signum) {
    // uintr_recv[myk()->kthread_idx]++;
        
#if defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
    if (!preempt_enabled()) {
		set_upreempt_needed();
		return;
	}
    thread_yield();
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
    ACCESS_ONCE(last[kidx]) = rdtsc();
}

void* uintr_timer(void*) {
    uintr_timer_tid = thread_gettid();
    pthread_barrier_wait(&uintr_timer_barrier);

#ifdef SIGNAL_PREEMPT
    signal_block();
#endif 

    while(!uintr_timer_flag);
    _clui();
    
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

    long long current;
    for (i = 0; i < maxks; ++i) {
        ACCESS_ONCE(last[i]) = rdtsc();
    }

    while (uintr_timer_flag != -1) { 
        for (i = 0; i < maxks; ++i) {
            current = rdtsc();
            if (!uintr_timer_flag) {
                ACCESS_ONCE(last[i]) = current;
                continue;
            }   
            if (current - ACCESS_ONCE(last[i]) >= TIMESLICE) {
#ifdef SMART_PREEMPT
                if (pending_uthreads(i) || pending_cqe(i)) {
#endif
                    // printf("kill %d (%d): %lld | %lld, %lld\n", i, kth_tid[i], current - ACCESS_ONCE(last[i]), uintr_sent[i], uintr_recv[i]);
#ifdef UINTR_PREEMPT
                    _senduipi(uipi_index[i]);
#elif defined(CONCORD_PREEMPT)
                    *(cpu_preempt_points[i]) = 1;
#elif defined(SIGNAL_PREEMPT)
                    pthread_kill(kth_tid[i], SIGUSR1);
#endif
                    ++uintr_sent[i];
                    ACCESS_ONCE(last[i]) = current;
#ifdef SMART_PREEMPT
                }
#endif
            }   
        }
    } 

    return NULL;
}

void uintr_timer_start_internal() {
#ifdef M5_UTIMER
    if (uthread_quantum_us < 100000000)
        m5_utimer(uthread_quantum_us * 1000000);
#endif
	uintr_timer_flag = 1;
}

void uintr_timer_start() {
    start = now();
    tsc1 = rdtsc();
}

void uintr_timer_end() {
#ifdef M5_UTIMER
    if (uthread_quantum_us < 100000000)
        m5_utimer_end();
#endif
    tsc2 = rdtsc();
	end = now();
	uintr_timer_flag = -1;
}

// void utimer_start() {
//   FILE *file = fopen("/tmp/experiment", "r+b");
//   int start_flag = 1;
  
//   int ret = fwrite(&start_flag, sizeof(int), 1, file);
//   if (ret != 1) {
//     log_err("fwrite error\n");
//     fclose(file);
//     return;
//   }
//   // log_info("write sync status 1 to /tmp/experiment");

//   ret = fflush(file);
//   if (ret) {
//     log_err("fflush error\n");
//     fclose(file);
//     return;
//   }

//   ret = fclose(file);
//   if (ret) {
//     log_err("fclose error\n");
//   }

//   uintr_timer_start();
// }

void utimer_end_ornot() {
    if (uintr_timer_flag != 1) // not start yet
        return;

    // log_info(":uintr_timer_flag: %d", uintr_timer_flag);

    FILE *file = fopen("/tmp/experiment", "rb");
    if (file == NULL) {
         log_err("Error opening file: %s\n", strerror(errno));
    }

    int end_flag;
    int ret = fread(&end_flag, sizeof(int), 1, file);
    if (ret != 1) {
        log_err("fread error\n");
        fclose(file);
        return;
    }

    if (end_flag == 2) { // END
        // log_info("end_flag: %d", end_flag);

        uintr_timer_end();

        long long switch_end_tsc = rdtsc() + 8500000;
		m5_switch_cpu_addr();
		while(rdtsc() < switch_end_tsc);
        
        uintr_timer_summary();
    }

    fclose(file);
}

#define GHZ 2
int uintr_init(void) {
    memset(uintr_fd, 0, sizeof(uintr_fd));
    memset(uintr_sent, 0, sizeof(uintr_sent));
    memset(uintr_recv, 0, sizeof(uintr_recv));

    TIMESLICE = uthread_quantum_us * 1000 * GHZ;
	log_info("quantum: %lld us", TIMESLICE / 1000 / GHZ);
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

#ifndef M5_UTIMER
	int uintr_fd_ = uintr_create_fd(kth_id, 0);
	if (uintr_fd_ < 0) {
		log_err("failure to create uintr fd");
    }

    uintr_fd[kth_id] = uintr_fd_; 
#endif

    _stui();
    return 0;
}

int uintr_init_early(void) {
#ifdef SIGNAL_PREEMPT
    struct sigaction action;
    action.sa_handler = signal_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGUSR1, &action, NULL) != 0) {
        log_err("signal handler registeration failed");
    }
#endif

    if (uthread_quantum_us < 100000000) {
        pthread_t timer_thread;
        int ret = pthread_create(&timer_thread, NULL, uintr_timer, NULL);
        BUG_ON(ret);
        log_info("UINTR timer pthread creates");
    }
#ifdef SIGNAL_PREEMPT
    // pthread_t timer_thread2;
    // int ret2 = pthread_create(&timer_thread2, NULL, signal_timer2, NULL);
	// BUG_ON(ret2);
    // log_info("Signal timer pthread2 creates");

    // pthread_t timer_thread3;
    // int ret3 = pthread_create(&timer_thread3, NULL, signal_timer3, NULL);
	// BUG_ON(ret3);
    // log_info("Signal timer pthread3 creates");
#endif

    return 0;
}

int cores_pin_thread(pid_t tid, int core)
{
	cpu_set_t cpuset;
	int ret;

	CPU_ZERO(&cpuset);
	CPU_SET(core, &cpuset);

	ret = sched_setaffinity(tid, sizeof(cpu_set_t), &cpuset);
	if (ret < 0) {
		log_warn("cores: failed to set affinity for thread %d on core %d size %d with err %d",
				tid, core, sizeof(cpu_set_t), errno);
		return -errno;
	}

	return 0;
}

int uintr_init_late(void) {
    cores_pin_thread(uintr_timer_tid, 4);
    return 0;
}

void uintr_timer_summary(void) {
    preempt_disable();
	printf("Execution: %.9f sec, %lld cycles\n", 1.*(end - start) / 1e9, tsc2 - tsc1);

    long long uintr_sent_total = 0, uintr_recv_total = 0;
    int i;
    for (i = 0; i < maxks; ++i) {
        uintr_sent_total += uintr_sent[i];
        uintr_recv_total += uintr_recv[i];
    }
    printf("Preemption_sent: %lld\n", uintr_sent_total);
    printf("Preemption_received: %lld\n", uintr_recv_total);	

    // printf("Number of intervals: %u\n", icnt);
    // for (i = 1; i < icnt; ++i) {
    //     printf("%d ", interval[i]);
    // }
    preempt_enable();
}