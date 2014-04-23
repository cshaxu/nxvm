/* This file is a part of NXVM project. */

/* Random-access Memory */

#ifndef NXVM_VRAM_H
#define NXVM_VRAM_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "vglobal.h"

typedef struct {
	t_bool    flaga20;                            /* 0 = disable, 1 = enable */
	t_vaddrcc base;                         /* memory base address is 20 bit */
	t_nubitcc size;                                   /* memory size in byte */
} t_ram;

extern t_ram vram;

#define vramWrapA20(offset) ((offset) & ((vram.flaga20) ? 0xffffffff : 0xffefffff))
#define vramAddr(physical) (vram.base + (t_vaddrcc)(vramWrapA20(physical)))
#define vramByte(physical)  (d_nubit8(vramAddr(physical)))
#define vramWord(physical)  (d_nubit16(vramAddr(physical)))
#define vramDWord(physical) (d_nubit32(vramAddr(physical)))
#define vramQWord(physical) (d_nubit64(vramAddr(physical)))

/* macros below are designed for real-addressing mode */
#define vramIsAddrInMem(ref) \
	(((t_vaddrcc)(ref) >= vram.base) && ((t_vaddrcc)(ref) < (vram.base + vram.size)))
#define vramGetRealAddr(segment, offset)  (vram.base + \
	(((((t_nubit16)(segment) << 4) + (t_nubit16)(offset)) & \
	  (vram.flaga20 ? 0xffffffff : 0xffefffff)) % vram.size))
#define vramRealByte(segment, offset)  (d_nubit8(vramGetRealAddr(segment, offset)))
#define vramRealWord(segment, offset)  (d_nubit16(vramGetRealAddr(segment, offset)))
#define vramRealDWord(segment, offset) (d_nubit32(vramGetRealAddr(segment, offset)))


void vramAlloc(t_nubitcc newsize);
void vramInit();
void vramReset();
void vramRefresh();
void vramFinal();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
