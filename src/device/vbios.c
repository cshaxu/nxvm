/* Copyright 2012-2014 Neko. */

/* VBIOS loads bios data, interrupt routines and post routines for all devices. */

#include "../utils.h"

#include "vram.h"
#include "vhdd.h"

#include "vbios.h"

t_bios vbios;

/* internal variable */
static t_bool flagBoot;
static t_nubit16 ics, iip;

static t_nubit32 assemble(const t_strptr stmt, t_nubit16 seg, t_nubit16 off) {
    t_nubit32 len = 0;
    t_nubit8 *code = NULL;
    t_nubitcc i;
    t_nubitcc insCount = 0; /* the number of instructions to be assembled */
    for (i = 0; i < strlen(stmt); ++i) {
        if (stmt[i] == '\n') {
            insCount++;
        }
    }
    if (strlen(stmt)) insCount++;
    /* 15 is the maximum length of each instruction */
    code = p_nubit8(MALLOC(15 * insCount * sizeof(t_nubit8)));
    len = utilsAasm32x(stmt, code, False);
    if (!len) {
        PRINTF("vbios: invalid x86 assembly instruction.\n");
    }
    MEMCPY((void *) vramGetRealAddr(seg, off), (void *) code, len);
    if (code) {
        FREE((void *) code);
    }
    return len;
}

static void biosLoadData() {
    MEMSET((void *) vramGetRealAddr(0x0040, Zero16), Zero8, 0x100);
    vramRealWord(Zero16, VBIOS_ADDR_SERI_PORT_COM1) = 0x03f8;
    vramRealWord(Zero16, VBIOS_ADDR_PARA_PORT_LPT1) = 0x0378;
    vramRealWord(Zero16, VBIOS_ADDR_PARA_PORT_LPT4) = 0x9fc0;
    vramRealWord(Zero16, VBIOS_ADDR_EQUIP_FLAG)     = 0x0021;
    vramRealWord(Zero16, VBIOS_ADDR_RAM_SIZE)       = 0x027f;
    vramRealByte(Zero16, VBIOS_ADDR_KEYB_FLAG0)     = 0x20;
    vramRealWord(Zero16, VBIOS_ADDR_KEYB_BUF_HEAD)  = 0x041e;
    vramRealWord(Zero16, VBIOS_ADDR_KEYB_BUF_TAIL)  = 0x041e;
    /* vramRealByte(0x0040, 0x001e) = 0x1c;
    vramRealByte(0x0040, 0x001f) = 0x0d;
    vramRealByte(0x0040, 0x0020) = 0x1c;
    vramRealByte(0x0040, 0x0021) = 0x0d;
    vramRealByte(0x0040, 0x0022) = 0x22;
    vramRealByte(0x0040, 0x0023) = 0x67; */
    vramRealByte(Zero16, VBIOS_ADDR_FDD_CALI_FLAG)     = 0x01;
    vramRealByte(Zero16, VBIOS_ADDR_FDD_MOTOR_TIMEOUT) = 0x25;
    vramRealByte(Zero16, VBIOS_ADDR_FDD_STATUS)        = 0x09;
    vramRealByte(Zero16, VBIOS_ADDR_FDC_CYLINDER)      = 0x01;
    vramRealByte(Zero16, VBIOS_ADDR_FDC_SECTOR)        = 0x01;
    vramRealByte(Zero16, VBIOS_ADDR_FDC_BYTE_COUNT)    = 0x02;
    vramRealByte(Zero16, VBIOS_ADDR_VGA_VIDEO_MODE)    = 0x03;
    vramRealWord(Zero16, VBIOS_ADDR_VGA_COLUMN)        = 0x0050;
    vramRealWord(Zero16, VBIOS_ADDR_VGA_PAGE_SIZE)     = 0x1000;
    vramRealWord(Zero16, VBIOS_ADDR_VGA_CURSOR_P0)     = 0x0500;
    vramRealByte(Zero16, VBIOS_ADDR_VGA_CURSOR_BOTTOM) = 0x0e;
    vramRealByte(Zero16, VBIOS_ADDR_VGA_CURSOR_TOP)    = 0x0d;
    vramRealWord(Zero16, VBIOS_ADDR_VGA_ACT_ADPT_PORT) = 0x03d4;
    vramRealByte(Zero16, VBIOS_ADDR_VGA_MODE_REGISTER) = 0x29;
    vramRealByte(Zero16, VBIOS_ADDR_VGA_COLOR_PALETTE) = 0x30;
    vramRealByte(Zero16, VBIOS_ADDR_HDD_LST_OP_STATUS) = 0x01;
    vramRealByte(Zero16, VBIOS_ADDR_HDD_NUMBER) = vhdd.connect.flagDiskExist ? 0x01 : Zero8; /* number of hard disks */
    vramRealByte(Zero16, VBIOS_ADDR_HDD_CONTROL)        = 0xc0;
    vramRealByte(Zero16, VBIOS_ADDR_PARA_TIMEOUT_LPT1)  = 0x14;
    vramRealByte(Zero16, VBIOS_ADDR_SERI_TIMEOUT_COM1)  = 0x0a;
    vramRealWord(Zero16, VBIOS_ADDR_KEYB_BUFFER_START)  = 0x041e;
    vramRealWord(Zero16, VBIOS_ADDR_KEYB_BUFFER_END)    = 0x043d;
    vramRealByte(Zero16, VBIOS_ADDR_VGA_ROW_NUMBER)     = 0x18;
    vramRealWord(Zero16, VBIOS_ADDR_VGA_CHAR_HEIGHT)    = 0x0010;
    vramRealByte(Zero16, VBIOS_ADDR_VGA_MODE_OPTIONS1)  = 0x60;
    vramRealByte(Zero16, VBIOS_ADDR_VGA_MODE_OPTIONS2)  = 0x09;
    vramRealByte(Zero16, VBIOS_ADDR_VGA_DISPLAY_DATA)   = 0x11;
    vramRealByte(Zero16, VBIOS_ADDR_VGA_DCC_INDEX)      = 0x0b;
    vramRealByte(Zero16, VBIOS_ADDR_DRV_SAME_FLAG)      = 0x77;
    vramRealByte(Zero16, VBIOS_ADDR_DRV_MEDIA_STATE_D0) = 0x17;
    vramRealByte(Zero16, VBIOS_ADDR_KEYB_MODE_TYPE)     = 0x10;
    vramRealByte(Zero16, VBIOS_ADDR_KEYB_LED_FLAG)      = 0x02;
    vramRealDWord(Zero16, VBIOS_ADDR_VGA_VIDEO_TAB_PTR) = 0xc0005d3a;
    vramRealByte(Zero16, VBIOS_ADDR_POST_WORK_AREA) = flagBoot ? 0x80 : Zero8; /* boot disk */
}
static void biosLoadRomInfo() {
    vramRealWord(VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 0) = 0x0008;
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 2) = 0xfc;
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 3) = Zero8;
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 4) = 0x01;
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 5) = 0xb4;
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 6) = 0x40;
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 7) = Zero8;
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 8) = Zero8;
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 9) = Zero8;
}
static void biosLoadInt() {
    t_nubitcc i;
    iip += (t_nubit16) assemble("iret", VBIOS_ADDR_START_SEG, VBIOS_ADDR_START_OFF);
    for (i = 0; i < 0x100; ++i) {
        if (vbios.connect.intTable[i]) {
            vramRealWord(Zero16, i * 4 + 0) = iip;
            vramRealWord(Zero16, i * 4 + 2) = ics;
            iip += (t_nubit16) assemble(vbios.connect.intTable[i], ics, iip);
        } else {
            vramRealWord(Zero16, i * 4 + 0) = VBIOS_ADDR_START_OFF;
            vramRealWord(Zero16, i * 4 + 2) = VBIOS_ADDR_START_SEG;
        }
    }
}
static void biosLoadPost() {
    t_nubitcc i;
    t_string stmt;
    SPRINTF(stmt, "jmp %04x:%04x", ics, iip);
    assemble(stmt, VBIOS_ADDR_POST_SEG, VBIOS_ADDR_POST_OFF);
    for (i = 0; i < vbios.connect.postCount; ++i) {
        iip += (t_nubit16) assemble(vbios.connect.postTable[i], ics, iip);
    }
    iip += (t_nubit16) assemble(VBIOS_POST_BOOT, ics, iip);
}
static void biosLoadAdditional() {
    /* hard disk param table */
    vramRealWord(Zero16, VBIOS_ADDR_HDD_PARAM_OFFSET) = VBIOS_ADDR_HDD_PARAM;
    vramRealWord(Zero16, VBIOS_ADDR_HDD_PARAM_SEGMENT) = VBIOS_ADDR_START_SEG;
    vramRealWord(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM +  0) = vhdd.data.ncyl;
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM +  2) = GetMax8(vhdd.data.nhead);
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM +  3) = 0xa0;
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM +  4) = GetMax8(vhdd.data.nsector);
    vramRealWord(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM +  5) = Max16;
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM +  7) = Zero8;
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM +  8) = 0x08;
    vramRealWord(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM +  9) = vhdd.data.ncyl;
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 11) = GetMax8(vhdd.data.nhead);
    vramRealWord(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 12) = Zero16;
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 14) = GetMax8(vhdd.data.nsector);
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 15) = Zero8;
}

