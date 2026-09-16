#include "lib/types/win32/scalar.h"
#include "lib/base/sync.h"

#include "lib/types/win32/sync.h"

struct base_sync_event { lib_win32_handle handle; };
struct base_sync_mutex { lib_win32_critical_section gate; };

lib_status base_sync_mutex_create(base_sync_mutex **out_mutex)
{
    base_sync_mutex *mutex;
    if (out_mutex == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_mutex = LIB_NULL;
    mutex = lib_allocate(sizeof(*mutex));
    if (mutex == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    if (!lib_win32_initialize_critical_section_and_spin_count(&mutex->gate, 0u)) {
        lib_release(mutex);
        return LIB_STATUS_IO_ERROR;
    }
    *out_mutex = mutex;
    return LIB_STATUS_OK;
}

void base_sync_mutex_destroy(base_sync_mutex *mutex)
{
    if (mutex == LIB_NULL) return;
    lib_win32_delete_critical_section(&mutex->gate);
    lib_release(mutex);
}

void base_sync_mutex_lock(base_sync_mutex *mutex)
{ lib_win32_enter_critical_section(&mutex->gate); }
void base_sync_mutex_unlock(base_sync_mutex *mutex)
{ lib_win32_leave_critical_section(&mutex->gate); }

struct base_sync_win32_task {
    base_sync_task task;
    lib_win32_handle thread;
};

static lib_win32_dword LIB_WIN32_WINAPI base_sync_platform_main(lib_win32_lpvoid opaque)
{
    struct base_sync_win32_task *state = opaque;
    state->task.entry(state->task.context, &state->task);
    return 0u;
}

void base_sync_platform_sleep_milliseconds(lib_u32 milliseconds)
{ lib_win32_sleep((lib_win32_dword)milliseconds); }
void base_sync_platform_yield(void) { lib_win32_sleep(0u); }

lib_status base_sync_platform_event_create(lib_bool manual_reset,
    base_sync_event **out_event)
{
    base_sync_event *event;
    if (out_event == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_event = LIB_NULL;
    event = lib_allocate_zero(1u, sizeof(*event));
    if (event == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    event->handle = lib_win32_create_event_a(LIB_NULL, manual_reset != LIB_FALSE, LIB_WIN32_FALSE, LIB_NULL);
    if (event->handle == LIB_NULL) { lib_release(event); return LIB_STATUS_IO_ERROR; }
    *out_event = event;
    return LIB_STATUS_OK;
}

void base_sync_platform_event_destroy(base_sync_event *event)
{ if (event != LIB_NULL) { if (event->handle != LIB_NULL) (void)lib_win32_close_handle(event->handle); lib_release(event); } }
lib_status base_sync_platform_event_signal(base_sync_event *event)
{ return lib_win32_set_event(event->handle) ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR; }
lib_status base_sync_platform_event_reset(base_sync_event *event)
{ return lib_win32_reset_event(event->handle) ? LIB_STATUS_OK : LIB_STATUS_IO_ERROR; }

lib_status base_sync_platform_event_wait_many(
    const base_sync_event *const *events, lib_u32 event_count,
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

lib_status base_sync_platform_task_create(base_sync_task_entry entry,
    void *context, base_sync_event *cancellation, base_sync_task **out_task)
{
    struct base_sync_win32_task *task;
    if (out_task == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_task = LIB_NULL;
    if (entry == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    task = lib_allocate_zero(1u, sizeof(*task));
    if (task == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    task->task.entry = entry; task->task.context = context;
    task->task.cancellation = cancellation;
    task->thread = lib_win32_create_thread(LIB_NULL, 0u, base_sync_platform_main, task, 0u, LIB_NULL);
    if (task->thread == LIB_NULL) { lib_release(task); return LIB_STATUS_IO_ERROR; }
    *out_task = &task->task;
    return LIB_STATUS_OK;
}

void base_sync_platform_task_join(base_sync_task *task)
{
    struct base_sync_win32_task *state = (struct base_sync_win32_task *)task;
    if (state != LIB_NULL) (void)lib_win32_wait_for_single_object(state->thread, LIB_WIN32_INFINITE);
}
void base_sync_platform_task_destroy(base_sync_task *task)
{
    struct base_sync_win32_task *state = (struct base_sync_win32_task *)task;
    if (state != LIB_NULL) { (void)lib_win32_close_handle(state->thread); lib_release(state); }
}
