#include "lib/host/sync.h"

typedef struct host_sync_smoke_state {
    host_sync_event *started;
    atomic_int cancelled;
} host_sync_smoke_state;

static void host_sync_smoke_wait_for_cancel(void *context,
    const host_sync_task *task)
{
    host_sync_smoke_state *state = context;

    host_sync_event_signal(state->started);
    if (host_sync_task_wait_cancel(task, 1000u) ==
        HOST_SYNC_WAIT_CANCELLED) atomic_store(&state->cancelled, LIB_TRUE);
}

int main(void)
{
    host_sync_event *event = LIB_NULL;
    host_sync_event *second_event = LIB_NULL;
    host_sync_task *task = LIB_NULL;
    host_sync_smoke_state state = {0};
    host_sync_event *events[2];
    lib_u32 event_index = UINT32_MAX;

    if (host_sync_event_create(&event) != LIB_STATUS_OK ||
        host_sync_event_create(&second_event) != LIB_STATUS_OK) goto failed;
    if (host_sync_event_wait(event, 0u) != HOST_SYNC_WAIT_TIMED_OUT) goto failed;
    host_sync_event_signal(event);
    if (host_sync_event_wait(event, 0u) != HOST_SYNC_WAIT_SIGNALED) goto failed;
    host_sync_event_reset(event);
    if (host_sync_event_wait(event, 0u) != HOST_SYNC_WAIT_TIMED_OUT) goto failed;
    events[0] = event;
    events[1] = second_event;
    if (host_sync_wait_any(events, 2u, LIB_NULL, 0u, &event_index) !=
        HOST_SYNC_WAIT_TIMED_OUT || event_index != UINT32_MAX) goto failed;
    host_sync_event_signal(second_event);
    if (host_sync_wait_any(events, 2u, LIB_NULL, 0u, &event_index) !=
        HOST_SYNC_WAIT_SIGNALED || event_index != 1u) goto failed;
    host_sync_event_reset(second_event);
    state.started = event;
    if (host_sync_task_create(host_sync_smoke_wait_for_cancel, &state, &task) !=
        LIB_STATUS_OK || host_sync_event_wait(event, 1000u) != HOST_SYNC_WAIT_SIGNALED)
        goto failed;
    host_sync_event_signal(event);
    host_sync_task_request_cancel(task);
    if (host_sync_wait_any(events, 2u, task, 0u, &event_index) !=
        HOST_SYNC_WAIT_CANCELLED || event_index != UINT32_MAX) goto failed;
    host_sync_task_join(task);
    if (!host_sync_task_cancelled(task) || !atomic_load(&state.cancelled)) goto failed;
    host_sync_task_destroy(task);
    task = LIB_NULL;
    host_sync_event_reset(event);
    atomic_store(&state.cancelled, LIB_FALSE);
    if (host_sync_task_create(host_sync_smoke_wait_for_cancel, &state, &task) !=
        LIB_STATUS_OK || host_sync_event_wait(event, 1000u) != HOST_SYNC_WAIT_SIGNALED)
        goto failed;
    host_sync_task_destroy(task);
    task = LIB_NULL;
    if (!atomic_load(&state.cancelled)) goto failed;
    host_sync_event_destroy(event);
    host_sync_event_destroy(second_event);
    puts("M5:T523:S4:HOST-SYNC:OK");
    return 0;

failed:
    host_sync_task_destroy(task);
    host_sync_event_destroy(event);
    host_sync_event_destroy(second_event);
    return 1;
}
