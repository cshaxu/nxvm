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

static t_bool bufPush(t_nubit16 code)
{
	if (bufIsFull) return 1;
	vramVarWord(0x0000, bufptrTail) = code;
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

t_bool vapiCallBackKeyboardGetFlag0CapsLock() {return GetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_CAPLCK);}
t_bool vapiCallBackKeyboardGetFlag0NumLock()  {return GetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_NUMLCK);}
t_bool vapiCallBackKeyboardGetFlag0Shift()    {return GetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_LSHIFT) || GetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_RSHIFT);}
t_bool vapiCallBackKeyboardGetFlag0Alt()  {return GetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_ALT);}
t_bool vapiCallBackKeyboardGetFlag0Ctrl() {return GetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_CTRL);}
void vapiCallBackKeyboardClrFlag0() {qdkeybVarFlag0 = 0x00;}
void vapiCallBackKeyboardClrFlag1() {qdkeybVarFlag1 = 0x00;}

void vapiCallBackKeyboardSetFlag0Insert()     {SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_INSERT);}
void vapiCallBackKeyboardSetFlag0CapLck()     {SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_CAPLCK);}
void vapiCallBackKeyboardSetFlag0NumLck()     {SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_NUMLCK);}
void vapiCallBackKeyboardSetFlag0ScrLck()     {SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_SCRLCK);}
void vapiCallBackKeyboardSetFlag0Alt()        {SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_ALT);}
void vapiCallBackKeyboardSetFlag0Ctrl()       {SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_CTRL);}
void vapiCallBackKeyboardSetFlag0LeftShift()  {SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_LSHIFT);}
void vapiCallBackKeyboardSetFlag0RightShift() {SetBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_RSHIFT);}

void vapiCallBackKeyboardClrFlag0Insert()     {ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_INSERT);}
void vapiCallBackKeyboardClrFlag0CapLck()     {ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_CAPLCK);}
void vapiCallBackKeyboardClrFlag0NumLck()     {ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_NUMLCK);}
void vapiCallBackKeyboardClrFlag0ScrLck()     {ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_A_SCRLCK);}
void vapiCallBackKeyboardClrFlag0Alt()        {ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_ALT);}
void vapiCallBackKeyboardClrFlag0Ctrl()       {ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_CTRL);}
void vapiCallBackKeyboardClrFlag0LeftShift()  {ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_LSHIFT);}
void vapiCallBackKeyboardClrFlag0RightShift() {ClrBit(qdkeybVarFlag0, QDKEYB_FLAG0_D_RSHIFT);}

void vapiCallBackKeyboardSetFlag1Insert()   {SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_INSERT);}
void vapiCallBackKeyboardSetFlag1CapLck()   {SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_CAPLCK);}
void vapiCallBackKeyboardSetFlag1NumLck()   {SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_NUMLCK);}
void vapiCallBackKeyboardSetFlag1ScrLck()   {SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_SCRLCK);}
void vapiCallBackKeyboardSetFlag1Pause()    {SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_PAUSE);}
void vapiCallBackKeyboardSetFlag1SysRq()    {SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_SYSRQ);}
void vapiCallBackKeyboardSetFlag1LeftAlt()  {SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_LALT);}
void vapiCallBackKeyboardSetFlag1LeftCtrl() {SetBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_LCTRL);}

void vapiCallBackKeyboardClrFlag1Insert()   {ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_INSERT);}
void vapiCallBackKeyboardClrFlag1CapLck()   {ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_CAPLCK);}
void vapiCallBackKeyboardClrFlag1NumLck()   {ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_NUMLCK);}
void vapiCallBackKeyboardClrFlag1ScrLck()   {ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_SCRLCK);}
void vapiCallBackKeyboardClrFlag1Pause()    {ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_PAUSE);}
void vapiCallBackKeyboardClrFlag1SysRq()    {ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_SYSRQ);}
void vapiCallBackKeyboardClrFlag1LeftAlt()  {ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_LALT);}
void vapiCallBackKeyboardClrFlag1LeftCtrl() {ClrBit(qdkeybVarFlag1, QDKEYB_FLAG1_D_LCTRL);}

void vapiCallBackKeyboardRecvKeyPress(t_nubit16 code)
{
//	while(bufPush(code)) vapiSleep(1);
	bufPush(code);
	vpicSetIRQ(0x01);
}

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
	/* TODO: this should have been working with INT 15 */
	while (bufIsEmpty) vapiSleep(10);
	if (bufIsEmpty) return;
	_ax = bufPop();
	vpicSetIRQ(0x01);
}
void qdkeybGetStatus()
{
	if (bufIsEmpty) {
		SetZF;
	} else {
		_ax = vramVarWord(0x0000, bufptrHead);
		ClrZF;
	}
}
void qdkeybGetShiftStatus()
{
	_al = qdkeybVarFlag0;
}

void qdkeybInit()
{
	vport.in[0x0064] = (t_faddrcc)IO_Read_0064;
}
void qdkeybFinal() {}
