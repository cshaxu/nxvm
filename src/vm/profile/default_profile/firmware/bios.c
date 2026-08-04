/* Copyright 2012-2014 Neko. */

/* VBIOS loads bios data, interrupt routines and post routines for all devices. */

#include "type.h"

#include "core/product/utils.h"


#include "core/machine/memory.h"

#include "core/machine/block_interface.h"

#include "core/machine/block_provider.h"


#include "bios.h"

static C_VOID bios_write_byte(t_ram *ram, uint16_t segment, uint16_t offset,
    type_unsigned_8 value)
{
    (C_VOID)core_machine_memory_write_real_to(ram, segment, offset, &value,
        sizeof(value));
}

static C_VOID bios_write_word(t_ram *ram, uint16_t segment, uint16_t offset,
    type_unsigned_16 value)
{
    (C_VOID)core_machine_memory_write_real_to(ram, segment, offset, &value,
        sizeof(value));
}

static C_VOID bios_write_dword(t_ram *ram, uint16_t segment, uint16_t offset,
    type_unsigned_32 value)
{
    (C_VOID)core_machine_memory_write_real_to(ram, segment, offset, &value,
        sizeof(value));
}

static C_VOID bios_load_keyboard_tables(t_ram *ram)
{
    static const uint8_t normal[0x59] = {
        [0x01] = 0x1bu, [0x02] = '1', [0x03] = '2', [0x04] = '3',
        [0x05] = '4', [0x06] = '5', [0x07] = '6', [0x08] = '7',
        [0x09] = '8', [0x0a] = '9', [0x0b] = '0', [0x0c] = '-',
        [0x0d] = '=', [0x0e] = 0x08u, [0x0f] = 0x09u,
        [0x10] = 'q', [0x11] = 'w', [0x12] = 'e', [0x13] = 'r',
        [0x14] = 't', [0x15] = 'y', [0x16] = 'u', [0x17] = 'i',
        [0x18] = 'o', [0x19] = 'p', [0x1a] = '[', [0x1b] = ']',
        [0x1c] = 0x0du, [0x1e] = 'a', [0x1f] = 's', [0x20] = 'd',
        [0x21] = 'f', [0x22] = 'g', [0x23] = 'h', [0x24] = 'j',
        [0x25] = 'k', [0x26] = 'l', [0x27] = ';', [0x28] = '\'',
        [0x29] = '`', [0x2b] = '\\', [0x2c] = 'z', [0x2d] = 'x',
        [0x2e] = 'c', [0x2f] = 'v', [0x30] = 'b', [0x31] = 'n',
        [0x32] = 'm', [0x33] = ',', [0x34] = '.', [0x35] = '/',
        [0x37] = '*', [0x39] = ' ', [0x4a] = '-', [0x4e] = '+'
    };
    static const uint8_t shifted[0x59] = {
        [0x01] = 0x1bu, [0x02] = '!', [0x03] = '@', [0x04] = '#',
        [0x05] = '$', [0x06] = '%', [0x07] = '^', [0x08] = '&',
        [0x09] = '*', [0x0a] = '(', [0x0b] = ')', [0x0c] = '_',
        [0x0d] = '+', [0x0e] = 0x08u, [0x10] = 'Q', [0x11] = 'W',
        [0x12] = 'E', [0x13] = 'R', [0x14] = 'T', [0x15] = 'Y',
        [0x16] = 'U', [0x17] = 'I', [0x18] = 'O', [0x19] = 'P',
        [0x1a] = '{', [0x1b] = '}', [0x1c] = 0x0du, [0x1e] = 'A',
        [0x1f] = 'S', [0x20] = 'D', [0x21] = 'F', [0x22] = 'G',
        [0x23] = 'H', [0x24] = 'J', [0x25] = 'K', [0x26] = 'L',
        [0x27] = ':', [0x28] = '"', [0x29] = '~', [0x2b] = '|',
        [0x2c] = 'Z', [0x2d] = 'X', [0x2e] = 'C', [0x2f] = 'V',
        [0x30] = 'B', [0x31] = 'N', [0x32] = 'M', [0x33] = '<',
        [0x34] = '>', [0x35] = '?', [0x37] = '*', [0x39] = ' ',
        [0x4a] = '-', [0x4e] = '+'
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(normal); ++index) {
        bios_write_byte(ram, VBIOS_ADDR_START_SEG,
            VBIOS_ADDR_KEYB_SCAN_ASCII_NORMAL + (uint16_t)index, normal[index]);
        bios_write_byte(ram, VBIOS_ADDR_START_SEG,
            VBIOS_ADDR_KEYB_SCAN_ASCII_SHIFT + (uint16_t)index, shifted[index]);
    }
}

