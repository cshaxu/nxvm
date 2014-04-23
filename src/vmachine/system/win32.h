/* This file is a part of NXVM project. */

#ifndef NXVM_WIN32_H
#define NXVM_WIN32_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "windows.h"

#include "win32con.h"
#include "win32app.h"

#define win32Sleep Sleep
void win32KeyboardMakeStatus();
void win32KeyboardMakeKey(UCHAR scanCode, UCHAR virtualKey);
void win32DisplaySetScreen(BOOL window);
void win32DisplayPaint(BOOL window);
void win32StartMachine(BOOL window);

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
