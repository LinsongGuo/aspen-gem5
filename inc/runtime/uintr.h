/*
 * uintr.h - Functions for user interrupts.
 */

#pragma once

#include <base/thread.h>
#include <x86intrin.h>
#include <runtime/thread.h>

// DECLARE_PERTHREAD(unsigned int, non_reentrance);
// DECLARE_PERTHREAD(unsigned int, uintr_pending);

extern void uintr_timer_start(void);
extern void uintr_timer_end(void);
extern void uintr_timer_summary(void);

extern void signal_block(void);
extern void signal_unblock(void);

// static inline void enter_non_reentrance(void) {
//     perthread_store(non_reentrance, 1);
//     barrier();
// }

// static inline void exit_non_reentrance(void) {
//     barrier();
//     perthread_store(non_reentrance, 0);
//     if (perthread_read(uintr_pending)) {
// #ifdef SIGNAL_PREEMPT
// 		signal_block();
// #else
// 		_clui();
// #endif
//         perthread_store(uintr_pending, 0);
//         thread_yield();
// #ifdef SIGNAL_PREEMPT
// 		signal_unblock();
// #else
// 		_stui();
// #endif
//     }
// }