/* Copyright 2012-2014 Neko. */

/* VBIOS loads bios data, interrupt routines and post routines for all devices. */

#include "type.h"

#include "core/machine/memory.h"

#include "core/machine/media_interface.h"


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

static uint16_t bios_write_code(t_ram *ram, uint16_t segment, uint16_t offset,
    const vm_profile_default_bios_code *code)
{
    if (ram == STD_NULL || code == STD_NULL || code->bytes == STD_NULL ||
        code->length == 0u) return 0u;
    if (core_machine_memory_write_real_to(ram, segment, offset, code->bytes,
            code->length) != TYPE_STATUS_OK) return 0u;
    return code->length;
}

static core_machine_media_info bios_media_info(
    const core_machine_media_registry *media_registry, core_machine_media_id hdd_media_id)
{
    core_machine_media_info info = {0};
    core_machine_media_result result;

    if (core_machine_media_query(media_registry, hdd_media_id, &info, &result) !=
            TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK) {
        STD_MEMSET(&info, TYPE_ZERO_8, sizeof(info));
    }
    return info;
}

static C_VOID bios_load_data(t_bios *bios, t_ram *ram,
    const core_machine_media_registry *media_registry, core_machine_media_id hdd_media_id) {
    core_machine_media_info media = bios_media_info(media_registry, hdd_media_id);
    (C_VOID)bios;
    C_UCHAR zeroes[0x100] = { 0 };
    (C_VOID)core_machine_memory_write_real_to(ram, 0x0040, 0u, zeroes, sizeof(zeroes));
    bios_write_word(ram, 0u, VBIOS_ADDR_SERI_PORT_COM1, 0x03f8); bios_write_word(ram, 0u, VBIOS_ADDR_PARA_PORT_LPT1, 0x0378); bios_write_word(ram, 0u, VBIOS_ADDR_PARA_PORT_LPT4, 0x9fc0); bios_write_word(ram, 0u, VBIOS_ADDR_EQUIP_FLAG, 0x0021); bios_write_word(ram, 0u, VBIOS_ADDR_RAM_SIZE, 0x027f);
    bios_write_byte(ram, 0u, VBIOS_ADDR_KEYB_FLAG0, 0x20); bios_write_word(ram, 0u, VBIOS_ADDR_KEYB_BUF_HEAD, 0x041e); bios_write_word(ram, 0u, VBIOS_ADDR_KEYB_BUF_TAIL, 0x041e); bios_write_byte(ram, 0u, VBIOS_ADDR_SOFT_RESET_FLAG, bios->flagBoot ? 0x80u : 0u);
    bios_write_byte(ram, 0u, VBIOS_ADDR_FDD_CALI_FLAG, 0x01); bios_write_byte(ram, 0u, VBIOS_ADDR_FDD_MOTOR_TIMEOUT, 0x25); bios_write_byte(ram, 0u, VBIOS_ADDR_FDD_STATUS, 0x09); bios_write_byte(ram, 0u, VBIOS_ADDR_FDC_CYLINDER, 0x01); bios_write_byte(ram, 0u, VBIOS_ADDR_FDC_SECTOR, 0x01); bios_write_byte(ram, 0u, VBIOS_ADDR_FDC_BYTE_COUNT, 0x02);
    bios_write_byte(ram, 0u, VBIOS_ADDR_VGA_VIDEO_MODE, 0x03); bios_write_word(ram, 0u, VBIOS_ADDR_VGA_COLUMN, 0x0050); bios_write_word(ram, 0u, VBIOS_ADDR_VGA_PAGE_SIZE, 0x1000); bios_write_word(ram, 0u, VBIOS_ADDR_VGA_CURSOR_P0, 0x0500); bios_write_byte(ram, 0u, VBIOS_ADDR_VGA_CURSOR_BOTTOM, 0x0e); bios_write_byte(ram, 0u, VBIOS_ADDR_VGA_CURSOR_TOP, 0x0d); bios_write_word(ram, 0u, VBIOS_ADDR_VGA_ACT_ADPT_PORT, 0x03d4); bios_write_byte(ram, 0u, VBIOS_ADDR_VGA_MODE_REGISTER, 0x29); bios_write_byte(ram, 0u, VBIOS_ADDR_VGA_COLOR_PALETTE, 0x30);
    bios_write_byte(ram, 0u, VBIOS_ADDR_HDD_LST_OP_STATUS, 0u); bios_write_byte(ram, 0u, VBIOS_ADDR_HDD_NUMBER, media.present ? 0x01 : 0u); bios_write_byte(ram, 0u, VBIOS_ADDR_HDD_CONTROL, 0xc0); bios_write_byte(ram, 0u, VBIOS_ADDR_PARA_TIMEOUT_LPT1, 0x14); bios_write_byte(ram, 0u, VBIOS_ADDR_SERI_TIMEOUT_COM1, 0x0a); bios_write_word(ram, 0u, VBIOS_ADDR_KEYB_BUFFER_START, 0x041e); bios_write_word(ram, 0u, VBIOS_ADDR_KEYB_BUFFER_END, 0x043d);
    bios_write_byte(ram, 0u, VBIOS_ADDR_VGA_ROW_NUMBER, 0x18); bios_write_word(ram, 0u, VBIOS_ADDR_VGA_CHAR_HEIGHT, 0x0010); bios_write_byte(ram, 0u, VBIOS_ADDR_VGA_MODE_OPTIONS1, 0x60); bios_write_byte(ram, 0u, VBIOS_ADDR_VGA_MODE_OPTIONS2, 0x09); bios_write_byte(ram, 0u, VBIOS_ADDR_VGA_DISPLAY_DATA, 0x11); bios_write_byte(ram, 0u, VBIOS_ADDR_VGA_DCC_INDEX, 0x0b); bios_write_byte(ram, 0u, VBIOS_ADDR_DRV_SAME_FLAG, 0x77); bios_write_byte(ram, 0u, VBIOS_ADDR_DRV_MEDIA_STATE_D0, 0x17); bios_write_byte(ram, 0u, VBIOS_ADDR_KEYB_MODE_TYPE, 0x10); bios_write_byte(ram, 0u, VBIOS_ADDR_KEYB_LED_FLAG, 0x02); bios_write_dword(ram, 0u, VBIOS_ADDR_VGA_VIDEO_TAB_PTR, 0xc0005d3a); bios_write_byte(ram, 0u, VBIOS_ADDR_POST_WORK_AREA, VBIOS_POST_REPORT_NONE);
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
    static const uint8_t iret[] = { 0xcfu };
    vm_profile_default_bios_code code;
    type_native_unsigned i;
    code.bytes = (uint8_t *)iret;
    code.length = sizeof(iret);
    bios->data.buildIP += bios_write_code(ram, VBIOS_ADDR_START_SEG,
        VBIOS_ADDR_START_OFF, &code);
    for (i = 0; i < 0x100; ++i) {
        if (bios->connect.intTable[i].bytes != STD_NULL) {
            bios_write_word(ram, 0u, i * 4 + 0, bios->data.buildIP);
            bios_write_word(ram, 0u, i * 4 + 2, bios->data.buildCS);
            bios->data.buildIP += bios_write_code(ram,
                bios->data.buildCS, bios->data.buildIP,
                &bios->connect.intTable[i]);
        } else {
            bios_write_word(ram, 0u, i * 4 + 0, VBIOS_ADDR_START_OFF);
            bios_write_word(ram, 0u, i * 4 + 2, VBIOS_ADDR_START_SEG);
        }
    }
}
static C_VOID bios_load_post(t_bios *bios, t_ram *ram) {
    uint8_t jump[] = { 0xeau, 0u, 0u, 0u, 0xf0u };
    vm_profile_default_bios_code code;
    type_native_unsigned i;
    jump[1] = TYPE_MASK_UNSIGNED_8(bios->data.buildIP);
    jump[2] = TYPE_MASK_UNSIGNED_8(bios->data.buildIP >> 8);
    code.bytes = jump;
    code.length = sizeof(jump);
    (C_VOID)bios_write_code(ram, VBIOS_ADDR_POST_SEG, VBIOS_ADDR_POST_OFF, &code);
    for (i = 0; i < bios->connect.postCount; ++i) {
        bios->data.buildIP += bios_write_code(ram, bios->data.buildCS,
            bios->data.buildIP, &bios->connect.postTable[i]);
    }
    bios->data.buildIP += bios_write_code(ram, bios->data.buildCS,
        bios->data.buildIP, &bios->connect.bootCode);
}
static C_VOID bios_load_additional(t_ram *ram,
    const core_machine_media_registry *media_registry, core_machine_media_id hdd_media_id) {
    core_machine_media_info media = bios_media_info(media_registry, hdd_media_id);
    /* hard disk param table */
    bios_write_word(ram, 0u, VBIOS_ADDR_HDD_PARAM_OFFSET, VBIOS_ADDR_HDD_PARAM);
    bios_write_word(ram, 0u, VBIOS_ADDR_HDD_PARAM_SEGMENT, VBIOS_ADDR_START_SEG);
    bios_write_word(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 0, TYPE_MASK_UNSIGNED_16(media.geometry.cylinders));
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 2, TYPE_MASK_UNSIGNED_8(media.geometry.heads));
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 3, 0xa0);
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 4, TYPE_MASK_UNSIGNED_8(media.geometry.sectors_per_track));
    bios_write_word(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 5, TYPE_MAX_UNSIGNED_16);
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 7, 0u);
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 8, TYPE_MASK_UNSIGNED_8(media.geometry.sectors_per_track));
    bios_write_word(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 9, TYPE_MASK_UNSIGNED_16(media.geometry.cylinders));
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 11, TYPE_MASK_UNSIGNED_8(media.geometry.heads));
    bios_write_word(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 12, 0u);
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 14, TYPE_MASK_UNSIGNED_8(media.geometry.sectors_per_track));
    bios_write_byte(ram, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 15, 0u);
}

