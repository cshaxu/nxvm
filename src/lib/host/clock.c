#include "lib/host/clock.h"

lib_status host_clock_monotonic_counter(lib_u64 *out_units,
    lib_u64 *out_units_per_second)
{
    if (out_units == LIB_NULL || out_units_per_second == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    return host_clock_platform_monotonic_counter(out_units, out_units_per_second);
}

lib_status host_clock_milliseconds(lib_u64 *out_milliseconds)
{
    lib_u64 units;
    lib_u64 units_per_second;

    if (out_milliseconds == LIB_NULL ||
        host_clock_monotonic_counter(&units, &units_per_second) != LIB_STATUS_OK ||
        units_per_second == 0u || units / units_per_second > UINT64_MAX / 1000u) {
        return LIB_STATUS_IO_ERROR;
    }
    *out_milliseconds = (units / units_per_second) * 1000u +
        ((units % units_per_second) * 1000u) / units_per_second;
    return LIB_STATUS_OK;
}
