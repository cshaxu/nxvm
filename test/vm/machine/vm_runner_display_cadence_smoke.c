#include "type.h"

#include "lib/host/sync_interface.h"
#include "vm/machine/runtime/control.h"
#include "vm/machine/runtime/display.h"
#include "vm/machine/runtime/lifecycle.h"
#include "vm/machine/runtime/machine_private.h"
#include "../support/rom/session_assets.h"

typedef struct vm_runner_display_lifecycle_log {
    vm_machine_lifecycle events[5u];
    STD_ATOMIC_INT count;
    STD_ATOMIC_INT display_count;
    type_unsigned_16 display_columns;
    type_unsigned_16 display_rows;
} vm_runner_display_lifecycle_log;

static C_VOID vm_runner_display_lifecycle_report(C_VOID *opaque,
    const vm_machine_result *result)
{
    vm_runner_display_lifecycle_log *log = opaque;
    C_INT index;
    vm_machine_lifecycle lifecycle;

    if (log == STD_NULL || result == STD_NULL) return;
    switch (result->kind) {
    case VM_MACHINE_RESULT_RUNNING: lifecycle = VM_MACHINE_RUNNING; break;
    case VM_MACHINE_RESULT_PAUSED: lifecycle = VM_MACHINE_PAUSED; break;
    case VM_MACHINE_RESULT_STOPPED: lifecycle = VM_MACHINE_STOPPED; break;
    case VM_MACHINE_RESULT_DISPLAY:
        ++log->display_count;
        log->display_columns = result->value.display.columns;
        log->display_rows = result->value.display.rows;
        return;
    default: return;
    }
    index = STD_ATOMIC_LOAD(&log->count);
    (C_VOID)STD_ATOMIC_EXCHANGE(&log->count, index + 1);
    if (index >= 0 && index < 5) log->events[index] = lifecycle;
}

static C_INT vm_runner_display_wait(const vm_machine *session, C_INT paused)
{
    C_UINT waited;

    for (waited = 0u; waited < 2000u; ++waited) {
        if (vm_machine_control_is_paused(&session->control) == paused) return TYPE_TRUE;
        host_sync_sleep_milliseconds(1u);
    }
    return TYPE_FALSE;
}

C_INT main(C_VOID)
{
    vm_machine *session = STD_NULL;
    vm_runner_display_lifecycle_log log = {0};
    C_INT failed = 0;

    STD_ATOMIC_INIT(&log.count, 0);
    STD_ATOMIC_INIT(&log.display_count, 0);
    if (vm_test_default_pc_at_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        vm_machine_set_speed(session, VM_MACHINE_SPEED_TURBO) != TYPE_STATUS_OK) {
        failed = 1;
        goto done;
    }
    vm_machine_set_result_sink(session, vm_runner_display_lifecycle_report,
        &log);
    (C_VOID)vm_machine_publish_display(session, TYPE_TRUE);
    if (STD_ATOMIC_LOAD(&log.display_count) != 1 ||
        log.display_columns != 80u || log.display_rows != 25u ||
        vm_machine_resume(session) != TYPE_STATUS_OK ||
        vm_machine_request_pause(session) != TYPE_STATUS_OK ||
        !vm_runner_display_wait(session, TYPE_TRUE) ||
        vm_machine_resume(session) != TYPE_STATUS_OK ||
        !vm_runner_display_wait(session, TYPE_FALSE) ||
        vm_machine_request_pause(session) != TYPE_STATUS_OK ||
        !vm_runner_display_wait(session, TYPE_TRUE)) {
        failed = 1;
        goto done;
    }
done:
    vm_machine_stop(session);
    failed |= STD_ATOMIC_LOAD(&log.count) != 5 ||
        log.events[0u] != VM_MACHINE_RUNNING || log.events[1u] != VM_MACHINE_PAUSED ||
        log.events[2u] != VM_MACHINE_RUNNING || log.events[3u] != VM_MACHINE_PAUSED ||
        log.events[4u] != VM_MACHINE_STOPPED;
    vm_machine_destroy(session);
    if (failed) return 1;
    puts("M5:T212:S2:RUNNER-CADENCE:OK");
    puts("M5:T526:S5:COMPOSITION-RUNNER-CADENCE:OK");
    return 0;
}
