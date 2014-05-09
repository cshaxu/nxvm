/* This file is a part of NXVM project. */

#include "memory.h"

#include "vbios.h"

/*
 * http://zouchzx.blog.163.com/blog/static/182366495201172842349614/
 * http://www.bioscentral.com/misc/bda.htm
 * http://stanislavs.org/helppc/bios_data_area.html
 */

#define VBIOS_ADDR_INT_VECTOR_TABLE   0x0000
#define VBIOS_ADDR_BOOT_STACK         0x0300
#define VBIOS_ADDR_KEYB_PRTSCR_STATUS 0x0500

#define VBIOS_ADDR_BOOT_CODE          0x07c00
/*
#define VBIOS_ADDR_VGA_GRAP_MODE_RAM  0xa0000
#define VBIOS_ADDR_MDA_DISPLAY_RAM    0xb0000
#define VBIOS_ADDR_CGA_DISPLAY_RAM    0xb8000
#define VBIOS_ADDR_VGA_BIOS_ROM       0xc0000
#define VBIOS_ADDR_VGA_ADAPT_ROM      0xc4000
*/
#define VBIOS_ADDR_ROM_GRAP_CHR_TABLE 0xffa6e
#define VBIOS_ADDR_ROM_BOOTSTRAP_CODE 0xffff0
#define VBIOS_ADDR_ROM_DATE           0xffff5
#define VBIOS_ADDR_ROM_MACHINE_ID     0xffffe

void vbiosInit() {
}

void vbiosFinal() {
}
