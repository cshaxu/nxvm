/* This file is a part of NXVM project. */

#ifndef NXVM_VAPI_H
#define NXVM_VAPI_H

#include "../vglobal.h"
extern int forceNone;	// use general output or system_related vdisplay
int vapiPrint(const char *, ...);
void vapiPrintByte(unsigned char n);
void vapiPrintWord(unsigned short n);
void vapiPrintAddr(unsigned short segment,unsigned short offset);
void vapiPause();

void vapiInsertFloppyDisk(const char *fname);
void vapiRemoveFloppyDisk(const char *fname);

#endif