#ifndef LIB_HOST_SYNC_H
#define LIB_HOST_SYNC_H

#include "lib/base/base.h"

typedef enum host_sync_wait_result {
    HOST_SYNC_WAIT_SIGNALED,
    HOST_SYNC_WAIT_CANCELLED,
    HOST_SYNC_WAIT_TIMED_OUT,
    HOST_SYNC_WAIT_INVALID_ARGUMENT,
    HOST_SYNC_WAIT_FAULT
} host_sync_wait_result;

typedef struct host_sync_event host_sync_event;
typedef struct host_sync_task host_sync_task;

/* Runs on a host_sync_task-owned native thread. */
typedef void (*host_sync_task_entry)(void *context,
    const host_sync_task *task);

void host_sync_sleep_milliseconds(lib_u32 milliseconds);
void host_sync_yield(void);
lib_status host_sync_event_create(host_sync_event **out_event);
void host_sync_event_destroy(host_sync_event *event);
void host_sync_event_signal(host_sync_event *event);
void host_sync_event_reset(host_sync_event *event);
host_sync_wait_result host_sync_event_wait(host_sync_event *event,
    lib_u32 timeout_milliseconds);
lib_status host_sync_task_create(host_sync_task_entry entry, void *context,
    host_sync_task **out_task);
void host_sync_task_request_cancel(host_sync_task *task);
int host_sync_task_cancelled(const host_sync_task *task);
host_sync_wait_result host_sync_task_wait_cancel(const host_sync_task *task,
    lib_u32 timeout_milliseconds);
void host_sync_task_join(host_sync_task *task);
void host_sync_task_destroy(host_sync_task *task);

#endif
