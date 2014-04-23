/* This file is a part of NXVM project. */

#ifndef NXVM_VAPI_H
#define NXVM_VAPI_H

#include "../vglobal.h"
extern int forceNone;	// use general output or system_related vdisplay
int vapiPrint(const char *, ...);
void vapiPrintByte(t_nubit8 n);
void vapiPrintWord(t_nubit16 n);
void vapiPrintAddr(t_nubit16 segment,t_nubit16 offset);
void vapiPause();

#endif