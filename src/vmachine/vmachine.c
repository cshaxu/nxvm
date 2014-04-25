/* Copyright 2012-2014 Neko. */

/*
 * VMACHINE is the hub that assembles all devices and
 * drives the kernel thread.
 */

#include "vapi.h"

#include "debug/record.h"
#include "debug/debug.h"
#include "vport.h"
#include "vram.h"
#include "vcpu.h"
#include "vpic.h"
#include "vcmos.h"
#include "vdma.h"
#include "vfdc.h"
#include "vfdd.h"
#include "vhdd.h"
#include "vpit.h"
#include "vkbc.h"
//#include "vkeyb.h"
#include "vvadp.h"
//#include "vdisp.h"

#include "vmachine.h"

#ifdef VMACHINE_DEBUG
#include "bios/qdbios.h"
#endif

t_machine vmachine;

/* Registers all devices */
static void doRegister() {
	recordRegister();
	debugRegister();
	vportRegister();
	vramRegister();
	vcpuRegister();
	vpicRegister();
	vpitRegister();
	vcmosRegister();
	vdmaRegister();
	vfdcRegister();
	vfddRegister();
	vhddRegister();
	vkbcRegister();
	//vkeybRegister();
	vvadpRegister();
	//vdispRegister();
	qdbiosRegister();
}

/* Initializes all devices, allocates space */
static void doInit() {
	t_nubitcc i;
	for (i = 0;i < vmachine.numDevices;++i) {
		ExecFun(vmachine.deviceTable[VMACHINE_DEVICE_INIT][i]);
	}
}

/* Resets all devices to initial values */
static void doReset() {
	t_nubitcc i;
	for (i = 0;i < vmachine.numDevices;++i) {
		ExecFun(vmachine.deviceTable[VMACHINE_DEVICE_RESET][i]);
	}
	vmachine.flagreset = 0;
}

/* Executes all devices in one loop */
static void doRefresh() {
	t_nubitcc i;
	for (i = 0;i < vmachine.numDevices;++i) {
		ExecFun(vmachine.deviceTable[VMACHINE_DEVICE_REFRESH][vmachine.numDevices - i - 1]);
	}
}

/* Finalize all devices, deallocates space */
static void doFinal() {
	t_nubitcc i;
	for (i = 0;i < vmachine.numDevices;++i) {
		ExecFun(vmachine.deviceTable[VMACHINE_DEVICE_FINAL][vmachine.numDevices - i - 1]);
	}
}

/* CallBack interface that drives the virtual machines as kernel thread */
void vapiCallBackMachineRun() {
	while (vmachine.flagrun) {
		if (vmachine.flagreset) {
			doReset();
		}
		doRefresh();
	}
}

/* Tells if virtual machine is running */
t_bool vapiCallBackMachineGetFlagRun() {
	return vmachine.flagrun;
}

/* CallBack interface that resets the virtual machine */
void vapiCallBackMachineReset() {
	vmachineReset();
}

/* CallBack interface that stops the virtual machine */
void vapiCallBackMachineStop() {
	vmachineStop();
}

/* Starts virtual machine from initial state */
void vmachineStart() {
	doReset();
	vmachineResume();
}

/* Issues resetting signal to virtual machine */
void vmachineReset() {
	if (vmachine.flagrun) {
		vmachine.flagreset = 1;
	} else {
		doReset();
	}
}

/* Starts virtual machine from last state */
void vmachineResume() {
	if (vmachine.flagrun) {
		return;
	}
	vmachine.flagrun = 1;
	vapiStartMachine();
}

/* Issues stopping signal to virtual machine */
void vmachineStop()  {
	vmachine.flagrun = 0;
}

/* Initializes virtual machine */
void vmachineInit() {
	memset(&vmachine, 0x00, sizeof(t_machine));
	doRegister();
	doInit();
	vmachine.flagmode = 1;
}

/* Finalizes virtual machine */
void vmachineFinal() {
	doFinal();
}
