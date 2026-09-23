#include "lib/types/types_interface.h"
#include "type.h"

#include "app-nxvm/machine/control.h"
#include "app-nxvm/machine/display.h"
#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/machine/machine_private.h"
#include "support/common_machine_fixture.h"
#include "support/rom/session_assets.h"

C_INT main(C_VOID)
{
    vm_machine *session = LIB_NULL;
    vm_test_common_machine_state_waiter waiter = {0};
    static common_machine_frame frame;
    C_INT failed = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != TYPE_STATUS_OK ||
        vm_test_common_machine_bind(session) != TYPE_STATUS_OK ||
        vm_test_common_machine_state_waiter_initialize(session, &waiter) !=
            TYPE_STATUS_OK ||
        vm_machine_set_speed(session, VM_MACHINE_SPEED_TURBO) != TYPE_STATUS_OK) {
        failed = 1;
        goto done;
    }
    (C_VOID)vm_machine_publish_display(session, LIB_TRUE);
    if (!vm_machine_copy_common_frame(session, &frame) ||
        frame.window.text.base.text_columns != 80u ||
        frame.window.text.base.text_rows != 25u ||
        vm_machine_resume(session) != TYPE_STATUS_OK ||
        !vm_test_common_machine_wait_state(session, &waiter,
            COMMON_MACHINE_RUNNING, 2000u) ||
        vm_machine_request_pause(session) != TYPE_STATUS_OK ||
        !vm_test_common_machine_wait_state(session, &waiter,
            COMMON_MACHINE_PAUSED, 2000u) ||
        vm_machine_resume(session) != TYPE_STATUS_OK ||
        !vm_test_common_machine_wait_state(session, &waiter,
            COMMON_MACHINE_RUNNING, 2000u) ||
        vm_machine_request_pause(session) != TYPE_STATUS_OK ||
        !vm_test_common_machine_wait_state(session, &waiter,
            COMMON_MACHINE_PAUSED, 2000u)) {
        failed = 1;
        goto done;
    }
done:
    vm_machine_stop(session);
    failed |= !vm_test_common_machine_wait_state(session, &waiter,
        COMMON_MACHINE_STOPPED, 2000u);
    vm_test_common_machine_unbind(session);
    vm_test_common_machine_state_waiter_finalize(&waiter);
    vm_machine_destroy(session);
    if (failed) return 1;
    puts("M5:T212:S2:RUNNER-CADENCE:OK");
    puts("M5:T526:S5:COMPOSITION-RUNNER-CADENCE:OK");
    return 0;
}