C_VOID vm_profile_default_bios_add_post_code(t_bios *bios, uint8_t *bytes,
    uint16_t length)
{
    if (bios == STD_NULL || bytes == STD_NULL || length == 0u ||
        bios->connect.postCount >= 0x100u) {
        STD_FREE(bytes);
        return;
    }
    bios->connect.postTable[bios->connect.postCount].bytes = bytes;
    bios->connect.postTable[bios->connect.postCount].length = length;
    bios->connect.postCount++;
}

C_VOID vm_profile_default_bios_add_interrupt_code(t_bios *bios, uint8_t *bytes,
    uint16_t length, uint8_t intid)
{
    if (bios == STD_NULL || bytes == STD_NULL || length == 0u) {
        STD_FREE(bytes);
        return;
    }
    STD_FREE(bios->connect.intTable[intid].bytes);
    bios->connect.intTable[intid].bytes = bytes;
    bios->connect.intTable[intid].length = length;
}

C_VOID vm_profile_default_bios_set_boot_code(t_bios *bios, uint8_t *bytes,
    uint16_t length)
{
    if (bios == STD_NULL || bytes == STD_NULL || length == 0u) {
        STD_FREE(bytes);
        return;
    }
    STD_FREE(bios->connect.bootCode.bytes);
    bios->connect.bootCode.bytes = bytes;
    bios->connect.bootCode.length = length;
}
C_VOID vm_profile_default_bios_initialize(t_bios *bios) {
    if (bios == STD_NULL) return;
    STD_MEMSET((C_VOID *)bios, TYPE_ZERO_8, sizeof(*bios));
    bios->flagBoot = TYPE_FALSE;
    bios->data.buildCS = bios->data.buildIP = TYPE_ZERO_16;
}

