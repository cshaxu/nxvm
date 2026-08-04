#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/platform/sleep.h"
#include "vm/composition/session/execution.h"
#include "vm/composition/session/display.h"
#include "vm/composition/session/session.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/runner.h"
#include "vm/machine/debug.h"
#include "vm/profile/default_profile/firmware/bios.h"

static C_INT vm_session_runner_consume_boot_failure_report(vm_session *session)
{
    if (session == STD_NULL ||
        !vm_profile_default_bios_take_boot_failure_report(
            vm_profile_default_context_memory(&session->default_profile_context))) {
        return 0;
    }
    vm_session_control_stop(&session->control);
    return 1;
}

C_VOID vm_session_runner_run(vm_session *session)
{
    core_machine_run_budget budget = {1u, 0u};
    core_machine_run_result result;
    vm_session_control_state *control;

    if (session == STD_NULL || session->core_machine == STD_NULL) return;
    control = &session->control;
    while (STD_ATOMIC_LOAD(&control->flagRun)) {
        if (vm_session_runner_consume_boot_failure_report(session)) {
            continue;
        }
        if (vm_platform_run_handle_take_stop_report(&session->platform_run_handle)) {
            vm_session_control_stop(control);
            continue;
        }
        if (STD_ATOMIC_EXCHANGE(&control->flagReset, TYPE_FALSE)) {
            vm_session_execution_context_reset(&control->execution_context);
        }
        if (STD_ATOMIC_LOAD(&control->pauseRequested)) {
            STD_ATOMIC_STORE(&control->paused, TYPE_TRUE);
        }
        while (STD_ATOMIC_LOAD(&control->flagRun) && STD_ATOMIC_LOAD(&control->paused)) {
            vm_session_execution_context_run_command_boundary(&control->execution_context);
            core_platform_sleep_milliseconds(1u);
        }
        if (!STD_ATOMIC_LOAD(&control->flagRun)) break;
        vm_session_execution_context_run_command_boundary(&control->execution_context);
        vm_session_execution_context_debug_refresh(&control->execution_context);
        if (STD_ATOMIC_LOAD(&control->pauseRequested)) continue;
        if (core_machine_run(session->core_machine, budget, &result) != TYPE_STATUS_OK) {
            vm_session_control_stop(control);
            continue;
        }
        vm_session_publish_display(session, TYPE_FALSE);
        if (result.reason == CORE_MACHINE_STOP_RESET_REQUESTED) {
            /* core_machine_run completed the one cold reset before returning. */
            vm_machine_debug_reset(&session->debug);
        }
        if (result.reason == CORE_MACHINE_STOP_REQUESTED) {
            vm_session_control_stop(control);
        }
        if (result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
            core_platform_sleep_milliseconds(1u);
        }
        if (STD_ATOMIC_EXCHANGE(&control->stepRequested, TYPE_FALSE)) {
            vm_session_control_request_pause(control, VM_SESSION_PAUSE_STEP);
        }
    }
}
