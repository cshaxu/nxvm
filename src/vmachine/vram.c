/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

#include "vram.h"

t_vaddrcc vrambase;
t_nubit32 vramsize = 0;

t_nubit8 vramGetByte(t_nubit16 segment,t_nubit16 offset)
{return *(t_nubit8 *)(vrambase+SHL4(segment)+offset);}
t_nubit16 vramGetWord(t_nubit16 segment,t_nubit16 offset)
{return *(t_nubit16 *)(vrambase+SHL4(segment)+offset);}
t_nubit32 vramGetDWord(t_nubit16 segment,t_nubit16 offset)
{return *(t_nubit32 *)(vrambase+SHL4(segment)+offset);}
void vramSetByte(t_nubit16 segment,t_nubit16 offset,t_nubit8 value)
{*(t_nubit8 *)(vrambase+SHL4(segment)+offset) = value;}
void vramSetWord(t_nubit16 segment,t_nubit16 offset,t_nubit16 value)
{*(t_nubit16 *)(vrambase+SHL4(segment)+offset) = value;}
void vramSetDWord(t_nubit16 segment,t_nubit16 offset,t_nubit32 value)
{*(t_nubit32 *)(vrambase+SHL4(segment)+offset) = value;}
t_vaddrcc vramGetAddress(t_nubit16 segment,t_nubit16 offset)
{return (vrambase+SHL4(segment)+offset);}

void RAMInit()
{
	if(vrambase) RAMTerm();
	if(!vramsize) vramsize = 1 << 20;	// 1MBytes
	vrambase = (t_vaddrcc)malloc(vramsize);
	memset((void *)vrambase,0,vramsize);
}
void RAMTerm()
{
	if(vrambase) {
		free((void *)vrambase);
		vrambase = (t_vaddrcc)NULL;
	}
}