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
#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/platform/sleep.h"
#include "vm/composition_control.h"
#include "vm/composition_live_machine.h"

static void device_execution_context_reset(void)
{
    vdebugReset();
    vmachineReset();
}

static const nxvm_execution_context_callbacks device_execution_callbacks = {
    device_execution_context_reset,
    vdebugRefresh,
    vmachineRefresh
};

/* Starts device thread */
void vm_composition_control_start(vm_composition_control_state *control) {
    if (control == NULL) return;
    nxvm_execution_context_enter(&control->execution_context);
    control->flagRun = True;
    control->flagFlip = !control->flagFlip;
    while (control->flagRun) {
        if (control->flagReset) {
            nxvm_execution_context_reset(&control->execution_context);
            control->flagReset = False;
        }
        if (control->pauseRequested) {
            control->paused = True;
        }
        while (control->flagRun && control->paused) {
            nxvm_execution_context_run_command_boundary(&control->execution_context);
            core_platform_sleep_milliseconds(1u);
        }
        if (!control->flagRun) break;
        nxvm_execution_context_run_command_boundary(&control->execution_context);
        nxvm_execution_context_debug_refresh(&control->execution_context);
        if (control->pauseRequested) continue;
        if (!control->flagRun) {
            break;
        }
        nxvm_execution_context_machine_refresh(&control->execution_context);
        if (vcpuConsumeResetRequest()) {
            vm_composition_control_reset(control);
        }
        if (vcpuConsumeStopRequest()) {
            vm_composition_control_stop(control);
        }
        if (control->stepRequested) {
            control->stepRequested = False;
            vm_composition_control_request_pause(control, VM_COMPOSITION_PAUSE_STEP);
        }
    }
    nxvm_execution_context_leave(&control->execution_context);
}

/* Issues resetting signal to device thread */
void vm_composition_control_reset(vm_composition_control_state *control) {
    if (control == NULL) return;
    if (control->flagRun) {
        control->flagReset = True;
    } else {
        nxvm_execution_context_reset(&control->execution_context);
        control->flagReset = False;
    }
}

/* Issues stopping signal to device thread */
void vm_composition_control_stop(vm_composition_control_state *control)  {
    if (control == NULL) return;
    control->flagRun = False;
    control->paused = False;
    control->pauseRequested = False;
}

void vm_composition_control_request_pause(vm_composition_control_state *control,
    vm_composition_pause_reason reason)
{
    if (control == NULL) return;
    if (!control->flagRun) {
        control->paused = True;
        control->pauseReason = reason;
        return;
    }
    control->pauseRequested = True;
    control->pauseReason = reason;
}

int vm_composition_control_wait_for_pause(vm_composition_control_state *control,
    unsigned milliseconds)
{
    unsigned waited = 0u;

    if (control == NULL) return False;
    while (control->flagRun && !control->paused &&
           waited < milliseconds) {
        core_platform_sleep_milliseconds(1u);
        ++waited;
    }
    return control->paused;
}

int vm_composition_control_is_paused(const vm_composition_control_state *control)
{
    return control != NULL && control->paused;
}

vm_composition_pause_reason vm_composition_control_get_pause_reason(
    const vm_composition_control_state *control)
{
    return control == NULL ? VM_COMPOSITION_PAUSE_NONE : control->pauseReason;
}

void vm_composition_control_continue(vm_composition_control_state *control)
{
    if (control == NULL) return;
    control->pauseRequested = False;
    control->paused = False;
    control->stepRequested = False;
    control->pauseReason = VM_COMPOSITION_PAUSE_NONE;
}

int vm_composition_control_step(vm_composition_control_state *control)
{
    if (control == NULL || !control->paused) return False;
    control->pauseRequested = False;
    control->paused = False;
    control->stepRequested = True;
    control->pauseReason = VM_COMPOSITION_PAUSE_NONE;
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
    nxvm_execution_context_initialize(&control->execution_context);
    nxvm_execution_context_bind_machine_state(
        &control->execution_context, machine->cpu, machine->ram, machine->port,
        control);
    nxvm_execution_context_bind_callbacks(
        &control->execution_context, &device_execution_callbacks);
    nxvm_execution_context_enter(&control->execution_context);
    vdebugInit();
    vmachineInit();
}

/* Finalizes devices */
void vm_composition_control_finalize(vm_composition_control_state *control,
    vm_composition_live_machine *machine) {
    if (control == NULL || machine == NULL) return;
    nxvm_execution_context_leave(&control->execution_context);
    vdebugFinal();
    vmachineFinal();
    (void)machine;
}

void vm_composition_control_print_status(const vm_composition_control_state *control) {
    PRINTF("Recording: %s\n", vdebug.connect.recordFile ? "Yes" : "No");
    PRINTF("Running:   %s\n", control != NULL && control->flagRun ? "Yes" : "No");
}

int vm_composition_control_is_running(const vm_composition_control_state *control)
{
    return control != NULL && control->flagRun && !control->paused;
}

int vm_composition_control_get_flip(const vm_composition_control_state *control)
{
    return control != NULL && control->flagFlip;
}
