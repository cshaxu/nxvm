/* This file is a part of NXVM project. */

#ifndef NXVM_VAPI_H
#define NXVM_VAPI_H

int vapiPrint(const char *, ...);
void vapiPrintByte(unsigned char n);
void vapiPrintWord(unsigned short n);
void vapiPrintAddr(unsigned short segment,unsigned short offset);
void vapiPause();
void vapiSleep(unsigned int milisec);

void vapiDisplayInit();
void vapiDisplaySetScreen();
void vapiDisplayPaint();
void vapiDisplayFinal();

void vapiCreateDisplay();
void vapiCreateKernel();

void vapiFloppyInsert(const char *fname);
void vapiFloppyRemove(const char *fname);

#endif