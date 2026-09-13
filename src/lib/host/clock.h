#ifndef LIB_HOST_CLOCK_H
#define LIB_HOST_CLOCK_H

#include "lib/types/types_interface.h"

/* Implemented once by the selected host platform source. */
lib_status host_clock_platform_counter(lib_u64 *out_units,
    lib_u64 *out_units_per_second);

#endif
