/* This file is a part of NXVM project. */

#ifndef NXVM_QDCGA_H
#define NXVM_QDCGA_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "../vvadp.h"
#include "../vram.h"

#define QDCGA_COUNT_MAX_PAGE               0x08
#define QDCGA_SIZE_CHAR_WIDTH              0x0a
#define QDCGA_SIZE_CHAR_HEIGHT             0x10
#define QDCGA_VBIOS_ADDR_VGA_MODE          0x0449
#define QDCGA_VBIOS_ADDR_VGA_ROW_SIZE      0x044a
#define QDCGA_VBIOS_ADDR_VGA_RAGEN_SIZE    0x044c
#define QDCGA_VBIOS_ADDR_VGA_PAGE_OFFSET   0x044e
#define QDCGA_VBIOS_ADDR_VGA_CURSOR_P0     0x0450
#define QDCGA_VBIOS_ADDR_VGA_CURSOR_P1     0x0452
#define QDCGA_VBIOS_ADDR_VGA_CURSOR_P2     0x0454
#define QDCGA_VBIOS_ADDR_VGA_CURSOR_P3     0x0456
#define QDCGA_VBIOS_ADDR_VGA_CURSOR_P4     0x0458
#define QDCGA_VBIOS_ADDR_VGA_CURSOR_P5     0x045a
#define QDCGA_VBIOS_ADDR_VGA_CURSOR_P6     0x045c
#define QDCGA_VBIOS_ADDR_VGA_CURSOR_P7     0x045e
#define QDCGA_VBIOS_ADDR_VGA_CURSOR_BOTTOM 0x0460
#define QDCGA_VBIOS_ADDR_VGA_CURSOR_TOP    0x0461
#define QDCGA_VBIOS_ADDR_VGA_ACT_PAGE_NUM  0x0462
#define QDCGA_SIZE_TEXT_MEMORY             0x1000
#define QDCGA_VBIOS_ADDR_CGA_DISPLAY_RAM_S 0xb800

#define qdcgaVarPageOffset \
	(vramVarWord(0x0000, QDCGA_VBIOS_ADDR_VGA_PAGE_OFFSET))
#define qdcgaVarRagenSize \
	(vramVarWord(0x0000, QDCGA_VBIOS_ADDR_VGA_RAGEN_SIZE))
#define qdcgaVarMode \
	(vramVarByte(0x0000, QDCGA_VBIOS_ADDR_VGA_MODE))
#define qdcgaVarRowSize \
	(vramVarWord(0x0000, QDCGA_VBIOS_ADDR_VGA_ROW_SIZE))
#define qdcgaVarPageNum \
	(vramVarByte(0x0000, QDCGA_VBIOS_ADDR_VGA_ACT_PAGE_NUM))
#define qdcgaGetTextMemAddr \
	(vramGetAddr(QDCGA_VBIOS_ADDR_CGA_DISPLAY_RAM_S, 0x0000))
#define qdcgaGetPageSize \
	(qdcgaVarRowSize * vvadp.colsize * 2)
#define qdcgaGetTextMemAddrPage(page) \
	(qdcgaGetTextMemAddr + (page) * qdcgaGetPageSize)
#define qdcgaGetTextMemAddrPageCur \
	(qdcgaGetTextMemAddrPage(qdcgaVarPageNum))

#define qdcgaVarChar(page,x,y) \
	(vramVarByte(QDCGA_VBIOS_ADDR_CGA_DISPLAY_RAM_S, \
	          (page) * qdcgaGetPageSize + ((x) * qdcgaVarRowSize + (y)) * 2 + 0))
#define qdcgaVarCharProp(page,x,y) \
	(vramVarByte(QDCGA_VBIOS_ADDR_CGA_DISPLAY_RAM_S, \
	          (page) * qdcgaGetPageSize + ((x) * qdcgaVarRowSize + (y)) * 2 + 1))
#define qdcgaGetCharAddr(page,x,y) \
	(qdcgaGetTextMemAddrPage(page) + ((x) * qdcgaVarRowSize + (y)) * 2 + 0)
#define qdcgaGetCharPropAddr(page,x,y) \
	(qdcgaGetTextMemAddrPage(page) + ((x) * qdcgaVarRowSize + (y)) * 2 + 1)

#define qdcgaVarCursorPosCol(id) \
	(vramVarByte(0x0000, QDCGA_VBIOS_ADDR_VGA_CURSOR_P0 + (id) * 2 + 0))
#define qdcgaVarCursorPosRow(id) \
	(vramVarByte(0x0000, QDCGA_VBIOS_ADDR_VGA_CURSOR_P0 + (id) * 2 + 1))
#define qdcgaVarCursorBottom \
	(vramVarByte(0x0000, QDCGA_VBIOS_ADDR_VGA_CURSOR_BOTTOM))
#define qdcgaVarCursorTop \
	(vramVarByte(0x0000, QDCGA_VBIOS_ADDR_VGA_CURSOR_TOP))
#define qdcgaGetCursorVisible \
	(!(qdcgaVarCursorTop & 0x08))

extern t_nubit32 qdcgaModeBufSize[0x14];

void qdcgaSetDisplayMode();
void qdcgaSetCursorShape();
void qdcgaSetCursorPos();
void qdcgaGetCursorPos();
void qdcgaSetDisplayPage();
void qdcgaScrollUp();
void qdcgaScrollDown();
void qdcgaGetCharProp();
void qdcgaDisplayCharProp();
void qdcgaDisplayChar();
void qdcgaGetAdapterStatus();
void qdcgaGenerateChar();
void qdcgaGetAdapterInfo();
void qdcgaDisplayStr();

void INT_10();

void qdcgaReset();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
