#include "type.h"

#include "vm/platform/virtual_time.h"

#if GLOBAL_PLATFORM == GLOBAL_PLATFORM_WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#define VM_PLATFORM_VIRTUAL_TIME_MAX_BATCH_TICKS 800000u

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

static type_status vm_platform_virtual_time_source_next(C_VOID *context,
    type_unsigned_64 *out_source_ticks)
{
    vm_platform_virtual_time_source *source =
        (vm_platform_virtual_time_source *)context;
    type_unsigned_64 units;
    type_unsigned_64 units_per_second;
    type_unsigned_64 elapsed_units;
    type_unsigned_64 whole_seconds;
    type_unsigned_64 partial_units;
    type_unsigned_64 ticks;

    if (source == STD_NULL || out_source_ticks == STD_NULL ||
        source->source_ticks_per_second == 0u) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_source_ticks = 0u;
    if (vm_platform_virtual_time_read_units(&units, &units_per_second) != TYPE_STATUS_OK) {
        return TYPE_STATUS_FAULT;
    }
    if (!source->initialized) {
        source->last_units = units;
        source->units_per_second = units_per_second;
        source->remainder = 0u;
        source->initialized = TYPE_TRUE;
        return TYPE_STATUS_OK;
    }
    if (source->units_per_second != units_per_second || units < source->last_units) {
        return TYPE_STATUS_FAULT;
    }
    elapsed_units = units - source->last_units;
    source->last_units = units;
    whole_seconds = elapsed_units / units_per_second;
    partial_units = elapsed_units % units_per_second;
    if (whole_seconds > UINT64_MAX / source->source_ticks_per_second ||
        partial_units > (UINT64_MAX - source->remainder) /
            source->source_ticks_per_second) return TYPE_STATUS_FAULT;
    ticks = whole_seconds * source->source_ticks_per_second;
    partial_units = (partial_units * source->source_ticks_per_second) + source->remainder;
    if (ticks > UINT64_MAX - (partial_units / units_per_second)) return TYPE_STATUS_FAULT;
    ticks += partial_units / units_per_second;
    source->remainder = partial_units % units_per_second;
    if (ticks > UINT64_MAX - source->pending_ticks) return TYPE_STATUS_FAULT;
    source->pending_ticks += ticks;
    *out_source_ticks = source->pending_ticks >
        VM_PLATFORM_VIRTUAL_TIME_MAX_BATCH_TICKS ?
        VM_PLATFORM_VIRTUAL_TIME_MAX_BATCH_TICKS : source->pending_ticks;
    source->pending_ticks -= *out_source_ticks;
    return TYPE_STATUS_OK;
}

static C_VOID vm_platform_virtual_time_source_reset(C_VOID *context)
{
    vm_platform_virtual_time_source *source =
        (vm_platform_virtual_time_source *)context;

    if (source == STD_NULL) return;
    source->last_units = 0u;
    source->remainder = 0u;
    source->pending_ticks = 0u;
    source->units_per_second = 0u;
    source->initialized = TYPE_FALSE;
}

type_status vm_platform_virtual_time_source_initialize(
    vm_platform_virtual_time_source *source, type_unsigned_64 source_ticks_per_second,
    vm_virtual_time_source *out_source)
{
    if (source == STD_NULL || out_source == STD_NULL || source_ticks_per_second == 0u) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    STD_MEMSET(source, 0, sizeof(*source));
    source->source_ticks_per_second = source_ticks_per_second;
    out_source->next = vm_platform_virtual_time_source_next;
    out_source->reset = vm_platform_virtual_time_source_reset;
    out_source->context = source;
    return TYPE_STATUS_OK;
}
