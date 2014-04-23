/* This file is a part of NXVM project. */

// CMOS; Real Time Clock: DS1302

#ifndef NXVM_VCMOS_H
#define NXVM_VCMOS_H

#include "vglobal.h"

extern t_nubit8 vcmosreg[0x40];

void CMOSInit();
void CMOSTerm();

#endif