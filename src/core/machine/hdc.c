#include "type.h"

#include "core/machine/media_interface.h"
#include "core/machine/pic.h"
#include "core/machine/hdc.h"

#define CORE_MACHINE_HDC_COMMAND_READ_SECTORS 0x20u
#define CORE_MACHINE_HDC_COMMAND_WRITE_SECTORS 0x30u
#define CORE_MACHINE_HDC_COMMAND_VERIFY_SECTORS 0x40u
#define CORE_MACHINE_HDC_COMMAND_INITIALIZE_DRIVE_PARAMETERS 0x91u
#define CORE_MACHINE_HDC_COMMAND_EXECUTE_DIAGNOSTICS 0x90u
#define CORE_MACHINE_HDC_COMMAND_IDENTIFY_DEVICE 0xecu
#define CORE_MACHINE_HDC_COMMAND_RECALIBRATE_MASK 0xf0u
#define CORE_MACHINE_HDC_COMMAND_RECALIBRATE_VALUE 0x10u
#define CORE_MACHINE_HDC_COMMAND_SEEK_MASK 0xf0u
#define CORE_MACHINE_HDC_COMMAND_SEEK_VALUE 0x70u
#define CORE_MACHINE_HDC_ERROR_DIAGNOSTIC_OK 0x01u
static C_INT core_machine_hdc_is_compaq_wd_40mb(const core_machine_hdc *hdc)
{
    return hdc != STD_NULL && hdc->connect.config.protocol ==
        CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB;
}

static C_INT core_machine_hdc_selected_master(const core_machine_hdc *hdc)
{ return hdc != STD_NULL && (hdc->data.drive_head & 0x10u) == 0u; }

static C_INT core_machine_hdc_lba_mode(const core_machine_hdc *hdc)
{
    return hdc != STD_NULL && !core_machine_hdc_is_compaq_wd_40mb(hdc) &&
        hdc->connect.config.lba28_supported && (hdc->data.drive_head & 0x40u) != 0u;
}

static C_INT core_machine_hdc_command_is_read(const core_machine_hdc *hdc,
    type_unsigned_8 command)
{
    if (core_machine_hdc_is_compaq_wd_40mb(hdc)) {
        return (command & 0xfeu) == CORE_MACHINE_HDC_COMMAND_READ_SECTORS;
    }
    return command == CORE_MACHINE_HDC_COMMAND_READ_SECTORS;
}

static C_INT core_machine_hdc_command_is_write(const core_machine_hdc *hdc,
    type_unsigned_8 command)
{
    if (core_machine_hdc_is_compaq_wd_40mb(hdc)) {
        return (command & 0xfeu) == CORE_MACHINE_HDC_COMMAND_WRITE_SECTORS;
    }
    return command == CORE_MACHINE_HDC_COMMAND_WRITE_SECTORS;
}

static C_VOID core_machine_hdc_clear_irq(core_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    hdc->data.irq_pending = TYPE_FALSE;
    core_machine_pic_irq_source_deassert(&hdc->connect.irq_source);
}

static C_VOID core_machine_hdc_raise_irq(core_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    if ((hdc->data.device_control & CORE_MACHINE_HDC_DEVICE_CONTROL_NIEN) != 0u) {
        hdc->data.irq_pending = TYPE_FALSE;
        core_machine_pic_irq_source_deassert(&hdc->connect.irq_source);
        return;
    }
    hdc->data.irq_pending = TYPE_TRUE;
    core_machine_pic_irq_source_assert(&hdc->connect.irq_source);
}

static type_unsigned_32 core_machine_hdc_lba(const core_machine_hdc *hdc)
{
    return (type_unsigned_32)hdc->data.sector_number |
        ((type_unsigned_32)hdc->data.cylinder_low << 8u) |
        ((type_unsigned_32)hdc->data.cylinder_high << 16u) |
        ((type_unsigned_32)(hdc->data.drive_head & 0x0fu) << 24u);
}

