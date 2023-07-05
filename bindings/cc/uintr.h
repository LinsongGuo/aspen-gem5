// support for user interrupts.

#pragma once

extern "C" {
#include <runtime/uintr.h>
}

namespace rt {

inline void UintrTimerStart(void) {
    uintr_timer_start();
}

inline void UintrTimerEnd(void) {
    uintr_timer_end();
}

inline void UintrTimerSummary(void) {
    uintr_timer_summary();
}

}  // namespace rt
