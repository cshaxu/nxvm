/* This file is a part of NXVM project. */

#include "../vapi.h"

#include "win32con.h"
#include "w32ckeyb.h"

static VOID w32ckeybMakeStatus()
{
	vapiCallBackKeyboardClrFlag0();
	vapiCallBackKeyboardClrFlag1();
	if (GetKeyState(VK_INSERT)  < 0) vapiCallBackKeyboardSetFlag1Insert();
	if (GetKeyState(VK_CAPITAL) < 0) vapiCallBackKeyboardSetFlag1CapLck();
	if (GetKeyState(VK_NUMLOCK) < 0) vapiCallBackKeyboardSetFlag1NumLck();
	if (GetKeyState(VK_SCROLL)  < 0) vapiCallBackKeyboardSetFlag1ScrLck();
	if (GetKeyState(VK_PAUSE)   < 0) vapiCallBackKeyboardSetFlag1Pause();
	if (GetKeyState(VK_MENU)    < 0) vapiCallBackKeyboardSetFlag1LeftAlt();
	if (GetKeyState(VK_CONTROL) < 0) vapiCallBackKeyboardSetFlag1LeftCtrl();
	if (GetKeyState(VK_INSERT)  < 0) vapiCallBackKeyboardSetFlag0Insert();
	if (GetKeyState(VK_CAPITAL) < 0) vapiCallBackKeyboardSetFlag0CapLck();
	if (GetKeyState(VK_NUMLOCK) < 0) vapiCallBackKeyboardSetFlag0NumLck();
	if (GetKeyState(VK_SCROLL)  < 0) vapiCallBackKeyboardSetFlag0ScrLck();
	if (GetKeyState(VK_MENU)    < 0) vapiCallBackKeyboardSetFlag0Alt();
	if (GetKeyState(VK_CONTROL) < 0) vapiCallBackKeyboardSetFlag0Ctrl();
	if (GetKeyState(VK_LSHIFT)  < 0) vapiCallBackKeyboardSetFlag0LeftShift();
	if (GetKeyState(VK_RSHIFT)  < 0) vapiCallBackKeyboardSetFlag0RightShift();
}
static VOID w32ckeybMakeKey(INPUT_RECORD inRec)
{
	t_nubit16 ascii = (inRec.Event.KeyEvent.wVirtualScanCode << 8);
	t_nubit16 vkey = inRec.Event.KeyEvent.wVirtualKeyCode;
	if (inRec.Event.KeyEvent.bKeyDown) {
		/*vapiPrint("key.vsc = %x, vk = %x, ascii = %x, uni = %x\n",
			inRec.Event.KeyEvent.wVirtualScanCode,
			inRec.Event.KeyEvent.wVirtualKeyCode,
			inRec.Event.KeyEvent.uChar.AsciiChar,
			inRec.Event.KeyEvent.uChar.UnicodeChar);*/
		switch (vkey) {
		case VK_F10:
			vapiCallBackMachineStop();
			break;
		case VK_F1:
		case VK_F2:
		case VK_F3:
		case VK_F4:
		case VK_F5:
		case VK_F6:
		case VK_F7:
		case VK_F8:
		case VK_F9:
		case VK_F11:
		case VK_F12://F1~~F12
			if(vapiCallBackKeyboardGetFlag0Shift())      ascii += 0x1900;
			else if(vapiCallBackKeyboardGetFlag0Alt())   ascii += 0x2d00;
			else if (vapiCallBackKeyboardGetFlag0Ctrl()) ascii += 0x2300;
			vapiCallBackKeyboardRecvKeyPress(ascii);
			break;
		/*case VK_ESCAPE://ESC
		case VK_BACK:  // BACKSPACE
		case VK_PRIOR: //PageUP
		case VK_NEXT: //pageDown
		case VK_END:
		case VK_HOME:
		case VK_LEFT:
		case VK_UP:
		case VK_RIGHT:
		case VK_DOWN:
		case VK_RETURN:
			vapiCallBackKeyboardRecvKeyPress(ascii |
				inRec.Event.KeyEvent.uChar.AsciiChar);
			break;*/
		default://剩下的字符可能是alt。。ctl与普通字符等，但是updateKBStatus会过滤掉普通字符
			w32ckeybMakeStatus();
			if (vapiCallBackKeyboardGetFlag0Alt())
				vapiCallBackKeyboardRecvKeyPress(ascii);
			else if (vapiCallBackKeyboardGetFlag0Ctrl())
				vapiCallBackKeyboardRecvKeyPress(ascii + vkey - 0x0041);
			else
				vapiCallBackKeyboardRecvKeyPress(ascii |
					inRec.Event.KeyEvent.uChar.AsciiChar);
				//如果不是按下ctrl，则是按下alt
			break;
		}
		
	} else w32ckeybMakeStatus();
}

VOID w32ckeybProcess()
{
	INPUT_RECORD inRec;
	DWORD res;
	
	GetNumberOfConsoleInputEvents(hIn, &res);
	if (!res) return;
	ReadConsoleInput(hIn,&inRec,1,&res);
	switch (inRec.EventType) {
	case KEY_EVENT:
		w32ckeybMakeKey(inRec);
		break;
	case FOCUS_EVENT:
		w32ckeybMakeStatus();
		break;
	default:
		break;
	}
}