static C_INT core_machine_hdc_media_info(const core_machine_hdc *hdc,
    core_machine_media_info *out_info, core_machine_media_result *out_result)
{
    return hdc != STD_NULL && hdc->connect.media_registry != STD_NULL &&
        core_machine_media_query(hdc->connect.media_registry, hdc->connect.media_id,
            out_info, out_result) == TYPE_STATUS_OK &&
        *out_result == CORE_MACHINE_MEDIA_RESULT_OK;
}

static STD_SIZE_T core_machine_hdc_sector_capacity(
    const core_machine_media_info *info)
{
    return info == STD_NULL || info->geometry.bytes_per_sector == 0u ? 0u :
        (STD_SIZE_T)info->geometry.logical_sector_count;
}

static C_VOID core_machine_hdc_complete(core_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    hdc->data.phase = CORE_MACHINE_HDC_PHASE_IDLE;
    hdc->data.data_index = 0u;
    hdc->data.status = CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC;
    core_machine_hdc_raise_irq(hdc);
}

static C_VOID core_machine_hdc_fail(core_machine_hdc *hdc, type_unsigned_8 error)
{
    if (hdc == STD_NULL) return;
    hdc->data.error = error;
    hdc->data.phase = CORE_MACHINE_HDC_PHASE_IDLE;
    hdc->data.data_index = 0u;
    hdc->data.status = CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_ERR;
    core_machine_hdc_raise_irq(hdc);
}

