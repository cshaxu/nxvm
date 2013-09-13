/* This file is a part of NXVM project. */

#include "../vapi.h"

#include "win32.h"
#include "w32cdisp.h"
#include "win32con.h"

HANDLE hIn, hOut;

static DWORD WINAPI ThreadDisplay(LPVOID lpParam)
{
	w32cdispInit();
	w32cdispPaint(TRUE);
	while (vapiCallBackMachineGetFlagRun()) {
		w32cdispPaint(FALSE);
		vapiSleep(100);
	}
	w32cdispFinal();
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
	CreateThread(NULL, 0, ThreadDisplay, NULL, 0, &ThreadIdDisplay);
	CreateThread(NULL, 0, ThreadKernel, NULL, 0, &ThreadIdKernel);
	while (vapiCallBackMachineGetFlagRun()) {
		vapiSleep(20);
		w32ckeybProcess();
	}
}
