/* Copyright 2012-2014 Neko. */

/* VHDC is Hard Disk Driver Controller, not yet implemented. */

#include "vmachine.h"
#include "vbios.h"
#include "vhdc.h"

static void init() {
	vbiosAddInt(VHDC_INT_SOFT_HDD_13, 0x13);
}

static void reset() {}

static void refresh() {}

static void final() {}

void vhdcRegister() {
	vmachine.deviceTable[VMACHINE_DEVICE_INIT][vmachine.numDevices] = (t_faddrcc) init;
	vmachine.deviceTable[VMACHINE_DEVICE_RESET][vmachine.numDevices] = (t_faddrcc) reset;
	vmachine.deviceTable[VMACHINE_DEVICE_REFRESH][vmachine.numDevices] = (t_faddrcc) refresh;
	vmachine.deviceTable[VMACHINE_DEVICE_FINAL][vmachine.numDevices] = (t_faddrcc) final;
	vmachine.numDevices++;
}
