#include "stdlib.h"
#include "memory.h"

#include "vcpu.h"
#include "vram.h"

#include "qdfdd.h"

t_fdd qdfdd;

#define SetStatus (vramSetByte(0x0040, 0x0041, vcpu.ah))
#define GetStatus (vramGetByte(0x0040, 0x0041))

t_vaddrcc qdfddGetAddress(t_nubit8 cyl, t_nubit8 head, t_nubit8 sector)
{return (qdfdd.base + ((cyl * 2 + head) * 18 + (sector-1)) * 512);}

void qdfddResetDrive()
{
	if (vcpu.dl) {
		/* only one drive */
		vcpu.ah = 0x0c;
		SetBit(vcpu.flags, VCPU_FLAG_CF);;
	} else {
		vcpu.ah = 0x00;
		ClrBit(vcpu.flags, VCPU_FLAG_CF);;
	}
	SetStatus;
}
void qdfddGetDiskStatus()
{
	vcpu.ah = GetStatus;
}
void qdfddReadSector()
{
	t_nubit8 drive  = vcpu.dl;
	t_nubit8 head   = vcpu.dh;
	t_nubit8 cyl    = vcpu.ch;
	t_nubit8 sector = vcpu.cl;
	if (drive || head > 1 || sector > 18 || cyl > 79) {
		/* sector not found */
		vcpu.ah = 0x04;
		SetBit(vcpu.flags, VCPU_FLAG_CF);;
		SetStatus;
	} else {
		memcpy((void *)vramGetRealAddress(vcpu.es,vcpu.bx),
			(void *)qdfddGetAddress(cyl,head,sector), vcpu.al * 512);
		vcpu.ah = 0x00;
		ClrBit(vcpu.flags, VCPU_FLAG_CF);;
		SetStatus;
	}
}
void qdfddWriteSector()
{
	t_nubit8 drive  = vcpu.dl;
	t_nubit8 head   = vcpu.dh;
	t_nubit8 cyl    = vcpu.ch;
	t_nubit8 sector = vcpu.cl;
	if (drive || head > 1 || sector > 18 || cyl > 79) {
		/* sector not found */
		vcpu.ah = 0x04;
		SetBit(vcpu.flags, VCPU_FLAG_CF);;
		SetStatus;
	} else {
		memcpy((void *)qdfddGetAddress(cyl,head,sector),
			(void *)vramGetRealAddress(vcpu.es,vcpu.bx), vcpu.al * 512);
		vcpu.ah = 0x00;
		ClrBit(vcpu.flags, VCPU_FLAG_CF);;
		SetStatus;
	}
}
void qdfddGetParameter()
{
	if (vcpu.dl == 0x80) {
		vcpu.ah = 0x01;
		SetBit(vcpu.flags, VCPU_FLAG_CF);
		SetStatus;
		ClrBit(vcpu.flags, VCPU_FLAG_PF);
		ClrBit(vcpu.flags, VCPU_FLAG_IF);
	} else {
		vcpu.ah = 0x00;
		vcpu.bl = 0x04;
		vcpu.cx = 0x4f12;
		vcpu.dx = 0x0102;
		vcpu.di = vramGetWord(0x0000, 0x1e * 4);
		vcpu.es = vramGetWord(0x0000, 0x1e * 4 + 2);
		ClrBit(vcpu.flags, VCPU_FLAG_CF);
		SetStatus;
	}
}
void qdfddGetDriveType()
{
	switch (vcpu.dl) {
	case 0x00:
	case 0x01:
		ClrBit(vcpu.flags, VCPU_FLAG_CF);
		SetStatus;
		vcpu.ah = 0x01;
		break;
	case 0x02:
	case 0x03:
		ClrBit(vcpu.flags, VCPU_FLAG_CF);
		SetStatus;
		vcpu.ah = 0x00;
		break;
	default:
		break;
	}
}

void qdfddInit()
{
	memset(&qdfdd, 0x00, sizeof(t_fdd));
	qdfdd.base = (t_vaddrcc)malloc(0x00168000);
}
void qdfddFinal()
{
	if (qdfdd.base) free((void *)qdfdd.base);
}