#include "lib/types/win32/scalar.h"
#include "lib/host/sync.h"

#include "lib/types/win32/sync.h"

struct host_sync_event { lib_win32_handle handle; };
struct host_sync_platform_task {
    lib_win32_handle thread;
    host_sync_platform_task_entry entry;
    void *context;
};

static lib_win32_dword LIB_WIN32_WINAPI host_sync_platform_main(lib_win32_lpvoid opaque)
{
    host_sync_platform_task *task = opaque;
    task->entry(task->context);
    return 0u;
}

void host_sync_platform_sleep_milliseconds(lib_u32 milliseconds)
{ lib_win32_sleep((lib_win32_dword)milliseconds); }
void host_sync_platform_yield(void) { lib_win32_sleep(0u); }

lib_status host_sync_platform_event_create(lib_bool manual_reset,
    host_sync_event **out_event)
{
    host_sync_event *event;
    if (out_event == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_event = LIB_NULL;
    event = lib_allocate_zero(1u, sizeof(*event));
    if (event == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    event->handle = lib_win32_create_event_a(LIB_NULL, manual_reset != LIB_FALSE, LIB_WIN32_FALSE, LIB_NULL);
    if (event->handle == LIB_NULL) { lib_release(event); return LIB_STATUS_IO_ERROR; }
    *out_event = event;
    return LIB_STATUS_OK;
}

void host_sync_platform_event_destroy(host_sync_event *event)
{ if (event != LIB_NULL) { if (event->handle != LIB_NULL) (void)lib_win32_close_handle(event->handle); lib_release(event); } }
void host_sync_platform_event_signal(host_sync_event *event)
{ if (event != LIB_NULL && event->handle != LIB_NULL) (void)lib_win32_set_event(event->handle); }
void host_sync_platform_event_reset(host_sync_event *event)
{ if (event != LIB_NULL && event->handle != LIB_NULL) (void)lib_win32_reset_event(event->handle); }

lib_status host_sync_platform_event_wait_many(
    const host_sync_event *const *events, lib_u32 event_count,
    lib_u32 timeout_milliseconds, lib_bool *out_signaled,
    lib_u32 *out_event_index)
{
    lib_win32_handle handles[LIB_WIN32_MAXIMUM_WAIT_OBJECTS];
    lib_win32_dword result;
    lib_u32 index;
    if (events == LIB_NULL || out_signaled == LIB_NULL || event_count == 0u ||
        event_count > (lib_u32)LIB_WIN32_MAXIMUM_WAIT_OBJECTS) return LIB_STATUS_INVALID_ARGUMENT;
    *out_signaled = LIB_FALSE;
    for (index = 0u; index < event_count; ++index) {
        if (events[index] == LIB_NULL || events[index]->handle == LIB_NULL)
            return LIB_STATUS_INVALID_ARGUMENT;
        handles[index] = events[index]->handle;
    }
    result = lib_win32_wait_for_multiple_objects((lib_win32_dword)event_count, handles, LIB_WIN32_FALSE,
        (lib_win32_dword)timeout_milliseconds);
    if (result < LIB_WIN32_WAIT_OBJECT_0 + event_count) {
        if (out_event_index != LIB_NULL) *out_event_index = result - LIB_WIN32_WAIT_OBJECT_0;
        *out_signaled = LIB_TRUE;
        return LIB_STATUS_OK;
    }
    return result == LIB_WIN32_WAIT_TIMEOUT ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR;
}

lib_status host_sync_platform_task_create(host_sync_platform_task_entry entry,
    void *context, host_sync_platform_task **out_task)
{
    host_sync_platform_task *task;
    if (entry == LIB_NULL || out_task == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_task = LIB_NULL;
    task = lib_allocate_zero(1u, sizeof(*task));
    if (task == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    task->entry = entry; task->context = context;
    task->thread = lib_win32_create_thread(LIB_NULL, 0u, host_sync_platform_main, task, 0u, LIB_NULL);
    if (task->thread == LIB_NULL) { lib_release(task); return LIB_STATUS_IO_ERROR; }
    *out_task = task;
    return LIB_STATUS_OK;
}

void host_sync_platform_task_join(host_sync_platform_task *task)
{ if (task != LIB_NULL && task->thread != LIB_NULL) (void)lib_win32_wait_for_single_object(task->thread, LIB_WIN32_INFINITE); }
void host_sync_platform_task_destroy(host_sync_platform_task *task)
{ if (task != LIB_NULL) { if (task->thread != LIB_NULL) (void)lib_win32_close_handle(task->thread); lib_release(task); } }
