/* This file is a part of NXVM project. */

#ifndef NXVM_VAPI_H
#define NXVM_VAPI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "windows.h"

void w32adispIncFlash();
void w32adispSetScreen();
void w32adispPaint();

void w32adispInit();
void w32adispFinal();

#ifdef __cplusplus
}
#endif

#endif
