/* Copyright 2012-2014 Neko. */

/* WIN32CON provides win32 console i/o interface. */

#include "../../utils.h"
#include "../../device/device.h"

#include "win32.h"
#include "w32cdisp.h"
#include "win32con.h"

HANDLE hIn, hOut;

static DWORD WINAPI ThreadDisplay(LPVOID lpParam) {
	w32cdispInit();
	w32cdispPaint(TRUE);
	while (device.flagRun) {
		w32cdispPaint(FALSE);
		utilsSleep(100);
	}
	w32cdispFinal();
	return 0;
}

static DWORD WINAPI ThreadKernel(LPVOID lpParam) {
	deviceStart();
	return 0;
}

static VOID w32ckeybProcess() {
	DWORD res;
	INPUT_RECORD inRec;
	UCHAR scanCode, virtualKey;

	GetNumberOfConsoleInputEvents(hIn, &res);
	if (!res) {
		return;
	}
	ReadConsoleInput(hIn,&inRec,1,&res);
	switch (inRec.EventType) {
	case KEY_EVENT:
		scanCode = (UCHAR)inRec.Event.KeyEvent.wVirtualScanCode;
		virtualKey = (UCHAR)inRec.Event.KeyEvent.wVirtualKeyCode;
		if (inRec.Event.KeyEvent.bKeyDown) {
			win32KeyboardMakeKey(scanCode, virtualKey);
		} else {
			win32KeyboardMakeStatus();
		}
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

VOID win32conStartMachine() {
	DWORD ThreadIdDisplay;
	DWORD ThreadIdKernel;
	BOOL oldDeviceFlip = device.flagFlip;
	hIn = GetStdHandle(STD_INPUT_HANDLE);
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CreateThread(NULL, 0, ThreadKernel, NULL, 0, &ThreadIdKernel);
	while (oldDeviceFlip == device.flagFlip) {
		utilsSleep(100);
	}
	CreateThread(NULL, 0, ThreadDisplay, NULL, 0, &ThreadIdDisplay);
	while (device.flagRun) {
		utilsSleep(20);
		w32ckeybProcess();
	}
}
