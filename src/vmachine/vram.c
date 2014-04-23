/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

#include "vram.h"

t_ram vram;

t_nubit8 vramGetByte(t_nubit16 segment,t_nubit16 offset)
{return *(t_nubit8 *)(vram.base+SHL4(segment)+offset);}
t_nubit16 vramGetWord(t_nubit16 segment,t_nubit16 offset)
{return *(t_nubit16 *)(vram.base+SHL4(segment)+offset);}
t_nubit32 vramGetDWord(t_nubit16 segment,t_nubit16 offset)
{return *(t_nubit32 *)(vram.base+SHL4(segment)+offset);}
void vramSetByte(t_nubit16 segment,t_nubit16 offset,t_nubit8 value)
{*(t_nubit8 *)(vram.base+SHL4(segment)+offset) = value;}
void vramSetWord(t_nubit16 segment,t_nubit16 offset,t_nubit16 value)
{*(t_nubit16 *)(vram.base+SHL4(segment)+offset) = value;}
void vramSetDWord(t_nubit16 segment,t_nubit16 offset,t_nubit32 value)
{*(t_nubit32 *)(vram.base+SHL4(segment)+offset) = value;}
t_vaddrcc vramGetAddress(t_vaddrcc immloc)
{return (vram.base+immloc);}
t_vaddrcc vramGetRealAddress(t_nubit16 segment,t_nubit16 offset)
{return (vram.base+SHL4(segment)+offset);}

void vramAlloc(t_nubitcc newsize)
{
	if (vram.base) vramFinal();
	if (newsize) {
		vram.size = newsize;
		vram.base = (t_vaddrcc)malloc(vram.size);
		memset((void *)vram.base, 0xcd, vram.size);
	}
}
void vramInit()
{
	memset(&vram, 0x00, sizeof(t_ram));
	vramAlloc(1<<20);
}
void vramRefresh() {}
void vramFinal()
{
	if (vram.base) {
		free((void *)vram.base);
		vram.base = (t_vaddrcc)NULL;
	}
}