
#include "stdio.h"

#include "../system/vapi.h"

#include "vpic.h"
#include "vcpu.h"
#include "vcpuins.h"

#include "qdbios.h"
#include "qdkeyb.h"

#define bufptrHead (vramWord(0x0000,QDKEYB_VBIOS_ADDR_KEYB_BUF_HEAD))
#define bufptrTail (vramWord(0x0000,QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL))
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
	vramWord(0x0000, bufptrTail) = ascii;
	bufptrAdvance(bufptrTail);
	return 0;
}
static t_nubit16 bufPop()
{
	t_nubit16 res = 0;
	if (bufIsEmpty) return res;
	res = vramWord(0x0000, bufptrHead);
	bufptrAdvance(bufptrHead);
	return res;
}

void IO_Read_0064()
{
	vcpu.iobyte = 0x10;
}
void IO_Write_00BB()
{
	vapiPrint("fuck BB!\n");
	INT_16();
	if (GetBit(vcpu.flags, VCPU_FLAG_ZF))
		vramWord(vcpu.ss,vcpu.sp) |=  VCPU_FLAG_ZF;
	else
		vramWord(vcpu.ss,vcpu.sp) &= ~VCPU_FLAG_ZF;
}

t_bool qdkeybRecvKeyPress(t_nubit16 ascii)
{
	return bufPush(ascii);
}

void qdkeybReadInput()
{
	if (bufIsEmpty) return;
	vcpu.ax = bufPop();
	vpicSetIRQ(0x01);
}
void qdkeybGetStatus()
{
	if (bufIsEmpty) {
		vapiSleep(0);
		SetBit(vcpu.flags, VCPU_FLAG_ZF);
	} else {
		vcpu.ax = vramWord(0x0000, bufptrHead);
		ClrBit(vcpu.flags, VCPU_FLAG_ZF);
	}
}
void qdkeybGetShiftStatus()
{
	vcpu.al = 0x20;
}

void qdkeybInit()
{
	vcpuinsOutPort[0x00bb] = (t_faddrcc)IO_Write_00BB;
	vcpuinsInPort[0x0064] = (t_faddrcc)IO_Read_0064;
}
void qdkeybFinal()
{}
