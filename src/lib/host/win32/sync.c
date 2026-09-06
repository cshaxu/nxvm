#include "lib/base/base.h"
#include "lib/host/sync.h"

#include <windows.h>

struct host_sync_event {
    HANDLE handle;
};

struct host_sync_task {
    HANDLE thread;
    HANDLE cancellation;
    host_sync_task_entry entry;
    void *context;
};

static DWORD WINAPI host_sync_task_main(LPVOID opaque)
{
    host_sync_task *task = opaque;

    if (task != LIB_NULL && task->entry != LIB_NULL) {
        task->entry(task->context, task);
    }
    return 0u;
}

static host_sync_wait_result host_sync_wait_handle(HANDLE handle,
    lib_u32 timeout_milliseconds)
{
    DWORD result;

    if (handle == NULL) return HOST_SYNC_WAIT_INVALID_ARGUMENT;
    result = WaitForSingleObject(handle, timeout_milliseconds);
    return result == WAIT_OBJECT_0 ? HOST_SYNC_WAIT_SIGNALED :
        result == WAIT_TIMEOUT ? HOST_SYNC_WAIT_TIMED_OUT : HOST_SYNC_WAIT_FAULT;
}

void host_sync_sleep_milliseconds(lib_u32 milliseconds)
{
    Sleep(milliseconds);
}

void host_sync_yield(void)
{
    Sleep(0u);
}

lib_status host_sync_event_create(host_sync_event **out_event)
{
    host_sync_event *event;

    if (out_event == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_event = LIB_NULL;
    event = calloc(1u, sizeof(*event));
    if (event == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    event->handle = CreateEventA(NULL, TRUE, FALSE, NULL);
    if (event->handle == NULL) {
        free(event);
        return LIB_STATUS_IO_ERROR;
    }
    *out_event = event;
    return LIB_STATUS_OK;
}

void host_sync_event_destroy(host_sync_event *event)
{
    if (event == LIB_NULL) return;
    if (event->handle != NULL) (void)CloseHandle(event->handle);
    free(event);
}

void host_sync_event_signal(host_sync_event *event)
{
    if (event != LIB_NULL && event->handle != NULL) (void)SetEvent(event->handle);
}

void host_sync_event_reset(host_sync_event *event)
{
    if (event != LIB_NULL && event->handle != NULL) (void)ResetEvent(event->handle);
}

host_sync_wait_result host_sync_event_wait(host_sync_event *event,
    lib_u32 timeout_milliseconds)
{
    return event == LIB_NULL ? HOST_SYNC_WAIT_INVALID_ARGUMENT :
        host_sync_wait_handle(event->handle, timeout_milliseconds);
}

lib_status host_sync_task_create(host_sync_task_entry entry, void *context,
    host_sync_task **out_task)
{
    host_sync_task *task;

    if (entry == LIB_NULL || out_task == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_task = LIB_NULL;
    task = calloc(1u, sizeof(*task));
    if (task == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    task->entry = entry;
    task->context = context;
    task->cancellation = CreateEventA(NULL, TRUE, FALSE, NULL);
    task->thread = task->cancellation == NULL ? NULL : CreateThread(NULL, 0u,
        host_sync_task_main, task, 0u, NULL);
    if (task->thread == NULL) {
        if (task->cancellation != NULL) (void)CloseHandle(task->cancellation);
        free(task);
        return LIB_STATUS_IO_ERROR;
    }
    *out_task = task;
    return LIB_STATUS_OK;
}

void host_sync_task_request_cancel(host_sync_task *task)
{
    if (task != LIB_NULL && task->cancellation != NULL) {
        (void)SetEvent(task->cancellation);
    }
}

int host_sync_task_cancelled(const host_sync_task *task)
{
    return task != LIB_NULL && task->cancellation != NULL &&
        WaitForSingleObject(task->cancellation, 0u) == WAIT_OBJECT_0;
}

host_sync_wait_result host_sync_task_wait_cancel(const host_sync_task *task,
    lib_u32 timeout_milliseconds)
{
    host_sync_wait_result result;

    if (task == LIB_NULL) return HOST_SYNC_WAIT_INVALID_ARGUMENT;
    result = host_sync_wait_handle(task->cancellation, timeout_milliseconds);
    return result == HOST_SYNC_WAIT_SIGNALED ? HOST_SYNC_WAIT_CANCELLED : result;
}

void host_sync_task_join(host_sync_task *task)
{
    if (task != LIB_NULL && task->thread != NULL) {
        (void)WaitForSingleObject(task->thread, INFINITE);
    }
}

void host_sync_task_destroy(host_sync_task *task)
{
    if (task == LIB_NULL) return;
    host_sync_task_join(task);
    if (task->thread != NULL) (void)CloseHandle(task->thread);
    if (task->cancellation != NULL) (void)CloseHandle(task->cancellation);
    free(task);
}
