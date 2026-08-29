#include "type.h"

#include "core/machine/media_interface.h"
#include "core/machine/dma.h"
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
#define CORE_MACHINE_HDC_IBM_RESTORE_STEP_LIMIT 1023u
static C_INT core_machine_hdc_is_compaq_wd_40mb(const core_machine_hdc *hdc)
{
    return hdc != STD_NULL && hdc->connect.config.protocol ==
        CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB;
}

static C_INT core_machine_hdc_is_ibm_wd1003(const core_machine_hdc *hdc)
{
    return hdc != STD_NULL && hdc->connect.config.protocol ==
        CORE_MACHINE_HDC_PROTOCOL_IBM_WD1003_ST506;
}

static C_INT core_machine_hdc_is_xebec_xt(const core_machine_hdc *hdc)
{
    return hdc != STD_NULL && hdc->connect.config.protocol ==
        CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT;
}

static C_INT core_machine_hdc_task_file_is_writable(const core_machine_hdc *hdc)
{
    return hdc != STD_NULL && (core_machine_hdc_is_compaq_wd_40mb(hdc) ||
        (hdc->data.status & (CORE_MACHINE_HDC_STATUS_BSY |
            CORE_MACHINE_HDC_STATUS_DRQ)) == 0u);
}

static C_INT core_machine_hdc_selected_master(const core_machine_hdc *hdc)
{
    return hdc != STD_NULL && (core_machine_hdc_is_compaq_wd_40mb(hdc) ?
        (hdc->data.drive_head & 0x20u) != 0u :
        (hdc->data.drive_head & 0x10u) == 0u);
}

static type_unsigned_8 core_machine_hdc_current_head(const core_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return 0u;
    if (core_machine_hdc_is_ibm_wd1003(hdc)) {
        return (type_unsigned_8)((hdc->data.drive_head & 0x07u) |
            (hdc->data.fixed_disk_register & 0x08u));
    }
    return hdc->data.drive_head & 0x0fu;
}

static C_VOID core_machine_hdc_set_current_head(core_machine_hdc *hdc,
    type_unsigned_8 head)
{
    if (hdc == STD_NULL) return;
    if (core_machine_hdc_is_ibm_wd1003(hdc)) {
        hdc->data.drive_head = (type_unsigned_8)((hdc->data.drive_head & 0xf8u) |
            (head & 0x07u));
        hdc->data.fixed_disk_register = (type_unsigned_8)((hdc->data.fixed_disk_register &
            0xf7u) | (head & 0x08u));
        return;
    }
    hdc->data.drive_head = (type_unsigned_8)((hdc->data.drive_head & 0xf0u) | head);
}

static C_INT core_machine_hdc_lba_mode(const core_machine_hdc *hdc)
{
    return hdc != STD_NULL && !core_machine_hdc_is_compaq_wd_40mb(hdc) &&
        hdc->connect.config.bus.task_file.lba28_supported &&
        (hdc->data.drive_head & 0x40u) != 0u;
}

static C_INT core_machine_hdc_command_is_read(const core_machine_hdc *hdc,
    type_unsigned_8 command)
{
    if (core_machine_hdc_is_compaq_wd_40mb(hdc)) {
        return (command & 0xfeu) == CORE_MACHINE_HDC_COMMAND_READ_SECTORS;
    }
    if (core_machine_hdc_is_ibm_wd1003(hdc)) {
        return (command & 0xfcu) == CORE_MACHINE_HDC_COMMAND_READ_SECTORS;
    }
    return command == CORE_MACHINE_HDC_COMMAND_READ_SECTORS;
}

static C_INT core_machine_hdc_command_is_write(const core_machine_hdc *hdc,
    type_unsigned_8 command)
{
    if (core_machine_hdc_is_compaq_wd_40mb(hdc)) {
        return (command & 0xfeu) == CORE_MACHINE_HDC_COMMAND_WRITE_SECTORS;
    }
    if (core_machine_hdc_is_ibm_wd1003(hdc)) {
        return (command & 0xfcu) == CORE_MACHINE_HDC_COMMAND_WRITE_SECTORS;
    }
    return command == CORE_MACHINE_HDC_COMMAND_WRITE_SECTORS;
}

static C_VOID core_machine_hdc_select_ibm_step_rate(core_machine_hdc *hdc,
    type_unsigned_8 selector, type_unsigned_16 pulse_limit)
{
    type_unsigned_32 ticks_per_second;

    if (!core_machine_hdc_is_ibm_wd1003(hdc)) return;
    ticks_per_second = hdc->connect.config.bus.task_file.clock_ticks_per_second;
    hdc->data.step_rate_selector = selector;
    hdc->data.step_pulse_limit = pulse_limit;
    hdc->data.step_rate_ticks = selector == 0u ?
        (ticks_per_second / 1000000u) * 35u :
        (ticks_per_second / 2000u) * selector;
}

