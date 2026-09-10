#ifndef LIB_HOST_CLOCK_H
#define LIB_HOST_CLOCK_H

#include "lib/host/clock_interface.h"

lib_status host_clock_platform_monotonic_counter(lib_u64 *out_units,
    lib_u64 *out_units_per_second);

#endif