static type_unsigned_32 assemble(t_ram *ram, const type_string_pointer stmt, type_unsigned_16 seg,
    type_unsigned_16 off) {
    type_unsigned_32 len = 0;
    type_unsigned_8 *code = STD_NULL;
    type_native_unsigned i;
    type_native_unsigned insCount = 0; /* the number of instructions to be assembled */
    for (i = 0; i < STD_STRLEN(stmt); ++i) {
        if (stmt[i] == '\n') {
            insCount++;
        }
    }
    if (STD_STRLEN(stmt)) insCount++;
    /* 15 is the maximum length of each instruction */
    code = TYPE_POINTER_UNSIGNED_8(STD_MALLOC(
        15 * insCount * sizeof(type_unsigned_8)));
    len = core_product_utils_aasm32x(stmt, code, TYPE_FALSE);
    if (!len) {
        STD_PRINTF("vbios: invalid x86 assembly instruction.\n");
    }
    (C_VOID)core_machine_memory_write_real_to(ram, seg, off, code, len);
    if (code) {
        STD_FREE((C_VOID *) code);
    }
    return len;
}

static C_VOID bios_load_data(t_bios *bios, t_ram *ram,
    const core_machine_block_provider_slot *block_provider) {
    core_machine_block_geometry geometry;
    core_machine_block_get_geometry_from(block_provider, &geometry);
    C_UCHAR zeroes[0x100] = { 0 };
    (C_VOID)core_machine_memory_write_real_to(ram, 0x0040, 0u, zeroes, sizeof(zeroes));
    bios_write_word(ram, 0u, VBIOS_ADDR_SERI_PORT_COM1, 0x03f8); bios_write_word(ram, 0u, VBIOS_ADDR_PARA_PORT_LPT1, 0x0378); bios_write_word(ram, 0u, VBIOS_ADDR_PARA_PORT_LPT4, 0x9fc0); bios_write_word(ram, 0u, VBIOS_ADDR_EQUIP_FLAG, 0x0021); bios_write_word(ram, 0u, VBIOS_ADDR_RAM_SIZE, 0x027f);
    bios_write_byte(ram, 0u, VBIOS_ADDR_KEYB_FLAG0, 0x20); bios_write_word(ram, 0u, VBIOS_ADDR_KEYB_BUF_HEAD, 0x041e); bios_write_word(ram, 0u, VBIOS_ADDR_KEYB_BUF_TAIL, 0x041e);
    bios_write_byte(ram, 0u, VBIOS_ADDR_FDD_CALI_FLAG, 0x01); bios_write_byte(ram, 0u, VBIOS_ADDR_FDD_MOTOR_TIMEOUT, 0x25); bios_write_byte(ram, 0u, VBIOS_ADDR_FDD_STATUS, 0x09); bios_write_byte(ram, 0u, VBIOS_ADDR_FDC_CYLINDER, 0x01); bios_write_byte(ram, 0u, VBIOS_ADDR_FDC_SECTOR, 0x01); bios_write_byte(ram, 0u, VBIOS_ADDR_FDC_BYTE_COUNT, 0x02);
    bios_write_byte(ram, 0u, VBIOS_ADDR_VGA_VIDEO_MODE, 0x03); bios_write_word(ram, 0u, VBIOS_ADDR_VGA_COLUMN, 0x0050); bios_write_word(ram, 0u, VBIOS_ADDR_VGA_PAGE_SIZE, 0x1000); bios_write_word(ram, 0u, VBIOS_ADDR_VGA_CURSOR_P0, 0x0500); bios_write_byte(ram, 0u, VBIOS_ADDR_VGA_CURSOR_BOTTOM, 0x0e); bios_write_byte(ram, 0u, VBIOS_ADDR_VGA_CURSOR_TOP, 0x0d); bios_write_word(ram, 0u, VBIOS_ADDR_VGA_ACT_ADPT_PORT, 0x03d4); bios_write_byte(ram, 0u, VBIOS_ADDR_VGA_MODE_REGISTER, 0x29); bios_write_byte(ram, 0u, VBIOS_ADDR_VGA_COLOR_PALETTE, 0x30);
    bios_write_byte(ram, 0u, VBIOS_ADDR_HDD_LST_OP_STATUS, 0x01); bios_write_byte(ram, 0u, VBIOS_ADDR_HDD_NUMBER, geometry.present ? 0x01 : 0u); bios_write_byte(ram, 0u, VBIOS_ADDR_HDD_CONTROL, 0xc0); bios_write_byte(ram, 0u, VBIOS_ADDR_PARA_TIMEOUT_LPT1, 0x14); bios_write_byte(ram, 0u, VBIOS_ADDR_SERI_TIMEOUT_COM1, 0x0a); bios_write_word(ram, 0u, VBIOS_ADDR_KEYB_BUFFER_START, 0x041e); bios_write_word(ram, 0u, VBIOS_ADDR_KEYB_BUFFER_END, 0x043d);
    bios_write_byte(ram, 0u, VBIOS_ADDR_VGA_ROW_NUMBER, 0x18); bios_write_word(ram, 0u, VBIOS_ADDR_VGA_CHAR_HEIGHT, 0x0010); bios_write_byte(ram, 0u, VBIOS_ADDR_VGA_MODE_OPTIONS1, 0x60); bios_write_byte(ram, 0u, VBIOS_ADDR_VGA_MODE_OPTIONS2, 0x09); bios_write_byte(ram, 0u, VBIOS_ADDR_VGA_DISPLAY_DATA, 0x11); bios_write_byte(ram, 0u, VBIOS_ADDR_VGA_DCC_INDEX, 0x0b); bios_write_byte(ram, 0u, VBIOS_ADDR_DRV_SAME_FLAG, 0x77); bios_write_byte(ram, 0u, VBIOS_ADDR_DRV_MEDIA_STATE_D0, 0x17); bios_write_byte(ram, 0u, VBIOS_ADDR_KEYB_MODE_TYPE, 0x10); bios_write_byte(ram, 0u, VBIOS_ADDR_KEYB_LED_FLAG, 0x02); bios_write_dword(ram, 0u, VBIOS_ADDR_VGA_VIDEO_TAB_PTR, 0xc0005d3a); bios_write_byte(ram, 0u, VBIOS_ADDR_POST_WORK_AREA, bios->flagBoot ? 0x80 : 0u);
}
static C_VOID bios_load_rom_info(t_ram *ram) {
    bios_write_word(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 0, 0x0008);
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 2, 0xfc);
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 3, 0u);
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 4, 0x01);
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 5, 0xb4);
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 6, 0x40);
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 7, 0u);
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 8, 0u);
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_ROM_INFO + 9, 0u);
}
static C_VOID bios_load_interrupts(t_bios *bios, t_ram *ram) {
    type_native_unsigned i;
    bios->data.buildIP += (type_unsigned_16)assemble(ram, "iret",
        VBIOS_ADDR_START_SEG, VBIOS_ADDR_START_OFF);
    for (i = 0; i < 0x100; ++i) {
        if (bios->connect.intTable[i]) {
            bios_write_word(ram, 0u, i * 4 + 0, bios->data.buildIP);
            bios_write_word(ram, 0u, i * 4 + 2, bios->data.buildCS);
            bios->data.buildIP += (type_unsigned_16)assemble(ram,
                bios->connect.intTable[i], bios->data.buildCS,
                bios->data.buildIP);
        } else {
            bios_write_word(ram, 0u, i * 4 + 0, VBIOS_ADDR_START_OFF);
            bios_write_word(ram, 0u, i * 4 + 2, VBIOS_ADDR_START_SEG);
        }
    }
}
static C_VOID bios_load_post(t_bios *bios, t_ram *ram) {
    type_native_unsigned i;
    type_string_buffer stmt;
    STD_SPRINTF(stmt, "jmp %04x:%04x", bios->data.buildCS, bios->data.buildIP);
    assemble(ram, stmt, VBIOS_ADDR_POST_SEG, VBIOS_ADDR_POST_OFF);
    for (i = 0; i < bios->connect.postCount; ++i) {
        bios->data.buildIP += (type_unsigned_16)assemble(ram,
            bios->connect.postTable[i], bios->data.buildCS, bios->data.buildIP);
    }
    bios->data.buildIP += (type_unsigned_16)assemble(ram, VBIOS_POST_BOOT,
        bios->data.buildCS, bios->data.buildIP);
}
static C_VOID bios_load_additional(t_ram *ram,
    const core_machine_block_provider_slot *block_provider) {
    core_machine_block_geometry geometry;
    core_machine_block_get_geometry_from(block_provider, &geometry);
    /* hard disk param table */
    bios_write_word(ram, 0u, VBIOS_ADDR_HDD_PARAM_OFFSET, VBIOS_ADDR_HDD_PARAM);
    bios_write_word(ram, 0u, VBIOS_ADDR_HDD_PARAM_SEGMENT, VBIOS_ADDR_START_SEG);
    bios_write_word(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 0, geometry.cylinders);
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 2, TYPE_MASK_UNSIGNED_8(geometry.heads));
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 3, 0xa0);
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 4, TYPE_MASK_UNSIGNED_8(geometry.sectors));
    bios_write_word(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 5, TYPE_MAX_UNSIGNED_16);
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 7, 0u);
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 8, 0x08);
    bios_write_word(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 9, geometry.cylinders);
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 11, TYPE_MASK_UNSIGNED_8(geometry.heads));
    bios_write_word(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 12, 0u);
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 14, TYPE_MASK_UNSIGNED_8(geometry.sectors));
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 15, 0u);
}

