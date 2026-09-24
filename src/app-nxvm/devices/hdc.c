#include "lib/types/types_interface.h"

#include "app-nxvm/devices/media_interface.h"
#include "app-nxvm/devices/dma.h"
#include "app-nxvm/devices/pic.h"
#include "app-nxvm/devices/hdc.h"

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
#define CORE_MACHINE_XEBEC_MASK_DMA_ENABLE 0x01u
#define CORE_MACHINE_XEBEC_MASK_IRQ_ENABLE 0x02u
static lib_i32 core_machine_hdc_is_compaq_wd_40mb(const core_machine_hdc *hdc)
{
    return hdc != LIB_NULL && hdc->connect.config.protocol ==
        CORE_MACHINE_HDC_PROTOCOL_COMPAQ_WD_40MB;
}

static lib_i32 core_machine_hdc_is_ibm_wd1003(const core_machine_hdc *hdc)
{
    return hdc != LIB_NULL && hdc->connect.config.protocol ==
        CORE_MACHINE_HDC_PROTOCOL_IBM_WD1003_ST506;
}

static lib_i32 core_machine_hdc_is_xebec_xt(const core_machine_hdc *hdc)
{
    return hdc != LIB_NULL && hdc->connect.config.protocol ==
        CORE_MACHINE_HDC_PROTOCOL_XEBEC_XT;
}

static lib_i32 core_machine_hdc_task_file_is_writable(const core_machine_hdc *hdc)
{
    return hdc != LIB_NULL && (core_machine_hdc_is_compaq_wd_40mb(hdc) ||
        (hdc->data.status & (CORE_MACHINE_HDC_STATUS_BSY |
            CORE_MACHINE_HDC_STATUS_DRQ)) == 0u);
}

static lib_i32 core_machine_hdc_selected_master(const core_machine_hdc *hdc)
{
    return hdc != LIB_NULL && (hdc->data.drive_head & 0x10u) == 0u;
}

static lib_u8 core_machine_hdc_current_head(const core_machine_hdc *hdc)
{
    if (hdc == LIB_NULL) return 0u;
    if (core_machine_hdc_is_ibm_wd1003(hdc)) {
        return (lib_u8)((hdc->data.drive_head & 0x07u) |
            (hdc->data.fixed_disk_register & 0x08u));
    }
    return hdc->data.drive_head & 0x0fu;
}

static void core_machine_hdc_set_current_head(core_machine_hdc *hdc,
    lib_u8 head)
{
    if (hdc == LIB_NULL) return;
    if (core_machine_hdc_is_ibm_wd1003(hdc)) {
        hdc->data.drive_head = (lib_u8)((hdc->data.drive_head & 0xf8u) |
            (head & 0x07u));
        hdc->data.fixed_disk_register = (lib_u8)((hdc->data.fixed_disk_register &
            0xf7u) | (head & 0x08u));
        return;
    }
    hdc->data.drive_head = (lib_u8)((hdc->data.drive_head & 0xf0u) | head);
}

static lib_i32 core_machine_hdc_lba_mode(const core_machine_hdc *hdc)
{
    return hdc != LIB_NULL && !core_machine_hdc_is_compaq_wd_40mb(hdc) &&
        hdc->connect.config.bus.task_file.lba28_supported &&
        (hdc->data.drive_head & 0x40u) != 0u;
}

static lib_i32 core_machine_hdc_command_is_read(const core_machine_hdc *hdc,
    lib_u8 command)
{
    if (core_machine_hdc_is_compaq_wd_40mb(hdc)) {
        return (command & 0xfeu) == CORE_MACHINE_HDC_COMMAND_READ_SECTORS;
    }
    if (core_machine_hdc_is_ibm_wd1003(hdc)) {
        return (command & 0xfcu) == CORE_MACHINE_HDC_COMMAND_READ_SECTORS;
    }
    return command == CORE_MACHINE_HDC_COMMAND_READ_SECTORS;
}

static lib_i32 core_machine_hdc_command_is_write(const core_machine_hdc *hdc,
    lib_u8 command)
{
    if (core_machine_hdc_is_compaq_wd_40mb(hdc)) {
        return (command & 0xfeu) == CORE_MACHINE_HDC_COMMAND_WRITE_SECTORS;
    }
    if (core_machine_hdc_is_ibm_wd1003(hdc)) {
        return (command & 0xfcu) == CORE_MACHINE_HDC_COMMAND_WRITE_SECTORS;
    }
    return command == CORE_MACHINE_HDC_COMMAND_WRITE_SECTORS;
}

static void core_machine_hdc_select_ibm_step_rate(core_machine_hdc *hdc,
    lib_u8 selector, lib_u16 pulse_limit)
{
    lib_u32 ticks_per_second;

    if (!core_machine_hdc_is_ibm_wd1003(hdc)) return;
    ticks_per_second = hdc->connect.config.bus.task_file.clock_ticks_per_second;
    hdc->data.step_rate_selector = selector;
    hdc->data.step_pulse_limit = pulse_limit;
    hdc->data.step_rate_ticks = selector == 0u ?
        (ticks_per_second / 1000000u) * 35u :
        (ticks_per_second / 2000u) * selector;
}

static void core_machine_hdc_sync_irq(core_machine_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    if (hdc->data.irq_pending &&
        (hdc->data.device_control & CORE_MACHINE_HDC_DEVICE_CONTROL_NIEN) == 0u) {
        core_machine_pic_irq_source_assert(&hdc->connect.irq_source);
    } else {
        core_machine_pic_irq_source_deassert(&hdc->connect.irq_source);
    }
}

static void core_machine_hdc_clear_irq(core_machine_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    hdc->data.irq_pending = LIB_FALSE;
    core_machine_hdc_sync_irq(hdc);
}

static void core_machine_hdc_raise_irq(core_machine_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    hdc->data.irq_pending = LIB_TRUE;
    core_machine_hdc_sync_irq(hdc);
}

