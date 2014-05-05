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
} t_ram_data;

typedef struct {
    t_vaddrcc pBase; /* memory base address is 20 bit */
    t_nubitcc size; /* memory size in byte */
} t_ram_connect;

typedef struct {
    t_ram_data data;
    t_ram_connect connect;
} t_ram;

extern t_ram vram;

#define VRAM_BIT_A20  0x00100000
#define VRAM_FLAG_A20 0x02

#define VRAM_WrapA20(offset)   ((offset) & (vram.data.flagA20 ? Max32 : ~VRAM_BIT_A20))
#define VRAM_GetAddr(physical) (vram.connect.pBase + (t_vaddrcc)(VRAM_WrapA20(physical)))

/* macros below are defined for real-addressing mode */
#define vramGetRealAddr(segment, offset) (vram.connect.pBase + \
    (VRAM_WrapA20((GetMax16(segment) << 4) + GetMax16(offset)) % vram.connect.size))

#define vramRealByte(segment, offset)  (d_nubit8(vramGetRealAddr(segment, offset)))
#define vramRealWord(segment, offset)  (d_nubit16(vramGetRealAddr(segment, offset)))
#define vramRealDWord(segment, offset) (d_nubit32(vramGetRealAddr(segment, offset)))

void vramReadPhysical(t_nubit32 physical, t_vaddrcc rdest, t_nubitcc size);
void vramWritePhysical(t_nubit32 physical, t_vaddrcc rsrc, t_nubitcc size);

void vramInit();
void vramReset();
void vramRefresh();
void vramFinal();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
