/* Copyright 2012-2014 Neko. */

/*
 * DEVICE provides the device interface to drive the device thread
 * and access the virtual devices.
 */

#include "../utils.h"

#include "vdebug.h"
#include "vmachine.h"

#include "device.h"

t_device device;

/* Starts device thread */
void deviceStart() {
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
    vdebugInit();
    vmachineInit();
}

/* Finalizes devices */
void deviceFinal() {
    vdebugFinal();
    vmachineFinal();
}

void devicePrintStatus() {
    PRINTF("Recording: %s\n", vdebug.connect.recordFile ? "Yes" : "No");
    PRINTF("Running:   %s\n", device.flagRun  ? "Yes" : "No");
}
