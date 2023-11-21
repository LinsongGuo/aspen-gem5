// #include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <x86intrin.h>
// #include <errno.h>
// #include <pthread.h>
// #include <signal.h>

// #include <base/log.h>
// #include <base/assert.h>
// #include <base/init.h>
// #include <base/thread.h>
// #include <runtime/uintr.h>
// // #include <runtime/thread.h>

// #include "defs.h"
// #include "sched.h"
// #ifdef DIRECTPATH
// #include "net/directpath/mlx5/mlx5.h"
// #endif 

// #define MAX_KTHREADS 32

// bool kth_flag[MAX_KTHREADS];
// int kthread_num = 0;
// long long signal_sent[MAX_KTHREADS], signal_recv[MAX_KTHREADS];
// volatile int signal_timer_flag = 0;

// void signal_block() {
//     sigset_t mask;
//     sigemptyset(&mask);
//     sigaddset(&mask, SIGUSR1);
//     pthread_sigmask(SIG_BLOCK, &mask, NULL);
// }

// void signal_unblock(void) {
//     sigset_t mask;
//     sigemptyset(&mask);
//     sigaddset(&mask, SIGUSR1);
//     pthread_sigmask(SIG_UNBLOCK, &mask, NULL);
// }

// #ifdef SIGNAL_PREEMPT
// long long TIMESLICE = 1000000;
// long long start, end;

// void set_thread_affinity(int core) {
// 	cpu_set_t mask;
// 	CPU_ZERO(&mask);
// 	CPU_SET(core, &mask);
// 	sched_setaffinity(0, sizeof(mask), &mask);
// }

// long long now() {
// 	struct timespec ts;
// 	timespec_get(&ts, TIME_UTC);
// 	return ts.tv_sec * 1e9 + ts.tv_nsec;
// }

// void signal_handler(int signum) {
//     ++signal_recv[0];

// #if defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
//     if (!preempt_enabled()) {
// 		set_upreempt_needed();
// 		return;
// 	}
//     thread_yield();
// #else
//     thread_yield();
// #endif
// }

// bool pending_uthreads(int kidx) {
// #ifdef DIRECTPATH
//     return ACCESS_ONCE(ks[kidx]->rq_tail) != ACCESS_ONCE(ks[kidx]->rq_head);
// #else
//     return true;
// #endif
// }

// bool pending_cqe(int kidx) {
// #ifdef DIRECTPATH
//     return mlx5_rxq_pending(&rxqs[kidx]);
// #else
//     return true;
// #endif
// }

// volatile long long last[MAX_KTHREADS];
// void uintr_timer_upd(int kidx) {
//     ACCESS_ONCE(last[kidx]) = now();
// }

// void* signal_timer(void*) {
//     base_init_thread();
//     signal_block();

//     set_thread_affinity(55);
    
//     int i;
//     long long current;
//     for (i = 0; i < (maxks>>1); ++i) {
//         ACCESS_ONCE(last[i]) = now();
//     }
//     while (signal_timer_flag != -1) {
//         for (i = 0; i < (maxks>>1); ++i) {
//             current = now();
		
//             if (!signal_timer_flag) {
//                 ACCESS_ONCE(last[i]) = current;
//                 continue;
//             }   
//             if (current - ACCESS_ONCE(last[i]) >= TIMESLICE) {
//                 if (pending_uthreads(i) || pending_cqe(i)) {
//                     pthread_kill(kth_tid[i], SIGUSR1);
//                     ++signal_sent[i];
//                     ACCESS_ONCE(last[i]) = current;
//                 }
//             }   
//         }
//     } 

//     return NULL;
// }

// void* signal_timer2(void*) {
//     signal_block();

//     set_thread_affinity(53);
    
//     int i;
//     long long current;
//     for (i = (maxks>>1); i < maxks; ++i) {
//         ACCESS_ONCE(last[i]) = now();
//     }
//     while (signal_timer_flag != -1) {
//         for (i = (maxks>>1); i < maxks; ++i) {
//             current = now();
		
//             if (!signal_timer_flag) {
//                 ACCESS_ONCE(last[i]) = current;
//                 continue;
//             }   
//             if (current - ACCESS_ONCE(last[i]) >= TIMESLICE) {
//                 if (pending_uthreads(i) || pending_cqe(i)) {
//                     pthread_kill(kth_tid[i], SIGUSR1);
//                     ++signal_sent[i];
//                     ACCESS_ONCE(last[i]) = current;
//                 }
//             }   
//         }
//     } 

//     return NULL;
// }

// void uintr_timer_start() {
//     signal_timer_flag = 1;
// 	start = now();
// }

// void uintr_timer_end() {
// 	end = now();
// 	signal_timer_flag = -1;
// }

// int uintr_init(void) {
//     memset(kth_flag, 0, sizeof(kth_flag));
//     memset(signal_sent, 0, sizeof(signal_sent));
//     memset(signal_recv, 0, sizeof(signal_recv));

//     // TIMESLICE = atoi(getenv("TIMESLICE")) * 1000L;
//     TIMESLICE = 20 * 1000L;
// 	// TIMESLICE = 1000000000L * 1000L;
// 	log_info("TIMESLICE: %lld us", TIMESLICE / 1000);
//     return 0;
// }

// int uintr_init_thread(void) {
//     int kth_id = myk()->kthread_idx;
//     assert(kth_id >= 0 && kth_id < MAX_KTHREADS);
//     kth_flag[kth_id] = 1;
//     return 0;
// }

// int uintr_init_late(void) {
//     struct sigaction action;
//     action.sa_handler = signal_handler;
//     sigemptyset(&action.sa_mask);
//     action.sa_flags = 0;
//     if (sigaction(SIGUSR1, &action, NULL) != 0) {
//         log_err("signal handler registeration failed");
//     }

//     int i, flag = 0;
//     for (i = 0; i < MAX_KTHREADS; ++i) {
//         if (kth_flag[i]) {
//             if (flag) {
//                 log_err("wrong kth");
//                 break;
//             }
//             kthread_num++;
//         }
//         else {
//             flag = 1;
//         }
//     }
//     log_info("kthread_num = %d", kthread_num);

//     pthread_t timer_thread;
//     int ret = pthread_create(&timer_thread, NULL, signal_timer, NULL);
// 	BUG_ON(ret);
//     log_info("Signal timer pthread creates");

//     pthread_t timer_thread2;
//     int ret2 = pthread_create(&timer_thread2, NULL, signal_timer2, NULL);
// 	BUG_ON(ret2);
//     log_info("Signal timer pthread2 creates");
// }

// void uintr_timer_summary(void) {
//     printf("Execution: %.9f\n", 1.*(end - start) / 1e9);
    
//     long long signal_sent_total = 0, signal_recv_total = 0;
//     int i;
//     for (i = 0; i < maxks; ++i) {
//         signal_sent_total += signal_sent[i];
//         signal_recv_total += signal_recv[i];
//     }
//     printf("Signals_sent: %lld\n", signal_sent_total);
//     printf("Signals_received: %lld\n", signal_recv_total);	
// }
// #endif // SIGNAL_PREEMPT
