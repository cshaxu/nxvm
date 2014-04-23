/* This file is a part of NXVM project. */

// Random-access Memory

#ifndef NXVM_VMEMORY_H
#define NXVM_VMEMORY_H

#include "vglobal.h"

extern t_vaddrcc vrambase;	// memory base address is 20 bit
extern t_nubit32 vramsize;	// memory size in byte

t_nubit8 vramGetByte(t_nubit16 segment,t_nubit16 offset);
t_nubit16 vramGetWord(t_nubit16 segment,t_nubit16 offset);
t_nubit32 vramGetDWord(t_nubit16 segment,t_nubit16 offset);
void vramSetByte(t_nubit16 segment,t_nubit16 offset,t_nubit8 value);
void vramSetWord(t_nubit16 segment,t_nubit16 offset,t_nubit16 value);
void vramSetDWord(t_nubit16 segment,t_nubit16 offset,t_nubit32 value);
t_vaddrcc vramGetAddress(t_nubit16 segment,t_nubit16 offset);

void RAMInit();
void RAMTerm();

#endif