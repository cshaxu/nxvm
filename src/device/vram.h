/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VRAM_H
#define NXVM_VRAM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vglobal.h"

#define NXVM_DEVICE_RAM "Unknown Random-access Memory"

typedef struct {
	t_bool flagA20; /* 0 = disable, 1 = enable */
	t_vaddrcc pBase; /* memory base address is 20 bit */
	t_nubitcc size; /* memory size in byte */
} t_ram;

extern t_ram vram;

#define VRAM_BIT_A20  0x00100000
#define VRAM_FLAG_A20 0x02

#define vramSize vram.size

#define vramWrapA20(offset) ((offset) & (vram.flagA20 ? Max32 : ~VRAM_BIT_A20))
#define vramAddr(physical)  (vram.pBase + (t_vaddrcc)(vramWrapA20(physical)))
#define vramByte(physical)  (d_nubit8(vramAddr(physical)))
#define vramWord(physical)  (d_nubit16(vramAddr(physical)))
#define vramDWord(physical) (d_nubit32(vramAddr(physical)))
#define vramQWord(physical) (d_nubit64(vramAddr(physical)))

/* macros below are designed for real-addressing mode */
#define vramIsAddrInMem(ref) \
	(((t_vaddrcc)(ref) >= vram.pBase) && ((t_vaddrcc)(ref) < (vram.pBase + vram.size)))
#define vramGetRealAddr(segment, offset) (vram.pBase + \
	(vramWrapA20((GetMax16(segment) << 4) + GetMax16(offset)) % vram.size))

#define vramRealByte(segment, offset)  (d_nubit8(vramGetRealAddr(segment, offset)))
#define vramRealWord(segment, offset)  (d_nubit16(vramGetRealAddr(segment, offset)))
#define vramRealDWord(segment, offset) (d_nubit32(vramGetRealAddr(segment, offset)))
#define vramRealQWord(segment, offset) (d_nubit64(vramGetRealAddr(segment, offset)))

void vramRegister();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
