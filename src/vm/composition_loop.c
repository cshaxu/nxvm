/* Copyright 2012-2014 Neko. */

/*
 * DEVICE provides the device interface to drive the device thread
 * and access the virtual devices.
 */

#include "core/product/utils.h"

#include "vm/machine/debug.h"
#include "vm/composition.h"
#include "core/product/runtime/execution_context.h"
#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/platform/sleep.h"
#include "vm/composition_control.h"
#include "vm/composition_machine.h"
#include "vm/composition_live_machine.h"
#include "vm/composition_display.h"

static void device_execution_context_reset(void *device)
{
    vm_composition_live_machine *machine =
        (vm_composition_live_machine *)device;
    vm_machine_debug_reset(machine->debug);
    if (core_machine_reset(machine->core_machine) != NXVM_CORE_STATUS_OK) {
        vm_composition_control_stop(machine->control);
    }
}

static void device_execution_context_debug_refresh(void *device)
{
    vm_composition_live_machine *machine =
        (vm_composition_live_machine *)device;
    vm_machine_debug_refresh(machine == NULL ? NULL : machine->debug);
}

static const nxvm_execution_context_callbacks device_execution_callbacks = {
    device_execution_context_reset,
    device_execution_context_debug_refresh
};

/* Starts device thread */
void vm_composition_control_start(vm_composition_control_state *control) {
    core_machine_run_budget budget = {1u, 0u};
    core_machine_run_result result;
    vm_composition_live_machine *machine;

    if (control == NULL) return;
    machine = (vm_composition_live_machine *)control->execution_context.device;
    if (machine == NULL || machine->core_machine == NULL) return;
    nxvm_execution_context_activate(&control->execution_context);
    atomic_store(&control->flagRun, True);
    atomic_store(&control->flagFlip, !atomic_load(&control->flagFlip));
    while (atomic_load(&control->flagRun)) {
        if (atomic_exchange(&control->flagReset, False)) {
            nxvm_execution_context_reset(&control->execution_context);
        }
        if (atomic_load(&control->pauseRequested)) {
            atomic_store(&control->paused, True);
        }
        while (atomic_load(&control->flagRun) && atomic_load(&control->paused)) {
            nxvm_execution_context_run_command_boundary(&control->execution_context);
            core_platform_sleep_milliseconds(1u);
        }
        if (!atomic_load(&control->flagRun)) break;
        nxvm_execution_context_run_command_boundary(&control->execution_context);
        nxvm_execution_context_debug_refresh(&control->execution_context);
        if (atomic_load(&control->pauseRequested)) continue;
        if (!atomic_load(&control->flagRun)) {
            break;
        }
        if (core_machine_run(machine->core_machine, budget, &result) !=
            NXVM_CORE_STATUS_OK) {
            vm_composition_control_stop(control);
            continue;
        }
        vm_composition_publish_display(machine, False);
        if (result.reason == CORE_MACHINE_STOP_RESET_REQUESTED) {
            vm_composition_control_reset(control);
        }
        if (result.reason == CORE_MACHINE_STOP_REQUESTED) {
            vm_composition_control_stop(control);
        }
        if (result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
            core_platform_sleep_milliseconds(1u);
        }
        if (atomic_exchange(&control->stepRequested, False)) {
            vm_composition_control_request_pause(control, VM_COMPOSITION_PAUSE_STEP);
        }
    }
    nxvm_execution_context_deactivate(&control->execution_context);
}

/* Issues resetting signal to device thread */
void vm_composition_control_reset(vm_composition_control_state *control) {
    if (control == NULL) return;
    if (atomic_load(&control->flagRun)) {
        atomic_store(&control->flagReset, True);
    } else {
        nxvm_execution_context_reset(&control->execution_context);
        atomic_store(&control->flagReset, False);
    }
}

/* Issues stopping signal to device thread */
void vm_composition_control_stop(vm_composition_control_state *control)  {
    vm_composition_live_machine *machine;

    if (control == NULL) return;
    machine = (vm_composition_live_machine *)control->execution_context.device;
    if (machine != NULL && machine->core_machine != NULL) {
        core_machine_request_stop(machine->core_machine);
    }
    atomic_store(&control->flagRun, False);
    atomic_store(&control->paused, False);
    atomic_store(&control->pauseRequested, False);
}

