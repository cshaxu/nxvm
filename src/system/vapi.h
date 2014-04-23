/* This file is a part of NXVM project. */

#ifndef NXVM_VAPI_H
#define NXVM_VAPI_H

int vapiPrint(const char *, ...);
void vapiPrintByte(unsigned char n);
void vapiPrintWord(unsigned short n);
void vapiPrintAddr(unsigned short segment,unsigned short offset);
void vapiPause();

void vapiDisplayPaint();
void vapiCreateDisplay();
void vapiCreateKernel();

void vapiInsertFloppyDisk(const char *fname);
void vapiRemoveFloppyDisk(const char *fname);

#endif