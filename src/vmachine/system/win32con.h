/* This file is a part of NXVM project. */

#ifndef NXVM_WIN32CON_H
#define NXVM_WIN32CON_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "windows.h"

extern HANDLE hIn, hOut;

VOID w32ckeybProcess();
VOID win32conDisplaySetScreen();
VOID win32conDisplayPaint();
VOID win32conStartMachine();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
