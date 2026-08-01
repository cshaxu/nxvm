/* Copyright 2012-2014 Neko. */

/*
 * DEVICE provides the device interface to drive the device thread
 * and access the virtual devices.
 */

#include "nxvm-baseline/utils.h"

#include "nxvm-baseline/device/vdebug.h"
#include "machine/vm/vmachine.h"
#include "machine/vm/execution_context.h"
#include "machine/core/vcpu.h"
#include "machine/core/vram.h"
#include "machine/core/vport.h"

#include "device.h"

t_device device;
static nxvm_execution_context device_execution_context;

/* Starts device thread */
void deviceStart() {
    nxvm_execution_context_enter(&device_execution_context);
    device.flagRun = True;
    device.flagFlip = !device.flagFlip;
    while (device.flagRun) {
        if (device.flagReset) {
            vdebugReset();
            vmachineReset();
            device.flagReset = False;
        }
        vdebugRefresh();
        if (!device.flagRun) {
            break;
        }
        vmachineRefresh();
    }
    nxvm_execution_context_leave(&device_execution_context);
}

/* Issues resetting signal to device thread */
void deviceReset() {
    if (device.flagRun) {
        device.flagReset = True;
    } else {
        vdebugReset();
        vmachineReset();
        device.flagReset = False;
    }
}

/* Issues stopping signal to device thread */
void deviceStop()  {
    device.flagRun = False;
}

/* Initializes devices */
void deviceInit() {
    MEMSET((void *)(&device), Zero8, sizeof(t_device));
    nxvm_execution_context_initialize(&device_execution_context);
    nxvm_execution_context_bind_machine_state(
        &device_execution_context, &vcpu, &vram, &vport, &device);
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
