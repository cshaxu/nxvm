#include "type.h"

#include "core/core/machine_interface.h"
#include "lib/base/sync_interface.h"
#include "core/machine/display.h"
#include "core/machine/fault.h"
#include "core/machine/lifecycle.h"
#include "core/machine/machine_private.h"
#include "core/machine/control.h"
#include "core/machine/runner.h"
#include "core/machine/waiting.h"
#include "core/machine/debug.h"

/* A normal quantum bounds Core work between host control turns without copying the text frame
 * after every guest instruction.  Its tick ceiling is a host-control budget,
 * not a second guest clock or a timing conversion.  Single-step remains
 * exactly one instruction and is therefore not cut short by that ceiling. */
#define VM_MACHINE_RUNNER_QUANTUM_INSTRUCTIONS 256u
#define VM_MACHINE_RUNNER_TURBO_QUANTUM_INSTRUCTIONS 4096u

C_VOID vm_machine_runner_run(vm_machine *session)
{
    core_machine_run_budget budget;
    core_machine_run_result result;
    vm_machine_control_state *control;

    if (session == STD_NULL || session->core_machine == STD_NULL) return;
    control = &session->control;
    while (vm_machine_executor_state_is_active(control->state)) {
        if (session->executor_callback != STD_NULL)
            session->executor_callback(session->executor_callback_context);
        if (!vm_machine_executor_state_is_active(control->state)) break;
        if (vm_machine_executor_state_take_reset(control->state)) {
            type_status reset_status = vm_machine_control_reset_at_boundary(control);

            (C_VOID)vm_machine_finish_reset(session, reset_status);
            if (reset_status != TYPE_STATUS_OK) continue;
        }
        vm_machine_control_refresh_debug(control);
        if (vm_machine_debug_breakpoint_due(&session->debug)) {
            vm_machine_debug_complete_breakpoint(&session->debug);
            continue;
        }
        if (vm_machine_executor_state_pause_requested(control->state)) {
            vm_machine_executor_state_acknowledge_pause(control->state);
            (C_VOID)vm_machine_publish_display(session, TYPE_TRUE);
            vm_machine_control_signal_completion(control);
            (C_VOID)base_sync_event_wait(control->control_ready, UINT32_MAX);
            base_sync_event_reset(control->control_ready);
            continue;
        }
        budget.instructions = vm_machine_control_step_requested(control) ? 1u :
            session->speed == VM_MACHINE_SPEED_TURBO ?
            VM_MACHINE_RUNNER_TURBO_QUANTUM_INSTRUCTIONS :
            VM_MACHINE_RUNNER_QUANTUM_INSTRUCTIONS;
        budget.instructions = vm_machine_debug_limit_instruction_budget(
            &session->debug, budget.instructions);
        /* Turbo remains bounded by instructions so control and presentation
         * stay responsive, but it must not impose a second tick throttle.
         * Core still advances every retired instruction and every device
         * deadline on its one guest-time axis. */
        budget.ticks = vm_machine_control_step_requested(control) ||
            session->speed == VM_MACHINE_SPEED_TURBO ? 0u :
            VM_MACHINE_RUNNER_QUANTUM_INSTRUCTIONS;
        {
            type_status run_status = core_machine_run(session->core_machine,
                budget, &result);

            if (run_status == TYPE_STATUS_FAULT ||
                result.reason == CORE_MACHINE_STOP_FAULT) {
                vm_machine_fault_capture(session, &result);
                vm_machine_control_fault(control);
                continue;
            }
            if (run_status != TYPE_STATUS_OK) {
                vm_machine_control_stop(control);
                continue;
            }
        }
        vm_machine_debug_complete_run(&session->debug, result.executed);
        if (vm_machine_control_take_step(control)) {
            vm_machine_control_request_pause(control, VM_MACHINE_PAUSE_STEP);
            continue;
        }
        {
            if (vm_machine_pacing_wait(session) != TYPE_STATUS_OK) {
                vm_machine_control_stop(control);
                continue;
            }
        }
        (C_VOID)vm_machine_publish_display(session, TYPE_FALSE);
        if (result.reason == CORE_MACHINE_STOP_RESET_REQUESTED) {
            /* Core reset the requested processor state before returning. */
            vm_machine_debug_reset(&session->debug);
        }
        if (result.reason == CORE_MACHINE_STOP_REQUESTED) {
            vm_machine_control_stop(control);
        }
        if (result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT &&
            !vm_machine_control_step_requested(control)) {
            C_INT advanced = 0;
            type_status time_status = vm_machine_waiting_advance(
                session, &result, &advanced);

            if (time_status != TYPE_STATUS_OK) vm_machine_control_stop(control);
            else if (!advanced) {
                /* Core has no source-qualified deadline to advance.  Yielding
                 * gives host input/control a turn without manufacturing guest
                 * time or restoring the old fixed-delay polling loop. */
                base_sync_yield();
            }
        }
    }
    vm_machine_control_signal_completion(control);
}