void vbiosAddPost(t_strptr stmt) {
    vbios.connect.postTable[vbios.connect.postCount++] = stmt;
}
void vbiosAddInt(t_strptr stmt, t_nubit8 intid) {
    vbios.connect.intTable[intid] = stmt;
}
void vbiosInit() {
    MEMSET((void *)(&vbios), Zero8, sizeof(t_bios));
    flagBoot = False;
    ics = iip = Zero16;
    vbiosAddInt(VBIOS_INT_SOFT_MISC_11, 0x11);
    vbiosAddInt(VBIOS_INT_SOFT_MISC_12, 0x12);
    vbiosAddInt(VBIOS_INT_SOFT_MISC_15, 0x15);
}

/* Loads bios to ram */
void vbiosReset() {
    /* bios area starts at f000:0000 */
    ics = VBIOS_ADDR_START_SEG;
    iip = VBIOS_ADDR_START_OFF;
    biosLoadData();       /* bios data area */
    biosLoadRomInfo();    /* bios rom info area */
    biosLoadInt();        /* bios interrupt services */
    biosLoadPost();       /* bios init/post routines */
    biosLoadAdditional(); /* additional bios data */
}
void vbiosRefresh() {}
void vbiosFinal() {}
void devicePrintBios() {
    PRINTF("Boot Disk: %s\n", flagBoot ? "Hard Drive" : "Floppy");
}

void deviceConnectBiosSetBoot(int flagHdd) {
    flagBoot = flagHdd;
}
int deviceConnectBiosGetBoot() {
    return flagBoot;
}
