/* Copyright 2012-2014 Neko. */

/*
 * DEVICE provides the device interface to drive the device thread
 * and access the virtual devices.
 */

#include "type.h"

#include "vm/machine/debug.h"

#include "vm/composition/session/provider_lifecycle.h"

#include "vm/composition/session/execution.h"
#include "vm/composition/session/fault.h"

#include "core/machine/machine_interface.h"

#include "core/platform/sleep.h"

#include "vm/composition/session/control.h"

#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/runner.h"

#include "vm/composition/session/session_private.h"

#include "vm/composition/session/display.h"

static type_status vm_session_execution_context_reset_callback(vm_session *machine)
{
    type_status status;

    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    vm_machine_debug_reset(&machine->debug);
    status = core_machine_reset(machine->core_machine);
    if (status != TYPE_STATUS_OK) {
        vm_session_control_stop(&machine->control);
    } else {
        vm_session_fault_clear(machine);
    }
    return status;
}

static C_VOID vm_session_execution_context_debug_refresh_callback(
    vm_session *machine)
{
    core_machine_debug_instruction_observation observation;

    if (machine == STD_NULL || core_machine_debug_capture_instruction_observation(
            machine->core_machine, &observation) != TYPE_STATUS_OK) return;
    vm_machine_debug_refresh(&machine->debug, &observation);
}

static const vm_session_execution_context_callbacks vm_session_execution_callbacks = {
    vm_session_execution_context_reset_callback,
    vm_session_execution_context_debug_refresh_callback
};

C_VOID vm_session_control_start(vm_session_control_state *control) {
    vm_session *machine;

    if (control == STD_NULL) return;
    machine = control->execution_context.session;
    if (machine == STD_NULL || machine->core_machine == STD_NULL) return;
    vm_session_execution_context_activate(&control->execution_context);
    STD_ATOMIC_STORE(&control->flagRun, TYPE_TRUE);
    STD_ATOMIC_STORE(&control->flagFlip, !STD_ATOMIC_LOAD(&control->flagFlip));
    vm_session_runner_run(machine);
    vm_session_execution_context_deactivate(&control->execution_context);
}

