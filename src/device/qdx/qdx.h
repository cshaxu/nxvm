/* Copyright 2012-2014 Neko. */

#ifndef NXVM_QDX_H
#define NXVM_QDX_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../vglobal.h"

extern t_faddrcc qdxTable[0x100];

void qdxExecInt(t_nubit8 intId);

void qdxInit();
void qdxReset();
void qdxRefresh();
void qdxFinal();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
