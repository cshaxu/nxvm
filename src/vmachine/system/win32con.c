/* This file is a part of NXVM project. */

#include "../vapi.h"

#include "win32.h"
#include "w32cdisp.h"
#include "win32con.h"

HANDLE hIn, hOut;

static DWORD WINAPI ThreadDisplay(LPVOID lpParam)
{
	while (vapiCallBackMachineGetFlagRun()) {
		w32cdispPaint(FALSE);
		vapiSleep(100);
	}
	return 0;
}
static DWORD WINAPI ThreadKernel(LPVOID lpParam)
{
	vapiCallBackMachineRun();
	return 0;
}

VOID w32ckeybProcess()
{
	DWORD res;
	INPUT_RECORD inRec;
	UCHAR scanCode, virtualKey;

	GetNumberOfConsoleInputEvents(hIn, &res);
	if (!res) return;
	ReadConsoleInput(hIn,&inRec,1,&res);
	switch (inRec.EventType) {
	case KEY_EVENT:
		scanCode = (UCHAR)inRec.Event.KeyEvent.wVirtualScanCode;
		virtualKey = (UCHAR)inRec.Event.KeyEvent.wVirtualKeyCode;
		if (inRec.Event.KeyEvent.bKeyDown)
			win32KeyboardMakeKey(scanCode, virtualKey);
	//		switch (virtualKey) {
	////		case VK_RCONTROL:
	////			vapiCallBackMachineStop();
	////			break;
	//		case VK_F1:
	//		case VK_F2:
	//		case VK_F3:
	//		case VK_F4:
	//		case VK_F5:
	//		case VK_F6:
	//		case VK_F7:
	//		case VK_F8:
	//		case VK_F9:
	//		case VK_F10:
	//		case VK_F11:
	//		case VK_F12://F1~~F12
	//			if(vapiCallBackKeyboardGetFlag0Shift())      ascii += 0x1900;
	//			else if(vapiCallBackKeyboardGetFlag0Alt())   ascii += 0x2d00;
	//			else if (vapiCallBackKeyboardGetFlag0Ctrl()) ascii += 0x2300;
	//			vapiCallBackKeyboardRecvKeyPress(ascii);
	//			break;
	//		/*case VK_ESCAPE://ESC
	//		case VK_BACK:  // BACKSPACE
	//		case VK_PRIOR: //PageUP
	//		case VK_NEXT: //pageDown
	//		case VK_END:
	//		case VK_HOME:
	//		case VK_LEFT:
	//		case VK_UP:
	//		case VK_RIGHT:
	//		case VK_DOWN:
	//		case VK_RETURN:
	//			vapiCallBackKeyboardRecvKeyPress(ascii |
	//				inRec.Event.KeyEvent.uChar.AsciiChar);
	//			break;*/
	//		default://剩下的字符可能是alt。。ctl与普通字符等，但是updateKBStatus会过滤掉普通字符
	//			win32KeyboardMakeStatus();
	//			if (vapiCallBackKeyboardGetFlag0Alt())
	//				vapiCallBackKeyboardRecvKeyPress(ascii);
	//			else if (vapiCallBackKeyboardGetFlag0Ctrl())
	//				vapiCallBackKeyboardRecvKeyPress(ascii + vkey - 0x0041);
	//			else
	//				vapiCallBackKeyboardRecvKeyPress(ascii |
	//					inRec.Event.KeyEvent.uChar.AsciiChar);
	//				//如果不是按下ctrl，则是按下alt
	//			break;
	//		}
	//	}
		else
			win32KeyboardMakeStatus();
		break;
	case FOCUS_EVENT:
		win32KeyboardMakeStatus();
		break;
	default:
		break;
	}
}
VOID win32conDisplaySetScreen() {w32cdispSetScreen();}
VOID win32conDisplayPaint() {w32cdispPaint(TRUE);}
void win32conStartMachine()
{
	DWORD ThreadIdDisplay;
	DWORD ThreadIdKernel;
	hIn = GetStdHandle(STD_INPUT_HANDLE);
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	w32cdispInit();
	w32cdispPaint(TRUE);
	CreateThread(NULL, 0, ThreadDisplay, NULL, 0, &ThreadIdDisplay);
	CreateThread(NULL, 0, ThreadKernel, NULL, 0, &ThreadIdKernel);
	while (vapiCallBackMachineGetFlagRun()) {
		vapiSleep(20);
		w32ckeybProcess();
	}
	w32cdispFinal();
}
