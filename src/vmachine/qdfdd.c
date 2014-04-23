#include "stdlib.h"
#include "memory.h"

#include "vcpu.h"
#include "vram.h"

#include "qdfdd.h"

t_fdd qdfdd;

#define SetStatus (vramVarByte(0x0040, 0x0041) = _ah)
#define GetStatus (vramVarByte(0x0040, 0x0041))

t_vaddrcc qdfddGetAddress(t_nubit8 cyl, t_nubit8 head, t_nubit8 sector)
{return (qdfdd.base + ((cyl * 2 + head) * 18 + (sector-1)) * 512);}

void qdfddResetDrive()
{
	if (_dl) {
		/* only one drive */
		_ah = 0x0c;
		SetBit(_flags, VCPU_FLAG_CF);;
	} else {
		_ah = 0x00;
		ClrBit(_flags, VCPU_FLAG_CF);;
	}
	SetStatus;
}
void qdfddGetDiskStatus()
{
	_ah = GetStatus;
}
void qdfddReadSector()
{
	t_nubit8 drive  = _dl;
	t_nubit8 head   = _dh;
	t_nubit8 cyl    = _ch;
	t_nubit8 sector = _cl;
	if (drive || head > 1 || sector > 18 || cyl > 79) {
		/* sector not found */
		_ah = 0x04;
		SetBit(_flags, VCPU_FLAG_CF);;
		SetStatus;
	} else {
		memcpy((void *)vramGetAddr(_es,_bx),
			(void *)qdfddGetAddress(cyl,head,sector), _al * 512);
		_ah = 0x00;
		ClrBit(_flags, VCPU_FLAG_CF);;
		SetStatus;
	}
}
void qdfddWriteSector()
{
	t_nubit8 drive  = _dl;
	t_nubit8 head   = _dh;
	t_nubit8 cyl    = _ch;
	t_nubit8 sector = _cl;
	if (drive || head > 1 || sector > 18 || cyl > 79) {
		/* sector not found */
		_ah = 0x04;
		SetBit(_flags, VCPU_FLAG_CF);;
		SetStatus;
	} else {
		memcpy((void *)qdfddGetAddress(cyl,head,sector),
			(void *)vramGetAddr(_es,_bx), _al * 512);
		_ah = 0x00;
		ClrBit(_flags, VCPU_FLAG_CF);;
		SetStatus;
	}
}
void qdfddGetParameter()
{
	if (_dl == 0x80) {
		_ah = 0x01;
		SetBit(_flags, VCPU_FLAG_CF);
		SetStatus;
		ClrBit(_flags, VCPU_FLAG_PF);
		ClrBit(_flags, VCPU_FLAG_IF);
	} else {
		_ah = 0x00;
		_bl = 0x04;
		_cx = 0x4f12;
		_dx = 0x0102;
		_di = vramVarWord(0x0000, 0x1e * 4);
		_es = vramVarWord(0x0000, 0x1e * 4 + 2);
		ClrBit(_flags, VCPU_FLAG_CF);
		SetStatus;
	}
}
void qdfddGetDriveType()
{
	switch (_dl) {
	case 0x00:
	case 0x01:
		ClrBit(_flags, VCPU_FLAG_CF);
		SetStatus;
		_ah = 0x01;
		break;
	case 0x02:
	case 0x03:
		ClrBit(_flags, VCPU_FLAG_CF);
		SetStatus;
		_ah = 0x00;
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