static lib_u32 core_machine_hdc_lba(const core_machine_hdc *hdc)
{
    return (lib_u32)hdc->data.sector_number |
        ((lib_u32)hdc->data.cylinder_low << 8u) |
        ((lib_u32)hdc->data.cylinder_high << 16u) |
        ((lib_u32)(hdc->data.drive_head & 0x0fu) << 24u);
}

static core_machine_media_id core_machine_hdc_selected_media_id(const core_machine_hdc *hdc)
{
    return hdc != LIB_NULL && (hdc->data.drive_head & 0x10u) != 0u ?
        hdc->connect.slave_media_id : hdc->connect.media_id;
}

static lib_i32 core_machine_hdc_media_info(const core_machine_hdc *hdc,
    core_machine_media_info *out_info, core_machine_media_result *out_result)
{
    return hdc != LIB_NULL && hdc->connect.media_registry != LIB_NULL &&
        core_machine_hdc_selected_media_id(hdc) != CORE_MACHINE_MEDIA_ID_INVALID &&
        core_machine_media_query(hdc->connect.media_registry,
            core_machine_hdc_selected_media_id(hdc), out_info, out_result) == LIB_STATUS_OK &&
        *out_result == CORE_MACHINE_MEDIA_RESULT_OK;
}

static void core_machine_hdc_refresh_compaq_selection_status(core_machine_hdc *hdc)
{
    if (!core_machine_hdc_is_compaq_wd_40mb(hdc) ||
        hdc->data.phase != CORE_MACHINE_HDC_PHASE_IDLE) return;
    /* Selecting a task-file drive is a controller operation, not media
     * insertion.  The Compaq firmware probes a fitted controller before it
     * knows whether a fixed disk is attached; keep its reset-ready state
     * visible here and let a command needing sectors fail through the sole
     * media route. */
    hdc->data.status = CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC;
}

static lib_size core_machine_hdc_sector_capacity(
    const core_machine_media_info *info)
{
    return info == LIB_NULL || info->geometry.bytes_per_sector == 0u ? 0u :
        (lib_size)info->geometry.logical_sector_count;
}

static void core_machine_hdc_complete(core_machine_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    hdc->data.phase = CORE_MACHINE_HDC_PHASE_IDLE;
    hdc->data.next_service_tick = 0u;
    hdc->data.data_index = 0u;
    hdc->data.status = CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC;
    core_machine_hdc_raise_irq(hdc);
}

static void core_machine_hdc_fail(core_machine_hdc *hdc, lib_u8 error)
{
    if (hdc == LIB_NULL) return;
    hdc->data.error = error;
    hdc->data.phase = CORE_MACHINE_HDC_PHASE_IDLE;
    hdc->data.next_service_tick = 0u;
    hdc->data.data_index = 0u;
    hdc->data.status = CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_ERR;
    core_machine_hdc_raise_irq(hdc);
}

static lib_i32 core_machine_hdc_resolve_sector(core_machine_hdc *hdc,
    lib_u8 write_to_media, lib_size *out_offset)
{
    core_machine_media_info info;
    core_machine_media_result media_result;
    lib_u16 cylinder;
    lib_u8 head;
    lib_u8 sector;
    lib_u32 lba;

    if (hdc == LIB_NULL || out_offset == LIB_NULL ||
        !core_machine_hdc_media_info(hdc, &info, &media_result) || !info.present ||
        (write_to_media && (info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_READ_ONLY) != 0u)) {
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    if (core_machine_hdc_lba_mode(hdc)) {
        lba = core_machine_hdc_lba(hdc);
        if (!core_machine_hdc_selected_master(hdc) ||
            (lib_size)lba >= core_machine_hdc_sector_capacity(&info) ||
            info.geometry.bytes_per_sector != sizeof(hdc->data.data)) {
            core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ID_NOT_FOUND);
            return 0;
        }
        *out_offset = (lib_size)lba * info.geometry.bytes_per_sector;
        return 1;
    }
    cylinder = (lib_u16)hdc->data.cylinder_low |
        ((lib_u16)hdc->data.cylinder_high << 8u);
    head = core_machine_hdc_current_head(hdc);
    sector = hdc->data.sector_number;
    if ((!core_machine_hdc_is_compaq_wd_40mb(hdc) && !core_machine_hdc_selected_master(hdc)) ||
        sector == 0u || cylinder >= info.geometry.cylinders ||
        head >= info.geometry.heads || sector > info.geometry.sectors_per_track ||
        info.geometry.bytes_per_sector != sizeof(hdc->data.data)) {
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ID_NOT_FOUND);
        return 0;
    }
    *out_offset = (((lib_size)cylinder * info.geometry.heads + head) *
        info.geometry.sectors_per_track + (sector - 1u)) * info.geometry.bytes_per_sector;
    return 1;
}

