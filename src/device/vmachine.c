/* Copyright 2012-2014 Neko. */

/* VMACHINE is the hub that assembles all devices. */

#include "../utils.h"

#include "vdebug.h"
#include "vport.h"
#include "vram.h"
#include "vcpu.h"
#include "vbios.h"
#include "vpic.h"
#include "vpit.h"
#include "vcmos.h"
#include "vdma.h"
#include "vfdc.h"
#include "vfdd.h"
#include "vhdc.h"
#include "vhdd.h"
#include "vkbc.h"
#include "vvadp.h"
#include "qdx/qdx.h"

#include "vmachine.h"

t_machine vmachine;

/* Ask all devices to register */
static void doRegister() {
    vdebugRegister();
    vportRegister();
    vramRegister();
    vcpuRegister();
    vbiosRegister();
    vpicRegister();
    vpitRegister();
    vcmosRegister();
    vdmaRegister();
    vfdcRegister();
    vfddRegister();
    vhdcRegister();
    vhddRegister();
    vkbcRegister();
    vvadpRegister();
    qdxRegister();
}

/* Device call this function to register itself */
void vmachineAddDevice(t_faddrcc fpInit, t_faddrcc fpReset, t_faddrcc fpRefresh,
                       t_faddrcc fpFinal) {
    vmachine.deviceTable[VMACHINE_DEVICE_INIT][vmachine.numDevices] = fpInit;
    vmachine.deviceTable[VMACHINE_DEVICE_RESET][vmachine.numDevices] = fpReset;
    vmachine.deviceTable[VMACHINE_DEVICE_REFRESH][vmachine.numDevices] = fpRefresh;
    vmachine.deviceTable[VMACHINE_DEVICE_FINAL][vmachine.numDevices] = fpFinal;
    vmachine.numDevices++;
}

/* Initializes all devices, allocates space */
void vmachineInit() {
    t_nubitcc i;
    MEMSET(&vmachine, Zero8, sizeof(t_machine));
    doRegister();
    for (i = 0; i < vmachine.numDevices; ++i) {
        ExecFun(vmachine.deviceTable[VMACHINE_DEVICE_INIT][i]);
    }
}

/* Resets all devices to initial values */
void vmachineReset() {
    t_nubitcc i;
    for (i = 0; i < vmachine.numDevices; ++i) {
        ExecFun(vmachine.deviceTable[VMACHINE_DEVICE_RESET][i]);
    }
}

/* Executes all devices in one loop */
void vmachineRefresh() {
    t_nubitcc i;
    for (i = 0; i < vmachine.numDevices; ++i) {
        ExecFun(vmachine.deviceTable[VMACHINE_DEVICE_REFRESH][vmachine.numDevices - i - 1]);
    }
}

/* Finalize all devices, deallocates space */
void vmachineFinal() {
    t_nubitcc i;
    for (i = 0; i < vmachine.numDevices; ++i) {
        ExecFun(vmachine.deviceTable[VMACHINE_DEVICE_FINAL][vmachine.numDevices - i - 1]);
    }
}

/* Print machine info */
void devicePrintMachine() {
    PRINTF("Machine:           %s\n", NXVM_DEVICE_MACHINE);
    PRINTF("CPU:               %s\n", NXVM_DEVICE_CPU);
    PRINTF("RAM Size:          %d MB\n", vram.size >> 20);
    PRINTF("Floppy Disk Drive: %s, %.2f MB, %s\n", NXVM_DEVICE_FDD,
           vfddGetImageSize * 1. / VFDD_BYTE_PER_MB,
           vfdd.flagDiskExist ? "inserted" : "not inserted");
    PRINTF("Hard Disk Drive:   %d cylinders, %.2f MB, %s\n", vhdd.ncyl,
           vhddGetImageSize * 1. / VHDD_BYTE_PER_MB,
           vhdd.flagDiskExist ? "connected" : "disconnected");
}
