#ifndef LIB_BASE_SYNC_H
#define LIB_BASE_SYNC_H

#include "lib/base/sync_interface.h"

/* First member of the selected platform task; one allocation owns both. */
struct base_sync_task {
    base_sync_event *cancellation;
    base_sync_task_entry entry;
    void *context;
};

void base_sync_platform_sleep_milliseconds(lib_u32 milliseconds);
void base_sync_platform_yield(void);
lib_status base_sync_platform_event_create(lib_bool manual_reset,
    base_sync_event **out_event);
void base_sync_platform_event_destroy(base_sync_event *event);
lib_status base_sync_platform_event_signal(base_sync_event *event);
lib_status base_sync_platform_event_reset(base_sync_event *event);
lib_status base_sync_platform_event_wait_many(
    const base_sync_event *const *events, lib_u32 event_count,
    lib_u32 timeout_milliseconds, lib_bool *out_signaled,
    lib_u32 *out_event_index);
lib_status base_sync_platform_task_create(base_sync_task_entry entry,
    void *context, base_sync_event *cancellation, base_sync_task **out_task);
lib_status base_sync_platform_task_join(base_sync_task *task);
void base_sync_platform_task_destroy(base_sync_task *task);

#endif
