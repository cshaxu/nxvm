/* Copyright 2012-2014 Neko. */

/* VBIOS loads bios data, interrupt routines and post routines for all devices. */

#include "type.h"

#include "core/product/utils.h"


#include "core/machine/memory.h"

#include "core/machine/block_interface.h"

#include "core/machine/block_provider.h"


#include "bios.h"

#define BIOS_BYTE(memory, segment, offset) \
    (*(ntvdm64_type_unsigned_8 *)core_machine_memory_real_address((memory), (segment), (offset)))
#define BIOS_WORD(memory, segment, offset) \
    (*(ntvdm64_type_unsigned_16 *)core_machine_memory_real_address((memory), (segment), (offset)))
#define BIOS_DWORD(memory, segment, offset) \
    (*(ntvdm64_type_unsigned_32 *)core_machine_memory_real_address((memory), (segment), (offset)))

static ntvdm64_type_unsigned_32 assemble(t_ram *ram, const ntvdm64_type_string_pointer stmt, ntvdm64_type_unsigned_16 seg,
    ntvdm64_type_unsigned_16 off) {
    ntvdm64_type_unsigned_32 len = 0;
    ntvdm64_type_unsigned_8 *code = STD_NULL;
    ntvdm64_type_native_unsigned i;
    ntvdm64_type_native_unsigned insCount = 0; /* the number of instructions to be assembled */
    for (i = 0; i < STD_STRLEN(stmt); ++i) {
        if (stmt[i] == '\n') {
            insCount++;
        }
    }
    if (STD_STRLEN(stmt)) insCount++;
    /* 15 is the maximum length of each instruction */
    code = NTVDM64_TYPE_POINTER_UNSIGNED_8(STD_MALLOC(
        15 * insCount * sizeof(ntvdm64_type_unsigned_8)));
    len = core_product_utils_aasm32x(stmt, code, NTVDM64_TYPE_FALSE);
    if (!len) {
        STD_PRINTF("vbios: invalid x86 assembly instruction.\n");
    }
    STD_MEMCPY(core_machine_memory_real_address(ram, seg, off), (C_VOID *) code, len);
    if (code) {
        STD_FREE((C_VOID *) code);
    }
    return len;
}

