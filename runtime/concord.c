// #include <stdint.h>
// #include <stdio.h>
// #include <unistd.h>
// #include <x86intrin.h>
// #include <errno.h>
// #include <pthread.h>

// #include <base/log.h>
// #include <base/assert.h>
// #include <base/init.h>
// #include <base/thread.h>
// #include <runtime/concord.h>
// #include <runtime/preempt.h>

// #include "defs.h"
// #include "sched.h"
// #ifdef DIRECTPATH
// #include "net/directpath/mlx5/mlx5.h"
// #endif 

// #define TOKEN 0
// #define MAX_KTHREADS 32


// volatile int *cpu_preempt_points[MAX_KTHREADS];
// __thread int concord_preempt_now;
// // DEFINE_PERTHREAD(int, concord_preempt_now);

// volatile int concord_timer_flag = 0;
// long long concord_sent[MAX_KTHREADS], concord_recv[MAX_KTHREADS];

// void concord_func() {
//     // printf("concord_lock_counter: %d\n", concord_lock_counter);
//     // if(concord_lock_counter)
//     //     return;
//     concord_preempt_now = 0;
//     // perthread_store(concord_preempt_now, 0);
//     concord_recv[myk()->kthread_idx]++;
//     // printf("---------------- thread_yield\n");
//     // thread_yield();

// #if defined(UNSAFE_PREEMPT_FLAG) || defined(UNSAFE_PREEMPT_SIMDREG)
//     if (likely(preempt_enabled())) {
//         // printf("---------------- thread_yield\n");
//      	thread_yield();
// 	}
//     else {
//         set_upreempt_needed();
//     }
// #else
//     thread_yield();
// #endif

// }

// void concord_disable() {
//     preempt_disable();
// }

// void concord_enable() {
//     preempt_enable();
// }

// void concord_set_preempt_flag(int flag) {
//     concord_preempt_now = flag;
// }

// #ifdef CONCORD_PREEMPT
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

// void* concord_timer(void*) {
//     base_init_thread();

//     set_thread_affinity(55);
    
//     // *(cpu_preempt_points[0]) = 1;
//     // return;

//     int i;
//     long long current;
//     for (i = 0; i < maxks; ++i) {
//         ACCESS_ONCE(last[i]) = now();
//     }
//     while (concord_timer_flag != -1) {
//         for (i = 0; i < maxks; ++i) {
//             current = now();
		
//             if (!concord_timer_flag) {
//                 ACCESS_ONCE(last[i]) = current;
//                 continue;
//             }   
//             if (current - ACCESS_ONCE(last[i]) >= TIMESLICE) {
//                 if (pending_uthreads(i) || pending_cqe(i)) {
//                     ++concord_sent[i];
//                     // printf("set %d\n", i);
//                     *(cpu_preempt_points[i]) = 1;
//                     ACCESS_ONCE(last[i]) = current;
//                 }
//             }   
//         }
//     } 

//     return NULL;
// }

// void uintr_timer_start() {
// 	concord_timer_flag = 1;
// 	start = now();
// }

// void uintr_timer_end() {
// 	end = now();
// 	concord_timer_flag = -1;
// }

// int uintr_init(void) {
//     memset(concord_sent, 0, sizeof(concord_sent));
//     memset(concord_recv, 0, sizeof(concord_recv));
    
//     // UINTR_TIMESLICE = atoi(getenv("UINTR_TIMESLICE")) * 1000L;
// 	TIMESLICE = 5 * 1000L;
//     // UINTR_TIMESLICE = 5 * 1000L;
//     // UINTR_TIMESLICE = 1000000000L * 1000L;
// 	log_info("TIMESLICE: %lld us", TIMESLICE / 1000);
//     return 0;
// }

// int uintr_init_thread(void) {
//     int kth_id = myk()->kthread_idx;
//     assert(kth_id >= 0 && kth_id < MAX_KTHREADS);
//     concord_preempt_now = 0;
//     cpu_preempt_points[kth_id] = &concord_preempt_now;
//     // perthread_store(concord_preempt_now, 0);
//     // cpu_preempt_points[kth_id] = perthread_ptr_stable(concord_preempt_now);
//     return 0;
// }

// int uintr_init_late(void) {
//     pthread_t timer_thread;
//     int ret = pthread_create(&timer_thread, NULL, concord_timer, NULL);
// 	BUG_ON(ret);
//     log_info("Concord timer pthread creates");

//     return 0;
// }

// void uintr_timer_summary(void) {
//     printf("Execution: %.9f\n", 1.*(end - start) / 1e9);
    
//     long long concord_sent_total = 0, concord_recv_total = 0;
//     int i;
//     for (i = 0; i < maxks; ++i) {
//         concord_sent_total += concord_sent[i];
//         concord_recv_total += concord_recv[i];
//     }
//     printf("Concord_sent: %lld\n", concord_sent_total);
//     printf("Concord_received: %lld\n", concord_recv_total);	
// }

// #endif // CONCORD_PREEMPT