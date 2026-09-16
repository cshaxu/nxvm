#include "lib/types/types_interface.h"
#include "lib/base/sync.h"

static base_sync_wait_result base_sync_wait_result_from_platform(lib_status status,
    lib_bool signaled)
{
    return status != LIB_STATUS_OK ? BASE_SYNC_WAIT_FAULT :
        signaled != LIB_FALSE ? BASE_SYNC_WAIT_SIGNALED :
        BASE_SYNC_WAIT_TIMED_OUT;
}

void base_sync_sleep_milliseconds(lib_u32 milliseconds)
{
    base_sync_platform_sleep_milliseconds(milliseconds);
}

void base_sync_yield(void)
{
    base_sync_platform_yield();
}

lib_status base_sync_event_create(base_sync_event_mode mode,
    base_sync_event **out_event)
{
    if (out_event == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_event = LIB_NULL;
    if (mode != BASE_SYNC_EVENT_MANUAL_RESET && mode != BASE_SYNC_EVENT_AUTO_RESET)
        return LIB_STATUS_INVALID_ARGUMENT;
    return base_sync_platform_event_create(mode == BASE_SYNC_EVENT_MANUAL_RESET,
        out_event);
}

void base_sync_event_destroy(base_sync_event *event)
{
    if (event == LIB_NULL) return;
    base_sync_platform_event_destroy(event);
}

lib_status base_sync_event_signal(base_sync_event *event)
{
    return event == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        base_sync_platform_event_signal(event);
}

lib_status base_sync_event_reset(base_sync_event *event)
{
    return event == LIB_NULL ? LIB_STATUS_INVALID_ARGUMENT :
        base_sync_platform_event_reset(event);
}

base_sync_wait_result base_sync_wait_any(base_sync_event *const *events,
    lib_u32 event_count, const base_sync_task *cancel_task,
    lib_u32 timeout_milliseconds, lib_u32 *out_event_index)
{
    const base_sync_event *platform_events[64];
    lib_u32 event_count_with_cancel = 0u;
    lib_u32 signaled_index = 0u;
    lib_u32 index;
    lib_bool signaled = LIB_FALSE;
    lib_status status;

    if ((event_count != 0u && events == LIB_NULL) ||
        (event_count == 0u && cancel_task == LIB_NULL) || event_count > 63u)
        return BASE_SYNC_WAIT_INVALID_ARGUMENT;
    if (out_event_index != LIB_NULL) *out_event_index = LIB_UINT32_MAX;
    if (cancel_task != LIB_NULL) {
        if (cancel_task->cancellation == LIB_NULL)
            return BASE_SYNC_WAIT_INVALID_ARGUMENT;
        platform_events[event_count_with_cancel++] = cancel_task->cancellation;
    }
    for (index = 0u; index < event_count; ++index) {
        if (events[index] == LIB_NULL)
            return BASE_SYNC_WAIT_INVALID_ARGUMENT;
        platform_events[event_count_with_cancel++] = events[index];
    }
    status = base_sync_platform_event_wait_many(platform_events, event_count_with_cancel,
        timeout_milliseconds, &signaled, &signaled_index);
    if (status != LIB_STATUS_OK || signaled == LIB_FALSE)
        return base_sync_wait_result_from_platform(status, signaled);
    if (cancel_task != LIB_NULL && signaled_index == 0u)
        return BASE_SYNC_WAIT_CANCELLED;
    if (out_event_index != LIB_NULL)
        *out_event_index = signaled_index - (cancel_task != LIB_NULL ? 1u : 0u);
    return BASE_SYNC_WAIT_SIGNALED;
}

base_sync_wait_result base_sync_event_wait(base_sync_event *event,
    lib_u32 timeout_milliseconds)
{
    return base_sync_wait_any(&event, 1u, LIB_NULL, timeout_milliseconds,
        LIB_NULL);
}

lib_status base_sync_task_create(base_sync_task_entry entry, void *context,
    base_sync_task **out_task)
{
    base_sync_event *cancellation;
    lib_status status;

    if (out_task == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_task = LIB_NULL;
    if (entry == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    status = base_sync_event_create(BASE_SYNC_EVENT_MANUAL_RESET, &cancellation);
    if (status != LIB_STATUS_OK) return status;
    status = base_sync_platform_task_create(entry, context, cancellation, out_task);
    if (status != LIB_STATUS_OK) base_sync_event_destroy(cancellation);
    return status;
}

void base_sync_task_request_cancel(base_sync_task *task)
{
    if (task != LIB_NULL) base_sync_event_signal(task->cancellation);
}

int base_sync_task_cancelled(const base_sync_task *task)
{
    const base_sync_event *events[1];
    lib_bool signaled = LIB_FALSE;

    if (task == LIB_NULL || task->cancellation == LIB_NULL) return 0;
    events[0] = task->cancellation;
    return base_sync_platform_event_wait_many(events, 1u, 0u, &signaled,
        LIB_NULL) == LIB_STATUS_OK && signaled != LIB_FALSE;
}

base_sync_wait_result base_sync_task_wait_cancel(const base_sync_task *task,
    lib_u32 timeout_milliseconds)
{
    if (task == LIB_NULL) return BASE_SYNC_WAIT_INVALID_ARGUMENT;
    return base_sync_wait_any(LIB_NULL, 0u, task, timeout_milliseconds,
        LIB_NULL);
}

void base_sync_task_join(base_sync_task *task)
{
    if (task != LIB_NULL) base_sync_platform_task_join(task);
}

void base_sync_task_destroy(base_sync_task *task)
{
    if (task == LIB_NULL) return;
    base_sync_task_request_cancel(task);
    base_sync_task_join(task);
    base_sync_event_destroy(task->cancellation);
    base_sync_platform_task_destroy(task);
}
