/* This file is a part of NXVM project. */

#ifndef NXVM_W32CMAIN_H
#define NXVM_W32CMAIN_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "windows.h"

extern HANDLE hIn, hOut;

void win32conDisplaySetScreen();
void win32conDisplayPaint();
void win32conStartMachine();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