C_VOID vm_profile_default_bios_add_post(t_bios *bios, type_string_pointer stmt) {
    if (bios == STD_NULL) return;
    bios->connect.postTable[bios->connect.postCount++] = stmt;
}
C_VOID vm_profile_default_bios_add_interrupt(t_bios *bios, type_string_pointer stmt,
    type_unsigned_8 intid) {
    if (bios == STD_NULL) return;
    bios->connect.intTable[intid] = stmt;
}
C_VOID vm_profile_default_bios_initialize(t_bios *bios) {
    if (bios == STD_NULL) return;
    STD_MEMSET((C_VOID *)bios, TYPE_ZERO_8, sizeof(*bios));
    bios->flagBoot = TYPE_FALSE;
    bios->data.buildCS = bios->data.buildIP = TYPE_ZERO_16;
    vm_profile_default_bios_add_interrupt(bios, VBIOS_INT_SOFT_MISC_11, 0x11);
    vm_profile_default_bios_add_interrupt(bios, VBIOS_INT_SOFT_MISC_12, 0x12);
    vm_profile_default_bios_add_interrupt(bios, VBIOS_INT_SOFT_MISC_15, 0x15);
}

/* Loads bios to ram */
C_VOID vm_profile_default_bios_reset(t_bios *bios, t_ram *ram,
    const core_machine_block_provider_slot *block_provider) {
    if (bios == STD_NULL || ram == STD_NULL) return;
    STD_MEMSET((C_VOID *)(&bios->data), TYPE_ZERO_8, sizeof(t_bios_data));
    /* bios area starts at f000:0000 */
    bios->data.buildCS = VBIOS_ADDR_START_SEG;
    bios->data.buildIP = VBIOS_ADDR_START_OFF;
    bios_load_data(bios, ram, block_provider);
    bios_load_keyboard_tables(ram);
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
