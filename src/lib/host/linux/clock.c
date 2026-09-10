#define _POSIX_C_SOURCE 200809L

#include "lib/host/clock.h"

#include <time.h>

lib_status host_clock_platform_monotonic_counter(lib_u64 *out_units,
    lib_u64 *out_units_per_second)
{
    struct timespec value;

    if (out_units == LIB_NULL || out_units_per_second == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (clock_gettime(CLOCK_MONOTONIC, &value) != 0 || value.tv_sec < 0 ||
        value.tv_nsec < 0) return LIB_STATUS_IO_ERROR;
    *out_units = ((lib_u64)value.tv_sec * 1000000000u) + (lib_u64)value.tv_nsec;
    *out_units_per_second = 1000000000u;
    return LIB_STATUS_OK;
}
