#include "lib/host/clock.h"

#include <windows.h>


lib_status host_clock_platform_monotonic_counter(lib_u64 *out_units,
    lib_u64 *out_units_per_second)
{
    LARGE_INTEGER counter;
    LARGE_INTEGER frequency;

    if (!QueryPerformanceCounter(&counter) || !QueryPerformanceFrequency(&frequency) ||
        counter.QuadPart < 0 || frequency.QuadPart <= 0) return LIB_STATUS_IO_ERROR;
    *out_units = (lib_u64)counter.QuadPart;
    *out_units_per_second = (lib_u64)frequency.QuadPart;
    return LIB_STATUS_OK;
}
