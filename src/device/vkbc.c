/* Copyright 2012-2014 Neko. */

/* VKBC is Keyboard Controller: Intel 8042 not implemented yet. */

#include "vmachine.h"
#include "vport.h"
#include "vbios.h"
#include "vkbc.h"

void io_read_0064() {vport.iobyte = 0x10;}

static void init() {
	vport.in[0x0064] = (t_faddrcc) io_read_0064;
    vbiosAddInt("qdx 09\niret", 0x09);
    vbiosAddInt("qdx 16\niret", 0x16);
}

static void reset() {}

static void refresh() {}

static void final() {}

void vkbcRegister() {
	vmachine.deviceTable[VMACHINE_DEVICE_INIT][vmachine.numDevices] = (t_faddrcc) init;
	vmachine.deviceTable[VMACHINE_DEVICE_RESET][vmachine.numDevices] = (t_faddrcc) reset;
	vmachine.deviceTable[VMACHINE_DEVICE_REFRESH][vmachine.numDevices] = (t_faddrcc) refresh;
	vmachine.deviceTable[VMACHINE_DEVICE_FINAL][vmachine.numDevices] = (t_faddrcc) final;
	vmachine.numDevices++;
}
