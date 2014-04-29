/* Copyright 2012-2014 Neko. */

/* VMACHINE is the hub that assembles all devices. */

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

/* Registers all devices */
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

/* Initializes all devices, allocates space */
void vmachineInit() {
	t_nubitcc i;
	memset(&vmachine, 0x00, sizeof(t_machine));
	doRegister();
	for (i = 0;i < vmachine.numDevices;++i) {
		ExecFun(vmachine.deviceTable[VMACHINE_DEVICE_INIT][i]);
	}
}

/* Resets all devices to initial values */
void vmachineReset() {
	t_nubitcc i;
	for (i = 0;i < vmachine.numDevices;++i) {
		ExecFun(vmachine.deviceTable[VMACHINE_DEVICE_RESET][i]);
	}
}

/* Executes all devices in one loop */
void vmachineRefresh() {
	t_nubitcc i;
	for (i = 0;i < vmachine.numDevices;++i) {
		ExecFun(vmachine.deviceTable[VMACHINE_DEVICE_REFRESH][vmachine.numDevices - i - 1]);
	}
}

/* Finalize all devices, deallocates space */
void vmachineFinal() {
	t_nubitcc i;
	for (i = 0;i < vmachine.numDevices;++i) {
		ExecFun(vmachine.deviceTable[VMACHINE_DEVICE_FINAL][vmachine.numDevices - i - 1]);
	}
}
