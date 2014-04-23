#ifndef NXVM_QDKEYB_H
#define NXVM_QDKEYB_H

#include "vglobal.h"

void IO_Read_0064();

void qdkeybReadInput();
void qdkeybGetStatus();
void qdkeybGetShift();

void qdkeybInit();
void qdkeybFinal();

#endif