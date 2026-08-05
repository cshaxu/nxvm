#include "type.h"

#include "core/platform/sleep.h"
#include "vm/composition/session/session.h"
#include "tests/support/vm_session_fixture.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/platform/platform.h"

int main(void)
{
    vm_session *session;
    vm_platform_run_handle event_handle;

    vm_platform_run_handle_initialize(&event_handle);
    vm_platform_run_handle_report(&event_handle,
        VM_PLATFORM_RUN_EVENT_KERNEL_COMPLETED);
    if (vm_platform_run_handle_get_last_event(&event_handle) !=
            VM_PLATFORM_RUN_EVENT_KERNEL_COMPLETED ||
        vm_platform_run_handle_take_stop_report(&event_handle)) return 1;
    vm_platform_run_handle_report(&event_handle,
        VM_PLATFORM_RUN_EVENT_STOP_REQUESTED);
    if (!vm_platform_run_handle_take_stop_report(&event_handle) ||
        vm_platform_run_handle_take_stop_report(&event_handle)) return 1;

    session = vm_session_fixture_allocate();
    if (session == STD_NULL) return 1;
    vm_session_initialize(session);
    if (!vm_session_fixture_is_active(session)) goto fail;
    vm_platform_run_context_set_window_display(vm_session_fixture_platform_run_context(session), 1);
    vm_session_start(session);
    if (!vm_platform_run_handle_is_active(vm_session_fixture_platform_run_handle(session))) goto fail;
    core_platform_sleep_milliseconds(50u);
    vm_platform_run_handle_report(vm_session_fixture_platform_run_handle(session),
        VM_PLATFORM_RUN_EVENT_STOP_REQUESTED);
    core_platform_sleep_milliseconds(50u);
    if (vm_session_control_is_running(vm_session_fixture_control(session))) goto fail;
    vm_session_reset(session);
    if (vm_platform_run_handle_is_active(vm_session_fixture_platform_run_handle(session)) ||
        vm_session_fixture_platform_run_handle(session)->backend != STD_NULL) goto fail;
    vm_session_start(session);
    if (!vm_platform_run_handle_is_active(vm_session_fixture_platform_run_handle(session))) goto fail;
    vm_session_stop(session);
    if (vm_platform_run_handle_is_active(vm_session_fixture_platform_run_handle(session)) ||
        vm_session_fixture_platform_run_handle(session)->backend != STD_NULL) goto fail;
    vm_session_finalize(session);
    vm_session_fixture_free(session);
    puts("M5:T194:S3:RUN-EVENT:OK");
    return 0;

fail:
    vm_session_stop(session);
    vm_session_finalize(session);
    vm_session_fixture_free(session);
    return 1;
}
