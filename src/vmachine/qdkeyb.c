
#include "stdio.h"

#include "../system/vapi.h"

#include "vport.h"
#include "vcpu.h"
#include "vpic.h"

#include "qdbios.h"
#include "qdkeyb.h"

#define bufptrHead (vramVarWord(0x0000,QDKEYB_VBIOS_ADDR_KEYB_BUF_HEAD))
#define bufptrTail (vramVarWord(0x0000,QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL))
#define bufGetSize (QDKEYB_VBIOS_ADDR_KEYB_BUFFER_END - \
                    QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START + 1)
#define bufIsEmpty (bufptrHead == bufptrTail)
#define bufIsFull  ((bufptrHead - QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START) == \
	(bufptrTail - QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START + 1) % bufGetSize)
#define bufptrAdvance(ptr) ((ptr) = (QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START + \
		((ptr) - QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START + 1) % bufGetSize))

static t_bool bufPush(t_nubit16 ascii)
{
	if (bufIsFull) return 1;
	// isNeedHandleKeyPress
	vramVarWord(0x0000, bufptrTail) = ascii;
	bufptrAdvance(bufptrTail);
	return 0;
}
static t_nubit16 bufPop()
{
	t_nubit16 res = 0;
	if (bufIsEmpty) return res;
	res = vramVarWord(0x0000, bufptrHead);
	bufptrAdvance(bufptrHead);
	return res;
}

void IO_Read_0064()
{
	vcpu.iobyte = 0x10;
}
void IO_Write_00BB()
{
	INT_16();
	if (GetBit(vcpu.flags, VCPU_FLAG_ZF))
		vramVarWord(vcpu.ss,vcpu.sp + 4) |=  VCPU_FLAG_ZF;
	else
		vramVarWord(vcpu.ss,vcpu.sp + 4) &= ~VCPU_FLAG_ZF;
}

t_bool qdkeybRecvKeyPress(t_nubit16 ascii)
{
	return bufPush(ascii);
}

void qdkeybReadInput()
{
	if (bufIsEmpty) return;
	vcpu.ax = bufPop();
	//vpicSetIRQ(0x01);
}
void qdkeybGetStatus()
{
	if (bufIsEmpty) {
		vapiSleep(0);
		SetBit(vcpu.flags, VCPU_FLAG_ZF);
	} else {
		vcpu.ax = vramVarWord(0x0000, bufptrHead);
		ClrBit(vcpu.flags, VCPU_FLAG_ZF);
	}
}
void qdkeybGetShiftStatus()
{
	vcpu.al = 0x20;
}

void qdkeybInit()
{
	vport.out[0x00bb] = (t_faddrcc)IO_Write_00BB;
	vport.in[0x0064] = (t_faddrcc)IO_Read_0064;
}
void qdkeybFinal()
{}
