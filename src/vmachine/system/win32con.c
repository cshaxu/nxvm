/* This file is a part of NXVM project. */

#include "../vapi.h"

#include "tchar.h"
#include "w32cdisp.h"
#include "w32ckeyb.h"
#include "win32con.h"
#include "win32app.h"

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

void win32conDisplaySetScreen() {w32cdispSetScreen();}
void win32conDisplayPaint() {w32cdispPaint(TRUE);}
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
