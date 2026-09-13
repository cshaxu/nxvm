#include "lib/types/types_interface.h"
#include "lib/host/sync.h"

struct host_sync_task {
    host_sync_platform_task *platform;
    host_sync_event *cancellation;
    host_sync_task_entry entry;
    void *context;
};

static void host_sync_task_main(void *opaque)
{
    host_sync_task *task = (host_sync_task *)opaque;

    if (task != LIB_NULL && task->entry != LIB_NULL)
        task->entry(task->context, task);
}

static host_sync_wait_result host_sync_wait_result_from_platform(lib_status status,
    lib_bool signaled)
{
    return status != LIB_STATUS_OK ? HOST_SYNC_WAIT_FAULT :
        signaled != LIB_FALSE ? HOST_SYNC_WAIT_SIGNALED :
        HOST_SYNC_WAIT_TIMED_OUT;
}

void host_sync_sleep_milliseconds(lib_u32 milliseconds)
{
    host_sync_platform_sleep_milliseconds(milliseconds);
}

void host_sync_yield(void)
{
    host_sync_platform_yield();
}

lib_status host_sync_event_create(host_sync_event **out_event)
{
    return host_sync_platform_event_create(LIB_TRUE, out_event);
}

void host_sync_event_destroy(host_sync_event *event)
{
    if (event == LIB_NULL) return;
    host_sync_platform_event_destroy(event);
}

void host_sync_event_signal(host_sync_event *event)
{
    if (event != LIB_NULL) host_sync_platform_event_signal(event);
}

void host_sync_event_reset(host_sync_event *event)
{
    if (event != LIB_NULL) host_sync_platform_event_reset(event);
}

host_sync_wait_result host_sync_wait_any(host_sync_event *const *events,
    lib_u32 event_count, const host_sync_task *cancel_task,
    lib_u32 timeout_milliseconds, lib_u32 *out_event_index)
{
    const host_sync_event *platform_events[64];
    lib_u32 event_count_with_cancel = 0u;
    lib_u32 signaled_index = 0u;
    lib_u32 index;
    lib_bool signaled = LIB_FALSE;
    lib_status status;

    if ((event_count != 0u && events == LIB_NULL) ||
        (event_count == 0u && cancel_task == LIB_NULL) || event_count > 63u)
        return HOST_SYNC_WAIT_INVALID_ARGUMENT;
    if (out_event_index != LIB_NULL) *out_event_index = LIB_UINT32_MAX;
    if (cancel_task != LIB_NULL) {
        if (cancel_task->cancellation == LIB_NULL)
            return HOST_SYNC_WAIT_INVALID_ARGUMENT;
        platform_events[event_count_with_cancel++] = cancel_task->cancellation;
    }
    for (index = 0u; index < event_count; ++index) {
        if (events[index] == LIB_NULL)
            return HOST_SYNC_WAIT_INVALID_ARGUMENT;
        platform_events[event_count_with_cancel++] = events[index];
    }
    status = host_sync_platform_event_wait_many(platform_events, event_count_with_cancel,
        timeout_milliseconds, &signaled, &signaled_index);
    if (status != LIB_STATUS_OK || signaled == LIB_FALSE)
        return host_sync_wait_result_from_platform(status, signaled);
    if (cancel_task != LIB_NULL && signaled_index == 0u)
        return HOST_SYNC_WAIT_CANCELLED;
    if (out_event_index != LIB_NULL)
        *out_event_index = signaled_index - (cancel_task != LIB_NULL ? 1u : 0u);
    return HOST_SYNC_WAIT_SIGNALED;
}

host_sync_wait_result host_sync_event_wait(host_sync_event *event,
    lib_u32 timeout_milliseconds)
{
    return host_sync_wait_any(&event, 1u, LIB_NULL, timeout_milliseconds,
        LIB_NULL);
}

lib_status host_sync_task_create(host_sync_task_entry entry, void *context,
    host_sync_task **out_task)
{
    host_sync_task *task;
    lib_status status;

    if (entry == LIB_NULL || out_task == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_task = LIB_NULL;
    task = (host_sync_task *)lib_allocate_zero(1u, sizeof(*task));
    if (task == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    task->entry = entry;
    task->context = context;
    status = host_sync_event_create(&task->cancellation);
    if (status == LIB_STATUS_OK)
        status = host_sync_platform_task_create(host_sync_task_main, task, &task->platform);
    if (status != LIB_STATUS_OK) {
        host_sync_event_destroy(task->cancellation);
        lib_release(task);
        return status;
    }
    *out_task = task;
    return LIB_STATUS_OK;
}

void host_sync_task_request_cancel(host_sync_task *task)
{
    if (task != LIB_NULL) host_sync_event_signal(task->cancellation);
}

int host_sync_task_cancelled(const host_sync_task *task)
{
    const host_sync_event *events[1];
    lib_bool signaled = LIB_FALSE;

    if (task == LIB_NULL || task->cancellation == LIB_NULL) return 0;
    events[0] = task->cancellation;
    return host_sync_platform_event_wait_many(events, 1u, 0u, &signaled,
        LIB_NULL) == LIB_STATUS_OK && signaled != LIB_FALSE;
}

host_sync_wait_result host_sync_task_wait_cancel(const host_sync_task *task,
    lib_u32 timeout_milliseconds)
{
    if (task == LIB_NULL) return HOST_SYNC_WAIT_INVALID_ARGUMENT;
    return host_sync_wait_any(LIB_NULL, 0u, task, timeout_milliseconds,
        LIB_NULL);
}

void host_sync_task_join(host_sync_task *task)
{
    if (task != LIB_NULL) host_sync_platform_task_join(task->platform);
}

void host_sync_task_destroy(host_sync_task *task)
{
    if (task == LIB_NULL) return;
    host_sync_task_request_cancel(task);
    host_sync_task_join(task);
    host_sync_platform_task_destroy(task->platform);
    host_sync_event_destroy(task->cancellation);
    lib_release(task);
}
