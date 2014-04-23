/* This file is a part of NXVM project. */

#include "stdio.h"
#include "memory.h"

#include "../vapi.h"
#include "../vmachine.h"

#include "qdbios.h"
#include "qddisk.h"

#define SetHddStatus (vramRealByte(0x0040, 0x0074) = _ah)
#define GetHddStatus (vramRealByte(0x0040, 0x0074))
t_vaddrcc vhddGetAddress(t_nubit8 cyl, t_nubit8 head, t_nubit8 sector)
{
	vhdd.cyl = cyl;
	vhdd.head = head;
	vhdd.sector = sector;
	vhddSetPointer;
	return vhdd.curr;
}
static void INT_13_02_HDD_ReadSector()
{
	t_nubit8 drive  = _dl;
	t_nubit8 head   = _dh;
	t_nubit8 cyl    = _ch | ((_cl & 0xc0) << 8);
	t_nubit8 sector = _cl & 0x3f;
	drive &= 0x7f;
	if (drive || !sector || head >= vhdd.nhead || sector > vhdd.nsector || cyl >= vhdd.ncyl) {
		/* sector not found */
		vapiCallBackMachineStop();
		_ah = 0x04;
		SetBit(_eflags, VCPU_EFLAGS_CF);
	} else {
		memcpy((void *)vramGetRealAddr(_es,_bx),
			(void *)vhddGetAddress(cyl,head,sector), _al * vhdd.nbyte);
		_ah = 0x00;
		ClrBit(_eflags, VCPU_EFLAGS_CF);
	}
}
static void INT_13_03_HDD_WriteSector()
{
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
		memcpy((void *)vhddGetAddress(cyl,head,sector),
			(void *)vramGetRealAddr(_es,_bx), _al * vhdd.nbyte);
		_ah = 0x00;
		ClrBit(_eflags, VCPU_EFLAGS_CF);
	}
}

void qddiskReset()
{
	qdbiosInt[0xa2] = (t_faddrcc)INT_13_02_HDD_ReadSector;
	qdbiosInt[0xa3] = (t_faddrcc)INT_13_03_HDD_WriteSector;
}