static C_VOID core_machine_hdc_sync_irq(core_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    if (hdc->data.irq_pending &&
        (hdc->data.device_control & CORE_MACHINE_HDC_DEVICE_CONTROL_NIEN) == 0u) {
        core_machine_pic_irq_source_assert(&hdc->connect.irq_source);
    } else {
        core_machine_pic_irq_source_deassert(&hdc->connect.irq_source);
    }
}

static C_VOID core_machine_hdc_clear_irq(core_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    hdc->data.irq_pending = TYPE_FALSE;
    core_machine_hdc_sync_irq(hdc);
}

static C_VOID core_machine_hdc_raise_irq(core_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    hdc->data.irq_pending = TYPE_TRUE;
    core_machine_hdc_sync_irq(hdc);
}

static type_unsigned_32 core_machine_hdc_lba(const core_machine_hdc *hdc)
{
    return (type_unsigned_32)hdc->data.sector_number |
        ((type_unsigned_32)hdc->data.cylinder_low << 8u) |
        ((type_unsigned_32)hdc->data.cylinder_high << 16u) |
        ((type_unsigned_32)(hdc->data.drive_head & 0x0fu) << 24u);
}

static core_machine_media_id core_machine_hdc_selected_media_id(const core_machine_hdc *hdc)
{
    return hdc != STD_NULL && (hdc->data.drive_head & 0x10u) != 0u ?
        hdc->connect.slave_media_id : hdc->connect.media_id;
}

static C_INT core_machine_hdc_media_info(const core_machine_hdc *hdc,
    core_machine_media_info *out_info, core_machine_media_result *out_result)
{
    return hdc != STD_NULL && hdc->connect.media_registry != STD_NULL &&
        core_machine_hdc_selected_media_id(hdc) != CORE_MACHINE_MEDIA_ID_INVALID &&
        core_machine_media_query(hdc->connect.media_registry,
            core_machine_hdc_selected_media_id(hdc), out_info, out_result) == TYPE_STATUS_OK &&
        *out_result == CORE_MACHINE_MEDIA_RESULT_OK;
}

static C_VOID core_machine_hdc_refresh_compaq_selection_status(core_machine_hdc *hdc)
{
    core_machine_media_info info;
    core_machine_media_result result;

    if (!core_machine_hdc_is_compaq_wd_40mb(hdc) ||
        hdc->data.phase != CORE_MACHINE_HDC_PHASE_IDLE) return;
    hdc->data.status = core_machine_hdc_media_info(hdc, &info, &result) ?
        CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC : 0u;
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

static C_INT core_machine_hdc_resolve_sector(core_machine_hdc *hdc,
    type_bool write_to_media, STD_SIZE_T *out_offset)
{
    core_machine_media_info info;
    core_machine_media_result media_result;
    type_unsigned_16 cylinder;
    type_unsigned_8 head;
    type_unsigned_8 sector;
    type_unsigned_32 lba;

    if (hdc == STD_NULL || out_offset == STD_NULL ||
        !core_machine_hdc_media_info(hdc, &info, &media_result) || !info.present ||
        (write_to_media && (info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_READ_ONLY) != 0u)) {
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    if (core_machine_hdc_lba_mode(hdc)) {
        lba = core_machine_hdc_lba(hdc);
        if (!core_machine_hdc_selected_master(hdc) ||
            (STD_SIZE_T)lba >= core_machine_hdc_sector_capacity(&info) ||
            info.geometry.bytes_per_sector != sizeof(hdc->data.data)) {
            core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ID_NOT_FOUND);
            return 0;
        }
        *out_offset = (STD_SIZE_T)lba * info.geometry.bytes_per_sector;
        return 1;
    }
    cylinder = (type_unsigned_16)hdc->data.cylinder_low |
        ((type_unsigned_16)hdc->data.cylinder_high << 8u);
    head = core_machine_hdc_current_head(hdc);
    sector = hdc->data.sector_number;
    if ((!core_machine_hdc_is_compaq_wd_40mb(hdc) && !core_machine_hdc_selected_master(hdc)) ||
        sector == 0u || cylinder >= info.geometry.cylinders ||
        head >= info.geometry.heads || sector > info.geometry.sectors_per_track ||
        info.geometry.bytes_per_sector != sizeof(hdc->data.data)) {
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ID_NOT_FOUND);
        return 0;
    }
    *out_offset = (((STD_SIZE_T)cylinder * info.geometry.heads + head) *
        info.geometry.sectors_per_track + (sector - 1u)) * info.geometry.bytes_per_sector;
    return 1;
}