static C_VOID bios_load_data(t_bios *bios, t_ram *ram,
    const core_machine_block_provider_slot *block_provider) {
    core_machine_block_geometry geometry;
    core_machine_block_get_geometry_from(block_provider, &geometry);
    STD_MEMSET(core_machine_memory_real_address(ram, 0x0040, NTVDM64_TYPE_ZERO_16), NTVDM64_TYPE_ZERO_8, 0x100);
    BIOS_WORD(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_SERI_PORT_COM1) = 0x03f8;
    BIOS_WORD(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_PARA_PORT_LPT1) = 0x0378;
    BIOS_WORD(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_PARA_PORT_LPT4) = 0x9fc0;
    BIOS_WORD(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_EQUIP_FLAG)     = 0x0021;
    BIOS_WORD(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_RAM_SIZE)       = 0x027f;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_KEYB_FLAG0)     = 0x20;
    BIOS_WORD(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_KEYB_BUF_HEAD)  = 0x041e;
    BIOS_WORD(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_KEYB_BUF_TAIL)  = 0x041e;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_FDD_CALI_FLAG)     = 0x01;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_FDD_MOTOR_TIMEOUT) = 0x25;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_FDD_STATUS)        = 0x09;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_FDC_CYLINDER)      = 0x01;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_FDC_SECTOR)        = 0x01;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_FDC_BYTE_COUNT)    = 0x02;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_VGA_VIDEO_MODE)    = 0x03;
    BIOS_WORD(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_VGA_COLUMN)        = 0x0050;
    BIOS_WORD(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_VGA_PAGE_SIZE)     = 0x1000;
    BIOS_WORD(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_VGA_CURSOR_P0)     = 0x0500;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_VGA_CURSOR_BOTTOM) = 0x0e;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_VGA_CURSOR_TOP)    = 0x0d;
    BIOS_WORD(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_VGA_ACT_ADPT_PORT) = 0x03d4;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_VGA_MODE_REGISTER) = 0x29;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_VGA_COLOR_PALETTE) = 0x30;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_HDD_LST_OP_STATUS) = 0x01;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_HDD_NUMBER) = geometry.present ? 0x01 : NTVDM64_TYPE_ZERO_8;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_HDD_CONTROL)        = 0xc0;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_PARA_TIMEOUT_LPT1)  = 0x14;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_SERI_TIMEOUT_COM1)  = 0x0a;
    BIOS_WORD(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_KEYB_BUFFER_START)  = 0x041e;
    BIOS_WORD(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_KEYB_BUFFER_END)    = 0x043d;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_VGA_ROW_NUMBER)     = 0x18;
    BIOS_WORD(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_VGA_CHAR_HEIGHT)    = 0x0010;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_VGA_MODE_OPTIONS1)  = 0x60;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_VGA_MODE_OPTIONS2)  = 0x09;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_VGA_DISPLAY_DATA)   = 0x11;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_VGA_DCC_INDEX)      = 0x0b;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_DRV_SAME_FLAG)      = 0x77;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_DRV_MEDIA_STATE_D0) = 0x17;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_KEYB_MODE_TYPE)     = 0x10;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_KEYB_LED_FLAG)      = 0x02;
    BIOS_DWORD(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_VGA_VIDEO_TAB_PTR) = 0xc0005d3a;
    BIOS_BYTE(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_POST_WORK_AREA) =
        bios->flagBoot ? 0x80 : NTVDM64_TYPE_ZERO_8;
}
static C_VOID bios_load_rom_info(t_ram *ram) {
    BIOS_WORD(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 0) = 0x0008;
    BIOS_BYTE(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 2) = 0xfc;
    BIOS_BYTE(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 3) = NTVDM64_TYPE_ZERO_8;
    BIOS_BYTE(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 4) = 0x01;
    BIOS_BYTE(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 5) = 0xb4;
    BIOS_BYTE(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 6) = 0x40;
    BIOS_BYTE(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 7) = NTVDM64_TYPE_ZERO_8;
    BIOS_BYTE(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 8) = NTVDM64_TYPE_ZERO_8;
    BIOS_BYTE(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 9) = NTVDM64_TYPE_ZERO_8;
}
static C_VOID bios_load_interrupts(t_bios *bios, t_ram *ram) {
    ntvdm64_type_native_unsigned i;
    bios->data.buildIP += (ntvdm64_type_unsigned_16)assemble(ram, "iret",
        VBIOS_ADDR_START_SEG, VBIOS_ADDR_START_OFF);
    for (i = 0; i < 0x100; ++i) {
        if (bios->connect.intTable[i]) {
            BIOS_WORD(ram, NTVDM64_TYPE_ZERO_16, i * 4 + 0) = bios->data.buildIP;
            BIOS_WORD(ram, NTVDM64_TYPE_ZERO_16, i * 4 + 2) = bios->data.buildCS;
            bios->data.buildIP += (ntvdm64_type_unsigned_16)assemble(ram,
                bios->connect.intTable[i], bios->data.buildCS,
                bios->data.buildIP);
        } else {
            BIOS_WORD(ram, NTVDM64_TYPE_ZERO_16, i * 4 + 0) = VBIOS_ADDR_START_OFF;
            BIOS_WORD(ram, NTVDM64_TYPE_ZERO_16, i * 4 + 2) = VBIOS_ADDR_START_SEG;
        }
    }
}
static C_VOID bios_load_post(t_bios *bios, t_ram *ram) {
    ntvdm64_type_native_unsigned i;
    ntvdm64_type_string_buffer stmt;
    STD_SPRINTF(stmt, "jmp %04x:%04x", bios->data.buildCS, bios->data.buildIP);
    assemble(ram, stmt, VBIOS_ADDR_POST_SEG, VBIOS_ADDR_POST_OFF);
    for (i = 0; i < bios->connect.postCount; ++i) {
        bios->data.buildIP += (ntvdm64_type_unsigned_16)assemble(ram,
            bios->connect.postTable[i], bios->data.buildCS, bios->data.buildIP);
    }
    bios->data.buildIP += (ntvdm64_type_unsigned_16)assemble(ram, VBIOS_POST_BOOT,
        bios->data.buildCS, bios->data.buildIP);
}
static C_VOID bios_load_additional(t_ram *ram,
    const core_machine_block_provider_slot *block_provider) {
    core_machine_block_geometry geometry;
    core_machine_block_get_geometry_from(block_provider, &geometry);
    /* hard disk param table */
    BIOS_WORD(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_HDD_PARAM_OFFSET) = VBIOS_ADDR_HDD_PARAM;
    BIOS_WORD(ram, NTVDM64_TYPE_ZERO_16, VBIOS_ADDR_HDD_PARAM_SEGMENT) = VBIOS_ADDR_START_SEG;
    BIOS_WORD(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 0) = geometry.cylinders;
    BIOS_BYTE(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 2) = NTVDM64_TYPE_MASK_UNSIGNED_8(geometry.heads);
    BIOS_BYTE(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 3) = 0xa0;
    BIOS_BYTE(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 4) = NTVDM64_TYPE_MASK_UNSIGNED_8(geometry.sectors);
    BIOS_WORD(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 5) = NTVDM64_TYPE_MAX_UNSIGNED_16;
    BIOS_BYTE(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 7) = NTVDM64_TYPE_ZERO_8;
    BIOS_BYTE(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 8) = 0x08;
    BIOS_WORD(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 9) = geometry.cylinders;
    BIOS_BYTE(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 11) = NTVDM64_TYPE_MASK_UNSIGNED_8(geometry.heads);
    BIOS_WORD(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 12) = NTVDM64_TYPE_ZERO_16;
    BIOS_BYTE(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 14) = NTVDM64_TYPE_MASK_UNSIGNED_8(geometry.sectors);
    BIOS_BYTE(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 15) = NTVDM64_TYPE_ZERO_8;
}

