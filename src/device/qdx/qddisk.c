/* Copyright 2012-2014 Neko. */

/* QDDISK implements quick and dirty hard drive control routines. */

#include "../vcpu.h"
#include "../vram.h"
#include "../vhdd.h"

#include "qdx.h"
#include "qddisk.h"

#define SetHddStatus (vramRealByte(0x0040, 0x0074) = _ah)
#define GetHddStatus (vramRealByte(0x0040, 0x0074))

static t_vaddrcc vhddGetAddress(t_nubit8 cyl, t_nubit8 head, t_nubit8 sector) {
	vhdd.cyl = cyl;
	vhdd.head = head;
	vhdd.sector = sector;
	vhddSetPointer;
	return vhdd.curr;
}

static void INT_13_02_HDD_ReadSector() {
	t_nubit8 drive  = _dl;
	t_nubit8 head   = _dh;
	t_nubit8 cyl    = _ch | ((_cl & 0xc0) << 8);
	t_nubit8 sector = _cl & 0x3f;
	drive &= 0x7f;
	if (drive || !sector || head >= vhdd.nhead || sector > vhdd.nsector || cyl >= vhdd.ncyl) {
		/* sector not found */
		_ah = 0x04;
		SetBit(_eflags, VCPU_EFLAGS_CF);
	} else {
		memcpy((void *) vramGetRealAddr(_es,_bx),
			(void *) vhddGetAddress(cyl,head,sector), _al * vhdd.nbyte);
		_ah = 0x00;
		ClrBit(_eflags, VCPU_EFLAGS_CF);
	}
}

static void INT_13_03_HDD_WriteSector() {
	t_nubit8 drive  = _dl;
	t_nubit8 head   = _dh;
	t_nubit8 cyl    = _ch | ((_cl & 0xc0) << 8);
	t_nubit8 sector = _cl & 0x3f;
	drive &= 0x7f;
	if (drive || !sector || head >= vhdd.nhead || sector > vhdd.nsector || cyl >= vhdd.ncyl) {
		/* sector not found */
		_ah = 0x04;
		SetBit(_eflags, VCPU_EFLAGS_CF);
	} else {
		memcpy((void *) vhddGetAddress(cyl,head,sector),
			(void *) vramGetRealAddr(_es,_bx), _al * vhdd.nbyte);
		_ah = 0x00;
		ClrBit(_eflags, VCPU_EFLAGS_CF);
	}
}

void qddiskInit() {
	qdxTable[0xa2] = (t_faddrcc) INT_13_02_HDD_ReadSector;
	qdxTable[0xa3] = (t_faddrcc) INT_13_03_HDD_WriteSector;
}
