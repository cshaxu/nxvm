#include "type.h"

#include "core/machine/machine_interface.h"
#include "lib/host/sync.h"
#include "vm/composition/session/execution.h"
#include "vm/composition/session/display.h"
#include "vm/composition/session/fault.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/control.h"
#include "vm/composition/session/runner.h"
#include "vm/composition/session/waiting.h"
#include "vm/machine/debug.h"

/* A normal quantum bounds Core work between host control turns without copying the text frame
 * after every guest instruction.  Its tick ceiling is a host-control budget,
 * not a second guest clock or a timing conversion.  Single-step remains
 * exactly one instruction and is therefore not cut short by that ceiling. */
#define VM_SESSION_RUNNER_QUANTUM_INSTRUCTIONS 256u
#define VM_SESSION_RUNNER_TURBO_QUANTUM_INSTRUCTIONS 4096u

static C_VOID vm_session_runner_update_mouse_policy(vm_session *session,
    const vm_session_control_state *control)
{
    C_INT capturable;

    if (session == STD_NULL || control == STD_NULL) return;
    capturable = vm_session_control_is_running(control) &&
        vm_platform_run_context_get_window_display(session->platform_run_context);
    (C_VOID)vm_platform_run_context_set_mouse_capturable(
        session->platform_run_context, capturable);
}

static C_VOID vm_session_runner_toggle_pause(vm_session *session,
    vm_session_control_state *control)
{
    if (session == STD_NULL || control == STD_NULL) return;
    if (vm_session_state_is_paused(control->state)) {
        vm_session_control_continue(control);
        vm_session_runner_update_mouse_policy(session, control);
        (C_VOID)vm_platform_run_context_set_window_title(
            session->platform_run_context, "NXVM (Running)");
    } else {
        vm_session_control_request_pause(control, VM_SESSION_PAUSE_EXPLICIT);
    }
}

C_VOID vm_session_runner_run(vm_session *session)
{
    core_machine_run_budget budget;
    core_machine_run_result result;
    vm_session_control_state *control;

    if (session == STD_NULL || session->core_machine == STD_NULL) return;
    control = &session->control;
    vm_session_runner_update_mouse_policy(session, control);
    while (vm_session_state_is_active(control->state)) {
        if (vm_platform_run_handle_take_stop_report(session->platform_run_handle)) {
            vm_session_control_stop(control);
            continue;
        }
        if (vm_platform_run_handle_take_pause_report(session->platform_run_handle)) {
            vm_session_runner_toggle_pause(session, control);
            continue;
        }
        if (vm_session_state_take_reset(control->state)) {
            type_status reset_status = vm_session_execution_context_reset(
                &control->execution_context);

            (C_VOID)vm_session_finish_reset(session, reset_status);
            if (reset_status != TYPE_STATUS_OK) continue;
        }
        if (vm_session_state_pause_requested(control->state)) {
            /* The runner exclusively owns Core mutation.  Publish the final
             * VADP snapshot before acknowledging pause, so a paused debugger
             * or presenter never observes a stale mailbox frame. */
            (C_VOID)vm_session_publish_display(session, TYPE_TRUE);
            (C_VOID)vm_platform_run_context_set_mouse_capturable(
                session->platform_run_context, TYPE_FALSE);
            vm_session_state_acknowledge_pause(control->state);
            vm_platform_run_context_set_window_title(session->platform_run_context,
                "NXVM (Paused)");
            /* A Console session owns the one process Console surface while
             * running. End its runner at a paused boundary so the display
             * thread releases that lease and START returns to the NXVM
             * command Console. A Window session keeps its presenter alive for
             * its paused frame and can resume in place. */
            if (vm_platform_run_handle_is_active(session->platform_run_handle) &&
                !vm_platform_run_handle_is_window_display(
                    session->platform_run_handle)) {
                vm_platform_run_handle_request_presenter_stop(
                    session->platform_run_handle);
                vm_session_state_stop(control->state);
                break;
            }
        }
        while (vm_session_state_is_active(control->state) &&
            vm_session_state_is_paused(control->state)) {
            if (vm_platform_run_handle_take_stop_report(session->platform_run_handle)) {
                vm_session_control_stop(control);
                continue;
            }
            if (vm_platform_run_handle_take_pause_report(session->platform_run_handle)) {
                vm_session_runner_toggle_pause(session, control);
                continue;
            }
            vm_session_execution_context_run_command_boundary(&control->execution_context);
            host_sync_sleep_milliseconds(1u);
        }
        if (!vm_session_state_is_active(control->state)) break;
        vm_session_execution_context_run_command_boundary(&control->execution_context);
        vm_session_execution_context_debug_refresh(&control->execution_context);
        if (vm_session_state_pause_requested(control->state)) continue;
        budget.instructions = vm_session_control_step_requested(control) ? 1u :
            session->speed == VM_SESSION_SPEED_TURBO ?
            VM_SESSION_RUNNER_TURBO_QUANTUM_INSTRUCTIONS :
            VM_SESSION_RUNNER_QUANTUM_INSTRUCTIONS;
        /* Turbo remains bounded by instructions so control and presentation
         * stay responsive, but it must not impose a second tick throttle.
         * Core still advances every retired instruction and every device
         * deadline on its one guest-time axis. */
        budget.ticks = vm_session_control_step_requested(control) ||
            session->speed == VM_SESSION_SPEED_TURBO ? 0u :
            VM_SESSION_RUNNER_QUANTUM_INSTRUCTIONS;
        {
            type_status run_status = core_machine_run(session->core_machine,
                budget, &result);

            if (run_status == TYPE_STATUS_FAULT ||
                result.reason == CORE_MACHINE_STOP_FAULT) {
                vm_session_fault_capture(session, &result);
                vm_session_control_fault(control);
                continue;
            }
            if (run_status != TYPE_STATUS_OK) {
                vm_session_control_stop(control);
                continue;
            }
        }
        {
            if (vm_session_pacing_wait(session) != TYPE_STATUS_OK) {
                vm_session_control_stop(control);
                continue;
            }
        }
        (C_VOID)vm_session_publish_display(session, TYPE_FALSE);
        if (result.reason == CORE_MACHINE_STOP_RESET_REQUESTED) {
            /* Core reset the requested processor state before returning. */
            vm_machine_debug_reset(&session->debug);
        }
        if (result.reason == CORE_MACHINE_STOP_REQUESTED) {
            vm_session_control_stop(control);
        }
        if (result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT &&
            !vm_session_control_step_requested(control)) {
            C_INT advanced = 0;
            type_status time_status = vm_session_waiting_advance(
                session, &result, &advanced);

            if (time_status != TYPE_STATUS_OK) vm_session_control_stop(control);
            else if (!advanced) {
                /* Core has no source-qualified deadline to advance.  Yielding
                 * gives host input/control a turn without manufacturing guest
                 * time or restoring the old fixed-delay polling loop. */
                host_sync_yield();
            }
        }
        if (vm_session_control_take_step(control)) {
            vm_session_control_request_pause(control, VM_SESSION_PAUSE_STEP);
        }
    }
    vm_platform_run_handle_request_presenter_stop(session->platform_run_handle);
}
