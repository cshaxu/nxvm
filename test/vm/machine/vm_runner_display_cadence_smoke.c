#include "type.h"

#include "lib/host/sync_interface.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_private.h"
#include "../support/rom/session_assets.h"

typedef struct vm_runner_display_lifecycle_log {
    vm_session_lifecycle events[5u];
    STD_ATOMIC_INT count;
} vm_runner_display_lifecycle_log;

static C_VOID vm_runner_display_lifecycle_report(C_VOID *opaque,
    vm_session_lifecycle lifecycle)
{
    vm_runner_display_lifecycle_log *log = opaque;
    C_INT index;

    if (log == STD_NULL) return;
    index = STD_ATOMIC_LOAD(&log->count);
    (C_VOID)STD_ATOMIC_EXCHANGE(&log->count, index + 1);
    if (index >= 0 && index < 5) log->events[index] = lifecycle;
}

static C_INT vm_runner_display_wait(const vm_session *session, C_INT paused)
{
    C_UINT waited;

    for (waited = 0u; waited < 2000u; ++waited) {
        if (vm_session_control_is_paused(&session->control) == paused) return TYPE_TRUE;
        host_sync_sleep_milliseconds(1u);
    }
    return TYPE_FALSE;
}

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    vm_runner_display_lifecycle_log log = {0};
    C_INT failed = 0;

    STD_ATOMIC_INIT(&log.count, 0);
    if (vm_test_default_pc_at_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        vm_session_set_speed(session, VM_SESSION_SPEED_TURBO) != TYPE_STATUS_OK) {
        failed = 1;
        goto done;
    }
    vm_session_set_lifecycle_reporter(session, vm_runner_display_lifecycle_report, &log);
    if (vm_session_resume(session) != TYPE_STATUS_OK ||
        vm_session_request_pause(session) != TYPE_STATUS_OK ||
        !vm_runner_display_wait(session, TYPE_TRUE) ||
        vm_session_resume(session) != TYPE_STATUS_OK ||
        !vm_runner_display_wait(session, TYPE_FALSE) ||
        vm_session_request_pause(session) != TYPE_STATUS_OK ||
        !vm_runner_display_wait(session, TYPE_TRUE)) {
        failed = 1;
        goto done;
    }
done:
    vm_session_stop(session);
    failed |= STD_ATOMIC_LOAD(&log.count) != 5 ||
        log.events[0u] != VM_SESSION_RUNNING || log.events[1u] != VM_SESSION_PAUSED ||
        log.events[2u] != VM_SESSION_RUNNING || log.events[3u] != VM_SESSION_PAUSED ||
        log.events[4u] != VM_SESSION_STOPPED;
    vm_session_destroy(session);
    if (failed) return 1;
    puts("M5:T212:S2:RUNNER-CADENCE:OK");
    puts("M5:T526:S5:COMPOSITION-RUNNER-CADENCE:OK");
    return 0;
}
