/* This file is a part of NekoVMac project. */

#include "stdio.h"
#include "stdlib.h"
#include "memory.h"
#include "vmachine.h"
#include "vmemory.h"
#include "vdisplay.h"

void MemoryInit()
{
	memoryBase = (t_vaddrcc)malloc(memorySize);
	memset((void *)memoryBase,0,memorySize);
}

void MemoryTest()
{
	t_nubit32 i;
	t_vaddrcc memPtr;
	memPtr = memoryBase;
	for(i = 0;i < memorySize;++i)
	{
		if(i % 1024 == 0) nvmprint("\rMemory Testing : %dK",i/1024);
		if(*(t_nubit8 *)memPtr != 0) {
			nvmprint("\nMemory test failed.\n");
			testFlag = 1;
			break;
		}
	}
	if(!testFlag) nvmprint("\rMemory Testing : %dK OK\n",i/1024);
	return;
}

void MemoryTerm()
{
	free((void *)memoryBase);
}