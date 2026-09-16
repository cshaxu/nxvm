#ifndef LIB_BASE_CLOCK_INTERFACE_H
#define LIB_BASE_CLOCK_INTERFACE_H

#include "lib/types/types_interface.h"

/* This counter reports only host monotonic time. */
lib_status base_clock_monotonic_counter(lib_u64 *out_units,
    lib_u64 *out_units_per_second);
lib_status base_clock_milliseconds(lib_u64 *out_milliseconds);

#endif
