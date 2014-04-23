/* This file is a part of NVMx86 project. */

#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

#include "vmemory.h"
#include "system/vapi.h"

t_vaddrcc memoryBase;
t_nubit32 memorySize = 0;

t_nubit8 vmemoryGetByte(t_nubit16 segment,t_nubit16 offset)
{return *(t_nubit8 *)(memoryBase+SHL4(segment)+offset);}
t_nubit16 vmemoryGetWord(t_nubit16 segment,t_nubit16 offset)
{return *(t_nubit16 *)(memoryBase+SHL4(segment)+offset);}
t_nubit32 vmemoryGetDWord(t_nubit16 segment,t_nubit16 offset)
{return *(t_nubit32 *)(memoryBase+SHL4(segment)+offset);}
void vmemorySetByte(t_nubit16 segment,t_nubit16 offset,t_nubit8 value)
{*(t_nubit8 *)(memoryBase+SHL4(segment)+offset) = value;}
void vmemorySetWord(t_nubit16 segment,t_nubit16 offset,t_nubit16 value)
{*(t_nubit16 *)(memoryBase+SHL4(segment)+offset) = value;}
void vmemorySetDWord(t_nubit16 segment,t_nubit16 offset,t_nubit32 value)
{*(t_nubit32 *)(memoryBase+SHL4(segment)+offset) = value;}
t_vaddrcc vmemoryGetAddress(t_nubit16 segment,t_nubit16 offset)
{return (memoryBase+SHL4(segment)+offset);}

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
		memoryBase = (t_vaddrcc)NULL;
	}
}