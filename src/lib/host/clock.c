#include "lib/host/clock_interface.h"
#include "lib/host/clock.h"

lib_status host_clock_monotonic_counter(lib_u64 *out_units,
    lib_u64 *out_units_per_second)
{
    if (out_units == LIB_NULL || out_units_per_second == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    return host_clock_platform_counter(out_units, out_units_per_second);
}

lib_status host_clock_milliseconds(lib_u64 *out_milliseconds)
{
    lib_u64 units;
    lib_u64 units_per_second;
    lib_status status;

    if (out_milliseconds == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = host_clock_monotonic_counter(&units, &units_per_second);
    if (status != LIB_STATUS_OK) return status;
    if (units_per_second == 0u) return LIB_STATUS_IO_ERROR;
    if (units_per_second > LIB_UINT64_MAX / 1000u ||
        units / units_per_second > LIB_UINT64_MAX / 1000u)
        return LIB_STATUS_LIMIT_EXCEEDED;
    *out_milliseconds = (units / units_per_second) * 1000u +
        ((units % units_per_second) * 1000u) / units_per_second;
    return LIB_STATUS_OK;
}
