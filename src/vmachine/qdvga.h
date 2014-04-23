/* This file is a part of NXVM project. */

#ifndef NXVM_QDVGA_H
#define NXVM_QDVGA_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "vglobal.h"
#include "vram.h"

#define QDVGA_COUNT_MAX_PAGE               0x08
#define QDVGA_SIZE_CHAR_WIDTH              0x0a
#define QDVGA_SIZE_CHAR_HEIGHT             0x10
#define QDVGA_VBIOS_ADDR_VGA_MODE          0x0449
#define QDVGA_VBIOS_ADDR_VGA_ROW_SIZE      0x044a
#define QDVGA_VBIOS_ADDR_VGA_RAGEN_SIZE    0x044c
#define QDVGA_VBIOS_ADDR_VGA_PAGE_OFFSET   0x044e
#define QDVGA_VBIOS_ADDR_VGA_CURSOR_P0     0x0450
#define QDVGA_VBIOS_ADDR_VGA_CURSOR_P1     0x0452
#define QDVGA_VBIOS_ADDR_VGA_CURSOR_P2     0x0454
#define QDVGA_VBIOS_ADDR_VGA_CURSOR_P3     0x0456
#define QDVGA_VBIOS_ADDR_VGA_CURSOR_P4     0x0458
#define QDVGA_VBIOS_ADDR_VGA_CURSOR_P5     0x045a
#define QDVGA_VBIOS_ADDR_VGA_CURSOR_P6     0x045c
#define QDVGA_VBIOS_ADDR_VGA_CURSOR_P7     0x045e
#define QDVGA_VBIOS_ADDR_VGA_CURSOR_BOTTOM 0x0460
#define QDVGA_VBIOS_ADDR_VGA_CURSOR_TOP    0x0461
#define QDVGA_VBIOS_ADDR_VGA_ACT_PAGE_NUM  0x0462
#define QDVGA_SIZE_TEXT_MEMORY             0x1000
#define QDVGA_VBIOS_ADDR_CGA_DISPLAY_RAM   0xb8000

typedef struct {
	t_bool   color;
	t_nubit8 colsize; // char per col
} t_vga;

#define qdvgaVarPageOffset \
	(vramVarWord(0x0000, QDVGA_VBIOS_ADDR_VGA_PAGE_OFFSET))
#define qdvgaVarRagenSize \
	(vramVarWord(0x0000, QDVGA_VBIOS_ADDR_VGA_RAGEN_SIZE))
#define qdvgaVarMode \
	(vramVarByte(0x0000, QDVGA_VBIOS_ADDR_VGA_MODE))
#define qdvgaVarRowSize \
	(vramVarWord(0x0000, QDVGA_VBIOS_ADDR_VGA_ROW_SIZE))
#define qdvgaVarPageNum \
	(vramVarByte(0x0000, QDVGA_VBIOS_ADDR_VGA_ACT_PAGE_NUM))
#define qdvgaGetTextMemAddr \
	(vramGetAddr(0x0000, QDVGA_VBIOS_ADDR_CGA_DISPLAY_RAM))
#define qdvgaGetPageSize \
	(qdvgaVarRowSize * qdvga.colsize * 2)
#define qdvgaGetTextMemAddrPage(page) \
	(qdvgaGetTextMemAddr + (page) * qdvgaGetPageSize)
#define qdvgaGetTextMemAddrPageCur \
	(qdvgaGetTextMemAddrPage(qdvgaVarPageNum))

#define qdvgaVarChar(page,x,y) \
	(vramVarByte(0x0000, QDVGA_VBIOS_ADDR_CGA_DISPLAY_RAM + \
	          (page) * qdvgaGetPageSize + ((x) * qdvgaVarRowSize + (y)) * 2 + 0))
#define qdvgaVarCharProp(page,x,y) \
	(vramVarByte(0x0000, QDVGA_VBIOS_ADDR_CGA_DISPLAY_RAM + \
	          (page) * qdvgaGetPageSize + ((x) * qdvgaVarRowSize + (y)) * 2 + 1))
#define qdvgaGetCharAddr(page,x,y) \
	(qdvgaGetTextMemAddrPage(page) + ((x) * qdvgaVarRowSize + (y)) * 2 + 0)
#define qdvgaGetCharPropAddr(page,x,y) \
	(qdvgaGetTextMemAddrPage(page) + ((x) * qdvgaVarRowSize + (y)) * 2 + 1)

#define qdvgaVarCursorPosCol(id) \
	(vramVarByte(0x0000, QDVGA_VBIOS_ADDR_VGA_CURSOR_P0 + (id) * 2 + 0))
#define qdvgaVarCursorPosRow(id) \
	(vramVarByte(0x0000, QDVGA_VBIOS_ADDR_VGA_CURSOR_P0 + (id) * 2 + 1))
#define qdvgaVarCursorBottom \
	(vramVarByte(0x0000, QDVGA_VBIOS_ADDR_VGA_CURSOR_BOTTOM))
#define qdvgaVarCursorTop \
	(vramVarByte(0x0000, QDVGA_VBIOS_ADDR_VGA_CURSOR_TOP))
#define qdvgaGetCursorVisible \
	(!GetBit(qdvgaVarCursorTop & 0x08))

extern t_vga qdvga;

//void qdvgaCheckVideoRam(t_vaddrcc addr);
void qdvgaSetDisplayMode();
void qdvgaSetCursorShape();
void qdvgaSetCursorPos();
void qdvgaGetCursorPos();
void qdvgaSetDisplayPage();
void qdvgaScrollUp();
void qdvgaScrollDown();
void qdvgaGetCharProp();
void qdvgaDisplayCharProp();
void qdvgaDisplayChar();
//void qdvgaSetPalette();
//void qdvgaDisplayPixel();
//void qdvgaGetPixel();
void qdvgaGetAdapterStatus();
void qdvgaGenerateChar();
void qdvgaGetAdapterInfo();
void qdvgaDisplayStr();
void qdvgaInit();
void qdvgaFinal();

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
