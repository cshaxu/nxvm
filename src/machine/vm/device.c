/* Copyright 2012-2014 Neko. */

/*
 * DEVICE provides the device interface to drive the device thread
 * and access the virtual devices.
 */

#include "nxvm-baseline/utils.h"

#include "vm/machine/vdebug.h"
#include "machine/vm/vmachine.h"
#include "machine/vm/execution_context.h"
#include "core/machine/vcpu.h"
#include "core/machine/vram.h"
#include "core/machine/vport.h"

#include "device.h"

t_device device;
static nxvm_execution_context device_execution_context;

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
void deviceStart() {
    nxvm_execution_context_enter(&device_execution_context);
    device.flagRun = True;
    device.flagFlip = !device.flagFlip;
    while (device.flagRun) {
        if (device.flagReset) {
            nxvm_execution_context_reset(&device_execution_context);
            device.flagReset = False;
        }
        nxvm_execution_context_run_command_boundary(&device_execution_context);
        nxvm_execution_context_debug_refresh(&device_execution_context);
        if (!device.flagRun) {
            break;
        }
        nxvm_execution_context_machine_refresh(&device_execution_context);
    }
    nxvm_execution_context_leave(&device_execution_context);
}

/* Issues resetting signal to device thread */
void deviceReset() {
    if (device.flagRun) {
        device.flagReset = True;
    } else {
        nxvm_execution_context_reset(&device_execution_context);
        device.flagReset = False;
    }
}

/* Issues stopping signal to device thread */
void deviceStop()  {
    device.flagRun = False;
}

void deviceConnectBindCommandBoundary(
    void (*callback)(void *opaque), void *opaque)
{
    nxvm_execution_context_bind_command_boundary(
        &device_execution_context, callback, opaque);
}

/* Initializes devices */
void deviceInit() {
    MEMSET((void *)(&device), Zero8, sizeof(t_device));
    nxvm_execution_context_initialize(&device_execution_context);
    nxvm_execution_context_bind_machine_state(
        &device_execution_context, &vcpu, &vram, &vport, &device);
    nxvm_execution_context_bind_callbacks(
        &device_execution_context, &device_execution_callbacks);
    nxvm_execution_context_enter(&device_execution_context);
    vdebugInit();
    vmachineInit();
}

/* Finalizes devices */
void deviceFinal() {
    nxvm_execution_context_leave(&device_execution_context);
    vdebugFinal();
    vmachineFinal();
}

void devicePrintStatus() {
    PRINTF("Recording: %s\n", vdebug.connect.recordFile ? "Yes" : "No");
    PRINTF("Running:   %s\n", device.flagRun  ? "Yes" : "No");
}