void vm_composition_control_request_pause(vm_composition_control_state *control,
    vm_composition_pause_reason reason)
{
    if (control == NULL) return;
    if (!atomic_load(&control->flagRun)) {
        atomic_store(&control->paused, True);
        atomic_store(&control->pauseReason, reason);
        return;
    }
    atomic_store(&control->pauseRequested, True);
    atomic_store(&control->pauseReason, reason);
}

int vm_composition_control_wait_for_pause(vm_composition_control_state *control,
    unsigned milliseconds)
{
    unsigned waited = 0u;

    if (control == NULL) return False;
    while (atomic_load(&control->flagRun) && !atomic_load(&control->paused) &&
           waited < milliseconds) {
        core_platform_sleep_milliseconds(1u);
        ++waited;
    }
    return atomic_load(&control->paused);
}

int vm_composition_control_is_paused(const vm_composition_control_state *control)
{
    return control != NULL && atomic_load(&control->paused);
}

vm_composition_pause_reason vm_composition_control_get_pause_reason(
    const vm_composition_control_state *control)
{
    return control == NULL ? VM_COMPOSITION_PAUSE_NONE :
        (vm_composition_pause_reason)atomic_load(&control->pauseReason);
}

void vm_composition_control_continue(vm_composition_control_state *control)
{
    if (control == NULL) return;
    atomic_store(&control->pauseRequested, False);
    atomic_store(&control->paused, False);
    atomic_store(&control->stepRequested, False);
    atomic_store(&control->pauseReason, VM_COMPOSITION_PAUSE_NONE);
}

int vm_composition_control_step(vm_composition_control_state *control)
{
    if (control == NULL || !atomic_load(&control->paused)) return False;
    atomic_store(&control->pauseRequested, False);
    atomic_store(&control->paused, False);
    atomic_store(&control->stepRequested, True);
    atomic_store(&control->pauseReason, VM_COMPOSITION_PAUSE_NONE);
    return True;
}

void vm_composition_control_bind_command_boundary(
    vm_composition_control_state *control,
    void (*callback)(void *opaque), void *opaque)
{
    nxvm_execution_context_bind_command_boundary(
        control == NULL ? NULL : &control->execution_context, callback, opaque);
}

/* Initializes devices */
void vm_composition_control_initialize(vm_composition_control_state *control,
    vm_composition_live_machine *machine) {

    if (control == NULL || machine == NULL) return;
    MEMSET((void *)(control), Zero8, sizeof(*control));
    atomic_init(&control->flagFlip, False);
    atomic_init(&control->flagRun, False);
    atomic_init(&control->flagReset, False);
    atomic_init(&control->pauseRequested, False);
    atomic_init(&control->paused, False);
    atomic_init(&control->stepRequested, False);
    atomic_init(&control->pauseReason, VM_COMPOSITION_PAUSE_NONE);
    nxvm_execution_context_initialize(&control->execution_context);
    nxvm_execution_context_bind_machine_state(
        &control->execution_context, machine->cpu, machine->ram, machine->port,
        machine);
    nxvm_execution_context_bind_callbacks(
        &control->execution_context, &device_execution_callbacks);
    nxvm_execution_context_activate(&control->execution_context);
    vm_machine_debug_initialize(machine->debug, machine->cpu, machine->cpuins);
    vmachineInit(machine);
    if (!vm_composition_bind_execution_provider(machine)) {
        vm_composition_control_stop(control);
    }
}

/* Finalizes devices */
void vm_composition_control_finalize(vm_composition_control_state *control,
    vm_composition_live_machine *machine) {
    if (control == NULL || machine == NULL) return;
    nxvm_execution_context_deactivate(&control->execution_context);
    vm_machine_debug_finalize(machine->debug);
    vmachineFinal(machine);
}

void vm_composition_control_print_status(const vm_composition_control_state *control) {
    PRINTF("Recording: %s\n", control != NULL &&
        control->execution_context.device != NULL &&
        ((vm_composition_live_machine *)control->execution_context.device)->debug->
            connect.recordFile ? "Yes" : "No");
    PRINTF("Running:   %s\n", control != NULL && atomic_load(&control->flagRun) ?
        "Yes" : "No");
}

int vm_composition_control_is_running(const vm_composition_control_state *control)
{
    return control != NULL && atomic_load(&control->flagRun) &&
        !atomic_load(&control->paused);
}

int vm_composition_control_get_flip(const vm_composition_control_state *control)
{
    return control != NULL && atomic_load(&control->flagFlip);
}
