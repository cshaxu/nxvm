/* This file is a part of NXVM project. */

#include "../vmachine/vmachine.h"

#include "win32app.h"
#include "w32akeyb.h"

#define W32AKEYB_QDKEYB

#ifdef W32AKEYB_QDKEYB
#include "../vmachine/qdkeyb.h"
#define vkeybRecvKeyPress qdkeybRecvKeyPress
#define vkeybVarFlag0 qdkeybVarFlag0
#define vkeybVarFlag1 qdkeybVarFlag1
#define vkeybGetShift qdkeybGetShift
#define vkeybGetAlt   qdkeybGetAlt
#define vkeybGetCtrl  qdkeybGetCtrl
#define VKEYB_FLAG0_A_INSERT  QDKEYB_FLAG0_A_INSERT
#define VKEYB_FLAG0_A_CAPLCK  QDKEYB_FLAG0_A_CAPLCK
#define VKEYB_FLAG0_A_NUMLCK  QDKEYB_FLAG0_A_NUMLCK
#define VKEYB_FLAG0_A_SCRLCK  QDKEYB_FLAG0_A_SCRLCK
#define VKEYB_FLAG0_D_ALT     QDKEYB_FLAG0_D_ALT
#define VKEYB_FLAG0_D_CTRL    QDKEYB_FLAG0_D_CTRL
#define VKEYB_FLAG0_D_LSHIFT  QDKEYB_FLAG0_D_LSHIFT
#define VKEYB_FLAG0_D_RSHIFT  QDKEYB_FLAG0_D_RSHIFT
#define VKEYB_FLAG1_D_INSERT  QDKEYB_FLAG1_D_INSERT
#define VKEYB_FLAG1_D_CAPLCK  QDKEYB_FLAG1_D_CAPLCK
#define VKEYB_FLAG1_D_NUMLCK  QDKEYB_FLAG1_D_NUMLCK
#define VKEYB_FLAG1_D_SCRLCK  QDKEYB_FLAG1_D_SCRLCK
#define VKEYB_FLAG1_D_PAUSE   QDKEYB_FLAG1_D_PAUSE
#define VKEYB_FLAG1_D_SYSRQ   QDKEYB_FLAG1_D_SYSRQ
#define VKEYB_FLAG1_D_LALT    QDKEYB_FLAG1_D_LALT
#define VKEYB_FLAG1_D_LCTRL   QDKEYB_FLAG1_D_LCTRL
#else
#include "../vmachine/vkeyb.h"
#endif

void w32akeybMakeStatus(UINT message, WPARAM wParam, LPARAM lParam)
{
	vkeybVarFlag0 = 0x00;
	vkeybVarFlag1 = 0x00;
	if(message == WM_KEYDOWN || message == WM_SYSKEYDOWN) {
		switch(wParam) { //如果是普通字符，则flag2是0 
		case VK_INSERT: SetBit(vkeybVarFlag1, VKEYB_FLAG1_D_INSERT);break;
		case VK_CAPITAL:SetBit(vkeybVarFlag1, VKEYB_FLAG1_D_CAPLCK);break;
		case VK_NUMLOCK:SetBit(vkeybVarFlag1, VKEYB_FLAG1_D_NUMLCK);break;
		case VK_SCROLL: SetBit(vkeybVarFlag1, VKEYB_FLAG1_D_SCRLCK);break;
		case VK_PAUSE:  SetBit(vkeybVarFlag1, VKEYB_FLAG1_D_PAUSE); break;
			//这里扫了Sys req 键的处理
		case VK_MENU:   SetBit(vkeybVarFlag1, VKEYB_FLAG1_D_LALT); break;
			//区别不了。。。就当做按下左边alt，也就是说按右边也会相应，但是不要紧~~
		case VK_CONTROL:SetBit(vkeybVarFlag1, VKEYB_FLAG1_D_LCTRL); break;
			//同上
		default: break;
		}
	} // else: keyUp, sysKeyUp, 功能键全部都处于没有按下状态
	//刷新功能键当前状态
	if (GetKeyState(VK_INSERT)  < 0) SetBit(vkeybVarFlag0, VKEYB_FLAG0_A_INSERT);
	if (GetKeyState(VK_CAPITAL) < 0) SetBit(vkeybVarFlag0, VKEYB_FLAG0_A_CAPLCK);
	if (GetKeyState(VK_NUMLOCK) < 0) SetBit(vkeybVarFlag0, VKEYB_FLAG0_A_NUMLCK);
	if (GetKeyState(VK_SCROLL)  < 0) SetBit(vkeybVarFlag0, VKEYB_FLAG0_A_SCRLCK);
	if (GetKeyState(VK_MENU)    < 0) SetBit(vkeybVarFlag0, VKEYB_FLAG0_D_ALT);
	if (GetKeyState(VK_CONTROL) < 0) SetBit(vkeybVarFlag0, VKEYB_FLAG0_D_CTRL);
	if (GetKeyState(VK_LSHIFT)  < 0) SetBit(vkeybVarFlag0, VKEYB_FLAG0_D_LSHIFT);
	if (GetKeyState(VK_RSHIFT)  < 0) SetBit(vkeybVarFlag0, VKEYB_FLAG0_D_RSHIFT);
}
void w32akeybMakeChar(WPARAM wParam, LPARAM lParam)
{
	UINT16 ascii = (UINT16)(((lParam & 0xffff0000) >> 8) | wParam);
	while(vkeybRecvKeyPress(ascii)) win32appSleep(10);
}
void w32akeybMakeKey(UINT message, WPARAM wParam, LPARAM lParam)
{
	UINT16 ascii = (UINT16)((lParam & 0xffff0000)>>8);
	if (WM_SYSKEYDOWN == message || WM_KEYDOWN == message) {
		switch(wParam) {
		case VK_F1:
		case VK_F2:
		case VK_F3:
		case VK_F4:
		case VK_F5:
		case VK_F6:
		case VK_F7:
		case VK_F8:
		case VK_F9:
		case VK_F10:
		case VK_F11:
		case VK_F12://F1~~F12
			if(vkeybGetShift)      ascii += 0x1900;
			else if(vkeybGetAlt)   ascii += 0x2d00;
			else if (vkeybGetCtrl) ascii += 0x2300;
			vkeybRecvKeyPress(ascii);
			break;
		case VK_ESCAPE://ESC
		case VK_PRIOR://pageUP
		case VK_NEXT://pageDown
		case VK_END:
		case VK_HOME:
		case VK_LEFT:
		case VK_UP:
		case VK_RIGHT:
		case VK_DOWN:
			 vkeybRecvKeyPress(ascii);
		     break;
		default://剩下的字符可能是alt。。ctl与普通字符等，但是updateKBStatus会过滤掉普通字符
			w32akeybMakeStatus(message, wParam, lParam);
			if (wParam>= 0x41 && wParam<=0x41+'Z'-'A') {
				if (vkeybGetAlt)  vkeybRecvKeyPress(ascii);
				if (vkeybGetCtrl) vkeybRecvKeyPress(ascii + wParam - 0x41);
				//如果不是按下ctrl，则是按下alt
			}
			break;
		}
		
	}
	else //keyUp, sysKeyUp
		w32akeybMakeStatus(message, wParam, lParam);
}
