/* Copyright 2012-2014 Neko. */

/*
 * DEVICE provides the device interface to drive the device thread
 * and access the virtual devices.
 */

#include "type.h"

#include "core/product/utils.h"


#include "vm/machine/debug.h"

#include "vm/composition/composition.h"

#include "core/product/runtime/execution_context.h"

#include "core/machine/cpu.h"

#include "core/machine/machine_interface.h"

#include "core/machine/memory.h"

#include "core/machine/port.h"

#include "core/platform/sleep.h"

#include "vm/composition/composition_control.h"

#include "vm/composition/composition_machine.h"

#include "vm/composition/composition_live_machine.h"

#include "vm/composition/composition_display.h"

static C_VOID device_execution_context_reset(C_VOID *device)
{
    vm_composition_live_machine *machine =
        (vm_composition_live_machine *)device;
    vm_machine_debug_reset(machine->debug);
    if (core_machine_reset(machine->core_machine) != NTVDM64_STATUS_OK) {
        vm_composition_control_stop(machine->control);
    }
}

static C_VOID device_execution_context_debug_refresh(C_VOID *device)
{
    vm_composition_live_machine *machine =
        (vm_composition_live_machine *)device;
    vm_machine_debug_refresh(machine == STD_NULL ? STD_NULL : machine->debug);
}

static const core_product_execution_context_callbacks device_execution_callbacks = {
    device_execution_context_reset,
    device_execution_context_debug_refresh
};

/* Starts device thread */
C_VOID vm_composition_control_start(vm_composition_control_state *control) {
    core_machine_run_budget budget = {1u, 0u};
    core_machine_run_result result;
    vm_composition_live_machine *machine;

    if (control == STD_NULL) return;
    machine = (vm_composition_live_machine *)control->execution_context.device;
    if (machine == STD_NULL || machine->core_machine == STD_NULL) return;
    core_product_execution_context_activate(&control->execution_context);
    STD_ATOMIC_STORE(&control->flagRun, NTVDM64_TYPE_TRUE);
    STD_ATOMIC_STORE(&control->flagFlip, !STD_ATOMIC_LOAD(&control->flagFlip));
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
        if (!STD_ATOMIC_LOAD(&control->flagRun)) {
            break;
        }
        if (core_machine_run(machine->core_machine, budget, &result) !=
            NTVDM64_STATUS_OK) {
            vm_composition_control_stop(control);
            continue;
        }
        vm_composition_publish_display(machine, NTVDM64_TYPE_FALSE);
        if (result.reason == CORE_MACHINE_STOP_RESET_REQUESTED) {
            vm_composition_control_reset(control);
        }
        if (result.reason == CORE_MACHINE_STOP_REQUESTED) {
            vm_composition_control_stop(control);
        }
        if (result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
            core_platform_sleep_milliseconds(1u);
        }
        if (STD_ATOMIC_EXCHANGE(&control->stepRequested, NTVDM64_TYPE_FALSE)) {
            vm_composition_control_request_pause(control, VM_COMPOSITION_PAUSE_STEP);
        }
    }
    core_product_execution_context_deactivate(&control->execution_context);
}

/* Issues resetting signal to device thread */
C_VOID vm_composition_control_reset(vm_composition_control_state *control) {
    if (control == STD_NULL) return;
    if (STD_ATOMIC_LOAD(&control->flagRun)) {
        STD_ATOMIC_STORE(&control->flagReset, NTVDM64_TYPE_TRUE);
    } else {
        core_product_execution_context_reset(&control->execution_context);
        STD_ATOMIC_STORE(&control->flagReset, NTVDM64_TYPE_FALSE);
    }
}

