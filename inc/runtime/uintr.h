/*
 * uintr.h - Functions for user interrupts.
 */

#pragma once

#include <base/thread.h>
#include <x86intrin.h>
#include <runtime/thread.h>

extern pthread_barrier_t uintr_timer_barrier;
extern pthread_barrier_t uintr_init_barrier;
extern void uintr_timer_start_internal(void);
extern void uintr_timer_start(void);
extern void uintr_timer_end(void);
extern void uintr_timer_summary(void);
extern void utimer_start(void);
extern void utimer_end_ornot(void);

extern void signal_block(void);
extern void signal_unblock(void);

extern void concord_func(void);
extern void concord_disable(void);
extern void concord_enable(void);
extern void concord_set_preempt_flag(int);