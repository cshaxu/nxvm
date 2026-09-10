#include "type.h"

#include "lib/host/sync_interface.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_private.h"
#include "../support/rom/session_assets.h"

static C_INT vm_console_pause_resume_wait(const vm_session *session, C_INT paused)
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
    C_INT failed = 0;

    if (vm_test_default_pc_at_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        vm_session_resume(session) != TYPE_STATUS_OK ||
        !vm_console_pause_resume_wait(session, TYPE_FALSE) ||
        vm_session_request_pause(session) != TYPE_STATUS_OK ||
        !vm_console_pause_resume_wait(session, TYPE_TRUE) ||
        !vm_session_state_is_active(session->control.state) ||
        vm_session_resume(session) != TYPE_STATUS_OK ||
        !vm_console_pause_resume_wait(session, TYPE_FALSE)) failed = 1;
    vm_session_stop(session);
    failed |= vm_session_control_is_running(&session->control) ||
        vm_session_control_is_paused(&session->control);
    vm_session_destroy(session);
    if (failed) return 1;
    puts("M5:T526:S5:COMPOSITION-PAUSE-RESUME:OK");
    return 0;
}