/* Issues resetting signal to device thread */
type_status vm_session_control_reset(vm_session_control_state *control) {
    if (control == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (STD_ATOMIC_LOAD(&control->flagRun)) {
        STD_ATOMIC_STORE(&control->flagReset, TYPE_TRUE);
        return TYPE_STATUS_OK;
    } else {
        type_status status = vm_session_execution_context_reset(
            &control->execution_context);

        STD_ATOMIC_STORE(&control->flagReset, TYPE_FALSE);
        return status;
    }
}

/* Issues stopping signal to device thread */
C_VOID vm_session_control_stop(vm_session_control_state *control)  {
    vm_session *machine;

    if (control == STD_NULL) return;
    machine = control->execution_context.session;
    if (machine != STD_NULL && machine->core_machine != STD_NULL) {
        core_machine_request_stop(machine->core_machine);
    }
    STD_ATOMIC_STORE(&control->flagRun, TYPE_FALSE);
    STD_ATOMIC_STORE(&control->paused, TYPE_FALSE);
    STD_ATOMIC_STORE(&control->pauseRequested, TYPE_FALSE);
}

C_VOID vm_session_control_yield_for_display_transition(
    vm_session_control_state *control)
{
    if (control == STD_NULL) return;
    /* The current instruction has completed and core is paused at the run
     * boundary. Unlike stop, this preserves guest state for the next presenter. */
    STD_ATOMIC_STORE(&control->flagRun, TYPE_FALSE);
    STD_ATOMIC_STORE(&control->paused, TYPE_FALSE);
    STD_ATOMIC_STORE(&control->pauseRequested, TYPE_FALSE);
}

C_VOID vm_session_control_fault(vm_session_control_state *control)
{
    if (control == STD_NULL) return;
    STD_ATOMIC_STORE(&control->flagRun, TYPE_FALSE);
    STD_ATOMIC_STORE(&control->paused, TYPE_FALSE);
    STD_ATOMIC_STORE(&control->pauseRequested, TYPE_FALSE);
    STD_ATOMIC_STORE(&control->stepRequested, TYPE_FALSE);
    STD_ATOMIC_STORE(&control->pauseReason, VM_SESSION_PAUSE_NONE);
}

C_VOID vm_session_control_request_pause(vm_session_control_state *control,
    vm_session_pause_reason reason)
{
    if (control == STD_NULL) return;
    if (!STD_ATOMIC_LOAD(&control->flagRun)) {
        STD_ATOMIC_STORE(&control->paused, TYPE_TRUE);
        STD_ATOMIC_STORE(&control->pauseReason, reason);
        return;
    }
    STD_ATOMIC_STORE(&control->pauseRequested, TYPE_TRUE);
    STD_ATOMIC_STORE(&control->pauseReason, reason);
}

C_INT vm_session_control_wait_for_pause(vm_session_control_state *control,
    C_UINT milliseconds)
{
    C_UINT waited = 0u;

    if (control == STD_NULL) return TYPE_FALSE;
    while (STD_ATOMIC_LOAD(&control->flagRun) && !STD_ATOMIC_LOAD(&control->paused) &&
           waited < milliseconds) {
        core_platform_sleep_milliseconds(1u);
        ++waited;
    }
    return STD_ATOMIC_LOAD(&control->paused);
}

C_INT vm_session_control_is_paused(const vm_session_control_state *control)
{
    return control != STD_NULL && STD_ATOMIC_LOAD(&control->paused);
}

vm_session_pause_reason vm_session_control_get_pause_reason(
    const vm_session_control_state *control)
{
    return control == STD_NULL ? VM_SESSION_PAUSE_NONE :
        (vm_session_pause_reason)STD_ATOMIC_LOAD(&control->pauseReason);
}

C_VOID vm_session_control_continue(vm_session_control_state *control)
{
    if (control == STD_NULL) return;
    STD_ATOMIC_STORE(&control->pauseRequested, TYPE_FALSE);
    STD_ATOMIC_STORE(&control->paused, TYPE_FALSE);
    STD_ATOMIC_STORE(&control->stepRequested, TYPE_FALSE);
    STD_ATOMIC_STORE(&control->pauseReason, VM_SESSION_PAUSE_NONE);
}

C_INT vm_session_control_step(vm_session_control_state *control)
{
    if (control == STD_NULL || !STD_ATOMIC_LOAD(&control->paused)) return TYPE_FALSE;
    STD_ATOMIC_STORE(&control->pauseRequested, TYPE_FALSE);
    STD_ATOMIC_STORE(&control->paused, TYPE_FALSE);
    STD_ATOMIC_STORE(&control->stepRequested, TYPE_TRUE);
    STD_ATOMIC_STORE(&control->pauseReason, VM_SESSION_PAUSE_NONE);
    return TYPE_TRUE;
}

C_VOID vm_session_control_bind_command_boundary(
    vm_session_control_state *control,
    C_VOID (*callback)(C_VOID *opaque), C_VOID *opaque)
{
    vm_session_execution_context_bind_command_boundary(
        control == STD_NULL ? STD_NULL : &control->execution_context, callback, opaque);
}

/* Initializes devices */
type_status vm_session_control_initialize(vm_session_control_state *control,
    vm_session *machine) {
    type_status status;

    if (control == STD_NULL || machine == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    STD_MEMSET((C_VOID *)(control), TYPE_ZERO_8, sizeof(*control));
    STD_ATOMIC_INIT(&control->flagFlip, TYPE_FALSE);
    STD_ATOMIC_INIT(&control->flagRun, TYPE_FALSE);
    STD_ATOMIC_INIT(&control->flagReset, TYPE_FALSE);
    STD_ATOMIC_INIT(&control->pauseRequested, TYPE_FALSE);
    STD_ATOMIC_INIT(&control->paused, TYPE_FALSE);
    STD_ATOMIC_INIT(&control->stepRequested, TYPE_FALSE);
    STD_ATOMIC_INIT(&control->pauseReason, VM_SESSION_PAUSE_NONE);
    vm_session_execution_context_initialize(&control->execution_context);
    vm_session_execution_context_bind_session(&control->execution_context,
        machine);
    vm_session_execution_context_bind_callbacks(
        &control->execution_context, &vm_session_execution_callbacks);
    vm_session_execution_context_activate(&control->execution_context);
    vm_machine_debug_initialize(&machine->debug);
    status = vm_session_provider_lifecycle_initialize(machine);
    if (status == TYPE_STATUS_OK) {
        status = vm_session_bind_execution_provider(machine);
    }
    if (status != TYPE_STATUS_OK) {
        vm_session_control_stop(control);
    }
    return status;
}

/* Finalizes devices */
C_VOID vm_session_control_finalize(vm_session_control_state *control,
    vm_session *machine) {
    if (control == STD_NULL || machine == STD_NULL) return;
    vm_session_execution_context_deactivate(&control->execution_context);
    vm_session_provider_lifecycle_finalize(machine);
    vm_machine_debug_finalize(&machine->debug);
}

C_VOID vm_session_control_print_status(const vm_session_control_state *control) {
    STD_PRINTF("Recording: %s\n", control != STD_NULL &&
        control->execution_context.session != STD_NULL &&
        control->execution_context.session->debug.
            connect.recordFile ? "Yes" : "No");
    STD_PRINTF("Running:   %s\n", control != STD_NULL && STD_ATOMIC_LOAD(&control->flagRun) ?
        "Yes" : "No");
}

C_INT vm_session_control_is_running(const vm_session_control_state *control)
{
    return control != STD_NULL && STD_ATOMIC_LOAD(&control->flagRun) &&
        !STD_ATOMIC_LOAD(&control->paused);
}

C_INT vm_session_control_get_flip(const vm_session_control_state *control)
{
    return control != STD_NULL && STD_ATOMIC_LOAD(&control->flagFlip);
}
