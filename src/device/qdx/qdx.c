/* Copyright 2012-2014 Neko. */

/* QDX implements quick and dirty instruction loader */

#include "../vmachine.h"

#include "qdcga.h"
#include "qdkeyb.h"
#include "qddisk.h"

#include "qdx.h"

t_faddrcc qdxTable[0x100];

void qdxExecInt(t_nubit8 intid) {
	if (qdxTable[intid]) {
		ExecFun(qdxTable[intid]);
	}
}

static void init() {}

/* Loads bios to ram */
static void reset() {
	t_nubit16 i;
	for (i = 0x0000;i < 0x0100;++i) {
		qdxTable[i] = (t_faddrcc) NULL;
	}
	qdkeybReset();
	qdcgaReset();
	qddiskReset();
}

static void refresh() {}

static void final() {}

void qdxRegister() {
	vmachine.deviceTable[VMACHINE_DEVICE_INIT][vmachine.numDevices] = (t_faddrcc) init;
	vmachine.deviceTable[VMACHINE_DEVICE_RESET][vmachine.numDevices] = (t_faddrcc) reset;
	vmachine.deviceTable[VMACHINE_DEVICE_REFRESH][vmachine.numDevices] = (t_faddrcc) refresh;
	vmachine.deviceTable[VMACHINE_DEVICE_FINAL][vmachine.numDevices] = (t_faddrcc) final;
	vmachine.numDevices++;
}
