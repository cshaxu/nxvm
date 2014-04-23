/* This file is a part of NXVM project. */

#ifndef NXVM_W32ADISP_H
#define NXVM_W32ADISP_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "windows.h"

void w32adispIncFlash();
void w32adispSetScreen();
void w32adispPaint();

void w32adispInit();
void w32adispFinal();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
