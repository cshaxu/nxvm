/* This file is a part of NXVM project. */

#ifndef NXVM_WIN32APP_H
#define NXVM_WIN32APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "windows.h"

VOID win32appDisplaySetScreen();
VOID win32appDisplayPaint();

VOID win32appStartMachine();

extern HWND w32aHWnd;

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
