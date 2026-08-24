#include "type.h"

#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"
#include "vm/platform/platform.h"

#if !defined(VM_PLATFORM_TEST_FAILURE_STAGE)
#error "This smoke requires a dedicated failure-stage build."
#endif

static C_INT stage_uses_window(C_INT stage)
{
    return stage >= 1 && stage <= 5;
}

static C_INT assert_failed_start(vm_session *session)
{
    type_status status = vm_platform_start(session->platform_run_context,
        session->platform_run_handle);

    return status != TYPE_STATUS_OK &&
        !vm_platform_run_handle_is_active(session->platform_run_handle);
}

static C_INT assert_failed_session_start(vm_session *session)
{
    type_status status = vm_session_resume(session);

    return status != TYPE_STATUS_OK && session->start_outcome.valid &&
        session->start_outcome.status == status &&
        !vm_session_control_is_running(&session->control) &&
        !vm_platform_run_handle_is_active(session->platform_run_handle);
}

int main(void)
{
    vm_session *session = ((vm_session *)STD_CALLOC(1u, sizeof(vm_session)));
    C_INT stage = VM_PLATFORM_TEST_FAILURE_STAGE;
    C_INT passed = 0;

    if (session == STD_NULL) return 1;
    vm_session_initialize(session);
    if (!session->active) goto done;
    vm_platform_run_context_set_window_display(session->platform_run_context,
        stage_uses_window(stage));

    /* Repeating the branch proves the first cleanup left its owner reusable. */
    if (!assert_failed_start(session) || !assert_failed_start(session) ||
        !assert_failed_session_start(session)) goto done;
    passed = 1;

done:
    vm_session_stop(session);
    vm_session_finalize(session);
    STD_FREE(session);
    if (!passed) return 1;
    STD_PRINTF("M5:T247:S2:WIN32-STARTUP-FAILURE:%d:OK\n", stage);
    return 0;
}
