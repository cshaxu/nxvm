/* This file is a part of NekoVMac project. */

#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

#include "vmemory.h"

t_vaddrcc memoryBase;
t_nubit32 memorySize = 0;

void MemoryInit()
{
	if(memoryBase) MemoryTerm();
	if(!memorySize) memorySize = 1 << 20;	// 1MBytes
	memoryBase = (t_vaddrcc)malloc(memorySize);
	memset((void *)memoryBase,0,memorySize);
}

void MemoryTerm()
{
	if(memoryBase) {
		free((void *)memoryBase);
		memoryBase = NULL;
	}
}