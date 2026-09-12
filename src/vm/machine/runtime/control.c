/* Copyright 2012-2014 Neko. */

/*
 * DEVICE provides the device interface to drive the device thread
 * and access the virtual devices.
 */

#include "type.h"

#include "vm/machine/debug.h"

#include "vm/machine/runtime/machine_devices.h"

#include "vm/machine/runtime/execution.h"
#include "vm/machine/runtime/fault.h"

#include "core/machine/machine_interface.h"

#include "lib/host/sync_interface.h"

#include "vm/machine/runtime/control.h"

#include "vm/machine/runtime/lifecycle.h"
#include "vm/machine/runtime/runner.h"

#include "vm/machine/runtime/machine_private.h"

#include "vm/machine/runtime/display.h"

static type_status vm_machine_execution_context_reset_callback(vm_machine *machine)
{
    type_status status;

    if (machine == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    vm_machine_debug_reset(&machine->debug);
    status = core_machine_reset(machine->core_machine);
    if (status != TYPE_STATUS_OK) {
        vm_machine_control_stop(&machine->control);
    } else {
        vm_machine_fault_clear(machine);
    }
    return status;
}

static C_VOID vm_machine_execution_context_debug_refresh_callback(
    vm_machine *machine)
{
    core_machine_debug_instruction_observation observation;

    if (machine == STD_NULL || core_machine_debug_capture_instruction_observation(
            machine->core_machine, &observation) != TYPE_STATUS_OK) return;
    vm_machine_debug_refresh(&machine->debug, &observation);
}

static const vm_machine_execution_context_callbacks vm_machine_execution_callbacks = {
    vm_machine_execution_context_reset_callback,
    vm_machine_execution_context_debug_refresh_callback
};

C_VOID vm_machine_control_start(vm_machine_control_state *control) {
    vm_machine *machine;

    if (control == STD_NULL) return;
    machine = control->execution_context.machine;
    if (machine == STD_NULL || machine->core_machine == STD_NULL) return;
    host_sync_event_reset(control->completion_ready);
    vm_machine_executor_state_start(control->state);
    vm_machine_execution_context_activate(&control->execution_context);
    vm_machine_runner_run(machine);
    vm_machine_execution_context_deactivate(&control->execution_context);
}

/* Issues resetting signal to device thread */
type_status vm_machine_control_reset(vm_machine_control_state *control) {
    if (control == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (vm_machine_executor_state_is_active(control->state)) {
        vm_machine_executor_state_request_reset(control->state);
        return TYPE_STATUS_OK;
    } else {
        type_status status = vm_machine_execution_context_reset(
            &control->execution_context);

        (C_VOID)vm_machine_executor_state_take_reset(control->state);
        return status;
    }
}

/* Issues stopping signal to device thread */
C_VOID vm_machine_control_stop(vm_machine_control_state *control)  {
    vm_machine *machine;

    if (control == STD_NULL) return;
    machine = control->execution_context.machine;
    if (machine != STD_NULL && machine->core_machine != STD_NULL) {
        common_machine_debug_invalidate(machine->executor);
        core_machine_request_stop(machine->core_machine);
    }
    atomic_store(&control->step_requested, TYPE_FALSE);
    atomic_store(&control->pause_reason, VM_MACHINE_PAUSE_NONE);
    vm_machine_executor_state_stop(control->state);
    host_sync_event_signal(control->control_changed);
}

C_VOID vm_machine_control_fault(vm_machine_control_state *control)
{
    vm_machine *machine;

    if (control == STD_NULL) return;
    machine = control->execution_context.machine;
    if (machine != STD_NULL) common_machine_debug_invalidate(machine->executor);
    atomic_store(&control->step_requested, TYPE_FALSE);
    atomic_store(&control->pause_reason, VM_MACHINE_PAUSE_NONE);
    vm_machine_executor_state_stop(control->state);
    host_sync_event_signal(control->control_changed);
}

C_VOID vm_machine_control_request_pause(vm_machine_control_state *control,
    vm_machine_pause_reason reason)
{
    if (control == STD_NULL) return;
    atomic_store(&control->step_requested, TYPE_FALSE);
    atomic_store(&control->pause_reason, reason);
    vm_machine_executor_state_request_pause(control->state);
    host_sync_event_signal(control->control_changed);
}

C_INT vm_machine_control_wait_for_pause(vm_machine_control_state *control,
    C_UINT milliseconds)
{
    if (control == STD_NULL) return TYPE_FALSE;
    if (!vm_machine_executor_state_is_paused(control->state)) (C_VOID)host_sync_event_wait(
        control->completion_ready, milliseconds);
    return vm_machine_executor_state_is_paused(control->state);
}

C_INT vm_machine_control_wait_for_completion(
    vm_machine_control_state *control)
{
    return control != STD_NULL && host_sync_event_wait(control->completion_ready,
        UINT32_MAX) == HOST_SYNC_WAIT_SIGNALED;
}

C_VOID vm_machine_control_signal_completion(
    vm_machine_control_state *control)
{
    if (control != STD_NULL) host_sync_event_signal(control->completion_ready);
}

C_INT vm_machine_control_is_paused(const vm_machine_control_state *control)
{
    return control != STD_NULL && vm_machine_executor_state_is_paused(control->state);
}

vm_machine_pause_reason vm_machine_control_get_pause_reason(
    const vm_machine_control_state *control)
{
    return control == STD_NULL ? VM_MACHINE_PAUSE_NONE :
        (vm_machine_pause_reason)atomic_load(&control->pause_reason);
}

C_VOID vm_machine_control_continue(vm_machine_control_state *control)
{
    if (control == STD_NULL) return;
    atomic_store(&control->step_requested, TYPE_FALSE);
    atomic_store(&control->pause_reason, VM_MACHINE_PAUSE_NONE);
    host_sync_event_reset(control->completion_ready);
    vm_machine_executor_state_resume(control->state);
    host_sync_event_signal(control->control_changed);
}

C_INT vm_machine_control_step(vm_machine_control_state *control)
{
    if (control == STD_NULL || !vm_machine_executor_state_is_paused(control->state)) {
        return TYPE_FALSE;
    }
    atomic_store(&control->step_requested, TYPE_TRUE);
    atomic_store(&control->pause_reason, VM_MACHINE_PAUSE_NONE);
    host_sync_event_reset(control->completion_ready);
    vm_machine_executor_state_resume(control->state);
    host_sync_event_signal(control->control_changed);
    return TYPE_TRUE;
}

C_INT vm_machine_control_step_requested(const vm_machine_control_state *control)
{ return control != STD_NULL && atomic_load(&control->step_requested); }

C_INT vm_machine_control_take_step(vm_machine_control_state *control)
{ return control != STD_NULL && atomic_exchange(&control->step_requested, TYPE_FALSE); }

C_VOID vm_machine_control_bind_command_boundary(
    vm_machine_control_state *control,
    C_VOID (*callback)(C_VOID *opaque), C_VOID *opaque)
{
    vm_machine_execution_context_bind_command_boundary(
        control == STD_NULL ? STD_NULL : &control->execution_context, callback, opaque);
}

/* Initializes devices */
type_status vm_machine_control_initialize(vm_machine_control_state *control,
    vm_machine *machine) {
    type_status status;

    if (control == STD_NULL || machine == STD_NULL) {
        return TYPE_STATUS_INVALID_ARGUMENT;
    }
    STD_MEMSET((C_VOID *)(control), TYPE_ZERO_8, sizeof(*control));
    atomic_init(&control->step_requested, TYPE_FALSE);
    atomic_init(&control->pause_reason, VM_MACHINE_PAUSE_NONE);
    status = vm_machine_executor_state_create(&control->state);
    if (status != LIB_STATUS_OK) return TYPE_STATUS_NO_MEMORY;
    if (host_sync_event_create(&control->completion_ready) != LIB_STATUS_OK ||
        host_sync_event_create(&control->control_changed) != LIB_STATUS_OK) {
        host_sync_event_destroy(control->completion_ready);
        host_sync_event_destroy(control->control_changed);
        vm_machine_executor_state_destroy(control->state);
        control->state = STD_NULL;
        return TYPE_STATUS_NO_MEMORY;
    }
    vm_machine_execution_context_initialize(&control->execution_context);
    vm_machine_execution_context_bind_machine(&control->execution_context,
        machine);
    vm_machine_execution_context_bind_callbacks(
        &control->execution_context, &vm_machine_execution_callbacks);
    vm_machine_execution_context_activate(&control->execution_context);
    vm_machine_debug_initialize(&machine->debug);
    status = vm_machine_devices_initialize_media(machine);
    if (status == TYPE_STATUS_OK) status = vm_machine_devices_bind_media(machine);
    if (status == TYPE_STATUS_OK) {
        status = vm_machine_bind_execution_provider(machine);
    }
    if (status != TYPE_STATUS_OK) {
        vm_machine_control_stop(control);
    }
    return status;
}

/* Finalizes devices */
C_VOID vm_machine_control_finalize(vm_machine_control_state *control,
    vm_machine *machine) {
    if (control == STD_NULL || machine == STD_NULL) return;
    vm_machine_execution_context_deactivate(&control->execution_context);
    vm_machine_devices_finalize(machine);
    vm_machine_debug_finalize(&machine->debug);
    host_sync_event_destroy(control->completion_ready);
    host_sync_event_destroy(control->control_changed);
    control->completion_ready = STD_NULL;
    control->control_changed = STD_NULL;
    vm_machine_executor_state_destroy(control->state);
    control->state = STD_NULL;
}

C_INT vm_machine_control_is_running(const vm_machine_control_state *control)
{
    return control != STD_NULL && vm_machine_executor_state_is_active(control->state) &&
        !vm_machine_executor_state_is_paused(control->state);
}
