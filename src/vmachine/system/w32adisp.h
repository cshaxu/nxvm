/* This file is a part of NXVM project. */

#ifndef NXVM_W32ADISP_H
#define NXVM_W32ADISP_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "windows.h"

VOID w32adispSetScreen();
VOID w32adispPaint();

VOID w32adispInit();
VOID w32adispFinal();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
