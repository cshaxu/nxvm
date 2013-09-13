/* This file is a part of NXVM project. */

/* Basic Input Output System: Fake BIOS Interrupts */

#ifndef NXVM_VBIOS_H
#define NXVM_VBIOS_H

#include "vglobal.h"

void BIOS_INT_00();

void vbiosInit();
void vbiosFinal();

#endif
