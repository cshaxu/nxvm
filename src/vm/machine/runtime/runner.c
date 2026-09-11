#include "type.h"

#include "core/machine/machine_interface.h"
#include "lib/host/sync_interface.h"
#include "vm/machine/runtime/execution.h"
#include "vm/machine/runtime/display.h"
#include "vm/machine/runtime/fault.h"
#include "vm/machine/runtime/lifecycle.h"
#include "vm/machine/runtime/machine_private.h"
#include "vm/machine/runtime/control.h"
#include "vm/machine/runtime/runner.h"
#include "vm/machine/runtime/waiting.h"
#include "vm/machine/debug.h"

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
    C_INT resumed;

    if (session == STD_NULL || session->core_machine == STD_NULL) return;
    control = &session->control;
    vm_machine_report_lifecycle(session, VM_MACHINE_RUNNING);
    while (vm_machine_executor_state_is_active(control->state)) {
        resumed = TYPE_FALSE;
        if (vm_machine_executor_state_take_reset(control->state)) {
            type_status reset_status = vm_machine_execution_context_reset(
                &control->execution_context);

            (C_VOID)vm_machine_finish_reset(session, reset_status);
            if (reset_status != TYPE_STATUS_OK) continue;
        }
        if (vm_machine_executor_state_pause_requested(control->state)) {
            /* The runner exclusively owns Core mutation.  Publish the final
             * VADP snapshot before acknowledging pause, so a paused debugger
             * or presenter never observes a stale mailbox frame. */
            (C_VOID)vm_machine_publish_display(session, TYPE_TRUE);
            vm_machine_executor_state_acknowledge_pause(control->state);
            vm_machine_control_signal_completion(control);
            vm_machine_report_lifecycle(session, VM_MACHINE_PAUSED);
        }
        while (vm_machine_executor_state_is_active(control->state) &&
            vm_machine_executor_state_is_paused(control->state)) {
            host_sync_event *wake_events[2];

            resumed = TYPE_TRUE;
            host_sync_event_reset(control->control_changed);
            vm_machine_execution_context_run_command_boundary(&control->execution_context);
            if (vm_machine_executor_state_is_active(control->state) &&
                vm_machine_executor_state_is_paused(control->state)) {
                wake_events[0] = vm_machine_executor_fifo_ready_event(
                    session->executor_fifo);
                wake_events[1] = control->control_changed;
                (C_VOID)host_sync_wait_any(wake_events, 2u, STD_NULL,
                    UINT32_MAX, STD_NULL);
            }
        }
        if (!vm_machine_executor_state_is_active(control->state)) break;
        if (resumed) vm_machine_report_lifecycle(session, VM_MACHINE_RUNNING);
        vm_machine_execution_context_run_command_boundary(&control->execution_context);
        vm_machine_execution_context_debug_refresh(&control->execution_context);
        if (vm_machine_executor_state_pause_requested(control->state)) continue;
        budget.instructions = vm_machine_control_step_requested(control) ? 1u :
            session->speed == VM_MACHINE_SPEED_TURBO ?
            VM_MACHINE_RUNNER_TURBO_QUANTUM_INSTRUCTIONS :
            VM_MACHINE_RUNNER_QUANTUM_INSTRUCTIONS;
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
                host_sync_yield();
            }
        }
        if (vm_machine_control_take_step(control)) {
            vm_machine_control_request_pause(control, VM_MACHINE_PAUSE_STEP);
        }
    }
    if (!vm_machine_executor_state_is_paused(control->state)) {
        vm_machine_report_lifecycle(session, VM_MACHINE_STOPPED);
    }
    vm_machine_control_signal_completion(control);
}