/* Issues stopping signal to device thread */
C_VOID vm_composition_control_stop(vm_composition_control_state *control)  {
    vm_composition_live_machine *machine;

    if (control == STD_NULL) return;
    machine = (vm_composition_live_machine *)control->execution_context.device;
    if (machine != STD_NULL && machine->core_machine != STD_NULL) {
        core_machine_request_stop(machine->core_machine);
    }
    STD_ATOMIC_STORE(&control->flagRun, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_STORE(&control->paused, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_STORE(&control->pauseRequested, NTVDM64_TYPE_FALSE);
}

C_VOID vm_composition_control_request_pause(vm_composition_control_state *control,
    vm_composition_pause_reason reason)
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

C_INT vm_composition_control_wait_for_pause(vm_composition_control_state *control,
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

C_INT vm_composition_control_is_paused(const vm_composition_control_state *control)
{
    return control != STD_NULL && STD_ATOMIC_LOAD(&control->paused);
}

vm_composition_pause_reason vm_composition_control_get_pause_reason(
    const vm_composition_control_state *control)
{
    return control == STD_NULL ? VM_COMPOSITION_PAUSE_NONE :
        (vm_composition_pause_reason)STD_ATOMIC_LOAD(&control->pauseReason);
}

C_VOID vm_composition_control_continue(vm_composition_control_state *control)
{
    if (control == STD_NULL) return;
    STD_ATOMIC_STORE(&control->pauseRequested, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_STORE(&control->paused, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_STORE(&control->stepRequested, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_STORE(&control->pauseReason, VM_COMPOSITION_PAUSE_NONE);
}

C_INT vm_composition_control_step(vm_composition_control_state *control)
{
    if (control == STD_NULL || !STD_ATOMIC_LOAD(&control->paused)) return NTVDM64_TYPE_FALSE;
    STD_ATOMIC_STORE(&control->pauseRequested, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_STORE(&control->paused, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_STORE(&control->stepRequested, NTVDM64_TYPE_TRUE);
    STD_ATOMIC_STORE(&control->pauseReason, VM_COMPOSITION_PAUSE_NONE);
    return NTVDM64_TYPE_TRUE;
}

C_VOID vm_composition_control_bind_command_boundary(
    vm_composition_control_state *control,
    C_VOID (*callback)(C_VOID *opaque), C_VOID *opaque)
{
    core_product_execution_context_bind_command_boundary(
        control == STD_NULL ? STD_NULL : &control->execution_context, callback, opaque);
}

/* Initializes devices */
C_VOID vm_composition_control_initialize(vm_composition_control_state *control,
    vm_composition_live_machine *machine) {

    if (control == STD_NULL || machine == STD_NULL) return;
    STD_MEMSET((C_VOID *)(control), NTVDM64_TYPE_ZERO_8, sizeof(*control));
    STD_ATOMIC_INIT(&control->flagFlip, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_INIT(&control->flagRun, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_INIT(&control->flagReset, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_INIT(&control->pauseRequested, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_INIT(&control->paused, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_INIT(&control->stepRequested, NTVDM64_TYPE_FALSE);
    STD_ATOMIC_INIT(&control->pauseReason, VM_COMPOSITION_PAUSE_NONE);
    core_product_execution_context_initialize(&control->execution_context);
    core_product_execution_context_bind_machine_state(
        &control->execution_context, machine->cpu, machine->ram, machine->port,
        machine);
    core_product_execution_context_bind_callbacks(
        &control->execution_context, &device_execution_callbacks);
    core_product_execution_context_activate(&control->execution_context);
    vm_machine_debug_initialize(machine->debug, machine->cpu, machine->cpuins);
    vm_composition_providers_initialize(machine);
    if (!vm_composition_bind_execution_provider(machine)) {
        vm_composition_control_stop(control);
    }
}

/* Finalizes devices */
C_VOID vm_composition_control_finalize(vm_composition_control_state *control,
    vm_composition_live_machine *machine) {
    if (control == STD_NULL || machine == STD_NULL) return;
    core_product_execution_context_deactivate(&control->execution_context);
    vm_machine_debug_finalize(machine->debug);
    vm_composition_providers_finalize(machine);
}

C_VOID vm_composition_control_print_status(const vm_composition_control_state *control) {
    STD_PRINTF("Recording: %s\n", control != STD_NULL &&
        control->execution_context.device != STD_NULL &&
        ((vm_composition_live_machine *)control->execution_context.device)->debug->
            connect.recordFile ? "Yes" : "No");
    STD_PRINTF("Running:   %s\n", control != STD_NULL && STD_ATOMIC_LOAD(&control->flagRun) ?
        "Yes" : "No");
}

C_INT vm_composition_control_is_running(const vm_composition_control_state *control)
{
    return control != STD_NULL && STD_ATOMIC_LOAD(&control->flagRun) &&
        !STD_ATOMIC_LOAD(&control->paused);
}

C_INT vm_composition_control_get_flip(const vm_composition_control_state *control)
{
    return control != STD_NULL && STD_ATOMIC_LOAD(&control->flagFlip);
}
