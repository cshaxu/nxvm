/* Copyright 2012-2014 Neko. */

/* VBIOS loads bios data, interrupt routines and post routines for all devices. */

#include "type.h"

#include "core/machine/media_interface.h"
#include "core/machine/firmware_interface.h"


#include "bios.h"

static C_VOID bios_write_byte(core_machine_firmware_context *firmware,
    type_unsigned_16 segment, type_unsigned_16 offset,
    type_unsigned_8 value)
{
    (C_VOID)core_machine_firmware_memory_write(firmware,
        (type_unsigned_32)segment * 16u + offset, &value,
        sizeof(value));
}

static C_VOID bios_write_word(core_machine_firmware_context *firmware,
    type_unsigned_16 segment, type_unsigned_16 offset,
    type_unsigned_16 value)
{
    (C_VOID)core_machine_firmware_memory_write(firmware,
        (type_unsigned_32)segment * 16u + offset, &value,
        sizeof(value));
}

static C_VOID bios_write_dword(core_machine_firmware_context *firmware,
    type_unsigned_16 segment, type_unsigned_16 offset,
    type_unsigned_32 value)
{
    (C_VOID)core_machine_firmware_memory_write(firmware,
        (type_unsigned_32)segment * 16u + offset, &value,
        sizeof(value));
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

static C_VOID bios_load_data(t_bios *bios, core_machine_firmware_context *firmware,
    const core_machine_media_registry *media_registry, core_machine_media_id hdd_media_id) {
    core_machine_media_info media = bios_media_info(media_registry, hdd_media_id);
    (C_VOID)bios;
    C_UCHAR zeroes[0x100] = { 0 };
    (C_VOID)core_machine_firmware_memory_write(firmware, 0x00400u, zeroes, sizeof(zeroes));
    bios_write_word(firmware, 0u, VBIOS_ADDR_SERI_PORT_COM1, 0x03f8); bios_write_word(firmware, 0u, VBIOS_ADDR_PARA_PORT_LPT1, 0x0378); bios_write_word(firmware, 0u, VBIOS_ADDR_PARA_PORT_LPT4, 0x9fc0); bios_write_word(firmware, 0u, VBIOS_ADDR_EQUIP_FLAG, 0x0021); bios_write_word(firmware, 0u, VBIOS_ADDR_RAM_SIZE, 0x027f);
    bios_write_byte(firmware, 0u, VBIOS_ADDR_KEYB_FLAG0, 0x20); bios_write_word(firmware, 0u, VBIOS_ADDR_KEYB_BUF_HEAD, 0x041e); bios_write_word(firmware, 0u, VBIOS_ADDR_KEYB_BUF_TAIL, 0x041e); bios_write_byte(firmware, 0u, VBIOS_ADDR_SOFT_RESET_FLAG, bios->flagBoot ? 0x80u : 0u);
    bios_write_byte(firmware, 0u, VBIOS_ADDR_FDD_CALI_FLAG, 0x01); bios_write_byte(firmware, 0u, VBIOS_ADDR_FDD_MOTOR_TIMEOUT, 0x25); bios_write_byte(firmware, 0u, VBIOS_ADDR_FDD_STATUS, 0x09); bios_write_byte(firmware, 0u, VBIOS_ADDR_FDC_CYLINDER, 0x01); bios_write_byte(firmware, 0u, VBIOS_ADDR_FDC_SECTOR, 0x01); bios_write_byte(firmware, 0u, VBIOS_ADDR_FDC_BYTE_COUNT, 0x02);
    bios_write_byte(firmware, 0u, VBIOS_ADDR_VGA_VIDEO_MODE, 0x03); bios_write_word(firmware, 0u, VBIOS_ADDR_VGA_COLUMN, 0x0050); bios_write_word(firmware, 0u, VBIOS_ADDR_VGA_PAGE_SIZE, 0x1000); bios_write_word(firmware, 0u, VBIOS_ADDR_VGA_CURSOR_P0, 0x0500); bios_write_byte(firmware, 0u, VBIOS_ADDR_VGA_CURSOR_BOTTOM, 0x0e); bios_write_byte(firmware, 0u, VBIOS_ADDR_VGA_CURSOR_TOP, 0x0d); bios_write_word(firmware, 0u, VBIOS_ADDR_VGA_ACT_ADPT_PORT, 0x03d4); bios_write_byte(firmware, 0u, VBIOS_ADDR_VGA_MODE_REGISTER, 0x29); bios_write_byte(firmware, 0u, VBIOS_ADDR_VGA_COLOR_PALETTE, 0x30);
    bios_write_byte(firmware, 0u, VBIOS_ADDR_HDD_LST_OP_STATUS, 0u); bios_write_byte(firmware, 0u, VBIOS_ADDR_HDD_NUMBER, media.present ? 0x01 : 0u); bios_write_byte(firmware, 0u, VBIOS_ADDR_HDD_CONTROL, 0xc0); bios_write_byte(firmware, 0u, VBIOS_ADDR_PARA_TIMEOUT_LPT1, 0x14); bios_write_byte(firmware, 0u, VBIOS_ADDR_SERI_TIMEOUT_COM1, 0x0a); bios_write_word(firmware, 0u, VBIOS_ADDR_KEYB_BUFFER_START, 0x041e); bios_write_word(firmware, 0u, VBIOS_ADDR_KEYB_BUFFER_END, 0x043d);
    bios_write_byte(firmware, 0u, VBIOS_ADDR_VGA_ROW_NUMBER, 0x18); bios_write_word(firmware, 0u, VBIOS_ADDR_VGA_CHAR_HEIGHT, 0x0010); bios_write_byte(firmware, 0u, VBIOS_ADDR_VGA_MODE_OPTIONS1, 0x60); bios_write_byte(firmware, 0u, VBIOS_ADDR_VGA_MODE_OPTIONS2, 0x09); bios_write_byte(firmware, 0u, VBIOS_ADDR_VGA_DISPLAY_DATA, 0x11); bios_write_byte(firmware, 0u, VBIOS_ADDR_VGA_DCC_INDEX, 0x0b); bios_write_byte(firmware, 0u, VBIOS_ADDR_DRV_SAME_FLAG, 0x77); bios_write_byte(firmware, 0u, VBIOS_ADDR_DRV_MEDIA_STATE_D0, 0x17); bios_write_byte(firmware, 0u, VBIOS_ADDR_KEYB_MODE_TYPE, 0x10); bios_write_byte(firmware, 0u, VBIOS_ADDR_KEYB_LED_FLAG, 0x02); bios_write_dword(firmware, 0u, VBIOS_ADDR_VGA_VIDEO_TAB_PTR, 0xc0005d3a); bios_write_byte(firmware, 0u, VBIOS_ADDR_POST_WORK_AREA, VBIOS_POST_REPORT_NONE);
}
static C_VOID bios_load_additional(core_machine_firmware_context *firmware,
    const core_machine_media_registry *media_registry, core_machine_media_id hdd_media_id) {
    core_machine_media_info media = bios_media_info(media_registry, hdd_media_id);
    /* hard disk param table */
    bios_write_word(firmware, 0u, VBIOS_ADDR_HDD_PARAM_OFFSET, VBIOS_ADDR_HDD_PARAM);
    bios_write_word(firmware, 0u, VBIOS_ADDR_HDD_PARAM_SEGMENT, VBIOS_ADDR_START_SEG);
    bios_write_word(firmware, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 0, TYPE_MASK_UNSIGNED_16(media.geometry.cylinders));
    bios_write_byte(firmware, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 2, TYPE_MASK_UNSIGNED_8(media.geometry.heads));
    bios_write_byte(firmware, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 3, 0xa0);
    bios_write_byte(firmware, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 4, TYPE_MASK_UNSIGNED_8(media.geometry.sectors_per_track));
    bios_write_word(firmware, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 5, TYPE_MAX_UNSIGNED_16);
    bios_write_byte(firmware, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 7, 0u);
    bios_write_byte(firmware, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 8, 0x08);
    bios_write_word(firmware, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 9, TYPE_MASK_UNSIGNED_16(media.geometry.cylinders));
    bios_write_byte(firmware, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 11, TYPE_MASK_UNSIGNED_8(media.geometry.heads));
    bios_write_word(firmware, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 12, 0u);
    bios_write_byte(firmware, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 14, TYPE_MASK_UNSIGNED_8(media.geometry.sectors_per_track));
    bios_write_byte(firmware, VBIOS_ADDR_START_SEG, VBIOS_ADDR_HDD_PARAM + 15, 0u);
}

static C_INT bios_image_write_code(type_unsigned_8 *image, type_unsigned_16 offset,
    const vm_profile_default_bios_code *code)
{
    if (image == STD_NULL || code == STD_NULL || code->bytes == STD_NULL ||
        code->length == 0u || code->length > 0x10000u - offset) return 0;
    STD_MEMCPY(image + offset, code->bytes, code->length);
    return code->length;
}

static C_VOID bios_image_load_keyboard_tables(type_unsigned_8 *image)
{
    static const type_unsigned_8 normal[0x59] = {
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
    static const type_unsigned_8 shifted[0x59] = {
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

    if (image == STD_NULL) return;
    STD_MEMCPY(image + VBIOS_ADDR_KEYB_SCAN_ASCII_NORMAL, normal, sizeof(normal));
    STD_MEMCPY(image + VBIOS_ADDR_KEYB_SCAN_ASCII_SHIFT, shifted, sizeof(shifted));
}

static C_INT bios_image_load(t_bios *bios, type_unsigned_8 *image, type_unsigned_8 *ivt)
{
    static const type_unsigned_8 iret[] = { 0xcfu };
    vm_profile_default_bios_code code = { (type_unsigned_8 *)iret, sizeof(iret) };
    type_native_unsigned index;
    type_unsigned_16 build_ip;

    if (bios == STD_NULL || image == STD_NULL || ivt == STD_NULL) return 0;
    bios_image_load_keyboard_tables(image);
    image[VBIOS_ADDR_ROM_INFO] = 0x08u;
    image[VBIOS_ADDR_ROM_INFO + 1u] = 0x00u;
    image[VBIOS_ADDR_ROM_INFO + 2u] = 0xfcu;
    image[VBIOS_ADDR_ROM_INFO + 4u] = 0x01u;
    image[VBIOS_ADDR_ROM_INFO + 5u] = 0xb4u;
    image[VBIOS_ADDR_ROM_INFO + 6u] = 0x40u;
    build_ip = 0u;
    build_ip = (type_unsigned_16)(build_ip + bios_image_write_code(image, build_ip, &code));
    for (index = 0u; index < 0x100u; ++index) {
        type_unsigned_16 vector_offset = bios->connect.intTable[index].bytes == STD_NULL ?
            VBIOS_ADDR_START_OFF : build_ip;

        ivt[index * 4u] = TYPE_MASK_UNSIGNED_8(vector_offset);
        ivt[index * 4u + 1u] = TYPE_MASK_UNSIGNED_8(vector_offset >> 8);
        ivt[index * 4u + 2u] = TYPE_MASK_UNSIGNED_8(VBIOS_ADDR_START_SEG);
        ivt[index * 4u + 3u] = TYPE_MASK_UNSIGNED_8(VBIOS_ADDR_START_SEG >> 8);
        if (bios->connect.intTable[index].bytes != STD_NULL) {
            type_unsigned_16 length = (type_unsigned_16)bios_image_write_code(image, build_ip,
                &bios->connect.intTable[index]);
            if (length == 0u) return 0;
            build_ip = (type_unsigned_16)(build_ip + length);
        }
    }
    image[VBIOS_ADDR_POST_OFF] = 0xeau;
    image[VBIOS_ADDR_POST_OFF + 1u] = TYPE_MASK_UNSIGNED_8(build_ip);
    image[VBIOS_ADDR_POST_OFF + 2u] = TYPE_MASK_UNSIGNED_8(build_ip >> 8);
    image[VBIOS_ADDR_POST_OFF + 3u] = 0x00u;
    image[VBIOS_ADDR_POST_OFF + 4u] = 0xf0u;
    for (index = 0u; index < bios->connect.postCount; ++index) {
        type_unsigned_16 length = (type_unsigned_16)bios_image_write_code(image, build_ip,
            &bios->connect.postTable[index]);
        if (length == 0u) return 0;
        build_ip = (type_unsigned_16)(build_ip + length);
    }
    if (bios_image_write_code(image, build_ip, &bios->connect.bootCode) == 0u) {
        return 0;
    }
    bios->data.buildCS = VBIOS_ADDR_START_SEG;
    bios->data.buildIP = build_ip;
    return 1;
}

C_VOID vm_profile_default_bios_add_post_code(t_bios *bios, type_unsigned_8 *bytes,
    type_unsigned_16 length)
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

C_VOID vm_profile_default_bios_add_interrupt_code(t_bios *bios, type_unsigned_8 *bytes,
    type_unsigned_16 length, type_unsigned_8 intid)
{
    if (bios == STD_NULL || bytes == STD_NULL || length == 0u) {
        STD_FREE(bytes);
        return;
    }
    STD_FREE(bios->connect.intTable[intid].bytes);
    bios->connect.intTable[intid].bytes = bytes;
    bios->connect.intTable[intid].length = length;
}

C_VOID vm_profile_default_bios_set_boot_code(t_bios *bios, type_unsigned_8 *bytes,
    type_unsigned_16 length)
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

C_INT vm_profile_default_bios_materialize(t_bios *bios,
    core_machine_firmware_context *firmware)
{
    type_unsigned_8 *image;
    const type_unsigned_32 physical_start = 0x000f0000u;
    const STD_SIZE_T bytes = 0x10000u;
    const STD_SIZE_T mutable_offset = VBIOS_ADDR_HDD_PARAM;
    const STD_SIZE_T mutable_bytes = 16u;
    C_INT result;

    if (bios == STD_NULL || firmware == STD_NULL || bios->rom_materialized) return 0;
    image = (type_unsigned_8 *)STD_CALLOC(1u, bytes);
    if (image == STD_NULL) return 0;
    if (!bios_image_load(bios, image, bios->reset_ivt)) {
        STD_FREE(image);
        return 0;
    }
    result = core_machine_firmware_register_immutable_rom(firmware, physical_start, image,
            mutable_offset) == TYPE_STATUS_OK &&
        core_machine_firmware_register_immutable_rom(firmware,
            physical_start + mutable_offset + mutable_bytes,
            image + mutable_offset + mutable_bytes,
            bytes - mutable_offset - mutable_bytes) == TYPE_STATUS_OK &&
        core_machine_firmware_register_immutable_rom(firmware, 0xffeffff0u,
            image + VBIOS_ADDR_POST_OFF, 16u) == TYPE_STATUS_OK;
    STD_FREE(image);
    if (!result) return 0;
    bios->rom_materialized = TYPE_TRUE;
    return 1;
}

/* Loads bios to ram */
C_VOID vm_profile_default_bios_reset(t_bios *bios,
    core_machine_firmware_context *firmware,
    const core_machine_media_registry *media_registry,
    core_machine_media_id hdd_media_id) {
    STD_SIZE_T index;

    if (bios == STD_NULL || firmware == STD_NULL) return;
    if (!bios->rom_materialized) return;
    bios_load_data(bios, firmware, media_registry, hdd_media_id);
    for (index = 0u; index < sizeof(bios->reset_ivt); ++index) {
        bios_write_byte(firmware, 0u, TYPE_MASK_UNSIGNED_16(index),
            bios->reset_ivt[index]);
    }
    bios_load_additional(firmware, media_registry, hdd_media_id);
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

C_INT vm_profile_default_bios_take_boot_failure_report(
    core_machine_firmware_context *firmware)
{
    type_unsigned_8 report = VBIOS_POST_REPORT_NONE;

    if (firmware == STD_NULL || core_machine_firmware_memory_read(firmware,
            VBIOS_ADDR_POST_WORK_AREA, &report, sizeof(report)) != TYPE_STATUS_OK ||
        report != VBIOS_POST_REPORT_BOOT_FAILURE_ACKNOWLEDGED) {
        return 0;
    }
    report = VBIOS_POST_REPORT_NONE;
    return core_machine_firmware_memory_write(firmware, VBIOS_ADDR_POST_WORK_AREA,
        &report, sizeof(report)) == TYPE_STATUS_OK;
}