static C_INT core_machine_hdc_load_sector(core_machine_hdc *hdc)
{
    core_machine_media_result media_result;
    STD_SIZE_T offset;

    if (!core_machine_hdc_resolve_sector(hdc, TYPE_FALSE, &offset)) return 0;
    if (core_machine_media_read_bytes(hdc->connect.media_registry,
        core_machine_hdc_selected_media_id(hdc), offset, hdc->data.data, sizeof(hdc->data.data),
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
    core_machine_media_result media_result;
    STD_SIZE_T offset;

    if (!core_machine_hdc_resolve_sector(hdc, TYPE_TRUE, &offset)) return 0;
    if (core_machine_media_write_bytes(hdc->connect.media_registry,
            core_machine_hdc_selected_media_id(hdc), offset, hdc->data.data, sizeof(hdc->data.data),
            &media_result) != TYPE_STATUS_OK ||
        media_result != CORE_MACHINE_MEDIA_RESULT_OK) {
        core_machine_hdc_fail(hdc, media_result == CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE ?
            CORE_MACHINE_HDC_ERROR_ID_NOT_FOUND : CORE_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    return 1;
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
    head = core_machine_hdc_current_head(hdc);
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
    core_machine_hdc_set_current_head(hdc, head);
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
    if (hdc == STD_NULL || hdc->data.reset_asserted ||
        !core_machine_hdc_task_file_is_writable(hdc)) return;
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
    STD_SIZE_T offset;

    if (!core_machine_hdc_resolve_sector(hdc, TYPE_TRUE, &offset)) return;
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
        (!core_machine_hdc_is_compaq_wd_40mb(hdc) && !core_machine_hdc_is_ibm_wd1003(hdc) &&
            (hdc->data.drive_head & 0x40u) != 0u &&
            !hdc->connect.config.bus.task_file.lba28_supported)) {
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
    if (core_machine_hdc_is_compaq_wd_40mb(hdc) || core_machine_hdc_is_ibm_wd1003(hdc)) {
        if (command == CORE_MACHINE_HDC_COMMAND_INITIALIZE_DRIVE_PARAMETERS) {
            core_machine_hdc_complete(hdc);
            return;
        }
        if (command == CORE_MACHINE_HDC_COMMAND_EXECUTE_DIAGNOSTICS) {
            core_machine_hdc_select_ibm_step_rate(hdc, 15u,
                CORE_MACHINE_HDC_IBM_RESTORE_STEP_LIMIT);
            hdc->data.error = CORE_MACHINE_HDC_ERROR_DIAGNOSTIC_OK;
            core_machine_hdc_complete(hdc);
            return;
        }
        if ((core_machine_hdc_is_ibm_wd1003(hdc) &&
                (command & 0xfeu) == CORE_MACHINE_HDC_COMMAND_VERIFY_SECTORS) ||
            (!core_machine_hdc_is_ibm_wd1003(hdc) &&
                command == CORE_MACHINE_HDC_COMMAND_VERIFY_SECTORS)) {
            if (core_machine_hdc_load_sector(hdc)) core_machine_hdc_complete(hdc);
            return;
        }
        if ((command & CORE_MACHINE_HDC_COMMAND_RECALIBRATE_MASK) ==
            CORE_MACHINE_HDC_COMMAND_RECALIBRATE_VALUE) {
            core_machine_hdc_select_ibm_step_rate(hdc, command & 0x0fu,
                CORE_MACHINE_HDC_IBM_RESTORE_STEP_LIMIT);
            hdc->data.cylinder_low = 0u;
            hdc->data.cylinder_high = 0u;
            core_machine_hdc_complete(hdc);
            return;
        }
        if ((command & CORE_MACHINE_HDC_COMMAND_SEEK_MASK) ==
            CORE_MACHINE_HDC_COMMAND_SEEK_VALUE) {
            core_machine_hdc_select_ibm_step_rate(hdc, command & 0x0fu, 0u);
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

static C_VOID core_machine_xebec_reset(core_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    if (hdc->connect.dma_request_deassert != STD_NULL) {
        hdc->connect.dma_request_deassert(hdc->connect.dma_request_owner,
            &hdc->connect.dma_request);
    }
    STD_MEMSET(&hdc->xebec, 0, sizeof(hdc->xebec));
    hdc->xebec.phase = CORE_MACHINE_XEBEC_PHASE_IDLE;
    core_machine_hdc_clear_irq(hdc);
}

static C_VOID core_machine_xebec_response(core_machine_hdc *hdc,
    type_unsigned_8 status, const type_unsigned_8 *sense)
{
    type_unsigned_8 drive;

    if (hdc == STD_NULL) return;
    drive = (hdc->xebec.dcb[1] >> 5u) & 1u;
    hdc->xebec.response[0] = (type_unsigned_8)((drive << 5u) | status);
    hdc->xebec.response_count = 1u;
    hdc->xebec.response_index = 0u;
    if ((status & 0x02u) != 0u && sense != STD_NULL) {
        STD_MEMCPY(hdc->xebec.last_sense, sense, sizeof(hdc->xebec.last_sense));
    }
    hdc->xebec.phase = CORE_MACHINE_XEBEC_PHASE_RESPONSE;
}

static C_VOID core_machine_xebec_request_dma(core_machine_hdc *hdc)
{
    if (hdc != STD_NULL && hdc->connect.dma_request_assert != STD_NULL) {
        hdc->connect.dma_request_assert(hdc->connect.dma_request_owner,
            &hdc->connect.dma_request);
    }
}

static C_VOID core_machine_xebec_release_dma(core_machine_hdc *hdc)
{
    if (hdc != STD_NULL && hdc->connect.dma_request_deassert != STD_NULL) {
        hdc->connect.dma_request_deassert(hdc->connect.dma_request_owner,
            &hdc->connect.dma_request);
    }
}

static C_INT core_machine_xebec_media_info(const core_machine_hdc *hdc,
    core_machine_media_info *out_info, core_machine_media_result *out_result)
{
    core_machine_media_id media_id;
    const core_machine_media_geometry *expected;

    if (hdc == STD_NULL || out_info == STD_NULL || out_result == STD_NULL ||
        hdc->connect.media_registry == STD_NULL ||
        ((hdc->xebec.dcb[1] >> 5u) & 1u) != 0u) return 0;
    media_id = hdc->connect.media_id;
    if (media_id == CORE_MACHINE_MEDIA_ID_INVALID ||
        core_machine_media_query(hdc->connect.media_registry, media_id, out_info,
            out_result) != TYPE_STATUS_OK || *out_result != CORE_MACHINE_MEDIA_RESULT_OK ||
        !out_info->present || (out_info->capabilities &
            CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN) == 0u) return 0;
    expected = &hdc->connect.config.bus.xebec.expected_media_geometry;
    return out_info->geometry.logical_sector_count == expected->logical_sector_count &&
        out_info->geometry.bytes_per_sector == expected->bytes_per_sector &&
        out_info->geometry.cylinders == expected->cylinders &&
        out_info->geometry.heads == expected->heads &&
        out_info->geometry.sectors_per_track == expected->sectors_per_track;
}

static C_INT core_machine_xebec_sector(const core_machine_hdc *hdc,
    type_unsigned_64 *out_sector)
{
    const core_machine_media_geometry *geometry;
    type_unsigned_16 cylinder;
    type_unsigned_8 head;
    type_unsigned_8 sector;

    if (hdc == STD_NULL || out_sector == STD_NULL) return 0;
    geometry = &hdc->connect.config.bus.xebec.expected_media_geometry;
    cylinder = (type_unsigned_16)hdc->xebec.dcb[3] |
        ((type_unsigned_16)(hdc->xebec.dcb[2] & 0xc0u) << 2u);
    head = hdc->xebec.dcb[1] & 0x1fu;
    sector = hdc->xebec.dcb[2] & 0x3fu;
    if (cylinder >= geometry->cylinders || head >= geometry->heads ||
        sector >= geometry->sectors_per_track) return 0;
    *out_sector = ((type_unsigned_64)cylinder * geometry->heads + head) *
        geometry->sectors_per_track + sector;
    return 1;
}

static C_INT core_machine_xebec_transfer_sector(core_machine_hdc *hdc,
    type_bool write_to_media)
{
    core_machine_media_info info;
    core_machine_media_result result;
    type_unsigned_64 sector;
    type_status status;

    if (!core_machine_xebec_media_info(hdc, &info, &result) ||
        !core_machine_xebec_sector(hdc, &sector) ||
        (write_to_media && (info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_READ_ONLY) != 0u)) {
        return 0;
    }
    if (write_to_media) {
        status = core_machine_media_write_sectors(hdc->connect.media_registry,
            hdc->connect.media_id, sector, 1u, hdc->data.data, &result);
    } else {
        status = core_machine_media_read_sectors(hdc->connect.media_registry,
            hdc->connect.media_id, sector, 1u, hdc->data.data, &result);
    }
    return status == TYPE_STATUS_OK && result == CORE_MACHINE_MEDIA_RESULT_OK;
}

static C_INT core_machine_xebec_can_transfer(const core_machine_hdc *hdc,
    type_bool write_to_media)
{
    core_machine_media_info info;
    core_machine_media_result result;
    type_unsigned_64 sector;

    return core_machine_xebec_media_info(hdc, &info, &result) &&
        core_machine_xebec_sector(hdc, &sector) &&
        (!write_to_media ||
            (info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_READ_ONLY) == 0u);
}

static C_INT core_machine_xebec_next_sector(core_machine_hdc *hdc)
{
    const core_machine_media_geometry *geometry;
    type_unsigned_16 cylinder;
    type_unsigned_8 head;
    type_unsigned_8 sector;

    if (hdc == STD_NULL) return 0;
    geometry = &hdc->connect.config.bus.xebec.expected_media_geometry;
    cylinder = (type_unsigned_16)hdc->xebec.dcb[3] |
        ((type_unsigned_16)(hdc->xebec.dcb[2] & 0xc0u) << 2u);
    head = hdc->xebec.dcb[1] & 0x1fu;
    sector = (type_unsigned_8)((hdc->xebec.dcb[2] & 0x3fu) + 1u);
    if (sector == geometry->sectors_per_track) {
        sector = 0u;
        if (++head == geometry->heads) {
            head = 0u;
            if (++cylinder == geometry->cylinders) return 0;
        }
    }
    hdc->xebec.dcb[1] = (hdc->xebec.dcb[1] & 0xe0u) | head;
    hdc->xebec.dcb[2] = (hdc->xebec.dcb[2] & 0x3fu) |
        (type_unsigned_8)((cylinder >> 2u) & 0xc0u);
    hdc->xebec.dcb[2] = (hdc->xebec.dcb[2] & 0xc0u) | sector;
    hdc->xebec.dcb[3] = (type_unsigned_8)cylinder;
    return 1;
}

static C_VOID core_machine_xebec_start_transfer(core_machine_hdc *hdc)
{
    type_unsigned_8 sense[4] = {0x04u, 0u, 0u, 0u};

    if (hdc == STD_NULL) return;
    sense[1] = hdc->xebec.dcb[1] & 0x20u;
    sense[2] = hdc->xebec.dcb[2];
    sense[3] = hdc->xebec.dcb[3];
    hdc->xebec.byte_index = 0u;
    if (hdc->xebec.dcb[0] == 0x08u) {
        if (hdc->xebec.dcb[4] == 0u || !core_machine_xebec_transfer_sector(hdc, TYPE_FALSE)) {
            core_machine_xebec_response(hdc, 0x02u, sense);
            return;
        }
        hdc->xebec.sectors_remaining = hdc->xebec.dcb[4];
        hdc->xebec.phase = CORE_MACHINE_XEBEC_PHASE_DMA_READ;
    } else {
        if (hdc->xebec.dcb[4] == 0u || !core_machine_xebec_can_transfer(hdc, TYPE_TRUE)) {
            /* A Write Data block count is documented, but zero's meaning is
             * not; reject it rather than inventing an implicit 256-sector form. */
            core_machine_xebec_response(hdc, 0x02u, sense);
            return;
        }
        hdc->xebec.sectors_remaining = hdc->xebec.dcb[4];
        hdc->xebec.phase = CORE_MACHINE_XEBEC_PHASE_DMA_WRITE;
    }
    core_machine_xebec_request_dma(hdc);
}

static C_INT core_machine_xebec_command_is_defined(type_unsigned_8 command)
{
    return (command <= 0x01u || (command >= 0x03u && command <= 0x08u) ||
        (command >= 0x0au && command <= 0x0fu) || command == 0xe0u ||
        (command >= 0xe3u && command <= 0xe6u));
}

static C_VOID core_machine_xebec_complete_dcb(core_machine_hdc *hdc)
{
    type_unsigned_8 sense[4] = {0u, 0u, 0u, 0u};

    if (hdc == STD_NULL) return;
    if (hdc->xebec.dcb[0] == 0x03u) {
        STD_MEMCPY(hdc->xebec.response, hdc->xebec.last_sense,
            sizeof(hdc->xebec.last_sense));
        hdc->xebec.response_count = sizeof(hdc->xebec.last_sense);
        hdc->xebec.response_index = 0u;
        hdc->xebec.phase = CORE_MACHINE_XEBEC_PHASE_RESPONSE;
        return;
    }
    if (!core_machine_xebec_command_is_defined(hdc->xebec.dcb[0])) {
        sense[0] = 0x20u;
        core_machine_xebec_response(hdc, 0x02u, sense);
        return;
    }
    if (hdc->xebec.dcb[0] == 0x08u || hdc->xebec.dcb[0] == 0x0au) {
        core_machine_xebec_start_transfer(hdc);
        return;
    }
    /* A selected image/printed geometry is the next receiver. Until then,
     * every DCB is an explicitly sourced "drive not ready" completion, not
     * an ATA fallback or a made-up media result. */
    sense[0] = 0x04u;
    sense[1] = (type_unsigned_8)((hdc->xebec.dcb[1] >> 5u) & 1u) << 5u;
    sense[2] = hdc->xebec.dcb[2];
    sense[3] = hdc->xebec.dcb[3];
    core_machine_xebec_response(hdc, 0x02u, sense);
}

static type_status core_machine_xebec_port_read(core_machine_hdc *hdc,
    type_unsigned_16 port, type_unsigned_32 *out_value)
{
    if (hdc == STD_NULL || out_value == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (port == hdc->connect.config.bus.xebec.data_port) {
        if (hdc->xebec.phase != CORE_MACHINE_XEBEC_PHASE_RESPONSE ||
            hdc->xebec.response_index >= hdc->xebec.response_count) return TYPE_STATUS_OK;
        *out_value = hdc->xebec.response[hdc->xebec.response_index++];
        if (hdc->xebec.response_index == hdc->xebec.response_count) {
            if (hdc->xebec.dcb[0] == 0x03u)
                STD_MEMSET(hdc->xebec.last_sense, 0, sizeof(hdc->xebec.last_sense));
            hdc->xebec.phase = CORE_MACHINE_XEBEC_PHASE_IDLE;
            hdc->xebec.dcb_count = 0u;
        }
        return TYPE_STATUS_OK;
    }
    return TYPE_STATUS_UNSUPPORTED;
}

static type_status core_machine_xebec_port_write(core_machine_hdc *hdc,
    type_unsigned_16 port, type_unsigned_32 value)
{
    if (hdc == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    if (port == hdc->connect.config.bus.xebec.hardware_status_reset_port) {
        core_machine_xebec_reset(hdc);
        return TYPE_STATUS_OK;
    }
    if (port == hdc->connect.config.bus.xebec.dma_irq_mask_port) {
        hdc->xebec.mask_pattern = (type_unsigned_8)value;
        return TYPE_STATUS_OK;
    }
    if (port == hdc->connect.config.bus.xebec.jumpers_select_port) {
        hdc->xebec.dcb_count = 0u;
        hdc->xebec.initialize_count = 0u;
        hdc->xebec.phase = CORE_MACHINE_XEBEC_PHASE_DCB;
        return TYPE_STATUS_OK;
    }
    if (port != hdc->connect.config.bus.xebec.data_port ||
        hdc->xebec.phase == CORE_MACHINE_XEBEC_PHASE_IDLE ||
        hdc->xebec.phase == CORE_MACHINE_XEBEC_PHASE_RESPONSE) return TYPE_STATUS_OK;
    if (hdc->xebec.phase == CORE_MACHINE_XEBEC_PHASE_DCB) {
        hdc->xebec.dcb[hdc->xebec.dcb_count++] = (type_unsigned_8)value;
        if (hdc->xebec.dcb_count == sizeof(hdc->xebec.dcb)) {
            if (hdc->xebec.dcb[0] == 0x0cu) hdc->xebec.phase = CORE_MACHINE_XEBEC_PHASE_INITIALIZE;
            else core_machine_xebec_complete_dcb(hdc);
        }
    } else if (hdc->xebec.phase == CORE_MACHINE_XEBEC_PHASE_INITIALIZE) {
        hdc->xebec.initialize[hdc->xebec.initialize_count++] = (type_unsigned_8)value;
        if (hdc->xebec.initialize_count == sizeof(hdc->xebec.initialize))
            core_machine_xebec_complete_dcb(hdc);
    }
    return TYPE_STATUS_OK;
}

static type_status core_machine_hdc_port_read(C_VOID *opaque, type_unsigned_16 port,
    type_unsigned_32 *out_value)
{
    core_machine_hdc *hdc = (core_machine_hdc *)opaque;
    type_unsigned_16 word;

    if (hdc == STD_NULL || out_value == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_value = 0u;
    if (core_machine_hdc_is_xebec_xt(hdc)) return core_machine_xebec_port_read(hdc, port, out_value);
    if (port == hdc->connect.config.bus.task_file.data_port) {
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
    if (port == hdc->connect.config.bus.task_file.error_features_port) {
        *out_value = hdc->data.error;
    } else if (port == hdc->connect.config.bus.task_file.sector_count_port) {
        *out_value = hdc->data.sector_count;
    } else if (port == hdc->connect.config.bus.task_file.sector_number_port) {
        *out_value = hdc->data.sector_number;
    } else if (port == hdc->connect.config.bus.task_file.cylinder_low_port) {
        *out_value = hdc->data.cylinder_low;
    } else if (port == hdc->connect.config.bus.task_file.cylinder_high_port) {
        *out_value = hdc->data.cylinder_high;
    } else if (port == hdc->connect.config.bus.task_file.drive_head_port) {
        *out_value = hdc->data.drive_head;
    } else if (port == hdc->connect.config.bus.task_file.status_command_port) {
        *out_value = hdc->data.status;
        core_machine_hdc_clear_irq(hdc);
    } else if (port == hdc->connect.config.bus.task_file.alternate_status_device_control_port) {
        if (core_machine_hdc_is_ibm_wd1003(hdc)) return TYPE_STATUS_UNSUPPORTED;
        *out_value = hdc->data.status;
    } else if (port == hdc->connect.config.bus.task_file.drive_address_port &&
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
    if (core_machine_hdc_is_xebec_xt(hdc)) return core_machine_xebec_port_write(hdc, port, value);
    if (port == hdc->connect.config.bus.task_file.data_port) {
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
    if (!core_machine_hdc_task_file_is_writable(hdc) &&
        (port == hdc->connect.config.bus.task_file.error_features_port ||
            port == hdc->connect.config.bus.task_file.sector_count_port ||
            port == hdc->connect.config.bus.task_file.sector_number_port ||
            port == hdc->connect.config.bus.task_file.cylinder_low_port ||
            port == hdc->connect.config.bus.task_file.cylinder_high_port ||
            port == hdc->connect.config.bus.task_file.drive_head_port)) return TYPE_STATUS_OK;
    if (port == hdc->connect.config.bus.task_file.error_features_port) {
        hdc->data.features = (type_unsigned_8)value;
    } else if (port == hdc->connect.config.bus.task_file.sector_count_port) {
        hdc->data.sector_count = (type_unsigned_8)value;
    } else if (port == hdc->connect.config.bus.task_file.sector_number_port) {
        hdc->data.sector_number = (type_unsigned_8)value;
    } else if (port == hdc->connect.config.bus.task_file.cylinder_low_port) {
        hdc->data.cylinder_low = (type_unsigned_8)value;
    } else if (port == hdc->connect.config.bus.task_file.cylinder_high_port) {
        hdc->data.cylinder_high = (type_unsigned_8)value;
    } else if (port == hdc->connect.config.bus.task_file.drive_head_port) {
        hdc->data.drive_head = (type_unsigned_8)value;
        core_machine_hdc_refresh_compaq_selection_status(hdc);
    } else if (port == hdc->connect.config.bus.task_file.status_command_port) {
        core_machine_hdc_capture_command(hdc, (type_unsigned_8)value);
    } else if (port == hdc->connect.config.bus.task_file.alternate_status_device_control_port &&
        core_machine_hdc_is_ibm_wd1003(hdc)) {
        hdc->data.fixed_disk_register = (type_unsigned_8)value & 0x08u;
    } else if (port == hdc->connect.config.bus.task_file.alternate_status_device_control_port) {
        type_unsigned_8 device_control = (type_unsigned_8)value;
        type_bool reset_asserted = (device_control &
            CORE_MACHINE_HDC_DEVICE_CONTROL_SRST) != 0u;

        hdc->data.device_control = device_control;
        core_machine_hdc_sync_irq(hdc);
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

static C_VOID core_machine_xebec_dma_read(C_VOID *owner, t_latch *latch)
{
    core_machine_hdc *hdc = owner;
    type_unsigned_8 sense[4] = {0x04u, 0u, 0u, 0u};

    if (hdc == STD_NULL || latch == STD_NULL ||
        hdc->xebec.phase != CORE_MACHINE_XEBEC_PHASE_DMA_READ ||
        hdc->xebec.byte_index >= sizeof(hdc->data.data)) return;
    latch->data.byte = hdc->data.data[hdc->xebec.byte_index++];
    if (hdc->xebec.byte_index == sizeof(hdc->data.data)) {
        if (--hdc->xebec.sectors_remaining == 0u) {
            core_machine_xebec_release_dma(hdc);
            core_machine_xebec_response(hdc, 0u, STD_NULL);
        } else if (!core_machine_xebec_next_sector(hdc) ||
            !core_machine_xebec_transfer_sector(hdc, TYPE_FALSE)) {
            sense[1] = hdc->xebec.dcb[1] & 0x20u;
            sense[2] = hdc->xebec.dcb[2];
            sense[3] = hdc->xebec.dcb[3];
            core_machine_xebec_release_dma(hdc);
            core_machine_xebec_response(hdc, 0x02u, sense);
        } else hdc->xebec.byte_index = 0u;
    }
}

static C_VOID core_machine_xebec_dma_write(C_VOID *owner, t_latch *latch)
{
    core_machine_hdc *hdc = owner;
    type_unsigned_8 sense[4] = {0x04u, 0u, 0u, 0u};

    if (hdc == STD_NULL || latch == STD_NULL ||
        hdc->xebec.phase != CORE_MACHINE_XEBEC_PHASE_DMA_WRITE ||
        hdc->xebec.byte_index >= sizeof(hdc->data.data)) return;
    hdc->data.data[hdc->xebec.byte_index++] = latch->data.byte;
    if (hdc->xebec.byte_index != sizeof(hdc->data.data)) return;
    sense[1] = hdc->xebec.dcb[1] & 0x20u;
    sense[2] = hdc->xebec.dcb[2];
    sense[3] = hdc->xebec.dcb[3];
    if (!core_machine_xebec_transfer_sector(hdc, TYPE_TRUE) ||
        hdc->xebec.sectors_remaining == 0u) {
        core_machine_xebec_release_dma(hdc);
        core_machine_xebec_response(hdc, 0x02u, sense);
        return;
    }
    if (--hdc->xebec.sectors_remaining == 0u) {
        core_machine_xebec_release_dma(hdc);
        core_machine_xebec_response(hdc, 0u, STD_NULL);
        return;
    }
    if (!core_machine_xebec_next_sector(hdc) ||
        !core_machine_xebec_can_transfer(hdc, TYPE_TRUE)) {
        core_machine_xebec_release_dma(hdc);
        core_machine_xebec_response(hdc, 0x02u, sense);
        return;
    }
    hdc->xebec.byte_index = 0u;
}

static C_VOID core_machine_xebec_dma_terminal(C_VOID *owner, t_latch *latch)
{
    core_machine_hdc *hdc = owner;
    type_unsigned_8 sense[4] = {0x04u, 0u, 0u, 0u};

    (C_VOID)latch;
    if (hdc == STD_NULL || (hdc->xebec.phase != CORE_MACHINE_XEBEC_PHASE_DMA_READ &&
        hdc->xebec.phase != CORE_MACHINE_XEBEC_PHASE_DMA_WRITE)) return;
    sense[1] = hdc->xebec.dcb[1] & 0x20u;
    sense[2] = hdc->xebec.dcb[2];
    sense[3] = hdc->xebec.dcb[3];
    core_machine_xebec_release_dma(hdc);
    core_machine_xebec_response(hdc, 0x02u, sense);
}

static const core_machine_dma_channel_provider core_machine_hdc_dma_channel = {
    core_machine_xebec_dma_read, core_machine_xebec_dma_write,
    core_machine_xebec_dma_terminal
};

C_VOID core_machine_hdc_connect(core_machine_hdc *hdc,
    const core_machine_media_registry *media_registry,
    core_machine_media_id media_id, core_machine_media_id slave_media_id,
    t_pic *pic_master, t_pic *pic_slave, const core_machine_hdc_config *config)
{
    if (hdc == STD_NULL || config == STD_NULL) return;
    hdc->connect.media_registry = media_registry;
    hdc->connect.media_id = media_id;
    hdc->connect.slave_media_id = slave_media_id;
    core_machine_pic_irq_source_bind(&hdc->connect.irq_source, pic_master,
        pic_slave, config->irq);
    hdc->connect.config = *config;
}

C_VOID core_machine_hdc_bind_dma_request(core_machine_hdc *hdc,
    const core_machine_dma_request_binding *binding,
    C_VOID (*request_assert)(C_VOID *owner,
        const core_machine_dma_request_binding *binding),
    C_VOID (*request_deassert)(C_VOID *owner,
        const core_machine_dma_request_binding *binding), C_VOID *owner)
{
    if (hdc == STD_NULL || binding == STD_NULL || request_assert == STD_NULL ||
        request_deassert == STD_NULL || owner == STD_NULL) return;
    hdc->connect.dma_request = *binding;
    hdc->connect.dma_request_assert = request_assert;
    hdc->connect.dma_request_deassert = request_deassert;
    hdc->connect.dma_request_owner = owner;
}

C_VOID core_machine_hdc_initialize(core_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    STD_MEMSET(&hdc->data, 0, sizeof(hdc->data));
    STD_MEMSET(&hdc->xebec, 0, sizeof(hdc->xebec));
    hdc->xebec.phase = CORE_MACHINE_XEBEC_PHASE_IDLE;
    core_machine_pic_irq_source_deassert(&hdc->connect.irq_source);
    core_machine_hdc_reset(hdc);
}

C_VOID core_machine_hdc_reset(core_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    STD_MEMSET(&hdc->data, 0, sizeof(hdc->data));
    if (core_machine_hdc_is_xebec_xt(hdc)) core_machine_xebec_reset(hdc);
    core_machine_hdc_clear_irq(hdc);
    if (!core_machine_hdc_is_compaq_wd_40mb(hdc)) {
        hdc->data.error = CORE_MACHINE_HDC_ERROR_DIAGNOSTIC_OK;
        hdc->data.sector_count = 1u;
        hdc->data.sector_number = 1u;
    }
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

C_VOID core_machine_hdc_finalize(core_machine_hdc *hdc)
{
    if (hdc == STD_NULL) return;
    core_machine_pic_irq_source_deassert(&hdc->connect.irq_source);
    STD_MEMSET(&hdc->data, 0, sizeof(hdc->data));
    STD_MEMSET(&hdc->xebec, 0, sizeof(hdc->xebec));
    STD_MEMSET(&hdc->connect, 0, sizeof(hdc->connect));
}

const core_machine_port_provider *core_machine_hdc_port_provider(C_VOID)
{
    return &core_machine_hdc_ports;
}

const core_machine_dma_channel_provider *core_machine_hdc_dma_provider(C_VOID)
{
    return &core_machine_hdc_dma_channel;
}

type_bool core_machine_hdc_irq_pending(const core_machine_hdc *hdc)
{
    return hdc != STD_NULL && hdc->data.irq_pending &&
        (hdc->data.device_control & CORE_MACHINE_HDC_DEVICE_CONTROL_NIEN) == 0u;
}
