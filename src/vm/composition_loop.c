/* Copyright 2012-2014 Neko. */

/*
 * DEVICE provides the device interface to drive the device thread
 * and access the virtual devices.
 */

#include "core/product/utils.h"

#include "vm/machine/vdebug.h"
#include "vm/composition.h"
#include "core/product/runtime/execution_context.h"
#include "core/machine/cpu.h"
#include "core/machine/vram.h"
#include "core/machine/vport.h"
#include "vm/composition_control.h"
#include "vm/composition_live_machine.h"

typedef struct vm_composition_control_state {
    int flagFlip;
    int flagRun;
    int flagReset;
} vm_composition_control_state;

static vm_composition_control_state vmCompositionControl;
static nxvm_execution_context device_execution_context;
static int vmCompositionControlOwnsLiveMachine;

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
void vm_composition_control_start(void) {
    nxvm_execution_context_enter(&device_execution_context);
    vmCompositionControl.flagRun = True;
    vmCompositionControl.flagFlip = !vmCompositionControl.flagFlip;
    while (vmCompositionControl.flagRun) {
        if (vmCompositionControl.flagReset) {
            nxvm_execution_context_reset(&device_execution_context);
            vmCompositionControl.flagReset = False;
        }
        nxvm_execution_context_run_command_boundary(&device_execution_context);
        nxvm_execution_context_debug_refresh(&device_execution_context);
        if (!vmCompositionControl.flagRun) {
            break;
        }
        nxvm_execution_context_machine_refresh(&device_execution_context);
        if (vcpuConsumeResetRequest()) {
            vm_composition_control_reset();
        }
        if (vcpuConsumeStopRequest()) {
            vm_composition_control_stop();
        }
    }
    nxvm_execution_context_leave(&device_execution_context);
}

/* Issues resetting signal to device thread */
void vm_composition_control_reset(void) {
    if (vmCompositionControl.flagRun) {
        vmCompositionControl.flagReset = True;
    } else {
        nxvm_execution_context_reset(&device_execution_context);
        vmCompositionControl.flagReset = False;
    }
}

/* Issues stopping signal to device thread */
void vm_composition_control_stop(void)  {
    vmCompositionControl.flagRun = False;
}

void vm_composition_control_bind_command_boundary(
    void (*callback)(void *opaque), void *opaque)
{
    nxvm_execution_context_bind_command_boundary(
        &device_execution_context, callback, opaque);
}

/* Initializes devices */
void vm_composition_control_initialize(void) {
    const vm_composition_live_machine *machine;

    MEMSET((void *)(&vmCompositionControl), Zero8,
        sizeof(vmCompositionControl));
    vmCompositionControlOwnsLiveMachine = 0;
    if (vm_composition_live_machine_current() == NULL) {
        vm_composition_live_machine_bind(&vram, &vport);
        vmCompositionControlOwnsLiveMachine = 1;
    }
    machine = vm_composition_live_machine_current();
    nxvm_execution_context_initialize(&device_execution_context);
    nxvm_execution_context_bind_machine_state(
        &device_execution_context, machine->cpu, machine->ram, machine->port,
        &vmCompositionControl);
    nxvm_execution_context_bind_callbacks(
        &device_execution_context, &device_execution_callbacks);
    nxvm_execution_context_enter(&device_execution_context);
    vdebugInit();
    vmachineInit();
}

/* Finalizes devices */
void vm_composition_control_finalize(void) {
    nxvm_execution_context_leave(&device_execution_context);
    vdebugFinal();
    vmachineFinal();
    if (vmCompositionControlOwnsLiveMachine) {
        vm_composition_live_machine_clear();
        vmCompositionControlOwnsLiveMachine = 0;
    }
}

void vm_composition_control_print_status(void) {
    PRINTF("Recording: %s\n", vdebug.connect.recordFile ? "Yes" : "No");
    PRINTF("Running:   %s\n", vmCompositionControl.flagRun ? "Yes" : "No");
}

int vm_composition_control_is_running(void)
{
    return vmCompositionControl.flagRun;
}

int vm_composition_control_get_flip(void)
{
    return vmCompositionControl.flagFlip;
}
