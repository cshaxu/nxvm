/* This file is a part of NXVM project. */

#ifndef NXVM_W32CDISP_H
#define NXVM_W32CDISP_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "windows.h"

void w32cdispIncFlash();
void w32cdispSetScreen();
void w32cdispPaint(BOOL force);

void w32cdispInit();
void w32cdispFinal();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
