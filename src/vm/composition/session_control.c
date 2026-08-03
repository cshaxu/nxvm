/* Copyright 2012-2014 Neko. */

/*
 * DEVICE provides the device interface to drive the device thread
 * and access the virtual devices.
 */

#include "type.h"

#include "core/product/utils.h"


#include "vm/machine/debug.h"

#include "vm/composition/providers.h"

#include "vm/composition/session_execution_context.h"

#include "core/machine/cpu.h"

#include "core/machine/machine_interface.h"

#include "core/machine/memory.h"

#include "core/machine/port.h"

#include "core/platform/sleep.h"

#include "vm/composition/session_control.h"

#include "vm/composition/session_lifecycle.h"
#include "vm/composition/session_runner.h"

#include "vm/composition/session.h"

#include "vm/composition/display_bridge.h"

static C_VOID device_execution_context_reset(C_VOID *device)
{
    vm_session *machine =
        (vm_session *)device;
    vm_machine_debug_reset(machine->debug);
    if (core_machine_reset(machine->core_machine) != NTVDM64_STATUS_OK) {
        vm_session_control_stop(machine->control);
    }
}

static C_VOID device_execution_context_debug_refresh(C_VOID *device)
{
    vm_session *machine =
        (vm_session *)device;
    vm_machine_debug_refresh(machine == STD_NULL ? STD_NULL : machine->debug);
}

static const vm_session_execution_context_callbacks device_execution_callbacks = {
    device_execution_context_reset,
    device_execution_context_debug_refresh
};

C_VOID vm_session_control_start(vm_session_control_state *control) {
    vm_session *machine;

    if (control == STD_NULL) return;
    machine = (vm_session *)control->execution_context.device;
    if (machine == STD_NULL || machine->core_machine == STD_NULL) return;
    vm_session_execution_context_activate(&control->execution_context);
    STD_ATOMIC_STORE(&control->flagRun, NTVDM64_TYPE_TRUE);
    STD_ATOMIC_STORE(&control->flagFlip, !STD_ATOMIC_LOAD(&control->flagFlip));
    vm_session_runner_run(machine);
    vm_session_execution_context_deactivate(&control->execution_context);
}

/* Issues resetting signal to device thread */
C_VOID vm_session_control_reset(vm_session_control_state *control) {
    if (control == STD_NULL) return;
    if (STD_ATOMIC_LOAD(&control->flagRun)) {
        STD_ATOMIC_STORE(&control->flagReset, NTVDM64_TYPE_TRUE);
    } else {
        vm_session_execution_context_reset(&control->execution_context);
        STD_ATOMIC_STORE(&control->flagReset, NTVDM64_TYPE_FALSE);
    }
}

/* Issues stopping signal to device thread */
C_VOID vm_session_control_stop(vm_session_control_state *control)  {
    vm_session *machine;

    if (control == STD_NULL) return;
    machine = (vm_session *)control->execution_context.device;
    if (machine != STD_NULL && machine->core_machine != STD_NULL) {
        core_machine_request_stop(machine->core_machine);
    }
    STD_ATOMIC_STORE(&control->flagRun, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_STORE(&control->paused, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_STORE(&control->pauseRequested, NTVDM64_TYPE_FALSE);
}

C_VOID vm_session_control_request_pause(vm_session_control_state *control,
    vm_session_pause_reason reason)
{
    if (control == STD_NULL) return;
    if (!STD_ATOMIC_LOAD(&control->flagRun)) {
        STD_ATOMIC_STORE(&control->paused, NTVDM64_TYPE_TRUE);
        STD_ATOMIC_STORE(&control->pauseReason, reason);
        return;
    }
    STD_ATOMIC_STORE(&control->pauseRequested, NTVDM64_TYPE_TRUE);
    STD_ATOMIC_STORE(&control->pauseReason, reason);
}

C_INT vm_session_control_wait_for_pause(vm_session_control_state *control,
    C_UINT milliseconds)
{
    C_UINT waited = 0u;

    if (control == STD_NULL) return NTVDM64_TYPE_FALSE;
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
    STD_ATOMIC_STORE(&control->pauseRequested, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_STORE(&control->paused, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_STORE(&control->stepRequested, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_STORE(&control->pauseReason, VM_SESSION_PAUSE_NONE);
}

C_INT vm_session_control_step(vm_session_control_state *control)
{
    if (control == STD_NULL || !STD_ATOMIC_LOAD(&control->paused)) return NTVDM64_TYPE_FALSE;
    STD_ATOMIC_STORE(&control->pauseRequested, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_STORE(&control->paused, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_STORE(&control->stepRequested, NTVDM64_TYPE_TRUE);
    STD_ATOMIC_STORE(&control->pauseReason, VM_SESSION_PAUSE_NONE);
    return NTVDM64_TYPE_TRUE;
}

C_VOID vm_session_control_bind_command_boundary(
    vm_session_control_state *control,
    C_VOID (*callback)(C_VOID *opaque), C_VOID *opaque)
{
    vm_session_execution_context_bind_command_boundary(
        control == STD_NULL ? STD_NULL : &control->execution_context, callback, opaque);
}

/* Initializes devices */
C_VOID vm_session_control_initialize(vm_session_control_state *control,
    vm_session *machine) {

    if (control == STD_NULL || machine == STD_NULL) return;
    STD_MEMSET((C_VOID *)(control), NTVDM64_TYPE_ZERO_8, sizeof(*control));
    STD_ATOMIC_INIT(&control->flagFlip, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_INIT(&control->flagRun, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_INIT(&control->flagReset, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_INIT(&control->pauseRequested, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_INIT(&control->paused, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_INIT(&control->stepRequested, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_INIT(&control->pauseReason, VM_SESSION_PAUSE_NONE);
    vm_session_execution_context_initialize(&control->execution_context);
    vm_session_execution_context_bind_machine_state(
        &control->execution_context,
        vm_composition_machine_access_cpu(machine->core_access),
        vm_composition_machine_access_memory(machine->core_access),
        vm_composition_machine_access_port(machine->core_access),
        machine);
    vm_session_execution_context_bind_callbacks(
        &control->execution_context, &device_execution_callbacks);
    vm_session_execution_context_activate(&control->execution_context);
    vm_machine_debug_initialize(machine->debug,
        vm_composition_machine_access_cpu(machine->core_access),
        vm_composition_machine_access_instructions(machine->core_access));
    vm_session_providers_initialize(machine);
    if (!vm_session_bind_execution_provider(machine)) {
        vm_session_control_stop(control);
    }
}

/* Finalizes devices */
C_VOID vm_session_control_finalize(vm_session_control_state *control,
    vm_session *machine) {
    if (control == STD_NULL || machine == STD_NULL) return;
    vm_session_execution_context_deactivate(&control->execution_context);
    vm_machine_debug_finalize(machine->debug);
    vm_session_providers_finalize(machine);
}

C_VOID vm_session_control_print_status(const vm_session_control_state *control) {
    STD_PRINTF("Recording: %s\n", control != STD_NULL &&
        control->execution_context.device != STD_NULL &&
        ((vm_session *)control->execution_context.device)->debug->
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
