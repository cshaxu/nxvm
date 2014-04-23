/* This file is a part of NXVM project. */

#ifndef NXVM_W32AMAIN_H
#define NXVM_W32AMAIN_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "windows.h"

void win32appDisplaySetScreen();
void win32appDisplayPaint();

void win32appStartMachine();

extern HWND w32aHWnd;

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
