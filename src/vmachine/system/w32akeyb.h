/* This file is a part of NXVM project. */

#ifndef NXVM_W32AKEYB_H
#define NXVM_W32AKEYB_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "windows.h"

void w32akeybMakeStatus(UINT message, WPARAM wParam, LPARAM lParam);
void w32akeybMakeChar(WPARAM wParam, LPARAM lParam);
void w32akeybMakeKey(UINT message, WPARAM wParam, LPARAM lParam);

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