static lib_i32 core_machine_hdc_load_sector(core_machine_hdc *hdc)
{
    core_machine_media_result media_result;
    lib_size offset;

    if (!core_machine_hdc_resolve_sector(hdc, LIB_FALSE, &offset)) return 0;
    if (core_machine_media_read_bytes(hdc->connect.media_registry,
        core_machine_hdc_selected_media_id(hdc), offset, hdc->data.data, sizeof(hdc->data.data),
            &media_result) != LIB_STATUS_OK ||
        media_result != CORE_MACHINE_MEDIA_RESULT_OK) {
        core_machine_hdc_fail(hdc, media_result == CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE ?
            CORE_MACHINE_HDC_ERROR_ID_NOT_FOUND : CORE_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    return 1;
}

static lib_i32 core_machine_hdc_store_sector(core_machine_hdc *hdc)
{
    core_machine_media_result media_result;
    lib_size offset;

    if (!core_machine_hdc_resolve_sector(hdc, LIB_TRUE, &offset)) return 0;
    if (core_machine_media_write_bytes(hdc->connect.media_registry,
            core_machine_hdc_selected_media_id(hdc), offset, hdc->data.data, sizeof(hdc->data.data),
            &media_result) != LIB_STATUS_OK ||
        media_result != CORE_MACHINE_MEDIA_RESULT_OK) {
        core_machine_hdc_fail(hdc, media_result == CORE_MACHINE_MEDIA_RESULT_INVALID_RANGE ?
            CORE_MACHINE_HDC_ERROR_ID_NOT_FOUND : CORE_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    return 1;
}

static void core_machine_hdc_identify(core_machine_hdc *hdc)
{
    core_machine_media_info info;
    core_machine_media_result media_result;
    lib_u16 word;

    if (hdc == LIB_NULL || !core_machine_hdc_media_info(hdc, &info, &media_result) ||
        !core_machine_hdc_selected_master(hdc) ||
        !info.present || info.geometry.bytes_per_sector != sizeof(hdc->data.data)) {
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ABORT);
        return;
    }
    lib_memory_set(hdc->data.data, 0, sizeof(hdc->data.data));
    word = 0x0040u;
    lib_memory_copy(&hdc->data.data[0], &word, sizeof(word));
    word = info.geometry.cylinders;
    lib_memory_copy(&hdc->data.data[2], &word, sizeof(word));
    word = info.geometry.heads;
    lib_memory_copy(&hdc->data.data[6], &word, sizeof(word));
    word = info.geometry.sectors_per_track;
    lib_memory_copy(&hdc->data.data[12], &word, sizeof(word));
    word = 0x0200u;
    lib_memory_copy(&hdc->data.data[98], &word, sizeof(word));
    word = (lib_u16)core_machine_hdc_sector_capacity(&info);
    lib_memory_copy(&hdc->data.data[120], &word, sizeof(word));
    word = (lib_u16)(core_machine_hdc_sector_capacity(&info) >> 16u);
    lib_memory_copy(&hdc->data.data[122], &word, sizeof(word));
    hdc->data.phase = CORE_MACHINE_HDC_PHASE_DATA_READ;
    hdc->data.data_index = 0u;
    hdc->data.error = 0u;
    hdc->data.status = CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC |
        CORE_MACHINE_HDC_STATUS_DRQ;
    core_machine_hdc_raise_irq(hdc);
}

static lib_i32 core_machine_hdc_advance_chs(core_machine_hdc *hdc)
{
    core_machine_media_info info;
    core_machine_media_result media_result;
    lib_u16 cylinder;
    lib_u8 head;
    lib_u8 sector;

    if (hdc == LIB_NULL || !core_machine_hdc_media_info(hdc, &info, &media_result) ||
        !info.present) {
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    cylinder = (lib_u16)hdc->data.cylinder_low |
        ((lib_u16)hdc->data.cylinder_high << 8u);
    head = core_machine_hdc_current_head(hdc);
    sector = (lib_u8)(hdc->data.sector_number + 1u);
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
    hdc->data.cylinder_low = (lib_u8)cylinder;
    hdc->data.cylinder_high = (lib_u8)(cylinder >> 8u);
    core_machine_hdc_set_current_head(hdc, head);
    return 1;
}

static lib_i32 core_machine_hdc_advance_lba(core_machine_hdc *hdc)
{
    core_machine_media_info info;
    core_machine_media_result media_result;
    lib_u32 lba;

    if (hdc == LIB_NULL || !core_machine_hdc_media_info(hdc, &info, &media_result) ||
        !info.present) {
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ABORT);
        return 0;
    }
    lba = core_machine_hdc_lba(hdc) + 1u;
    if ((lib_size)lba >= core_machine_hdc_sector_capacity(&info)) {
        core_machine_hdc_fail(hdc, CORE_MACHINE_HDC_ERROR_ID_NOT_FOUND);
        return 0;
    }
    hdc->data.sector_number = (lib_u8)lba;
    hdc->data.cylinder_low = (lib_u8)(lba >> 8u);
    hdc->data.cylinder_high = (lib_u8)(lba >> 16u);
    hdc->data.drive_head = (hdc->data.drive_head & 0xf0u) |
        (lib_u8)(lba >> 24u);
    return 1;
}

static lib_i32 core_machine_hdc_advance_sector(core_machine_hdc *hdc)
{
    return core_machine_hdc_lba_mode(hdc) ? core_machine_hdc_advance_lba(hdc) :
        core_machine_hdc_advance_chs(hdc);
}

static void core_machine_hdc_complete_data_sector(core_machine_hdc *hdc)
{
    if (hdc == LIB_NULL || hdc->data.sectors_remaining == 0u) return;
    --hdc->data.sectors_remaining;
    --hdc->data.sector_count;
}

static void core_machine_hdc_next_read_sector(core_machine_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
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

static void core_machine_hdc_next_write_sector(core_machine_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    if (!core_machine_hdc_store_sector(hdc)) return;
    core_machine_hdc_complete_data_sector(hdc);
    if (hdc->data.sectors_remaining == 0u) {
        core_machine_hdc_complete(hdc);
        return;
    }
    if (!core_machine_hdc_advance_sector(hdc)) return;
    hdc->data.data_index = 0u;
    lib_memory_set(hdc->data.data, 0, sizeof(hdc->data.data));
    hdc->data.phase = CORE_MACHINE_HDC_PHASE_DATA_WRITE;
    hdc->data.status = CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC |
        CORE_MACHINE_HDC_STATUS_DRQ;
    core_machine_hdc_raise_irq(hdc);
}

static void core_machine_hdc_schedule_service(core_machine_hdc *hdc,
    lib_u32 service_ticks)
{
    if (hdc == LIB_NULL) return;
    hdc->data.next_service_tick = service_ticks == 0u ? hdc->data.elapsed_ticks :
        hdc->data.elapsed_ticks + service_ticks;
}

static void core_machine_hdc_capture_command(core_machine_hdc *hdc,
    lib_u8 command)
{
    if (hdc == LIB_NULL || hdc->data.reset_asserted ||
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
    core_machine_hdc_schedule_service(hdc, hdc->connect.config.service.command_ticks);
}

static void core_machine_hdc_begin_read(core_machine_hdc *hdc)
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

static void core_machine_hdc_begin_write(core_machine_hdc *hdc)
{
    lib_size offset;

    if (!core_machine_hdc_resolve_sector(hdc, LIB_TRUE, &offset)) return;
    hdc->data.phase = CORE_MACHINE_HDC_PHASE_DATA_WRITE;
    hdc->data.sectors_remaining = hdc->data.sector_count == 0u ? 256u :
        hdc->data.sector_count;
    hdc->data.data_index = 0u;
    hdc->data.status = CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC |
        CORE_MACHINE_HDC_STATUS_DRQ;
    core_machine_hdc_raise_irq(hdc);
}

static void core_machine_hdc_execute_command(core_machine_hdc *hdc, lib_u8 command)
{
    lib_u16 cylinder;

    if (hdc == LIB_NULL) return;
    hdc->data.last_command = command;
    ++hdc->data.command_count;
    if ((!core_machine_hdc_is_compaq_wd_40mb(hdc) &&
            !core_machine_hdc_selected_master(hdc)) ||
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
            cylinder = (lib_u16)hdc->data.cylinder_low |
                ((lib_u16)hdc->data.cylinder_high << 8u);
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

static void core_machine_xebec_reset(core_machine_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    if (hdc->connect.dma_request_deassert != LIB_NULL) {
        hdc->connect.dma_request_deassert(hdc->connect.dma_request_owner,
            &hdc->connect.dma_request);
    }
    lib_memory_set(&hdc->xebec, 0, sizeof(hdc->xebec));
    hdc->xebec.phase = CORE_MACHINE_XEBEC_PHASE_IDLE;
    core_machine_hdc_clear_irq(hdc);
}

static void core_machine_xebec_response(core_machine_hdc *hdc,
    lib_u8 status, const lib_u8 *sense)
{
    lib_u8 drive;

    if (hdc == LIB_NULL) return;
    drive = (hdc->xebec.dcb[1] >> 5u) & 1u;
    hdc->xebec.response[0] = (lib_u8)((drive << 5u) | status);
    hdc->xebec.response_count = 1u;
    hdc->xebec.response_index = 0u;
    if ((status & 0x02u) != 0u && sense != LIB_NULL) {
        lib_memory_copy(hdc->xebec.last_sense, sense, sizeof(hdc->xebec.last_sense));
    }
    hdc->xebec.phase = CORE_MACHINE_XEBEC_PHASE_RESPONSE;
    if ((hdc->xebec.mask_pattern & CORE_MACHINE_XEBEC_MASK_IRQ_ENABLE) != 0u) {
        core_machine_hdc_raise_irq(hdc);
    }
}

static void core_machine_xebec_request_dma(core_machine_hdc *hdc)
{
    if (hdc != LIB_NULL && hdc->connect.dma_request_assert != LIB_NULL) {
        hdc->connect.dma_request_assert(hdc->connect.dma_request_owner,
            &hdc->connect.dma_request);
    }
}

static void core_machine_xebec_release_dma(core_machine_hdc *hdc)
{
    if (hdc != LIB_NULL && hdc->connect.dma_request_deassert != LIB_NULL) {
        hdc->connect.dma_request_deassert(hdc->connect.dma_request_owner,
            &hdc->connect.dma_request);
    }
}

static void core_machine_xebec_sync_dma_request(core_machine_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    if ((hdc->xebec.mask_pattern & CORE_MACHINE_XEBEC_MASK_DMA_ENABLE) != 0u &&
        (hdc->xebec.phase == CORE_MACHINE_XEBEC_PHASE_DMA_READ ||
            hdc->xebec.phase == CORE_MACHINE_XEBEC_PHASE_DMA_WRITE)) {
        core_machine_xebec_request_dma(hdc);
    } else {
        core_machine_xebec_release_dma(hdc);
    }
}

static lib_i32 core_machine_xebec_media_info(const core_machine_hdc *hdc,
    core_machine_media_info *out_info, core_machine_media_result *out_result)
{
    core_machine_media_id media_id;
    const core_machine_media_geometry *expected;

    if (hdc == LIB_NULL || out_info == LIB_NULL || out_result == LIB_NULL ||
        hdc->connect.media_registry == LIB_NULL ||
        ((hdc->xebec.dcb[1] >> 5u) & 1u) != 0u) return 0;
    media_id = hdc->connect.media_id;
    if (media_id == CORE_MACHINE_MEDIA_ID_INVALID ||
        core_machine_media_query(hdc->connect.media_registry, media_id, out_info,
            out_result) != LIB_STATUS_OK || *out_result != CORE_MACHINE_MEDIA_RESULT_OK ||
        !out_info->present || (out_info->capabilities &
            CORE_MACHINE_MEDIA_CAPABILITY_GEOMETRY_KNOWN) == 0u) return 0;
    expected = &hdc->connect.config.bus.xebec.expected_media_geometry;
    return out_info->geometry.logical_sector_count == expected->logical_sector_count &&
        out_info->geometry.bytes_per_sector == expected->bytes_per_sector &&
        out_info->geometry.cylinders == expected->cylinders &&
        out_info->geometry.heads == expected->heads &&
        out_info->geometry.sectors_per_track == expected->sectors_per_track;
}

static lib_i32 core_machine_xebec_sector(const core_machine_hdc *hdc,
    lib_u64 *out_sector)
{
    const core_machine_media_geometry *geometry;
    lib_u16 cylinder;
    lib_u8 head;
    lib_u8 sector;

    if (hdc == LIB_NULL || out_sector == LIB_NULL) return 0;
    geometry = &hdc->connect.config.bus.xebec.expected_media_geometry;
    cylinder = (lib_u16)hdc->xebec.dcb[3] |
        ((lib_u16)(hdc->xebec.dcb[2] & 0xc0u) << 2u);
    head = hdc->xebec.dcb[1] & 0x1fu;
    sector = hdc->xebec.dcb[2] & 0x3fu;
    if (cylinder >= geometry->cylinders || head >= geometry->heads ||
        sector >= geometry->sectors_per_track) return 0;
    *out_sector = ((lib_u64)cylinder * geometry->heads + head) *
        geometry->sectors_per_track + sector;
    return 1;
}

static lib_i32 core_machine_xebec_transfer_sector(core_machine_hdc *hdc,
    lib_u8 write_to_media)
{
    core_machine_media_info info;
    core_machine_media_result result;
    lib_u64 sector;
    lib_status status;

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
    return status == LIB_STATUS_OK && result == CORE_MACHINE_MEDIA_RESULT_OK;
}

static lib_i32 core_machine_xebec_can_transfer(const core_machine_hdc *hdc,
    lib_u8 write_to_media)
{
    core_machine_media_info info;
    core_machine_media_result result;
    lib_u64 sector;

    return core_machine_xebec_media_info(hdc, &info, &result) &&
        core_machine_xebec_sector(hdc, &sector) &&
        (!write_to_media ||
            (info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_READ_ONLY) == 0u);
}

static lib_i32 core_machine_xebec_next_sector(core_machine_hdc *hdc)
{
    const core_machine_media_geometry *geometry;
    lib_u16 cylinder;
    lib_u8 head;
    lib_u8 sector;

    if (hdc == LIB_NULL) return 0;
    geometry = &hdc->connect.config.bus.xebec.expected_media_geometry;
    cylinder = (lib_u16)hdc->xebec.dcb[3] |
        ((lib_u16)(hdc->xebec.dcb[2] & 0xc0u) << 2u);
    head = hdc->xebec.dcb[1] & 0x1fu;
    sector = (lib_u8)((hdc->xebec.dcb[2] & 0x3fu) + 1u);
    if (sector == geometry->sectors_per_track) {
        sector = 0u;
        if (++head == geometry->heads) {
            head = 0u;
            if (++cylinder == geometry->cylinders) return 0;
        }
    }
    hdc->xebec.dcb[1] = (hdc->xebec.dcb[1] & 0xe0u) | head;
    hdc->xebec.dcb[2] = (hdc->xebec.dcb[2] & 0x3fu) |
        (lib_u8)((cylinder >> 2u) & 0xc0u);
    hdc->xebec.dcb[2] = (hdc->xebec.dcb[2] & 0xc0u) | sector;
    hdc->xebec.dcb[3] = (lib_u8)cylinder;
    return 1;
}

static void core_machine_xebec_start_transfer(core_machine_hdc *hdc)
{
    lib_u8 sense[4] = {0x04u, 0u, 0u, 0u};

    if (hdc == LIB_NULL) return;
    sense[1] = hdc->xebec.dcb[1] & 0x20u;
    sense[2] = hdc->xebec.dcb[2];
    sense[3] = hdc->xebec.dcb[3];
    hdc->xebec.byte_index = 0u;
    if (hdc->xebec.dcb[0] == 0x08u) {
        if (hdc->xebec.dcb[4] == 0u || !core_machine_xebec_transfer_sector(hdc, LIB_FALSE)) {
            core_machine_xebec_response(hdc, 0x02u, sense);
            return;
        }
        hdc->xebec.sectors_remaining = hdc->xebec.dcb[4];
        hdc->xebec.phase = CORE_MACHINE_XEBEC_PHASE_DMA_READ;
    } else {
        if (hdc->xebec.dcb[4] == 0u || !core_machine_xebec_can_transfer(hdc, LIB_TRUE)) {
            /* A Write Data block count is documented, but zero's meaning is
             * not; reject it rather than inventing an implicit 256-sector form. */
            core_machine_xebec_response(hdc, 0x02u, sense);
            return;
        }
        hdc->xebec.sectors_remaining = hdc->xebec.dcb[4];
        hdc->xebec.phase = CORE_MACHINE_XEBEC_PHASE_DMA_WRITE;
    }
    core_machine_xebec_sync_dma_request(hdc);
}

static lib_i32 core_machine_xebec_command_is_defined(lib_u8 command)
{
    return (command <= 0x01u || (command >= 0x03u && command <= 0x08u) ||
        (command >= 0x0au && command <= 0x0fu) || command == 0xe0u ||
        (command >= 0xe3u && command <= 0xe6u));
}

static void core_machine_xebec_complete_dcb(core_machine_hdc *hdc)
{
    lib_u8 sense[4] = {0u, 0u, 0u, 0u};

    if (hdc == LIB_NULL) return;
    if (hdc->xebec.dcb[0] == 0x03u) {
        lib_memory_copy(hdc->xebec.response, hdc->xebec.last_sense,
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
    sense[1] = (lib_u8)((hdc->xebec.dcb[1] >> 5u) & 1u) << 5u;
    sense[2] = hdc->xebec.dcb[2];
    sense[3] = hdc->xebec.dcb[3];
    core_machine_xebec_response(hdc, 0x02u, sense);
}

static void core_machine_xebec_schedule_dcb_completion(core_machine_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    hdc->xebec.phase = CORE_MACHINE_XEBEC_PHASE_PENDING_COMMAND;
    core_machine_hdc_schedule_service(hdc, hdc->connect.config.service.command_ticks);
}

static lib_status core_machine_xebec_port_read(core_machine_hdc *hdc,
    lib_u16 port, lib_u32 *out_value)
{
    if (hdc == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == hdc->connect.config.bus.xebec.data_port) {
        if (hdc->xebec.phase != CORE_MACHINE_XEBEC_PHASE_RESPONSE ||
            hdc->xebec.response_index >= hdc->xebec.response_count) return LIB_STATUS_OK;
        *out_value = hdc->xebec.response[hdc->xebec.response_index++];
        core_machine_hdc_clear_irq(hdc);
        if (hdc->xebec.response_index == hdc->xebec.response_count) {
            if (hdc->xebec.dcb[0] == 0x03u)
                lib_memory_set(hdc->xebec.last_sense, 0, sizeof(hdc->xebec.last_sense));
            hdc->xebec.phase = CORE_MACHINE_XEBEC_PHASE_IDLE;
            hdc->xebec.dcb_count = 0u;
        }
        return LIB_STATUS_OK;
    }
    return LIB_STATUS_UNSUPPORTED;
}

static lib_status core_machine_xebec_port_write(core_machine_hdc *hdc,
    lib_u16 port, lib_u32 value)
{
    if (hdc == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (port == hdc->connect.config.bus.xebec.hardware_status_reset_port) {
        core_machine_xebec_reset(hdc);
        return LIB_STATUS_OK;
    }
    if (port == hdc->connect.config.bus.xebec.dma_irq_mask_port) {
        hdc->xebec.mask_pattern = (lib_u8)value;
        core_machine_xebec_sync_dma_request(hdc);
        return LIB_STATUS_OK;
    }
    if (port == hdc->connect.config.bus.xebec.jumpers_select_port) {
        hdc->xebec.dcb_count = 0u;
        hdc->xebec.initialize_count = 0u;
        hdc->xebec.phase = CORE_MACHINE_XEBEC_PHASE_DCB;
        return LIB_STATUS_OK;
    }
    if (port != hdc->connect.config.bus.xebec.data_port ||
        hdc->xebec.phase == CORE_MACHINE_XEBEC_PHASE_IDLE ||
        hdc->xebec.phase == CORE_MACHINE_XEBEC_PHASE_RESPONSE) return LIB_STATUS_OK;
    if (hdc->xebec.phase == CORE_MACHINE_XEBEC_PHASE_DCB) {
        hdc->xebec.dcb[hdc->xebec.dcb_count++] = (lib_u8)value;
        if (hdc->xebec.dcb_count == sizeof(hdc->xebec.dcb)) {
            if (hdc->xebec.dcb[0] == 0x0cu) hdc->xebec.phase = CORE_MACHINE_XEBEC_PHASE_INITIALIZE;
            else core_machine_xebec_schedule_dcb_completion(hdc);
        }
    } else if (hdc->xebec.phase == CORE_MACHINE_XEBEC_PHASE_INITIALIZE) {
        hdc->xebec.initialize[hdc->xebec.initialize_count++] = (lib_u8)value;
        if (hdc->xebec.initialize_count == sizeof(hdc->xebec.initialize))
            core_machine_xebec_schedule_dcb_completion(hdc);
    }
    return LIB_STATUS_OK;
}

static lib_status core_machine_hdc_port_read(void *opaque, lib_u16 port,
    lib_u32 *out_value)
{
    core_machine_hdc *hdc = (core_machine_hdc *)opaque;
    lib_u16 word;

    if (hdc == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_value = 0u;
    if (core_machine_hdc_is_xebec_xt(hdc)) return core_machine_xebec_port_read(hdc, port, out_value);
    if (port == hdc->connect.config.bus.task_file.data_port) {
        if (hdc->data.phase != CORE_MACHINE_HDC_PHASE_DATA_READ ||
            hdc->data.data_index >= sizeof(hdc->data.data)) {
            return LIB_STATUS_OK;
        }
        lib_memory_copy(&word, &hdc->data.data[hdc->data.data_index], sizeof(word));
        *out_value = word;
        hdc->data.data_index = (lib_u16)(hdc->data.data_index + sizeof(word));
        if (hdc->data.data_index == sizeof(hdc->data.data)) {
            hdc->data.phase = CORE_MACHINE_HDC_PHASE_PENDING_READ_SECTOR;
            hdc->data.status = CORE_MACHINE_HDC_STATUS_BSY;
            core_machine_hdc_schedule_service(hdc,
                hdc->connect.config.service.next_sector_ticks);
        }
        return LIB_STATUS_OK;
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
        if (core_machine_hdc_is_ibm_wd1003(hdc)) return LIB_STATUS_UNSUPPORTED;
        *out_value = hdc->data.status;
    } else if (port == hdc->connect.config.bus.task_file.drive_address_port &&
        core_machine_hdc_is_compaq_wd_40mb(hdc)) {
        *out_value = hdc->data.drive_head & 0x1fu;
    } else {
        return LIB_STATUS_UNSUPPORTED;
    }
    return LIB_STATUS_OK;
}

static lib_status core_machine_hdc_port_write(void *opaque, lib_u16 port,
    lib_u32 value)
{
    core_machine_hdc *hdc = (core_machine_hdc *)opaque;

    if (hdc == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (core_machine_hdc_is_xebec_xt(hdc)) return core_machine_xebec_port_write(hdc, port, value);
    if (port == hdc->connect.config.bus.task_file.data_port) {
        lib_u16 word = (lib_u16)value;
        if (hdc->data.phase != CORE_MACHINE_HDC_PHASE_DATA_WRITE ||
            hdc->data.data_index >= sizeof(hdc->data.data)) {
            return LIB_STATUS_OK;
        }
        lib_memory_copy(&hdc->data.data[hdc->data.data_index], &word, sizeof(word));
        hdc->data.data_index = (lib_u16)(hdc->data.data_index + sizeof(word));
        if (hdc->data.data_index == sizeof(hdc->data.data)) {
            hdc->data.phase = CORE_MACHINE_HDC_PHASE_PENDING_WRITE_SECTOR;
            hdc->data.status = CORE_MACHINE_HDC_STATUS_BSY;
            core_machine_hdc_schedule_service(hdc,
                hdc->connect.config.service.next_sector_ticks);
        }
        return LIB_STATUS_OK;
    }
    if (!core_machine_hdc_task_file_is_writable(hdc) &&
        (port == hdc->connect.config.bus.task_file.error_features_port ||
            port == hdc->connect.config.bus.task_file.sector_count_port ||
            port == hdc->connect.config.bus.task_file.sector_number_port ||
            port == hdc->connect.config.bus.task_file.cylinder_low_port ||
            port == hdc->connect.config.bus.task_file.cylinder_high_port ||
            port == hdc->connect.config.bus.task_file.drive_head_port)) return LIB_STATUS_OK;
    if (port == hdc->connect.config.bus.task_file.error_features_port) {
        hdc->data.features = (lib_u8)value;
    } else if (port == hdc->connect.config.bus.task_file.sector_count_port) {
        hdc->data.sector_count = (lib_u8)value;
    } else if (port == hdc->connect.config.bus.task_file.sector_number_port) {
        hdc->data.sector_number = (lib_u8)value;
    } else if (port == hdc->connect.config.bus.task_file.cylinder_low_port) {
        hdc->data.cylinder_low = (lib_u8)value;
    } else if (port == hdc->connect.config.bus.task_file.cylinder_high_port) {
        hdc->data.cylinder_high = (lib_u8)value;
    } else if (port == hdc->connect.config.bus.task_file.drive_head_port) {
        hdc->data.drive_head = (lib_u8)value;
        core_machine_hdc_refresh_compaq_selection_status(hdc);
    } else if (port == hdc->connect.config.bus.task_file.status_command_port) {
        core_machine_hdc_capture_command(hdc, (lib_u8)value);
    } else if (port == hdc->connect.config.bus.task_file.alternate_status_device_control_port &&
        core_machine_hdc_is_ibm_wd1003(hdc)) {
        hdc->data.fixed_disk_register = (lib_u8)value & 0x08u;
    } else if (port == hdc->connect.config.bus.task_file.alternate_status_device_control_port) {
        lib_u8 device_control = (lib_u8)value;
        lib_u8 reset_asserted = (device_control &
            CORE_MACHINE_HDC_DEVICE_CONTROL_SRST) != 0u;

        hdc->data.device_control = device_control;
        core_machine_hdc_sync_irq(hdc);
        if (reset_asserted && !hdc->data.reset_asserted) {
            hdc->data.reset_asserted = LIB_TRUE;
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
        return LIB_STATUS_UNSUPPORTED;
    }
    return LIB_STATUS_OK;
}

static const core_machine_port_provider core_machine_hdc_ports = {
    core_machine_hdc_port_read,
    core_machine_hdc_port_write
};

static void core_machine_xebec_dma_read(void *owner, t_latch *latch)
{
    core_machine_hdc *hdc = owner;
    lib_u8 sense[4] = {0x04u, 0u, 0u, 0u};

    if (hdc == LIB_NULL || latch == LIB_NULL ||
        hdc->xebec.phase != CORE_MACHINE_XEBEC_PHASE_DMA_READ ||
        hdc->xebec.byte_index >= sizeof(hdc->data.data)) return;
    latch->data.byte = hdc->data.data[hdc->xebec.byte_index++];
    if (hdc->xebec.byte_index == sizeof(hdc->data.data)) {
        if (--hdc->xebec.sectors_remaining == 0u) {
            core_machine_xebec_release_dma(hdc);
            core_machine_xebec_response(hdc, 0u, LIB_NULL);
        } else if (!core_machine_xebec_next_sector(hdc) ||
            !core_machine_xebec_transfer_sector(hdc, LIB_FALSE)) {
            sense[1] = hdc->xebec.dcb[1] & 0x20u;
            sense[2] = hdc->xebec.dcb[2];
            sense[3] = hdc->xebec.dcb[3];
            core_machine_xebec_release_dma(hdc);
            core_machine_xebec_response(hdc, 0x02u, sense);
        } else hdc->xebec.byte_index = 0u;
    }
}

static void core_machine_xebec_dma_write(void *owner, t_latch *latch)
{
    core_machine_hdc *hdc = owner;
    lib_u8 sense[4] = {0x04u, 0u, 0u, 0u};

    if (hdc == LIB_NULL || latch == LIB_NULL ||
        hdc->xebec.phase != CORE_MACHINE_XEBEC_PHASE_DMA_WRITE ||
        hdc->xebec.byte_index >= sizeof(hdc->data.data)) return;
    hdc->data.data[hdc->xebec.byte_index++] = latch->data.byte;
    if (hdc->xebec.byte_index != sizeof(hdc->data.data)) return;
    sense[1] = hdc->xebec.dcb[1] & 0x20u;
    sense[2] = hdc->xebec.dcb[2];
    sense[3] = hdc->xebec.dcb[3];
    if (!core_machine_xebec_transfer_sector(hdc, LIB_TRUE) ||
        hdc->xebec.sectors_remaining == 0u) {
        core_machine_xebec_release_dma(hdc);
        core_machine_xebec_response(hdc, 0x02u, sense);
        return;
    }
    if (--hdc->xebec.sectors_remaining == 0u) {
        core_machine_xebec_release_dma(hdc);
        core_machine_xebec_response(hdc, 0u, LIB_NULL);
        return;
    }
    if (!core_machine_xebec_next_sector(hdc) ||
        !core_machine_xebec_can_transfer(hdc, LIB_TRUE)) {
        core_machine_xebec_release_dma(hdc);
        core_machine_xebec_response(hdc, 0x02u, sense);
        return;
    }
    hdc->xebec.byte_index = 0u;
}

static void core_machine_xebec_dma_terminal(void *owner, t_latch *latch)
{
    core_machine_hdc *hdc = owner;
    lib_u8 sense[4] = {0x04u, 0u, 0u, 0u};

    (void)latch;
    if (hdc == LIB_NULL || (hdc->xebec.phase != CORE_MACHINE_XEBEC_PHASE_DMA_READ &&
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

void core_machine_hdc_connect(core_machine_hdc *hdc,
    const core_machine_media_registry *media_registry,
    core_machine_media_id media_id, core_machine_media_id slave_media_id,
    t_pic *pic_master, t_pic *pic_slave, const core_machine_hdc_config *config)
{
    if (hdc == LIB_NULL || config == LIB_NULL) return;
    hdc->connect.media_registry = media_registry;
    hdc->connect.media_id = media_id;
    hdc->connect.slave_media_id = slave_media_id;
    core_machine_pic_irq_source_bind(&hdc->connect.irq_source, pic_master,
        pic_slave, config->irq);
    hdc->connect.config = *config;
}

void core_machine_hdc_bind_dma_request(core_machine_hdc *hdc,
    const core_machine_dma_request_binding *binding,
    void (*request_assert)(void *owner,
        const core_machine_dma_request_binding *binding),
    void (*request_deassert)(void *owner,
        const core_machine_dma_request_binding *binding), void *owner)
{
    if (hdc == LIB_NULL || binding == LIB_NULL || request_assert == LIB_NULL ||
        request_deassert == LIB_NULL || owner == LIB_NULL) return;
    hdc->connect.dma_request = *binding;
    hdc->connect.dma_request_assert = request_assert;
    hdc->connect.dma_request_deassert = request_deassert;
    hdc->connect.dma_request_owner = owner;
}

void core_machine_hdc_initialize(core_machine_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    lib_memory_set(&hdc->data, 0, sizeof(hdc->data));
    lib_memory_set(&hdc->xebec, 0, sizeof(hdc->xebec));
    hdc->xebec.phase = CORE_MACHINE_XEBEC_PHASE_IDLE;
    core_machine_pic_irq_source_deassert(&hdc->connect.irq_source);
    core_machine_hdc_reset(hdc);
}

void core_machine_hdc_reset(core_machine_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    lib_memory_set(&hdc->data, 0, sizeof(hdc->data));
    if (core_machine_hdc_is_xebec_xt(hdc)) core_machine_xebec_reset(hdc);
    core_machine_hdc_clear_irq(hdc);
    hdc->data.error = CORE_MACHINE_HDC_ERROR_DIAGNOSTIC_OK;
    hdc->data.sector_count = 1u;
    hdc->data.sector_number = 1u;
    hdc->data.status = CORE_MACHINE_HDC_STATUS_DRDY | CORE_MACHINE_HDC_STATUS_DSC;
}

void core_machine_hdc_advance_elapsed(core_machine_hdc *hdc,
    lib_u64 elapsed_ticks)
{
    if (hdc == LIB_NULL || elapsed_ticks == 0u || UINT64_MAX -
        hdc->data.elapsed_ticks < elapsed_ticks) return;
    hdc->data.elapsed_ticks += elapsed_ticks;
    if (hdc->data.next_service_tick != 0u &&
        hdc->data.elapsed_ticks < hdc->data.next_service_tick) return;
    hdc->data.next_service_tick = 0u;
    if (core_machine_hdc_is_xebec_xt(hdc) &&
        hdc->xebec.phase == CORE_MACHINE_XEBEC_PHASE_PENDING_COMMAND) {
        core_machine_xebec_complete_dcb(hdc);
    } else if (hdc->data.phase == CORE_MACHINE_HDC_PHASE_PENDING_COMMAND) {
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

void core_machine_hdc_advance(core_machine_hdc *hdc)
{
    lib_u64 ticks = 1u;

    if (hdc == LIB_NULL) return;
    if (hdc->data.next_service_tick > hdc->data.elapsed_ticks) {
        ticks = hdc->data.next_service_tick - hdc->data.elapsed_ticks;
    }
    core_machine_hdc_advance_elapsed(hdc, ticks);
}

lib_status core_machine_hdc_next_due_tick(const core_machine_hdc *hdc,
    lib_u64 *out_due_tick)
{
    if (hdc == LIB_NULL || out_due_tick == LIB_NULL ||
        hdc->data.next_service_tick == 0u) return LIB_STATUS_INVALID_STATE;
    if (core_machine_hdc_is_xebec_xt(hdc)) {
        if (hdc->xebec.phase != CORE_MACHINE_XEBEC_PHASE_PENDING_COMMAND)
            return LIB_STATUS_INVALID_STATE;
    } else if (hdc->data.phase == CORE_MACHINE_HDC_PHASE_IDLE) {
        return LIB_STATUS_INVALID_STATE;
    }
    *out_due_tick = hdc->data.next_service_tick;
    return LIB_STATUS_OK;
}

void core_machine_hdc_finalize(core_machine_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    core_machine_pic_irq_source_deassert(&hdc->connect.irq_source);
    lib_memory_set(&hdc->data, 0, sizeof(hdc->data));
    lib_memory_set(&hdc->xebec, 0, sizeof(hdc->xebec));
    lib_memory_set(&hdc->connect, 0, sizeof(hdc->connect));
}

const core_machine_port_provider *core_machine_hdc_port_provider(void)
{
    return &core_machine_hdc_ports;
}

const core_machine_dma_channel_provider *core_machine_hdc_dma_provider(void)
{
    return &core_machine_hdc_dma_channel;
}

lib_u8 core_machine_hdc_irq_pending(const core_machine_hdc *hdc)
{
    return hdc != LIB_NULL && hdc->data.irq_pending &&
        (hdc->data.device_control & CORE_MACHINE_HDC_DEVICE_CONTROL_NIEN) == 0u;
}
