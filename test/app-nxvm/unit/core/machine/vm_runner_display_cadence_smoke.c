#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/machine/control.h"
#include "app-nxvm/machine/display.h"
#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/machine/machine_private.h"
#include "support/common_machine_fixture.h"
#include "support/rom/session_assets.h"

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    vm_test_common_machine_state_waiter waiter = {0};
    static common_machine_frame frame;
    lib_i32 failed = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK ||
        vm_test_common_machine_bind(session) != LIB_STATUS_OK ||
        vm_test_common_machine_state_waiter_initialize(session, &waiter) !=
            LIB_STATUS_OK ||
        vm_machine_set_speed(session, VM_MACHINE_SPEED_TURBO) != LIB_STATUS_OK) {
        failed = 1;
        goto done;
    }
    (void)vm_machine_publish_display(session, LIB_TRUE);
    if (!vm_machine_copy_common_frame(session, &frame) ||
        frame.window.text.base.text_columns != 80u ||
        frame.window.text.base.text_rows != 25u ||
        vm_machine_resume(session) != LIB_STATUS_OK ||
        !vm_test_common_machine_wait_state(session, &waiter,
            COMMON_MACHINE_RUNNING, 2000u) ||
        vm_machine_request_pause(session) != LIB_STATUS_OK ||
        !vm_test_common_machine_wait_state(session, &waiter,
            COMMON_MACHINE_PAUSED, 2000u) ||
        vm_machine_resume(session) != LIB_STATUS_OK ||
        !vm_test_common_machine_wait_state(session, &waiter,
            COMMON_MACHINE_RUNNING, 2000u) ||
        vm_machine_request_pause(session) != LIB_STATUS_OK ||
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
