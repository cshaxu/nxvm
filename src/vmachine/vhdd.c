/* Copyright 2012-2014 Neko. */

/* VHDD implements Hard Disk Drive: 10 MBytes, cyl = 20, head = 16, sector = 63 */

#include "vmachine.h"
#include "vdma.h"
#include "vhdd.h"

t_hdd vhdd;

#define IsTrackEnd (vhdd.sector >= (vhdd.nsector + 1))
#define IsCylEnd   (vhdd.head == (vhdd.nhead - 1) && IsTrackEnd)

void vhddTransRead() {
	if (IsCylEnd) {
		return;
	}
	vlatch.byte = d_nubit8(vhdd.curr);
	vhdd.curr++;
	vhdd.count++;
	if (!(vhdd.count % vhdd.nbyte)) {
		vhdd.sector++;
		if (IsTrackEnd) {
			vhdd.sector = 0x01;
			vhdd.head++;
		}
		vhddSetPointer;
	}
}

void vhddTransWrite() {
	if (IsCylEnd) {
		return;
	}
	d_nubit8(vhdd.curr) = vlatch.byte;
	vhdd.curr++;
	vhdd.count++;
	if (!(vhdd.count % vhdd.nbyte)) {
		vhdd.sector++;
		if (IsTrackEnd) {
			vhdd.sector = 0x01;
			vhdd.head++;
		}
		vhddSetPointer;
	}
}

void vhddFormatTrack(t_nubit8 fillbyte) {
	t_nubit8 i;
	if (vhdd.cyl >= vhdd.ncyl) {
		return;
	}
	for (i = 0;i < vhdd.nhead;++i) {
		vhdd.head = i;
		vhdd.sector = 0x01;
		vhddSetPointer;
		memset((void *) vhdd.curr, fillbyte, vhdd.nsector * vhdd.nbyte);
		vhdd.sector = vhdd.nsector;
	}
}

void vhddAlloc() {
	if (vhdd.base) {
		free((void *) vhdd.base);
	}
	vhdd.base = (t_vaddrcc) malloc(vhddGetImageSize);
	memset((void *) vhdd.base, 0x00, vhddGetImageSize);
}

static void init() {
	memset(&vhdd, 0x00, sizeof(t_hdd));
	vhdd.ncyl    = 0;
	vhdd.nhead   = 16;
	vhdd.nsector = 63;
	vhdd.nbyte   = 512;
	vhdd.base    = (t_vaddrcc) NULL;
}

static void reset() {}

static void refresh() {}

static void final() {
	if (vhdd.base) {
		free((void *) vhdd.base);
	}
	vhdd.base = (t_vaddrcc) NULL;
}

void vhddRegister() {
	vmachine.deviceTable[VMACHINE_DEVICE_INIT][vmachine.numDevices] = (t_faddrcc) init;
	vmachine.deviceTable[VMACHINE_DEVICE_RESET][vmachine.numDevices] = (t_faddrcc) reset;
	vmachine.deviceTable[VMACHINE_DEVICE_REFRESH][vmachine.numDevices] = (t_faddrcc) refresh;
	vmachine.deviceTable[VMACHINE_DEVICE_FINAL][vmachine.numDevices] = (t_faddrcc) final;
	vmachine.numDevices++;
}
