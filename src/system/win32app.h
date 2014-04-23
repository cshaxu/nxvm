/* This file is a part of NXVM project. */

#ifndef NXVM_W32AMAIN_H
#define NXVM_W32AMAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "windows.h"

void win32appSleep(DWORD milisec);

void win32appDisplaySetScreen();
void win32appDisplayPaint();

void win32appStartDisplay();
void win32appStartKernel();

extern HWND hWnd;

#ifdef __cplusplus
}
#endif

#endif
