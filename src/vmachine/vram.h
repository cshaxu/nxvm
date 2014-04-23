/* This file is a part of NXVM project. */

// Random-access Memory

#ifndef NXVM_VRAM_H
#define NXVM_VRAM_H

#include "vglobal.h"

typedef struct {
	t_vaddrcc base;                         /* memory base address is 20 bit */
	t_nubitcc size;                                   /* memory size in byte */
} t_ram;

extern t_ram vram;

t_nubit8 vramGetByte(t_nubit16 segment,t_nubit16 offset);
t_nubit16 vramGetWord(t_nubit16 segment,t_nubit16 offset);
t_nubit32 vramGetDWord(t_nubit16 segment,t_nubit16 offset);
void vramSetByte(t_nubit16 segment,t_nubit16 offset,t_nubit8 value);
void vramSetWord(t_nubit16 segment,t_nubit16 offset,t_nubit16 value);
void vramSetDWord(t_nubit16 segment,t_nubit16 offset,t_nubit32 value);
t_vaddrcc vramGetAddress(t_vaddrcc immloc);
t_vaddrcc vramGetRealAddress(t_nubit16 segment,t_nubit16 offset);

void vramAlloc(t_nubitcc newsize);

void vramInit();
void vramRefresh();
void vramFinal();

#endif