static C_INT core_machine_hdc_load_chs_sector(core_machine_hdc *hdc)
{
    core_machine_media_info info;
    core_machine_media_result media_result;
    type_unsigned_16 cylinder;
    type_unsigned_8 head;
    type_unsigned_8 sector;
    STD_SIZE_T offset;

    if (hdc == STD_NULL || !core_machine_hdc_media_info(hdc, &info, &media_result) ||
        !info.present) {
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    cylinder = (type_unsigned_16)hdc->data.cylinder_low |
        ((type_unsigned_16)hdc->data.cylinder_high << 8u);
    head = hdc->data.drive_head & 0x0fu;
    sector = hdc->data.sector_number;
    if (!core_machine_hdc_selected_master(hdc) || core_machine_hdc_lba_mode(hdc) ||
        sector == 0u || cylinder >= info.geometry.cylinders ||
        head >= info.geometry.heads || sector > info.geometry.sectors_per_track ||
        info.geometry.bytes_per_sector != sizeof(hdc->data.data)) {
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ID_NOT_FOUND);
        return 0;
    }
    offset = (((STD_SIZE_T)cylinder * info.geometry.heads + head) *
        info.geometry.sectors_per_track + (sector - 1u)) * info.geometry.bytes_per_sector;
    if (core_machine_media_read_bytes(hdc->connect.media_registry,
            hdc->connect.media_id, offset, hdc->data.data, sizeof(hdc->data.data),
            &media_result) != TYPE_STATUS_OK ||
        media_result != CORE_MACHINE_MEDIA_RESULT_OK) {
        core_machine_hdc_fail(hdc, media_result == CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE ?
            CORE_MACHINE_HDC_ERROR_ID_NOT_FOUND : CORE_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    return 1;
}

static C_INT core_machine_hdc_load_lba_sector(core_machine_hdc *hdc)
{
    core_machine_media_info info;
    core_machine_media_result media_result;
    type_unsigned_32 lba;
    STD_SIZE_T offset;

    if (hdc == STD_NULL || !core_machine_hdc_media_info(hdc, &info, &media_result) ||
        !info.present) {
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    lba = core_machine_hdc_lba(hdc);
    if (!core_machine_hdc_selected_master(hdc) || !core_machine_hdc_lba_mode(hdc) ||
        (STD_SIZE_T)lba >= core_machine_hdc_sector_capacity(&info) ||
        info.geometry.bytes_per_sector != sizeof(hdc->data.data)) {
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ID_NOT_FOUND);
        return 0;
    }
    offset = (STD_SIZE_T)lba * info.geometry.bytes_per_sector;
    if (core_machine_media_read_bytes(hdc->connect.media_registry,
            hdc->connect.media_id, offset, hdc->data.data, sizeof(hdc->data.data),
            &media_result) != TYPE_STATUS_OK ||
        media_result != CORE_MACHINE_MEDIA_RESULT_OK) {
        core_machine_hdc_fail(hdc, media_result == CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE ?
            CORE_MACHINE_HDC_ERROR_ID_NOT_FOUND : CORE_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    return 1;
}

static C_INT core_machine_hdc_load_sector(core_machine_hdc *hdc)
{
    return core_machine_hdc_lba_mode(hdc) ? core_machine_hdc_load_lba_sector(hdc) :
        core_machine_hdc_load_chs_sector(hdc);
}

static C_INT core_machine_hdc_store_chs_sector(core_machine_hdc *hdc)
{
    core_machine_media_info info;
    core_machine_media_result media_result;
    type_unsigned_16 cylinder;
    type_unsigned_8 head;
    type_unsigned_8 sector;
    STD_SIZE_T offset;

    if (hdc == STD_NULL || !core_machine_hdc_media_info(hdc, &info, &media_result) ||
        !info.present || (info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_READ_ONLY) != 0u) {
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    cylinder = (type_unsigned_16)hdc->data.cylinder_low |
        ((type_unsigned_16)hdc->data.cylinder_high << 8u);
    head = hdc->data.drive_head & 0x0fu;
    sector = hdc->data.sector_number;
    if (!core_machine_hdc_selected_master(hdc) || core_machine_hdc_lba_mode(hdc) ||
        sector == 0u || cylinder >= info.geometry.cylinders ||
        head >= info.geometry.heads || sector > info.geometry.sectors_per_track ||
        info.geometry.bytes_per_sector != sizeof(hdc->data.data)) {
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ID_NOT_FOUND);
        return 0;
    }
    offset = (((STD_SIZE_T)cylinder * info.geometry.heads + head) *
        info.geometry.sectors_per_track + (sector - 1u)) * info.geometry.bytes_per_sector;
    if (core_machine_media_write_bytes(hdc->connect.media_registry,
            hdc->connect.media_id, offset, hdc->data.data, sizeof(hdc->data.data),
            &media_result) != TYPE_STATUS_OK ||
        media_result != CORE_MACHINE_MEDIA_RESULT_OK) {
        core_machine_hdc_fail(hdc, media_result == CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE ?
            CORE_MACHINE_HDC_ERROR_ID_NOT_FOUND : CORE_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    return 1;
}

static C_INT core_machine_hdc_store_lba_sector(core_machine_hdc *hdc)
{
    core_machine_media_info info;
    core_machine_media_result media_result;
    type_unsigned_32 lba;
    STD_SIZE_T offset;

    if (hdc == STD_NULL || !core_machine_hdc_media_info(hdc, &info, &media_result) ||
        !info.present || (info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_READ_ONLY) != 0u) {
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    lba = core_machine_hdc_lba(hdc);
    if (!core_machine_hdc_selected_master(hdc) || !core_machine_hdc_lba_mode(hdc) ||
        (STD_SIZE_T)lba >= core_machine_hdc_sector_capacity(&info) ||
        info.geometry.bytes_per_sector != sizeof(hdc->data.data)) {
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ID_NOT_FOUND);
        return 0;
    }
    offset = (STD_SIZE_T)lba * info.geometry.bytes_per_sector;
    if (core_machine_media_write_bytes(hdc->connect.media_registry,
            hdc->connect.media_id, offset, hdc->data.data, sizeof(hdc->data.data),
            &media_result) != TYPE_STATUS_OK ||
        media_result != CORE_MACHINE_MEDIA_RESULT_OK) {
        core_machine_hdc_fail(hdc, media_result == CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE ?
            CORE_MACHINE_HDC_ERROR_ID_NOT_FOUND : CORE_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    return 1;
}

static C_INT core_machine_hdc_store_sector(core_machine_hdc *hdc)
{
    return core_machine_hdc_lba_mode(hdc) ? core_machine_hdc_store_lba_sector(hdc) :
        core_machine_hdc_store_chs_sector(hdc);
}

static C_VOID core_machine_hdc_identify(core_machine_hdc *hdc)
{
    core_machine_media_info info;
    core_machine_media_result media_result;
    type_unsigned_16 word;

    if (hdc == STD_NULL || !core_machine_hdc_media_info(hdc, &info, &media_result) ||
        !core_machine_hdc_selected_master(hdc) ||
        !info.present || info.geometry.bytes_per_sector != sizeof(hdc->data.data)) {
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ABORT);
        return;
    }
    STD_MEMSET(hdc->data.data, 0, sizeof(hdc->data.data));
    word = 0x0040u;
    STD_MEMCPY(&hdc->data.data[0], &word, sizeof(word));
    word = info.geometry.cylinders;
    STD_MEMCPY(&hdc->data.data[2], &word, sizeof(word));
    word = info.geometry.heads;
    STD_MEMCPY(&hdc->data.data[6], &word, sizeof(word));
    word = info.geometry.sectors_per_track;
    STD_MEMCPY(&hdc->data.data[12], &word, sizeof(word));
    word = 0x0200u;
    STD_MEMCPY(&hdc->data.data[98], &word, sizeof(word));
    word = (type_unsigned_16)core_machine_hdc_sector_capacity(&info);
    STD_MEMCPY(&hdc->data.data[120], &word, sizeof(word));
    word = (type_unsigned_16)(core_machine_hdc_sector_capacity(&info) >> 16u);
    STD_MEMCPY(&hdc->data.data[122], &word, sizeof(word));
    hdc->data.phase = CORE_MACHINE_HDC_PHASE_DATA_READ;
    hdc->data.data_index = 0u;
    hdc->data.error = 0u;
    hdc->data.status = CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC |
        CORE_MACHINE_HDC_STATUS_DRQ;
    core_machine_hdc_raise_irq(hdc);
}

static C_INT core_machine_hdc_advance_chs(core_machine_hdc *hdc)
{
    core_machine_media_info info;
    core_machine_media_result media_result;
    type_unsigned_16 cylinder;
    type_unsigned_8 head;
    type_unsigned_8 sector;

    if (hdc == STD_NULL || !core_machine_hdc_media_info(hdc, &info, &media_result) ||
        !info.present) {
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    cylinder = (type_unsigned_16)hdc->data.cylinder_low |
        ((type_unsigned_16)hdc->data.cylinder_high << 8u);
    head = hdc->data.drive_head & 0x0fu;
    sector = (type_unsigned_8)(hdc->data.sector_number + 1u);
    if (sector > info.geometry.sectors_per_track) {
        sector = 1u;
        ++head;
        if (head >= info.geometry.heads) {
            head = 0u;
            ++cylinder;
        }
    }
    if (cylinder >= info.geometry.cylinders) {
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ID_NOT_FOUND);
        return 0;
    }
    hdc->data.sector_number = sector;
    hdc->data.cylinder_low = (type_unsigned_8)cylinder;
    hdc->data.cylinder_high = (type_unsigned_8)(cylinder >> 8u);
    hdc->data.drive_head = (hdc->data.drive_head & 0xf0u) | head;
    return 1;
}

static C_INT core_machine_hdc_advance_lba(core_machine_hdc *hdc)
{
    core_machine_media_info info;
    core_machine_media_result media_result;
    type_unsigned_32 lba;

    if (hdc == STD_NULL || !core_machine_hdc_media_info(hdc, &info, &media_result) ||
        !info.present) {
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    lba = core_machine_hdc_lba(hdc) + 1u;
    if ((STD_SIZE_T)lba >= core_machine_hdc_sector_capacity(&info)) {
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ID_NOT_FOUND);
        return 0;
    }
    hdc->data.sector_number = (type_unsigned_8)lba;
    hdc->data.cylinder_low = (type_unsigned_8)(lba >> 8u);
    hdc->data.cylinder_high = (type_unsigned_8)(lba >> 16u);
    hdc->data.drive_head = (hdc->data.drive_head & 0xf0u) |
        (type_unsigned_8)(lba >> 24u);
    return 1;
}

static C_INT core_machine_hdc_advance_sector(core_machine_hdc *hdc)
{
    return core_machine_hdc_lba_mode(hdc) ? core_machine_hdc_advance_lba(hdc) :
        core_machine_hdc_advance_chs(hdc);
}

static C_VOID core_machine_hdc_complete_data_sector(core_machine_hdc *hdc)
{
    if (hdc == STD_NULL || hdc->data.sectors_remaining == 0u) return;
    --hdc->data.sectors_remaining;
    --hdc->data.sector_count;
}

static C_VOID core_machine_hdc_next_read_sector(core_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    core_machine_hdc_complete_data_sector(hdc);
    if (hdc->data.sectors_remaining == 0u) {
        core_machine_hdc_complete(hdc);
        return;
    }
    if (!core_machine_hdc_advance_sector(hdc) || !core_machine_hdc_load_sector(hdc)) {
        return;
    }
    hdc->data.data_index = 0u;
    hdc->data.phase = CORE_MACHINE_HDC_PHASE_DATA_READ;
    hdc->data.status = CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC |
        CORE_MACHINE_HDC_STATUS_DRQ;
    core_machine_hdc_raise_irq(hdc);
}

static C_VOID core_machine_hdc_next_write_sector(core_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    if (!core_machine_hdc_store_sector(hdc)) return;
    core_machine_hdc_complete_data_sector(hdc);
    if (hdc->data.sectors_remaining == 0u) {
        core_machine_hdc_complete(hdc);
        return;
    }
    if (!core_machine_hdc_advance_sector(hdc)) return;
    hdc->data.data_index = 0u;
    STD_MEMSET(hdc->data.data, 0, sizeof(hdc->data.data));
    hdc->data.phase = CORE_MACHINE_HDC_PHASE_DATA_WRITE;
    hdc->data.status = CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC |
        CORE_MACHINE_HDC_STATUS_DRQ;
    core_machine_hdc_raise_irq(hdc);
}

static C_VOID core_machine_hdc_capture_command(core_machine_hdc *hdc,
    type_unsigned_8 command)
{
    if (hdc == STD_NULL || hdc->data.reset_asserted) return;
    hdc->data.pending_command = command;
    hdc->data.pending_features = hdc->data.features;
    hdc->data.pending_sector_count = hdc->data.sector_count;
    hdc->data.pending_sector_number = hdc->data.sector_number;
    hdc->data.pending_cylinder_low = hdc->data.cylinder_low;
    hdc->data.pending_cylinder_high = hdc->data.cylinder_high;
    hdc->data.pending_drive_head = hdc->data.drive_head;
    hdc->data.error = 0u;
    core_machine_hdc_clear_irq(hdc);
    hdc->data.phase = CORE_MACHINE_HDC_PHASE_PENDING_COMMAND;
    hdc->data.status = CORE_MACHINE_HDC_STATUS_BSY;
}

static C_VOID core_machine_hdc_begin_read(core_machine_hdc *hdc)
{
    if (!core_machine_hdc_load_sector(hdc)) return;
    hdc->data.phase = CORE_MACHINE_HDC_PHASE_DATA_READ;
    hdc->data.sectors_remaining = hdc->data.sector_count == 0u ? 256u :
        hdc->data.sector_count;
    hdc->data.data_index = 0u;
    hdc->data.status = CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC |
        CORE_MACHINE_HDC_STATUS_DRQ;
    core_machine_hdc_raise_irq(hdc);
}

static C_VOID core_machine_hdc_begin_write(core_machine_hdc *hdc)
{
    if (!core_machine_hdc_load_sector(hdc)) return;
    hdc->data.phase = CORE_MACHINE_HDC_PHASE_DATA_WRITE;
    hdc->data.sectors_remaining = hdc->data.sector_count == 0u ? 256u :
        hdc->data.sector_count;
    hdc->data.data_index = 0u;
    hdc->data.status = CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC |
        CORE_MACHINE_HDC_STATUS_DRQ;
    core_machine_hdc_raise_irq(hdc);
}

static C_VOID core_machine_hdc_execute_command(core_machine_hdc *hdc, type_unsigned_8 command)
{
    type_unsigned_16 cylinder;

    if (hdc == STD_NULL) return;
    hdc->data.last_command = command;
    ++hdc->data.command_count;
    if (!core_machine_hdc_selected_master(hdc) ||
        (core_machine_hdc_is_compaq_wd_40mb(hdc) &&
            (hdc->data.drive_head & 0x20u) == 0u) ||
        (!core_machine_hdc_is_compaq_wd_40mb(hdc) &&
            (hdc->data.drive_head & 0x40u) != 0u &&
            !hdc->connect.config.lba28_supported)) {
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ABORT);
        return;
    }
    if (core_machine_hdc_command_is_read(hdc, command)) {
        core_machine_hdc_begin_read(hdc);
        return;
    }
    if (core_machine_hdc_command_is_write(hdc, command)) {
        core_machine_hdc_begin_write(hdc);
        return;
    }
    if (core_machine_hdc_is_compaq_wd_40mb(hdc)) {
        if (command == CORE_MACHINE_HDC_COMMAND_INITIALIZE_DRIVE_PARAMETERS) {
            core_machine_hdc_complete(hdc);
            return;
        }
        if (command == CORE_MACHINE_HDC_COMMAND_EXECUTE_DIAGNOSTICS) {
            hdc->data.error = CORE_MACHINE_HDC_ERROR_DIAGNOSTIC_OK;
            core_machine_hdc_complete(hdc);
            return;
        }
        if (command == CORE_MACHINE_HDC_COMMAND_VERIFY_SECTORS) {
            if (core_machine_hdc_load_sector(hdc)) core_machine_hdc_complete(hdc);
            return;
        }
        if ((command & CORE_MACHINE_HDC_COMMAND_RECALIBRATE_MASK) ==
            CORE_MACHINE_HDC_COMMAND_RECALIBRATE_VALUE) {
            hdc->data.cylinder_low = 0u;
            hdc->data.cylinder_high = 0u;
            core_machine_hdc_complete(hdc);
            return;
        }
        if ((command & CORE_MACHINE_HDC_COMMAND_SEEK_MASK) ==
            CORE_MACHINE_HDC_COMMAND_SEEK_VALUE) {
            cylinder = (type_unsigned_16)hdc->data.cylinder_low |
                ((type_unsigned_16)hdc->data.cylinder_high << 8u);
            if (cylinder > 1023u) {
                core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ID_NOT_FOUND);
            } else {
                core_machine_hdc_complete(hdc);
            }
            return;
        }
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ABORT);
        return;
    }
    if (command == CORE_MACHINE_HDC_COMMAND_IDENTIFY_DEVICE) {
        core_machine_hdc_identify(hdc);
    } else {
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ABORT);
    }
}
static type_status core_machine_hdc_port_read(C_VOID *opaque, type_unsigned_16 port,
    type_unsigned_32 *out_value)
{
    core_machine_hdc *hdc = (core_machine_hdc *)opaque;
    type_unsigned_16 word;

    if (hdc == STD_NULL || out_value == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_value = 0u;
    if (port == hdc->connect.config.data_port) {
        if (hdc->data.phase != CORE_MACHINE_HDC_PHASE_DATA_READ ||
            hdc->data.data_index >= sizeof(hdc->data.data)) {
            return TYPE_STATUS_OK;
        }
        STD_MEMCPY(&word, &hdc->data.data[hdc->data.data_index], sizeof(word));
        *out_value = word;
        hdc->data.data_index = (type_unsigned_16)(hdc->data.data_index + sizeof(word));
        if (hdc->data.data_index == sizeof(hdc->data.data)) {
            hdc->data.phase = CORE_MACHINE_HDC_PHASE_PENDING_READ_SECTOR;
            hdc->data.status = CORE_MACHINE_HDC_STATUS_BSY;
        }
        return TYPE_STATUS_OK;
    }
    if (port == hdc->connect.config.error_features_port) {
        *out_value = hdc->data.error;
    } else if (port == hdc->connect.config.sector_count_port) {
        *out_value = hdc->data.sector_count;
    } else if (port == hdc->connect.config.sector_number_port) {
        *out_value = hdc->data.sector_number;
    } else if (port == hdc->connect.config.cylinder_low_port) {
        *out_value = hdc->data.cylinder_low;
    } else if (port == hdc->connect.config.cylinder_high_port) {
        *out_value = hdc->data.cylinder_high;
    } else if (port == hdc->connect.config.drive_head_port) {
        *out_value = hdc->data.drive_head;
    } else if (port == hdc->connect.config.status_command_port) {
        *out_value = hdc->data.status;
        core_machine_hdc_clear_irq(hdc);
    } else if (port == hdc->connect.config.alternate_status_device_control_port) {
        *out_value = hdc->data.status;
    } else if (port == hdc->connect.config.drive_address_port &&
        core_machine_hdc_is_compaq_wd_40mb(hdc)) {
        *out_value = hdc->data.drive_head & 0x1fu;
    } else {
        return TYPE_STATUS_UNSUPPORTED;
    }
    return TYPE_STATUS_OK;
}

static type_status core_machine_hdc_port_write(C_VOID *opaque, type_unsigned_16 port,
    type_unsigned_32 value)
{
    core_machine_hdc *hdc = (core_machine_hdc *)opaque;

    if (hdc == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (port == hdc->connect.config.data_port) {
        type_unsigned_16 word = (type_unsigned_16)value;
        if (hdc->data.phase != CORE_MACHINE_HDC_PHASE_DATA_WRITE ||
            hdc->data.data_index >= sizeof(hdc->data.data)) {
            return TYPE_STATUS_OK;
        }
        STD_MEMCPY(&hdc->data.data[hdc->data.data_index], &word, sizeof(word));
        hdc->data.data_index = (type_unsigned_16)(hdc->data.data_index + sizeof(word));
        if (hdc->data.data_index == sizeof(hdc->data.data)) {
            hdc->data.phase = CORE_MACHINE_HDC_PHASE_PENDING_WRITE_SECTOR;
            hdc->data.status = CORE_MACHINE_HDC_STATUS_BSY;
        }
        return TYPE_STATUS_OK;
    }
    if (port == hdc->connect.config.error_features_port) {
        hdc->data.features = (type_unsigned_8)value;
    } else if (port == hdc->connect.config.sector_count_port) {
        hdc->data.sector_count = (type_unsigned_8)value;
    } else if (port == hdc->connect.config.sector_number_port) {
        hdc->data.sector_number = (type_unsigned_8)value;
    } else if (port == hdc->connect.config.cylinder_low_port) {
        hdc->data.cylinder_low = (type_unsigned_8)value;
    } else if (port == hdc->connect.config.cylinder_high_port) {
        hdc->data.cylinder_high = (type_unsigned_8)value;
    } else if (port == hdc->connect.config.drive_head_port) {
        hdc->data.drive_head = (type_unsigned_8)value;
    } else if (port == hdc->connect.config.status_command_port) {
        core_machine_hdc_capture_command(hdc, (type_unsigned_8)value);
    } else if (port == hdc->connect.config.alternate_status_device_control_port) {
        type_unsigned_8 device_control = (type_unsigned_8)value;
        type_bool reset_asserted = (device_control &
            CORE_MACHINE_HDC_DEVICE_CONTROL_SRST) != 0u;

        hdc->data.device_control = device_control;
        if ((device_control & CORE_MACHINE_HDC_DEVICE_CONTROL_NIEN) != 0u) {
            core_machine_hdc_clear_irq(hdc);
        }
        if (reset_asserted && !hdc->data.reset_asserted) {
            hdc->data.reset_asserted = TYPE_TRUE;
            hdc->data.phase = CORE_MACHINE_HDC_PHASE_IDLE;
            hdc->data.data_index = 0u;
            hdc->data.sectors_remaining = 0u;
            hdc->data.error = 0u;
            hdc->data.status = CORE_MACHINE_HDC_STATUS_BSY;
            core_machine_hdc_clear_irq(hdc);
        } else if (!reset_asserted && hdc->data.reset_asserted) {
            core_machine_hdc_reset(hdc);
            hdc->data.device_control = device_control;
        }
    } else {
        return TYPE_STATUS_UNSUPPORTED;
    }
    return TYPE_STATUS_OK;
}

static const core_machine_port_provider core_machine_hdc_ports = {
    core_machine_hdc_port_read,
    core_machine_hdc_port_write
};

C_VOID core_machine_hdc_connect(core_machine_hdc *hdc,
    const core_machine_media_registry *media_registry,
    core_machine_media_id media_id,
    t_pic *pic_master, t_pic *pic_slave, const core_machine_hdc_config *config)
{
    if (hdc == STD_NULL || config == STD_NULL) return;
    hdc->connect.media_registry = media_registry;
    hdc->connect.media_id = media_id;
    core_machine_pic_irq_source_bind(&hdc->connect.irq_source, pic_master,
        pic_slave, config->irq);
    hdc->connect.config = *config;
}

C_VOID core_machine_hdc_initialize(core_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    STD_MEMSET(&hdc->data, 0, sizeof(hdc->data));
    core_machine_pic_irq_source_deassert(&hdc->connect.irq_source);
    core_machine_hdc_reset(hdc);
}

C_VOID core_machine_hdc_reset(core_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    STD_MEMSET(&hdc->data, 0, sizeof(hdc->data));
    core_machine_hdc_clear_irq(hdc);
    hdc->data.status = CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC;
}

C_VOID core_machine_hdc_advance(core_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    if (hdc->data.phase == CORE_MACHINE_HDC_PHASE_PENDING_COMMAND) {
        hdc->data.features = hdc->data.pending_features;
        hdc->data.sector_count = hdc->data.pending_sector_count;
        hdc->data.sector_number = hdc->data.pending_sector_number;
        hdc->data.cylinder_low = hdc->data.pending_cylinder_low;
        hdc->data.cylinder_high = hdc->data.pending_cylinder_high;
        hdc->data.drive_head = hdc->data.pending_drive_head;
        core_machine_hdc_execute_command(hdc, hdc->data.pending_command);
    } else if (hdc->data.phase == CORE_MACHINE_HDC_PHASE_PENDING_READ_SECTOR) {
        core_machine_hdc_next_read_sector(hdc);
    } else if (hdc->data.phase == CORE_MACHINE_HDC_PHASE_PENDING_WRITE_SECTOR) {
        core_machine_hdc_next_write_sector(hdc);
    }
}

C_VOID core_machine_hdc_refresh(core_machine_hdc *hdc)
{
    (C_VOID)hdc;
}

C_VOID core_machine_hdc_finalize(core_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    core_machine_pic_irq_source_deassert(&hdc->connect.irq_source);
    STD_MEMSET(&hdc->data, 0, sizeof(hdc->data));
    STD_MEMSET(&hdc->connect, 0, sizeof(hdc->connect));
}

const core_machine_port_provider *core_machine_hdc_port_provider(C_VOID)
{
    return &core_machine_hdc_ports;
}

type_bool core_machine_hdc_irq_pending(const core_machine_hdc *hdc)
{
    return hdc != STD_NULL && hdc->data.irq_pending;
}
