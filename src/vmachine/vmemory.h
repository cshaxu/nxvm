/* This file is a part of NVMx86 project. */

#ifndef NVM_VMEMORY_H
#define NVM_VMEMORY_H

#include "vglobal.h"

extern t_vaddrcc memoryBase;	// memory base address is 20 bit
extern t_nubit32 memorySize;	// memory size in byte

t_nubit8 vmemoryGetByte(t_nubit16 segment,t_nubit16 offset);
t_nubit16 vmemoryGetWord(t_nubit16 segment,t_nubit16 offset);
t_nubit32 vmemoryGetDWord(t_nubit16 segment,t_nubit16 offset);
void vmemorySetByte(t_nubit16 segment,t_nubit16 offset,t_nubit8 value);
void vmemorySetWord(t_nubit16 segment,t_nubit16 offset,t_nubit16 value);
void vmemorySetDWord(t_nubit16 segment,t_nubit16 offset,t_nubit32 value);
t_vaddrcc vmemoryGetAddress(t_nubit16 segment,t_nubit16 offset);

void MemoryInit();
void MemoryTerm();

#endif