C_VOID vm_profile_default_bios_add_post(t_bios *bios, ntvdm64_type_string_pointer stmt) {
    if (bios == STD_NULL) return;
    bios->connect.postTable[bios->connect.postCount++] = stmt;
}
C_VOID vm_profile_default_bios_add_interrupt(t_bios *bios, ntvdm64_type_string_pointer stmt,
    ntvdm64_type_unsigned_8 intid) {
    if (bios == STD_NULL) return;
    bios->connect.intTable[intid] = stmt;
}
C_VOID vm_profile_default_bios_initialize(t_bios *bios) {
    if (bios == STD_NULL) return;
    STD_MEMSET((C_VOID *)bios, NTVDM64_TYPE_ZERO_8, sizeof(*bios));
    bios->flagBoot = NTVDM64_TYPE_FALSE;
    bios->data.buildCS = bios->data.buildIP = NTVDM64_TYPE_ZERO_16;
    vm_profile_default_bios_add_interrupt(bios, VBIOS_INT_SOFT_MISC_11, 0x11);
    vm_profile_default_bios_add_interrupt(bios, VBIOS_INT_SOFT_MISC_12, 0x12);
    vm_profile_default_bios_add_interrupt(bios, VBIOS_INT_SOFT_MISC_15, 0x15);
}

/* Loads bios to ram */
C_VOID vm_profile_default_bios_reset(t_bios *bios, t_ram *ram,
    const core_machine_block_provider_slot *block_provider) {
    if (bios == STD_NULL || ram == STD_NULL) return;
    STD_MEMSET((C_VOID *)(&bios->data), NTVDM64_TYPE_ZERO_8, sizeof(t_bios_data));
    /* bios area starts at f000:0000 */
    bios->data.buildCS = VBIOS_ADDR_START_SEG;
    bios->data.buildIP = VBIOS_ADDR_START_OFF;
    bios_load_data(bios, ram, block_provider);
    bios_load_rom_info(ram);
    bios_load_interrupts(bios, ram);
    bios_load_post(bios, ram);
    bios_load_additional(ram, block_provider);
}
C_VOID vm_profile_default_bios_refresh(t_bios *bios) { (C_VOID)bios; }
C_VOID vm_profile_default_bios_finalize(t_bios *bios) { (C_VOID)bios; }
C_VOID vm_profile_default_bios_print(const t_bios *bios) {
    STD_PRINTF("Boot Disk: %s\n", bios != STD_NULL && bios->flagBoot ? "Hard Drive" : "Floppy");
}

C_VOID vm_profile_default_bios_set_boot_hdd(t_bios *bios, C_INT enabled) {
    if (bios != STD_NULL) bios->flagBoot = enabled;
}
C_INT vm_profile_default_bios_get_boot_hdd(const t_bios *bios) {
    return bios != STD_NULL && bios->flagBoot;
}
