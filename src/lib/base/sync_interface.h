#ifndef LIB_BASE_SYNC_INTERFACE_H
#define LIB_BASE_SYNC_INTERFACE_H

#include "lib/types/types_interface.h"

typedef enum base_sync_wait_result {
    BASE_SYNC_WAIT_SIGNALED,
    BASE_SYNC_WAIT_CANCELLED,
    BASE_SYNC_WAIT_TIMED_OUT,
    BASE_SYNC_WAIT_INVALID_ARGUMENT,
    BASE_SYNC_WAIT_FAULT
} base_sync_wait_result;

typedef struct base_sync_event base_sync_event;
typedef enum base_sync_event_mode {
    BASE_SYNC_EVENT_MANUAL_RESET,
    BASE_SYNC_EVENT_AUTO_RESET
} base_sync_event_mode;
typedef struct base_sync_mutex base_sync_mutex;
typedef struct base_sync_task base_sync_task;

/* Blocking mutual exclusion. A live mutex is required for lock/unlock.
 * Recursive locking is forbidden; only the owning thread may unlock.
 * Destroy requires no owner or waiters; destroying NULL is harmless. */
lib_status base_sync_mutex_create(base_sync_mutex **out_mutex);
void base_sync_mutex_destroy(base_sync_mutex *mutex);
void base_sync_mutex_lock(base_sync_mutex *mutex);
void base_sync_mutex_unlock(base_sync_mutex *mutex);

typedef void (*base_sync_task_entry)(void *context,
    const base_sync_task *task);

void base_sync_sleep_milliseconds(lib_u32 milliseconds);
void base_sync_yield(void);
/* Initially nonsignaled. Manual reset stays signaled until reset; auto reset
 * releases one waiter per observed signal. Repeated signals may coalesce.
 * Signal/reset return IO_ERROR on native failure; NULL is INVALID_ARGUMENT.
 * Destroy requires all users/waiters quiescent. */
lib_status base_sync_event_create(base_sync_event_mode mode,
    base_sync_event **out_event);
void base_sync_event_destroy(base_sync_event *event);
lib_status base_sync_event_signal(base_sync_event *event);
lib_status base_sync_event_reset(base_sync_event *event);
base_sync_wait_result base_sync_event_wait(base_sync_event *event,
    lib_u32 timeout_milliseconds);
/* Cancellation wins when it is observable together with an event; otherwise
 * the lowest event index wins. */
base_sync_wait_result base_sync_wait_any(base_sync_event *const *events,
    lib_u32 event_count, const base_sync_task *cancel_task,
    lib_u32 timeout_milliseconds, lib_u32 *out_event_index);
lib_status base_sync_task_create(base_sync_task_entry entry, void *context,
    base_sync_task **out_task);
lib_status base_sync_task_request_cancel(base_sync_task *task);
int base_sync_task_cancelled(const base_sync_task *task);
base_sync_wait_result base_sync_task_wait_cancel(const base_sync_task *task,
    lib_u32 timeout_milliseconds);
/* The owner serializes join/destroy; neither may run on this task itself.
 * Join waits indefinitely, returning IO_ERROR if native exit cannot be proven.
 * Destroy cancels and joins; failure retains the task and all caller-owned
 * context must remain alive. Success consumes it. NULL is a successful no-op. */
lib_status base_sync_task_join(base_sync_task *task);
lib_status base_sync_task_destroy(base_sync_task *task);

#endif
