/* This file is a part of NXVM project. */

#include "stdio.h"

#include "vapi.h"
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
	(bufptrTail - QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START + 2) % bufGetSize)
#define bufptrAdvance(ptr) ((ptr) = (QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START + \
		((ptr) - QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START + 2) % bufGetSize))

static t_bool bufPush(t_nubit16 ascii)
{
	if (bufIsFull) return 1;
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

t_bool vapiCallBackKeyboardGetShift()
{
	return GetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_LSHIFT) || \
	       GetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_RSHIFT);
}
t_bool vapiCallBackKeyboardGetAlt()
{return GetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_ALT);}
t_bool vapiCallBackKeyboardGetCtrl()
{return GetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_CTRL);}
void vapiCallBackKeyboardClrFlag0()
{qdkeybVarFlag0 = 0x00;}
void vapiCallBackKeyboardClrFlag1()
{qdkeybVarFlag1 = 0x00;}
void vapiCallBackKeyboardSetFlag0Insert()
{/*vapiPrint("0.ins\n");*/SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_INSERT);}
void vapiCallBackKeyboardSetFlag0CapLck()
{/*vapiPrint("0.cap\n");*/SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_CAPLCK);}
void vapiCallBackKeyboardSetFlag0NumLck()
{/*vapiPrint("0.num\n");*/SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_NUMLCK);}
void vapiCallBackKeyboardSetFlag0ScrLck()
{/*vapiPrint("0.scr\n");*/SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_SCRLCK);}
void vapiCallBackKeyboardSetFlag0Alt()
{/*vapiPrint("0.alt\n");*/SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_ALT);}
void vapiCallBackKeyboardSetFlag0Ctrl()
{/*vapiPrint("0.ctrl\n");*/(qdkeybVarFlag0, QDKEYB_FLAG0_D_CTRL);}
void vapiCallBackKeyboardSetFlag0LeftShift()
{/*vapiPrint("0.lshift\n");*/SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_LSHIFT);}
void vapiCallBackKeyboardSetFlag0RightShift()
{/*vapiPrint("0.rshift\n");*/SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_RSHIFT);}

void vapiCallBackKeyboardSetFlag1Insert()
{/*vapiPrint("1.ins\n");*/SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_INSERT);}
void vapiCallBackKeyboardSetFlag1CapLck()
{/*vapiPrint("1.cap\n");*/SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_CAPLCK);}
void vapiCallBackKeyboardSetFlag1NumLck()
{/*vapiPrint("1.num\n");*/SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_NUMLCK);}
void vapiCallBackKeyboardSetFlag1ScrLck()
{/*vapiPrint("1.scr\n");*/SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_SCRLCK);}
void vapiCallBackKeyboardSetFlag1Pause()
{/*vapiPrint("1.pau\n");*/SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_PAUSE);}
void vapiCallBackKeyboardSetFlag1SysRq()
{/*vapiPrint("1.sys\n");*/SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_SYSRQ);}
void vapiCallBackKeyboardSetFlag1LeftAlt()
{/*vapiPrint("1.lalt\n");*/SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_LALT);}
void vapiCallBackKeyboardSetFlag1LeftCtrl()
{/*vapiPrint("1.lctrl\n");*/SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_LCTRL);}
t_bool vapiCallBackKeyboardRecvKeyPress(t_nubit16 ascii)
{/*vapiPrint("push ascii = %x\n",ascii);*/vpicSetIRQ(0x01);return bufPush(ascii);}

void IO_Read_0064()
{
	vport.iobyte = 0x10;
}
/*void IO_Write_00BB()
{
	qdbiosExecInt(0x16);
	if (GetBit(_flags, VCPU_FLAG_ZF))
		vramVarWord(_ss,_sp + 4) |=  VCPU_FLAG_ZF;
	else
		vramVarWord(_ss,_sp + 4) &= ~VCPU_FLAG_ZF;
}*/

void qdkeybReadInput()
{
	if (bufIsEmpty) return;
	_ax = bufPop();
	vpicSetIRQ(0x01);
}
void qdkeybGetStatus()
{
	if (bufIsEmpty) {
		vapiSleep(0);
		SetZF;
	} else {
		_ax = vramVarWord(0x0000, bufptrHead);
		ClrZF;
	}
}
void qdkeybGetShiftStatus()
{
	_al = 0x20;
}

void qdkeybInit()
{
//	vport.out[0x00bb] = (t_faddrcc)IO_Write_00BB;
	vport.in[0x0064] = (t_faddrcc)IO_Read_0064;
}
void qdkeybFinal()
{}
