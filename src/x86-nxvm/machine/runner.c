#include "lib/types/types_interface.h"

#include "x86-core/machine/machine_interface.h"
#include "lib/base/sync_interface.h"
#include "x86-nxvm/machine/display.h"
#include "x86-nxvm/machine/fault.h"
#include "x86-nxvm/machine/lifecycle.h"
#include "x86-nxvm/machine/machine_private.h"
#include "x86-nxvm/machine/control.h"
#include "x86-nxvm/machine/runner.h"
#include "x86-nxvm/machine/waiting.h"
#include "x86-nxvm/machine/debug.h"

/* A normal quantum bounds Core work between host control turns without copying the text frame
 * after every guest instruction.  Its tick ceiling is a host-control budget,
 * not a second guest clock or a timing conversion.  Single-step remains
 * exactly one instruction and is therefore not cut short by that ceiling. */
#define X86_NXVM_MACHINE_RUNNER_QUANTUM_INSTRUCTIONS 256u
#define X86_NXVM_MACHINE_RUNNER_TURBO_QUANTUM_INSTRUCTIONS 4096u

static void x86_nxvm_machine_runner_fail(x86_nxvm_machine *session)
{
    if (session == LIB_NULL) return;
    session->runner_failed = LIB_TRUE;
    x86_nxvm_machine_control_end_run(&session->control);
}

void x86_nxvm_machine_runner_run(x86_nxvm_machine *session)
{
    x86_core_machine_run_budget budget;
    x86_core_machine_run_result result;
    x86_nxvm_machine_control_state *control;

    if (session == LIB_NULL || session->x86_core_machine == LIB_NULL) return;
    control = &session->control;
    while (x86_nxvm_machine_executor_state_is_active(&control->state)) {
        if (session->executor_callback != LIB_NULL)
            session->executor_callback(session->executor_callback_context);
        if (!x86_nxvm_machine_executor_state_is_active(&control->state)) break;
        if (x86_nxvm_machine_executor_state_take_reset(&control->state)) {
            lib_status reset_status = x86_nxvm_machine_control_reset_at_boundary(control);

            (void)x86_nxvm_machine_finish_reset(session, reset_status);
            if (reset_status != LIB_STATUS_OK) {
                x86_nxvm_machine_runner_fail(session);
                continue;
            }
        }
        x86_nxvm_machine_control_refresh_debug(control);
        if (x86_nxvm_machine_debug_breakpoint_due(&session->debug)) {
            x86_nxvm_machine_debug_complete_breakpoint(&session->debug);
            continue;
        }
        budget.instructions = session->speed == X86_NXVM_MACHINE_SPEED_TURBO ?
            X86_NXVM_MACHINE_RUNNER_TURBO_QUANTUM_INSTRUCTIONS :
            X86_NXVM_MACHINE_RUNNER_QUANTUM_INSTRUCTIONS;
        budget.instructions = x86_nxvm_machine_debug_limit_instruction_budget(
            &session->debug, budget.instructions);
        /* Turbo remains bounded by instructions so control and presentation
         * stay responsive, but it must not impose a second tick throttle.
         * Core still advances every retired instruction and every device
         * deadline on its one guest-time axis. */
        budget.ticks = session->speed == X86_NXVM_MACHINE_SPEED_TURBO ? 0u :
            X86_NXVM_MACHINE_RUNNER_QUANTUM_INSTRUCTIONS;
        {
            lib_status run_status = x86_core_machine_run(session->x86_core_machine,
                budget, &result);

            if (run_status == LIB_STATUS_INTERNAL_ERROR ||
                result.reason == X86_CORE_MACHINE_STOP_FAULT) {
                x86_nxvm_machine_fault_capture(session, &result);
                x86_nxvm_machine_runner_fail(session);
                continue;
            }
            if (run_status != LIB_STATUS_OK) {
                x86_nxvm_machine_runner_fail(session);
                continue;
            }
        }
        x86_nxvm_machine_control_refresh_debug(control);
        if (result.reason == X86_CORE_MACHINE_STOP_PAUSED &&
            session->debug.observation_valid && session->debug.observation.watch_hit) {
            x86_nxvm_machine_debug_complete_watchpoint(&session->debug);
            continue;
        }
        x86_nxvm_machine_debug_complete_run(&session->debug, result.executed);
        /* A completed Core quantum is a safe Common rendezvous.  It must
         * precede wall-clock pacing: otherwise Standard can defer a pending
         * pause, reset, or debug completion behind a host wait even though
         * Core is no longer executing. */
        if (session->executor_callback != LIB_NULL)
            session->executor_callback(session->executor_callback_context);
        if (!x86_nxvm_machine_executor_state_is_active(&control->state)) break;
        {
            if (x86_nxvm_machine_pacing_wait(session) != LIB_STATUS_OK) {
                x86_nxvm_machine_runner_fail(session);
                continue;
            }
        }
        if (x86_nxvm_machine_publish_display(session, LIB_FALSE) != LIB_STATUS_OK) {
            x86_nxvm_machine_runner_fail(session);
            continue;
        }
        if (result.reason == X86_CORE_MACHINE_STOP_RESET_REQUESTED) {
            /* Core reset the requested processor state before returning. */
            x86_nxvm_machine_debug_reset(&session->debug);
        }
        if (result.reason == X86_CORE_MACHINE_STOP_REQUESTED) {
            x86_nxvm_machine_control_stop(control);
        }
        if (result.reason == X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
            /* Architectural shutdown is a resident CPU state, not a normal
             * HLT wait and not a Core fault.  End this host run without
             * requesting product stop, so only CPU reset, a qualified NMI,
             * or an explicit board reset can release the chip state. */
            if (result.detail == VCPUINS_EXCEPT_SHUTDOWN) {
                x86_nxvm_machine_control_end_run(control);
                continue;
            }
            lib_bool advanced = LIB_FALSE;
            lib_status time_status = x86_nxvm_machine_waiting_advance(
                session, &result, &advanced);

            if (time_status != LIB_STATUS_OK) x86_nxvm_machine_runner_fail(session);
            else if (!advanced) {
                /* Core has no source-qualified deadline to advance.  Yielding
                 * gives host input/control a turn without manufacturing guest
                 * time or restoring the old fixed-delay polling loop. */
                base_sync_yield();
            }
        }
    }
}
