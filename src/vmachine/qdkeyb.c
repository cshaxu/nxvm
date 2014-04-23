
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
{SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_INSERT);}
void vapiCallBackKeyboardSetFlag0CapLck()
{SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_CAPLCK);}
void vapiCallBackKeyboardSetFlag0NumLck()
{SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_NUMLCK);}
void vapiCallBackKeyboardSetFlag0ScrLck()
{SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_SCRLCK);}
void vapiCallBackKeyboardSetFlag0Alt()
{SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_ALT);}
void vapiCallBackKeyboardSetFlag0Ctrl()
{SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_CTRL);}
void vapiCallBackKeyboardSetFlag0LeftShift()
{SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_LSHIFT);}
void vapiCallBackKeyboardSetFlag0RightShift()
{SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_RSHIFT);}

void vapiCallBackKeyboardSetFlag1Insert()
{SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_INSERT);}
void vapiCallBackKeyboardSetFlag1CapLck()
{SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_CAPLCK);}
void vapiCallBackKeyboardSetFlag1NumLck()
{SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_NUMLCK);}
void vapiCallBackKeyboardSetFlag1ScrLck()
{SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_SCRLCK);}
void vapiCallBackKeyboardSetFlag1Pause()
{SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_PAUSE);}
void vapiCallBackKeyboardSetFlag1SysRq()
{SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_SYSRQ);}
void vapiCallBackKeyboardSetFlag1LeftAlt()
{SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_LALT);}
void vapiCallBackKeyboardSetFlag1LeftCtrl()
{SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_LCTRL);}
t_bool vapiCallBackKeyboardRecvKeyPress(t_nubit16 ascii)
{return bufPush(ascii);}

void IO_Read_0064()
{
	vcpu.iobyte = 0x10;
}
void IO_Write_00BB()
{
	INT_16();
	if (GetBit(_flags, VCPU_FLAG_ZF))
		vramVarWord(_ss,_sp + 4) |=  VCPU_FLAG_ZF;
	else
		vramVarWord(_ss,_sp + 4) &= ~VCPU_FLAG_ZF;
}

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
		SetBit(_flags, VCPU_FLAG_ZF);
	} else {
		_ax = vramVarWord(0x0000, bufptrHead);
		ClrBit(_flags, VCPU_FLAG_ZF);
	}
}
void qdkeybGetShiftStatus()
{
	_al = 0x20;
}

void qdkeybInit()
{
	vport.out[0x00bb] = (t_faddrcc)IO_Write_00BB;
	vport.in[0x0064] = (t_faddrcc)IO_Read_0064;
}
void qdkeybFinal()
{}
