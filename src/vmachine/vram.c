/* This file is a part of NXVM project. */

#include "stdio.h"
#include "stdlib.h"
#include "memory.h"

#include "vport.h"

#include "vram.h"

t_ram vram;

void IO_Read_0092()
{
	vport.iobyte = vram.flaga20 ? 0x02 : 0x00;
}
void IO_Write_0092()
{
	vram.flaga20 = !!GetBit(vport.iobyte, 0x02);
}

void vramAlloc(t_nubitcc newsize)
{
	if (newsize) {
		vram.size = newsize;
		if (vram.base) free((void *)vram.base);
		vram.base = (t_vaddrcc)malloc(vram.size);
		memset((void *)vram.base, 0x00, vram.size);
	}
}
void vramInit()
{
	memset(&vram, 0x00, sizeof(t_ram));
	vport.in[0x0092] = (t_faddrcc)IO_Read_0092;
	vport.out[0x0092] = (t_faddrcc)IO_Write_0092;
	vramAlloc(1 << 22);
}
void vramReset()
{
	memset((void *)vram.base, 0x00, vram.size);
}
void vramRefresh() {}
void vramFinal()
{
	if (vram.base) free((void *)(vram.base));
}
