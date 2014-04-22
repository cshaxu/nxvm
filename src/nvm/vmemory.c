/* This file is a part of NekoVMac project. */

#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

#include "vmachine.h"
#include "vmemory.h"

void MemoryInit()
{
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