#ifndef LIB_HOST_CLOCK_INTERFACE_H
#define LIB_HOST_CLOCK_INTERFACE_H

#include "lib/base/base_interface.h"

/* This counter reports only host monotonic time. */
lib_status host_clock_monotonic_counter(lib_u64 *out_units,
    lib_u64 *out_units_per_second);
lib_status host_clock_milliseconds(lib_u64 *out_milliseconds);

#endif
