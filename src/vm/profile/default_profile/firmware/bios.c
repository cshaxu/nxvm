/* Copyright 2012-2014 Neko. */

/* VBIOS loads bios data, interrupt routines and post routines for all devices. */

#include "core/product/utils.h"

#include "core/machine/memory.h"
#include "core/machine/block_interface.h"
#include "core/machine/block_provider.h"

#include "bios.h"

static t_nubit32 assemble(t_ram *ram, const t_strptr stmt, t_nubit16 seg,
    t_nubit16 off) {
    t_nubit32 len = 0;
    t_nubit8 *code = NULL;
    t_nubitcc i;
    t_nubitcc insCount = 0; /* the number of instructions to be assembled */
    for (i = 0; i < STRLEN(stmt); ++i) {
        if (stmt[i] == '\n') {
            insCount++;
        }
    }
    if (STRLEN(stmt)) insCount++;
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

static void bios_load_data(t_bios *bios, t_ram *ram,
    const core_machine_block_provider_slot *block_provider) {
    core_machine_block_geometry geometry;
    core_machine_block_get_geometry_from(block_provider, &geometry);
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
    vramRealByte(Zero16, VBIOS_ADDR_HDD_NUMBER) = geometry.present ? 0x01 : Zero8; /* number of hard disks */
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
    vramRealByte(Zero16, VBIOS_ADDR_POST_WORK_AREA) = bios->flagBoot ? 0x80 : Zero8; /* boot disk */
}
static void bios_load_rom_info(t_ram *ram) {
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
static void bios_load_interrupts(t_bios *bios, t_ram *ram) {
    t_nubitcc i;
    bios->data.buildIP += (t_nubit16)assemble(ram, "iret",
        VBIOS_ADDR_START_SEG, VBIOS_ADDR_START_OFF);
    for (i = 0; i < 0x100; ++i) {
        if (bios->connect.intTable[i]) {
            vramRealWord(Zero16, i * 4 + 0) = bios->data.buildIP;
            vramRealWord(Zero16, i * 4 + 2) = bios->data.buildCS;
            bios->data.buildIP += (t_nubit16)assemble(ram,
                bios->connect.intTable[i], bios->data.buildCS,
                bios->data.buildIP);
        } else {
            vramRealWord(Zero16, i * 4 + 0) = VBIOS_ADDR_START_OFF;
            vramRealWord(Zero16, i * 4 + 2) = VBIOS_ADDR_START_SEG;
        }
    }
}
static void bios_load_post(t_bios *bios, t_ram *ram) {
    t_nubitcc i;
    t_string stmt;
    SPRINTF(stmt, "jmp %04x:%04x", bios->data.buildCS, bios->data.buildIP);
    assemble(ram, stmt, VBIOS_ADDR_POST_SEG, VBIOS_ADDR_POST_OFF);
    for (i = 0; i < bios->connect.postCount; ++i) {
        bios->data.buildIP += (t_nubit16)assemble(ram,
            bios->connect.postTable[i], bios->data.buildCS, bios->data.buildIP);
    }
    bios->data.buildIP += (t_nubit16)assemble(ram, VBIOS_POST_BOOT,
        bios->data.buildCS, bios->data.buildIP);
}
static void bios_load_additional(t_ram *ram,
    const core_machine_block_provider_slot *block_provider) {
    core_machine_block_geometry geometry;
    core_machine_block_get_geometry_from(block_provider, &geometry);
    /* hard disk param table */
    vramRealWord(Zero16, VBIOS_ADDR_HDD_PARAM_OFFSET) = VBIOS_ADDR_HDD_PARAM;
    vramRealWord(Zero16, VBIOS_ADDR_HDD_PARAM_SEGMENT) = VBIOS_ADDR_START_SEG;
    vramRealWord(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM +  0) = geometry.cylinders;
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM +  2) = GetMax8(geometry.heads);
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM +  3) = 0xa0;
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM +  4) = GetMax8(geometry.sectors);
    vramRealWord(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM +  5) = Max16;
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM +  7) = Zero8;
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM +  8) = 0x08;
    vramRealWord(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM +  9) = geometry.cylinders;
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 11) = GetMax8(geometry.heads);
    vramRealWord(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 12) = Zero16;
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 14) = GetMax8(geometry.sectors);
    vramRealByte(VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 15) = Zero8;
}

void vm_profile_default_bios_add_post(t_bios *bios, t_strptr stmt) {
    if (bios == NULL) return;
    bios->connect.postTable[bios->connect.postCount++] = stmt;
}
void vm_profile_default_bios_add_interrupt(t_bios *bios, t_strptr stmt,
    t_nubit8 intid) {
    if (bios == NULL) return;
    bios->connect.intTable[intid] = stmt;
}
void vm_profile_default_bios_initialize(t_bios *bios) {
    if (bios == NULL) return;
    MEMSET((void *)bios, Zero8, sizeof(*bios));
    bios->flagBoot = False;
    bios->data.buildCS = bios->data.buildIP = Zero16;
    vm_profile_default_bios_add_interrupt(bios, VBIOS_INT_SOFT_MISC_11, 0x11);
    vm_profile_default_bios_add_interrupt(bios, VBIOS_INT_SOFT_MISC_12, 0x12);
    vm_profile_default_bios_add_interrupt(bios, VBIOS_INT_SOFT_MISC_15, 0x15);
}

/* Loads bios to ram */
void vm_profile_default_bios_reset(t_bios *bios, t_ram *ram,
    const core_machine_block_provider_slot *block_provider) {
    if (bios == NULL || ram == NULL) return;
    MEMSET((void *)(&bios->data), Zero8, sizeof(t_bios_data));
    /* bios area starts at f000:0000 */
    bios->data.buildCS = VBIOS_ADDR_START_SEG;
    bios->data.buildIP = VBIOS_ADDR_START_OFF;
    bios_load_data(bios, ram, block_provider);
    bios_load_rom_info(ram);
    bios_load_interrupts(bios, ram);
    bios_load_post(bios, ram);
    bios_load_additional(ram, block_provider);
}
void vm_profile_default_bios_refresh(t_bios *bios) { (void)bios; }
void vm_profile_default_bios_finalize(t_bios *bios) { (void)bios; }
void vm_profile_default_bios_print(const t_bios *bios) {
    PRINTF("Boot Disk: %s\n", bios != NULL && bios->flagBoot ? "Hard Drive" : "Floppy");
}

void vm_profile_default_bios_set_boot_hdd(t_bios *bios, int enabled) {
    if (bios != NULL) bios->flagBoot = enabled;
}
int vm_profile_default_bios_get_boot_hdd(const t_bios *bios) {
    return bios != NULL && bios->flagBoot;
}
