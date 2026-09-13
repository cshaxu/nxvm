#include "lib/host/clock.h"
#include "lib/types/win32/clock.h"

lib_status host_clock_platform_counter(lib_u64 *out_units,
    lib_u64 *out_units_per_second)
{
    lib_win32_counter counter;
    lib_win32_counter frequency;

    if (out_units == LIB_NULL || out_units_per_second == LIB_NULL ||
        !lib_win32_query_performance_counter(&counter) ||
        !lib_win32_query_performance_frequency(&frequency) || counter.QuadPart < 0 ||
        frequency.QuadPart <= 0) return LIB_STATUS_IO_ERROR;
    *out_units = (lib_u64)counter.QuadPart;
    *out_units_per_second = (lib_u64)frequency.QuadPart;
    return LIB_STATUS_OK;
}
