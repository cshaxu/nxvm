#ifndef LIB_HOST_CLOCK_H
#define LIB_HOST_CLOCK_H

#include "lib/base/base.h"

/* This counter is host-only. It never represents or advances guest time. */
lib_status host_clock_monotonic_counter(lib_u64 *out_units,
    lib_u64 *out_units_per_second);
lib_status host_clock_milliseconds(lib_u64 *out_milliseconds);

#endif
