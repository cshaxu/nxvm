/* This file is a part of NXVM project. */

#include "stdio.h"

#include "../vmachine.h"

#include "qdbios.h"
#include "qdmisc.h"


/* device test*/
void INT_11()
{
	_ax = vramVarWord(0x0040, 0x0010);
}
/* memory test*/
void INT_12()
{
	_ax = vramVarWord(0x0040, 0x0013);
}
/* bios: device detect */
void INT_15()
{
	int MemorySize = 1;
	switch (_ah)
	{
	case 0xc0:
		_es = 0xf000;
		_bx = 0xe6f5;
		_ah = 0x00;
		// remember: all flags set in INT handler should be modified
		ClrCF;
		/*ClrBit(vramVarWord(_ss, _sp + 4), VCPU_FLAG_CF);*/
		break;
	case 0x24:
		if (_al == 0x03) {
			ClrCF;
			/*ClrBit(vramVarWord(_ss, _sp + 4), VCPU_FLAG_CF);*/
			_ah = 0x00;
			_bx = 0x0003;
		}
		break;
	case 0x88:
		ClrCF;
		/*ClrBit(vramVarWord(_ss, _sp + 4), VCPU_FLAG_CF);*/
		if (MemorySize > 16)		
			_ax = 0x3c00;					
		else		
			_ax = MemorySize * 1024 - 256;
		break;
	case 0xd8:
		SetCF;
		/* SetBit(vramVarWord(_ss, _sp + 4), VCPU_FLAG_CF);*/
		_ah = 0x86;
		break;
	}
}

void qdmiscReset()
{	
	qdbiosInt[0x11] = (t_faddrcc)INT_11; /* soft bios */
	qdbiosInt[0x12] = (t_faddrcc)INT_12; /* soft bios */
	qdbiosInt[0x15] = (t_faddrcc)INT_15; /* soft bios */
/* special: INT 11 */
	qdbiosMakeInt(0x11, "qdx 11;iret");
/* special: INT 12 */
	qdbiosMakeInt(0x12, "qdx 12;iret");
/* special: INT 15 */
	qdbiosMakeInt(0x15, "qdx 15;iret");
}
