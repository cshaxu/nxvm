/* This file is a part of NXVM project. */

#include "../vapi.h"

#include "win32app.h"
#include "w32akeyb.h"

void w32akeybMakeStatus(UINT message, WPARAM wParam, LPARAM lParam)
{
	vapiCallBackKeyboardClrFlag0();
	vapiCallBackKeyboardClrFlag1();
	if(message == WM_KEYDOWN || message == WM_SYSKEYDOWN) {
		switch(wParam) { //如果是普通字符，则flag2是0 
		case VK_INSERT: vapiCallBackKeyboardSetFlag1Insert();  break;
		case VK_CAPITAL:vapiCallBackKeyboardSetFlag1CapLck();  break;
		case VK_NUMLOCK:vapiCallBackKeyboardSetFlag1NumLck();  break;
		case VK_SCROLL: vapiCallBackKeyboardSetFlag1ScrLck();  break;
		case VK_PAUSE:  vapiCallBackKeyboardSetFlag1Pause();   break;
			//这里少了Sys req 键的处理
		case VK_MENU:   vapiCallBackKeyboardSetFlag1LeftAlt(); break;
			//区别不了。。。就当做按下左边alt，也就是说按右边也会相应，但是不要紧~~
		case VK_CONTROL:vapiCallBackKeyboardSetFlag1LeftCtrl();break;
			//同上
		default: break;
		}
	} // else: keyUp, sysKeyUp, 功能键全部都处于没有按下状态
	//刷新功能键当前状态
	if (GetKeyState(VK_INSERT)  < 0) vapiCallBackKeyboardSetFlag0Insert();
	if (GetKeyState(VK_CAPITAL) < 0) vapiCallBackKeyboardSetFlag0CapLck();
	if (GetKeyState(VK_NUMLOCK) < 0) vapiCallBackKeyboardSetFlag0NumLck();
	if (GetKeyState(VK_SCROLL)  < 0) vapiCallBackKeyboardSetFlag0ScrLck();
	if (GetKeyState(VK_MENU)    < 0) vapiCallBackKeyboardSetFlag0Alt();
	if (GetKeyState(VK_CONTROL) < 0) vapiCallBackKeyboardSetFlag0Ctrl();
	if (GetKeyState(VK_LSHIFT)  < 0) vapiCallBackKeyboardSetFlag0LeftShift();
	if (GetKeyState(VK_RSHIFT)  < 0) vapiCallBackKeyboardSetFlag0RightShift();
}
void w32akeybMakeChar(WPARAM wParam, LPARAM lParam)
{
	UINT16 ascii = (UINT16)(((lParam & 0xffff0000) >> 8) | wParam);
	while(vapiCallBackKeyboardRecvKeyPress((UINT8)ascii)) win32appSleep(10);
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
			if(vapiCallBackKeyboardGetShift())      ascii += 0x1900;
			else if(vapiCallBackKeyboardGetAlt())   ascii += 0x2d00;
			else if (vapiCallBackKeyboardGetCtrl()) ascii += 0x2300;
			vapiCallBackKeyboardRecvKeyPress((UINT8)ascii);
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
			 vapiCallBackKeyboardRecvKeyPress((UINT8)ascii);
		     break;
		default://剩下的字符可能是alt。。ctl与普通字符等，但是updateKBStatus会过滤掉普通字符
			w32akeybMakeStatus(message, wParam, lParam);
			if (wParam>= 0x41 && wParam<=0x41+'Z'-'A') {
				if (vapiCallBackKeyboardGetAlt())
					vapiCallBackKeyboardRecvKeyPress((UINT8)ascii);
				if (vapiCallBackKeyboardGetCtrl())
					vapiCallBackKeyboardRecvKeyPress(ascii + wParam - 0x41);
				//如果不是按下ctrl，则是按下alt
			}
			break;
		}
		
	}
	else //keyUp, sysKeyUp
		w32akeybMakeStatus(message, wParam, lParam);
}
