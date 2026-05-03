#include "timing.h"

#if defined(__APPLE__)
#include <mach/mach_time.h>

uint64_t now_ns(void) {
    static mach_timebase_info_data_t timebase;
    static int initialized;
    if (!initialized) {
        (void)mach_timebase_info(&timebase);
        initialized = 1;
    }
    return mach_absolute_time() * timebase.numer / timebase.denom;
}
#else
#include <time.h>

uint64_t now_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}
#endif
