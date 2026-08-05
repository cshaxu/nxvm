#include "type.h"

#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session.h"
#include "tests/support/vm_session_fixture.h"
#include "vm/platform/platform.h"

#if !defined(NTVDM64_VM_PLATFORM_TEST_FAILURE_STAGE)
#error "This smoke requires a dedicated failure-stage build."
#endif

static C_INT stage_uses_window(C_INT stage)
{
    return stage >= 1 && stage <= 5;
}

static C_INT assert_failed_start(vm_session *session)
{
    type_status status = vm_platform_start(vm_session_fixture_platform_run_context(session),
        vm_session_fixture_platform_run_handle(session));

    return status != TYPE_STATUS_OK &&
        !vm_platform_run_handle_is_active(vm_session_fixture_platform_run_handle(session)) &&
        vm_session_fixture_platform_run_handle(session)->backend == STD_NULL;
}

int main(void)
{
    vm_session *session = vm_session_fixture_allocate();
    C_INT stage = NTVDM64_VM_PLATFORM_TEST_FAILURE_STAGE;
    C_INT passed = 0;

    if (session == STD_NULL) return 1;
    vm_session_initialize(session);
    if (!vm_session_fixture_is_active(session)) goto done;
    vm_platform_run_context_set_window_display(vm_session_fixture_platform_run_context(session),
        stage_uses_window(stage));

    /* Repeating the branch proves the first cleanup left its owner reusable. */
    if (!assert_failed_start(session) || !assert_failed_start(session)) goto done;
    passed = 1;

done:
    vm_session_stop(session);
    vm_session_finalize(session);
    vm_session_fixture_free(session);
    if (!passed) return 1;
    STD_PRINTF("M5:T195:S3:WIN32-STARTUP-FAILURE:%d:OK\n", stage);
    return 0;
}
