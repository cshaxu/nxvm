#ifndef LIB_HOST_SYNC_H
#define LIB_HOST_SYNC_H

#include "lib/host/sync_interface.h"

typedef struct host_sync_platform_task host_sync_platform_task;
typedef void (*host_sync_platform_task_entry)(void *context);

void host_sync_platform_sleep_milliseconds(lib_u32 milliseconds);
void host_sync_platform_yield(void);
lib_status host_sync_platform_event_create(lib_bool manual_reset,
    host_sync_event **out_event);
void host_sync_platform_event_destroy(host_sync_event *event);
void host_sync_platform_event_signal(host_sync_event *event);
void host_sync_platform_event_reset(host_sync_event *event);
lib_status host_sync_platform_event_wait_many(
    const host_sync_event *const *events, lib_u32 event_count,
    lib_u32 timeout_milliseconds, lib_bool *out_signaled,
    lib_u32 *out_event_index);
lib_status host_sync_platform_task_create(host_sync_platform_task_entry entry,
    void *context, host_sync_platform_task **out_task);
void host_sync_platform_task_join(host_sync_platform_task *task);
void host_sync_platform_task_destroy(host_sync_platform_task *task);

#endif
