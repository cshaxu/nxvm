
#include "vram.h"
#include "vcpu.h"
#include "vcpuins.h"

#include "qdkeyb.h"

#define QDKEYB_VBIOS_ADDR_KEYB_FLAG1         0x0417
#define QDKEYB_VBIOS_ADDR_KEYB_FLAG2         0x0418
#define QDKEYB_VBIOS_ADDR_KEYB_BUF_HEAD      0x041a
#define QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL      0x041c
#define QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START  0x041e
#define QDKEYB_VBIOS_ADDR_KEYB_BUFFER_END    0x043d

#define bufptrHead (vramWord(0x0000,QDKEYB_VBIOS_ADDR_KEYB_BUF_HEAD))
#define bufptrTail (vramWord(0x0000,QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL))
#define bufGetSize (QDKEYB_VBIOS_ADDR_KEYB_BUFFER_END - \
                    QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START + 1)
#define bufIsEmpty (bufptrHead == bufptrTail)
#define bufIsFull  ((bufptrHead - QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START) == \
	(bufptrTail - QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START + 1) % bufGetSize)
#define bufptrAdvance(ptr) ((ptr) = (QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START + \
		((ptr) - QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START + 1) % bufGetSize))

static t_bool bufPush(t_nubit16 scancode)
{
	if (bufIsFull) return 1;
	// isNeedHandleKeyPress
	vramWord(0x0000, bufptrTail) = scancode;
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

void qdkeybReadInput()
{
	if (bufIsEmpty) return;
	vcpu.ax = bufPop();
	// isKeyhandle = true;
}
void qdkeybGetStatus()
{
	if (bufIsEmpty) {
		// sleep(0);
		SetBit(vcpu.flags, VCPU_FLAG_ZF);
	} else {
		vcpu.ax = vramWord(0x0000, bufptrHead);
		ClrBit(vcpu.flags, VCPU_FLAG_ZF);
	}
}
void qdkeybGetShift()
{
	vcpu.al = 0x20;
}

void qdkeybInit()
{
	vcpuinsInPort[0x0064] = (t_faddrcc)IO_Read_0064;
}
void qdkeybFinal()
{}