#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/platform/sleep.h"
#include "core/product/runtime/execution_context.h"
#include "vm/composition/display_bridge.h"
#include "vm/composition/session.h"
#include "vm/composition/session_control.h"
#include "vm/composition/session_runner.h"

C_VOID vm_session_runner_run(vm_session *session)
{
    core_machine_run_budget budget = {1u, 0u};
    core_machine_run_result result;
    vm_session_control_state *control;

    if (session == STD_NULL || session->core_machine == STD_NULL ||
        session->control == STD_NULL) return;
    control = session->control;
    while (STD_ATOMIC_LOAD(&control->flagRun)) {
        if (STD_ATOMIC_EXCHANGE(&control->flagReset, NTVDM64_TYPE_FALSE)) {
            core_product_execution_context_reset(&control->execution_context);
        }
        if (STD_ATOMIC_LOAD(&control->pauseRequested)) {
            STD_ATOMIC_STORE(&control->paused, NTVDM64_TYPE_TRUE);
        }
        while (STD_ATOMIC_LOAD(&control->flagRun) && STD_ATOMIC_LOAD(&control->paused)) {
            core_product_execution_context_run_command_boundary(&control->execution_context);
            core_platform_sleep_milliseconds(1u);
        }
        if (!STD_ATOMIC_LOAD(&control->flagRun)) break;
        core_product_execution_context_run_command_boundary(&control->execution_context);
        core_product_execution_context_debug_refresh(&control->execution_context);
        if (STD_ATOMIC_LOAD(&control->pauseRequested)) continue;
        if (core_machine_run(session->core_machine, budget, &result) != NTVDM64_STATUS_OK) {
            vm_session_control_stop(control);
            continue;
        }
        vm_session_publish_display(session, NTVDM64_TYPE_FALSE);
        if (result.reason == CORE_MACHINE_STOP_RESET_REQUESTED) {
            vm_session_control_reset(control);
        }
        if (result.reason == CORE_MACHINE_STOP_REQUESTED) {
            vm_session_control_stop(control);
        }
        if (result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
            core_platform_sleep_milliseconds(1u);
        }
        if (STD_ATOMIC_EXCHANGE(&control->stepRequested, NTVDM64_TYPE_FALSE)) {
            vm_session_control_request_pause(control, VM_SESSION_PAUSE_STEP);
        }
    }
}
