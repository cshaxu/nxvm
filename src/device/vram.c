/* Copyright 2012-2014 Neko. */

/* VRAM is the random accessing memory module.  */

#include "vport.h"
#include "vmachine.h"
#include "vram.h"

t_ram vram;

/* Allocates memory for virtual machine ram */
void vramAlloc(t_nubitcc newsize) {
	if (newsize) {
		vram.size = newsize;
		if (vram.base) free((void *)vram.base);
		vram.base = (t_vaddrcc)malloc(vram.size);
		memset((void *)vram.base, 0x00, vram.size);
	}
}

static void io_read_0092() {
	vport.iobyte = vram.flaga20 ? 0x02 : 0x00;
}

static void io_write_0092() {
	vram.flaga20 = !!GetBit(vport.iobyte, 0x02);
}

static void init() {
	memset(&vram, 0x00, sizeof(t_ram));
	vport.in[0x0092] = (t_faddrcc) io_read_0092;
	vport.out[0x0092] = (t_faddrcc) io_write_0092;
	/* 16 MB */
	vramAlloc(1 << 24);
}

static void reset() {
	memset((void *)vram.base, 0x00, vram.size);
	vram.flaga20 = 0;
}

static void refresh() {}

static void final() {
	if (vram.base) {
		free((void *)(vram.base));
	}
}

void vramRegister() {
	vmachine.deviceTable[VMACHINE_DEVICE_INIT][vmachine.numDevices] = (t_faddrcc) init;
	vmachine.deviceTable[VMACHINE_DEVICE_RESET][vmachine.numDevices] = (t_faddrcc) reset;
	vmachine.deviceTable[VMACHINE_DEVICE_REFRESH][vmachine.numDevices] = (t_faddrcc) refresh;
	vmachine.deviceTable[VMACHINE_DEVICE_FINAL][vmachine.numDevices] = (t_faddrcc) final;
	vmachine.numDevices++;
}
