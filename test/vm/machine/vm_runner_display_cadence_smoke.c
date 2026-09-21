#include "type.h"

#include "lib/base/sync_interface.h"
#include "vm/machine/control.h"
#include "vm/machine/display.h"
#include "vm/machine/lifecycle.h"
#include "vm/machine/machine_private.h"
#include "../support/common_machine_fixture.h"
#include "../support/rom/session_assets.h"

static C_INT vm_runner_display_wait(const vm_machine *session, C_INT paused)
{
    C_UINT waited;

    for (waited = 0u; waited < 2000u; ++waited) {
        if (common_machine_state_get(session->executor) ==
            (paused ? COMMON_MACHINE_PAUSED : COMMON_MACHINE_RUNNING)) return TYPE_TRUE;
        base_sync_sleep_milliseconds(1u);
    }
    return TYPE_FALSE;
}

static C_INT vm_runner_display_wait_stopped(const vm_machine *session)
{
    C_UINT waited;

    for (waited = 0u; waited < 2000u; ++waited) {
        if (common_machine_state_get(session->executor) == COMMON_MACHINE_STOPPED)
            return TYPE_TRUE;
        base_sync_sleep_milliseconds(1u);
    }
    return TYPE_FALSE;
}

C_INT main(C_VOID)
{
    vm_machine *session = STD_NULL;
    common_machine_frame frame;
    C_INT failed = 0;

    if (vm_test_default_pc_at_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        vm_test_common_machine_bind(session) != TYPE_STATUS_OK ||
        vm_machine_set_speed(session, VM_MACHINE_SPEED_TURBO) != TYPE_STATUS_OK) {
        failed = 1;
        goto done;
    }
    (C_VOID)vm_machine_publish_display(session, TYPE_TRUE);
    if (!vm_machine_copy_common_frame(session, &frame) ||
        frame.window.text.base.text_columns != 80u ||
        frame.window.text.base.text_rows != 25u ||
        vm_machine_resume(session) != TYPE_STATUS_OK ||
        !vm_runner_display_wait(session, TYPE_FALSE) ||
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
    failed |= !vm_runner_display_wait_stopped(session);
    vm_test_common_machine_unbind(session);
    vm_machine_destroy(session);
    if (failed) return 1;
    puts("M5:T212:S2:RUNNER-CADENCE:OK");
    puts("M5:T526:S5:COMPOSITION-RUNNER-CADENCE:OK");
    return 0;
}
