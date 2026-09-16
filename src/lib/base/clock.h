#ifndef LIB_BASE_CLOCK_H
#define LIB_BASE_CLOCK_H

#include "lib/types/types_interface.h"

/* Implemented once by the selected host platform source. */
lib_status base_clock_platform_counter(lib_u64 *out_units,
    lib_u64 *out_units_per_second);

#endif
