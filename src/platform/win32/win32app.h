/* Copyright 2012-2014 Neko. */

#ifndef NXVM_WIN32APP_H
#define NXVM_WIN32APP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

extern HWND w32aHWnd;

VOID win32appDisplaySetScreen();
VOID win32appDisplayPaint();

VOID win32appStartMachine();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
