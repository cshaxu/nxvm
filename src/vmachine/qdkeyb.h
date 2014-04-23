#ifndef NXVM_QDKEYB_H
#define NXVM_QDKEYB_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vglobal.h"
#include "vram.h"

#define QDKEYB_VBIOS_ADDR_KEYB_FLAG0         0x0417
#define QDKEYB_VBIOS_ADDR_KEYB_FLAG1         0x0418
#define QDKEYB_VBIOS_ADDR_KEYB_BUF_HEAD      0x041a
#define QDKEYB_VBIOS_ADDR_KEYB_BUF_TAIL      0x041c
#define QDKEYB_VBIOS_ADDR_KEYB_BUFFER_START  0x041e
#define QDKEYB_VBIOS_ADDR_KEYB_BUFFER_END    0x043d

#define QDKEYB_FLAG0_A_INSERT  0x08
#define QDKEYB_FLAG0_A_CAPLCK  0x04
#define QDKEYB_FLAG0_A_NUMLCK  0x02
#define QDKEYB_FLAG0_A_SCRLCK  0x01
#define QDKEYB_FLAG0_D_ALT     0x08
#define QDKEYB_FLAG0_D_CTRL    0x04
#define QDKEYB_FLAG0_D_LSHIFT  0x02
#define QDKEYB_FLAG0_D_RSHIFT  0x01

#define QDKEYB_FLAG1_D_INSERT  0x80
#define QDKEYB_FLAG1_D_CAPLCK  0x40
#define QDKEYB_FLAG1_D_NUMLCK  0x20
#define QDKEYB_FLAG1_D_SCRLCK  0x10
#define QDKEYB_FLAG1_D_PAUSE   0x08
#define QDKEYB_FLAG1_D_SYSRQ   0x04
#define QDKEYB_FLAG1_D_LALT    0x02
#define QDKEYB_FLAG1_D_LCTRL   0x01

void IO_Read_0064();

void qdkeybReadInput();
void qdkeybGetStatus();
void qdkeybGetShiftStatus();

#define qdkeybVarFlag0 (vramVarByte(0x0000,QDKEYB_VBIOS_ADDR_KEYB_FLAG0))
#define qdkeybVarFlag1 (vramVarByte(0x0000,QDKEYB_VBIOS_ADDR_KEYB_FLAG1))

void qdkeybInit();
void qdkeybFinal();

#ifdef __cplusplus
}
#endif

#endif