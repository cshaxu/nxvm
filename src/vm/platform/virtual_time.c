#include "type.h"

#include "vm/platform/virtual_time.h"

#if GLOBAL_PLATFORM == GLOBAL_PLATFORM_WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif


static type_status vm_platform_virtual_time_read_units(type_unsigned_64 *out_units,
    type_unsigned_64 *out_units_per_second)
{
#if GLOBAL_PLATFORM == GLOBAL_PLATFORM_WIN32
    LARGE_INTEGER counter;
    LARGE_INTEGER frequency;

    if (!QueryPerformanceCounter(&counter) || !QueryPerformanceFrequency(&frequency) ||
        counter.QuadPart < 0 || frequency.QuadPart <= 0) return TYPE_STATUS_FAULT;
    *out_units = (type_unsigned_64)counter.QuadPart;
    *out_units_per_second = (type_unsigned_64)frequency.QuadPart;
#else
    struct timeval value;

    if (gettimeofday(&value, STD_NULL) != 0 || value.tv_sec < 0 || value.tv_usec < 0) {
        return TYPE_STATUS_FAULT;
    }
    *out_units = ((type_unsigned_64)value.tv_sec * 1000000u) +
        (type_unsigned_64)value.tv_usec;
    *out_units_per_second = 1000000u;
#endif
    return TYPE_STATUS_OK;
}

type_status vm_platform_host_milliseconds(type_unsigned_64 *out_milliseconds)
{
    type_unsigned_64 units;
    type_unsigned_64 units_per_second;

    if (out_milliseconds == STD_NULL ||
        vm_platform_virtual_time_read_units(&units, &units_per_second) != TYPE_STATUS_OK ||
        units_per_second == 0u || units / units_per_second > UINT64_MAX / 1000u) {
        return TYPE_STATUS_FAULT;
    }
    *out_milliseconds = (units / units_per_second) * 1000u +
        ((units % units_per_second) * 1000u) / units_per_second;
    return TYPE_STATUS_OK;
}
