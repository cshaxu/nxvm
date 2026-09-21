#include "type.h"

#include "lib/base/sync_interface.h"
#include "core/machine/control.h"
#include "core/machine/lifecycle.h"
#include "core/machine/machine_private.h"
#include "support/common_machine_fixture.h"
#include "support/rom/session_assets.h"

static C_INT vm_console_pause_resume_wait(const vm_machine *session, C_INT paused)
{
    C_UINT waited;

    for (waited = 0u; waited < 2000u; ++waited) {
        if (common_machine_state_get(session->executor) ==
            (paused ? COMMON_MACHINE_PAUSED : COMMON_MACHINE_RUNNING)) return TYPE_TRUE;
        base_sync_sleep_milliseconds(1u);
    }
    return TYPE_FALSE;
}

C_INT main(C_VOID)
{
    vm_machine *session = STD_NULL;
    C_INT failed = 0;
    C_UINT waited;

    if (vm_test_default_pc_at_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        vm_test_common_machine_bind(session) != TYPE_STATUS_OK ||
        vm_machine_resume(session) != TYPE_STATUS_OK ||
        !vm_console_pause_resume_wait(session, TYPE_FALSE) ||
        vm_machine_request_pause(session) != TYPE_STATUS_OK ||
        !vm_console_pause_resume_wait(session, TYPE_TRUE) ||
        common_machine_state_get(session->executor) != COMMON_MACHINE_PAUSED ||
        vm_machine_resume(session) != TYPE_STATUS_OK ||
        !vm_console_pause_resume_wait(session, TYPE_FALSE)) failed = 1;
    vm_machine_stop(session);
    for (waited = 0u; waited < 2000u &&
        common_machine_state_get(session->executor) != COMMON_MACHINE_STOPPED; ++waited)
        base_sync_sleep_milliseconds(1u);
    failed |= common_machine_state_get(session->executor) != COMMON_MACHINE_STOPPED;
    vm_test_common_machine_unbind(session);
    vm_machine_destroy(session);
    if (failed) return 1;
    puts("M5:T526:S5:COMPOSITION-PAUSE-RESUME:OK");
    return 0;
}
