/* Copyright 2012-2014 Neko. */

#ifndef NXVM_WIN32_H
#define NXVM_WIN32_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

VOID win32KeyboardMakeStatus();
VOID win32KeyboardMakeKey(UCHAR scanCode, UCHAR virtualKey);

#define win32Sleep Sleep
VOID win32DisplaySetScreen(BOOL window);
VOID win32DisplayPaint(BOOL window);
VOID win32StartMachine(BOOL window);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
