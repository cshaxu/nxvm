/* Copyright 2012-2014 Neko. */

#ifndef NXVM_W32CDISP_H
#define NXVM_W32CDISP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <windows.h>

VOID w32cdispSetScreen();
VOID w32cdispPaint(BOOL flagForce);

VOID w32cdispInit();
VOID w32cdispFinal();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
