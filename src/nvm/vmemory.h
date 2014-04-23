/* This file is a part of NekoVMac project. */

#ifndef NVM_VMEMORY_H
#define NVM_VMEMORY_H

#include "vglobal.h"

extern t_vaddrcc memoryBase;	// memory base address is 20 bit
extern t_nubit32 memorySize;	// memory size in byte

void MemoryInit();
void MemoryTerm();

#endif