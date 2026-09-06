#ifndef LIB_HOST_CLOCK_INTERNAL_H
#define LIB_HOST_CLOCK_INTERNAL_H

#include "lib/host/clock.h"

lib_status host_clock_platform_monotonic_counter(lib_u64 *out_units,
    lib_u64 *out_units_per_second);

#endif
