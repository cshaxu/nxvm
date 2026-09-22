#include "type.h"

#include "core/machine/control.h"
#include "core/machine/lifecycle.h"
#include "core/machine/machine_private.h"
#include "support/common_machine_fixture.h"
#include "support/rom/session_assets.h"

C_INT main(C_VOID)
{
    vm_machine *session = STD_NULL;
    vm_test_common_machine_state_waiter waiter = {0};
    C_INT failed = 0;

    if (vm_test_default_pc_at_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        vm_test_common_machine_bind(session) != TYPE_STATUS_OK ||
        vm_test_common_machine_state_waiter_initialize(session, &waiter) !=
            TYPE_STATUS_OK ||
        vm_machine_resume(session) != TYPE_STATUS_OK ||
        !vm_test_common_machine_wait_state(session, &waiter,
            COMMON_MACHINE_RUNNING, 2000u) ||
        vm_machine_request_pause(session) != TYPE_STATUS_OK ||
        !vm_test_common_machine_wait_state(session, &waiter,
            COMMON_MACHINE_PAUSED, 2000u) ||
        common_machine_state_get(session->executor) != COMMON_MACHINE_PAUSED ||
        vm_machine_resume(session) != TYPE_STATUS_OK ||
        !vm_test_common_machine_wait_state(session, &waiter,
            COMMON_MACHINE_RUNNING, 2000u)) failed = 1;
    vm_machine_stop(session);
    failed |= !vm_test_common_machine_wait_state(session, &waiter,
        COMMON_MACHINE_STOPPED, 2000u);
    vm_test_common_machine_unbind(session);
    vm_test_common_machine_state_waiter_finalize(&waiter);
    vm_machine_destroy(session);
    if (failed) return 1;
    puts("M5:T526:S5:COMPOSITION-PAUSE-RESUME:OK");
    return 0;
}
