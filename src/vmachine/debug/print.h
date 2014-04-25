/* Copyright 2012-2014 Neko. */

#ifndef NXVM_PRINT_H
#define NXVM_PRINT_H

#ifdef __cplusplus
extern "C" {
#endif

void printCpuSreg();
void printCpuCreg();
void printCpuReg();
void printCpuMem();
void printPic();
void printPit();
void printDma();
void printFdc();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
