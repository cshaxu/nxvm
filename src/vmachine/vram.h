/* This file is a part of NXVM project. */

/* Random-access Memory */

#ifndef NXVM_VRAM_H
#define NXVM_VRAM_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "vglobal.h"

typedef struct {
	t_vaddrcc base;                         /* memory base address is 20 bit */
	t_nubitcc size;                                   /* memory size in byte */
} t_ram;

extern t_ram vram;

#define vramIsAddrInMem(addr) \
	(((t_vaddrcc)(addr) >= vram.base) && ((t_vaddrcc)(addr) < (vram.base + vram.size)))
#ifdef ECPUACT
#define vramGetAddr(segment, offset)  (vram.base + \
	((((segment) << 4) + (offset)) % vram.size))
#else
#define vramGetAddr(segment, offset)  (vram.base + \
	((((t_nubit16)(segment) << 4) + (t_nubit16)(offset)) % vram.size))
#endif
#define vramVarByte(segment, offset)  (d_nubit8(vramGetAddr(segment, offset)))
#define vramVarWord(segment, offset)  (d_nubit16(vramGetAddr(segment, offset)))
#define vramVarDWord(segment, offset) (d_nubit32(vramGetAddr(segment, offset)))

void vramAlloc(t_nubitcc newsize);
void vramInit();
void vramReset();
void vramRefresh();
void vramFinal();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