/* Loads bios to ram */
C_VOID vm_profile_default_bios_reset(t_bios *bios, t_ram *ram,
    const core_machine_media_registry *media_registry,
    core_machine_media_id hdd_media_id) {
    if (bios == STD_NULL || ram == STD_NULL) return;
    STD_MEMSET((C_VOID *)(&bios->data), TYPE_ZERO_8, sizeof(t_bios_data));
    /* bios area starts at f000:0000 */
    bios->data.buildCS = VBIOS_ADDR_START_SEG;
    bios->data.buildIP = VBIOS_ADDR_START_OFF;
    bios_load_data(bios, ram, media_registry, hdd_media_id);
    bios_load_keyboard_tables(ram);
    bios_load_rom_info(ram);
    bios_load_interrupts(bios, ram);
    bios_load_post(bios, ram);
    bios_load_additional(ram, media_registry, hdd_media_id);
}
C_VOID vm_profile_default_bios_refresh(t_bios *bios) { (C_VOID)bios; }
C_VOID vm_profile_default_bios_finalize(t_bios *bios)
{
    type_native_unsigned index;
    if (bios == STD_NULL) return;
    for (index = 0u; index < bios->connect.postCount; ++index) {
        STD_FREE(bios->connect.postTable[index].bytes);
    }
    for (index = 0u; index < 0x100u; ++index) {
        STD_FREE(bios->connect.intTable[index].bytes);
    }
    STD_FREE(bios->connect.bootCode.bytes);
    STD_MEMSET(&bios->connect, TYPE_ZERO_8, sizeof(bios->connect));
}
C_VOID vm_profile_default_bios_print(const t_bios *bios) {
    STD_PRINTF("Boot Disk: %s\n", bios != STD_NULL && bios->flagBoot ? "Hard Drive" : "Floppy");
}

C_VOID vm_profile_default_bios_set_boot_hdd(t_bios *bios, C_INT enabled) {
    if (bios != STD_NULL) bios->flagBoot = enabled;
}
C_INT vm_profile_default_bios_get_boot_hdd(const t_bios *bios) {
    return bios != STD_NULL && bios->flagBoot;
}

C_INT vm_profile_default_bios_take_boot_failure_report(t_ram *ram)
{
    uint8_t report = VBIOS_POST_REPORT_NONE;

    if (ram == STD_NULL || core_machine_memory_read_real_from(ram, 0u,
            VBIOS_ADDR_POST_WORK_AREA, &report, sizeof(report)) != TYPE_STATUS_OK ||
        report != VBIOS_POST_REPORT_BOOT_FAILURE_ACKNOWLEDGED) {
        return 0;
    }
    report = VBIOS_POST_REPORT_NONE;
    return core_machine_memory_write_real_to(ram, 0u, VBIOS_ADDR_POST_WORK_AREA,
        &report, sizeof(report)) == TYPE_STATUS_OK;
}
