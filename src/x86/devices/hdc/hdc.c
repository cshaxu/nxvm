#include "x86/devices/hdc/hdc.h"


#define X86_HDC_COMMAND_READ_SECTORS 0x20u
#define X86_HDC_COMMAND_WRITE_SECTORS 0x30u
#define X86_HDC_COMMAND_VERIFY_SECTORS 0x40u
#define X86_HDC_COMMAND_INITIALIZE_DRIVE_PARAMETERS 0x91u
#define X86_HDC_COMMAND_EXECUTE_DIAGNOSTICS 0x90u
#define X86_HDC_COMMAND_IDENTIFY_DEVICE 0xecu
#define X86_HDC_COMMAND_RECALIBRATE_MASK 0xf0u
#define X86_HDC_COMMAND_RECALIBRATE_VALUE 0x10u
#define X86_HDC_COMMAND_SEEK_MASK 0xf0u
#define X86_HDC_COMMAND_SEEK_VALUE 0x70u
#define X86_HDC_IBM_RESTORE_STEP_LIMIT 1023u
#define X86_XEBEC_MASK_DMA_ENABLE 0x01u
#define X86_XEBEC_MASK_IRQ_ENABLE 0x02u
static void x86_hdc_reset_controller(x86_hdc *hdc);
static lib_i32 x86_hdc_is_compaq_wd_40mb(const x86_hdc *hdc)
{
    return hdc != LIB_NULL && hdc->config.protocol ==
        X86_HDC_PROTOCOL_COMPAQ_WD_40MB;
}

static lib_i32 x86_hdc_is_ibm_wd1003(const x86_hdc *hdc)
{
    return hdc != LIB_NULL && hdc->config.protocol ==
        X86_HDC_PROTOCOL_IBM_WD1003_ST506;
}

static lib_i32 x86_hdc_is_xebec_xt(const x86_hdc *hdc)
{
    return hdc != LIB_NULL && hdc->config.protocol ==
        X86_HDC_PROTOCOL_XEBEC_XT;
}

static lib_i32 x86_hdc_task_file_is_writable(const x86_hdc *hdc)
{
    return hdc != LIB_NULL && (x86_hdc_is_compaq_wd_40mb(hdc) ||
        (hdc->data.status & (X86_HDC_STATUS_BSY |
            X86_HDC_STATUS_DRQ)) == 0u);
}

static lib_i32 x86_hdc_selected_master(const x86_hdc *hdc)
{
    return hdc != LIB_NULL && (hdc->data.drive_head & 0x10u) == 0u;
}

static lib_u8 x86_hdc_current_head(const x86_hdc *hdc)
{
    if (hdc == LIB_NULL) return 0u;
    if (x86_hdc_is_ibm_wd1003(hdc)) {
        return (lib_u8)((hdc->data.drive_head & 0x07u) |
            (hdc->data.fixed_disk_register & 0x08u));
    }
    return hdc->data.drive_head & 0x0fu;
}

static void x86_hdc_set_current_head(x86_hdc *hdc,
    lib_u8 head)
{
    if (hdc == LIB_NULL) return;
    if (x86_hdc_is_ibm_wd1003(hdc)) {
        hdc->data.drive_head = (lib_u8)((hdc->data.drive_head & 0xf8u) |
            (head & 0x07u));
        hdc->data.fixed_disk_register = (lib_u8)((hdc->data.fixed_disk_register &
            0xf7u) | (head & 0x08u));
        return;
    }
    hdc->data.drive_head = (lib_u8)((hdc->data.drive_head & 0xf0u) | head);
}

static lib_i32 x86_hdc_lba_mode(const x86_hdc *hdc)
{
    return hdc != LIB_NULL && !x86_hdc_is_compaq_wd_40mb(hdc) &&
        hdc->config.lba28_supported &&
        (hdc->data.drive_head & 0x40u) != 0u;
}

static lib_i32 x86_hdc_command_is_read(const x86_hdc *hdc,
    lib_u8 command)
{
    if (x86_hdc_is_compaq_wd_40mb(hdc)) {
        return (command & 0xfeu) == X86_HDC_COMMAND_READ_SECTORS;
    }
    if (x86_hdc_is_ibm_wd1003(hdc)) {
        return (command & 0xfcu) == X86_HDC_COMMAND_READ_SECTORS;
    }
    return command == X86_HDC_COMMAND_READ_SECTORS;
}

static lib_i32 x86_hdc_command_is_write(const x86_hdc *hdc,
    lib_u8 command)
{
    if (x86_hdc_is_compaq_wd_40mb(hdc)) {
        return (command & 0xfeu) == X86_HDC_COMMAND_WRITE_SECTORS;
    }
    if (x86_hdc_is_ibm_wd1003(hdc)) {
        return (command & 0xfcu) == X86_HDC_COMMAND_WRITE_SECTORS;
    }
    return command == X86_HDC_COMMAND_WRITE_SECTORS;
}

static void x86_hdc_select_ibm_step_rate(x86_hdc *hdc,
    lib_u8 selector, lib_u16 pulse_limit)
{
    if (!x86_hdc_is_ibm_wd1003(hdc)) return;
    hdc->data.step_rate_selector = selector;
    hdc->data.step_pulse_limit = pulse_limit;
    hdc->data.step_rate_ticks = hdc->config.step_ticks[selector];
}

static void x86_hdc_sync_irq(x86_hdc *hdc)
{
    if (hdc != LIB_NULL && hdc->connect.irq != LIB_NULL)
        hdc->connect.irq(hdc->connect.context, x86_hdc_irq_pending(hdc));
}

static void x86_hdc_clear_irq(x86_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    hdc->data.irq_pending = LIB_FALSE;
    x86_hdc_sync_irq(hdc);
}

static void x86_hdc_raise_irq(x86_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    hdc->data.irq_pending = LIB_TRUE;
    x86_hdc_sync_irq(hdc);
}

static lib_u32 x86_hdc_lba(const x86_hdc *hdc)
{
    return (lib_u32)hdc->data.sector_number |
        ((lib_u32)hdc->data.cylinder_low << 8u) |
        ((lib_u32)hdc->data.cylinder_high << 16u) |
        ((lib_u32)(hdc->data.drive_head & 0x0fu) << 24u);
}

static lib_u8 x86_hdc_selected_unit(const x86_hdc *hdc)
{
    return (hdc->data.drive_head >> 4u) & 1u;
}

static lib_i32 x86_hdc_media_info(const x86_hdc *hdc,
    x86_hdc_medium *out_info, x86_hdc_record_result *out_result)
{
    if (hdc == LIB_NULL || hdc->connect.query == LIB_NULL) return 0;
    lib_memory_set(out_info, 0, sizeof(*out_info));
    *out_result = hdc->connect.query(hdc->connect.context,
        x86_hdc_selected_unit(hdc), out_info);
    return *out_result == X86_HDC_RECORD_OK;
}

static void x86_hdc_refresh_compaq_selection_status(x86_hdc *hdc)
{
    if (!x86_hdc_is_compaq_wd_40mb(hdc) ||
        hdc->data.phase != X86_HDC_PHASE_IDLE) return;
    /* Selecting a task-file drive is a controller operation, not media
     * insertion.  The Compaq firmware probes a fitted controller before it
     * knows whether a fixed disk is attached; keep its reset-ready state
     * visible here and let a command needing sectors fail through the sole
     * media route. */
    hdc->data.status = X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC;
}

static lib_u64 x86_hdc_sector_capacity(
    const x86_hdc_medium *info)
{
    return info == LIB_NULL || info->geometry.bytes_per_sector == 0u ? 0u :
        (lib_u64)info->geometry.logical_sector_count;
}

static void x86_hdc_complete(x86_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    hdc->data.phase = X86_HDC_PHASE_IDLE;
    hdc->data.next_service_tick = 0u;
    hdc->data.data_index = 0u;
    hdc->data.status = X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC;
    x86_hdc_raise_irq(hdc);
}

static void x86_hdc_fail(x86_hdc *hdc, lib_u8 error)
{
    if (hdc == LIB_NULL) return;
    hdc->data.error = error;
    hdc->data.phase = X86_HDC_PHASE_IDLE;
    hdc->data.next_service_tick = 0u;
    hdc->data.data_index = 0u;
    hdc->data.status = X86_HDC_STATUS_DRDY | X86_HDC_STATUS_ERR;
    x86_hdc_raise_irq(hdc);
}

static lib_i32 x86_hdc_resolve_sector(x86_hdc *hdc,
    lib_u8 write_to_media, lib_u64 *out_sector)
{
    x86_hdc_medium info;
    x86_hdc_record_result media_result;
    lib_u16 cylinder;
    lib_u8 head;
    lib_u8 sector;
    lib_u32 lba;

    if (hdc == LIB_NULL || out_sector == LIB_NULL ||
        !x86_hdc_media_info(hdc, &info, &media_result) || !info.present ||
        (write_to_media && info.read_only)) {
        x86_hdc_fail(hdc, X86_HDC_ERROR_ABORT);
        return 0;
    }
    if (x86_hdc_lba_mode(hdc)) {
        lba = x86_hdc_lba(hdc);
        if (!x86_hdc_selected_master(hdc) ||
            (lib_u64)lba >= x86_hdc_sector_capacity(&info) ||
            info.geometry.bytes_per_sector != sizeof(hdc->data.data)) {
            x86_hdc_fail(hdc, X86_HDC_ERROR_ID_NOT_FOUND);
            return 0;
        }
        *out_sector = lba;
        return 1;
    }
    cylinder = (lib_u16)hdc->data.cylinder_low |
        ((lib_u16)hdc->data.cylinder_high << 8u);
    head = x86_hdc_current_head(hdc);
    sector = hdc->data.sector_number;
    if ((!x86_hdc_is_compaq_wd_40mb(hdc) && !x86_hdc_selected_master(hdc)) ||
        sector == 0u || cylinder >= info.geometry.cylinders ||
        head >= info.geometry.heads || sector > info.geometry.sectors_per_track ||
        info.geometry.bytes_per_sector != sizeof(hdc->data.data)) {
        x86_hdc_fail(hdc, X86_HDC_ERROR_ID_NOT_FOUND);
        return 0;
    }
    *out_sector = ((lib_u64)cylinder * info.geometry.heads + head) *
        info.geometry.sectors_per_track + (sector - 1u);
    return 1;
}

static lib_i32 x86_hdc_load_sector(x86_hdc *hdc)
{
    x86_hdc_record_result media_result;
    lib_u64 sector;

    if (!x86_hdc_resolve_sector(hdc, LIB_FALSE, &sector)) return 0;
    media_result = hdc->connect.read == LIB_NULL ? X86_HDC_RECORD_FAILURE :
        hdc->connect.read(hdc->connect.context, x86_hdc_selected_unit(hdc),
            sector, hdc->data.data);
    if (media_result != X86_HDC_RECORD_OK) {
        x86_hdc_fail(hdc, media_result == X86_HDC_RECORD_RANGE ?
            X86_HDC_ERROR_ID_NOT_FOUND : X86_HDC_ERROR_ABORT);
        return 0;
    }
    return 1;
}

static lib_i32 x86_hdc_store_sector(x86_hdc *hdc)
{
    x86_hdc_record_result media_result;
    lib_u64 sector;

    if (!x86_hdc_resolve_sector(hdc, LIB_TRUE, &sector)) return 0;
    media_result = hdc->connect.write == LIB_NULL ? X86_HDC_RECORD_FAILURE :
        hdc->connect.write(hdc->connect.context, x86_hdc_selected_unit(hdc),
            sector, hdc->data.data);
    if (media_result != X86_HDC_RECORD_OK) {
        x86_hdc_fail(hdc, media_result == X86_HDC_RECORD_RANGE ?
            X86_HDC_ERROR_ID_NOT_FOUND : X86_HDC_ERROR_ABORT);
        return 0;
    }
    return 1;
}

static void x86_hdc_identify(x86_hdc *hdc)
{
    x86_hdc_medium info;
    x86_hdc_record_result media_result;
    lib_u16 word;

    if (hdc == LIB_NULL || !x86_hdc_media_info(hdc, &info, &media_result) ||
        !x86_hdc_selected_master(hdc) ||
        !info.present || info.geometry.bytes_per_sector != sizeof(hdc->data.data)) {
        x86_hdc_fail(hdc, X86_HDC_ERROR_ABORT);
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
    word = (lib_u16)x86_hdc_sector_capacity(&info);
    lib_memory_copy(&hdc->data.data[120], &word, sizeof(word));
    word = (lib_u16)(x86_hdc_sector_capacity(&info) >> 16u);
    lib_memory_copy(&hdc->data.data[122], &word, sizeof(word));
    hdc->data.phase = X86_HDC_PHASE_DATA_READ;
    hdc->data.data_index = 0u;
    hdc->data.error = 0u;
    hdc->data.status = X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC |
        X86_HDC_STATUS_DRQ;
    x86_hdc_raise_irq(hdc);
}

static lib_i32 x86_hdc_advance_chs(x86_hdc *hdc)
{
    x86_hdc_medium info;
    x86_hdc_record_result media_result;
    lib_u16 cylinder;
    lib_u8 head;
    lib_u8 sector;

    if (hdc == LIB_NULL || !x86_hdc_media_info(hdc, &info, &media_result) ||
        !info.present) {
        x86_hdc_fail(hdc, X86_HDC_ERROR_ABORT);
        return 0;
    }
    cylinder = (lib_u16)hdc->data.cylinder_low |
        ((lib_u16)hdc->data.cylinder_high << 8u);
    head = x86_hdc_current_head(hdc);
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
        x86_hdc_fail(hdc, X86_HDC_ERROR_ID_NOT_FOUND);
        return 0;
    }
    hdc->data.sector_number = sector;
    hdc->data.cylinder_low = (lib_u8)cylinder;
    hdc->data.cylinder_high = (lib_u8)(cylinder >> 8u);
    x86_hdc_set_current_head(hdc, head);
    return 1;
}

static lib_i32 x86_hdc_advance_lba(x86_hdc *hdc)
{
    x86_hdc_medium info;
    x86_hdc_record_result media_result;
    lib_u32 lba;

    if (hdc == LIB_NULL || !x86_hdc_media_info(hdc, &info, &media_result) ||
        !info.present) {
        x86_hdc_fail(hdc, X86_HDC_ERROR_ABORT);
        return 0;
    }
    lba = x86_hdc_lba(hdc) + 1u;
    if ((lib_u64)lba >= x86_hdc_sector_capacity(&info)) {
        x86_hdc_fail(hdc, X86_HDC_ERROR_ID_NOT_FOUND);
        return 0;
    }
    hdc->data.sector_number = (lib_u8)lba;
    hdc->data.cylinder_low = (lib_u8)(lba >> 8u);
    hdc->data.cylinder_high = (lib_u8)(lba >> 16u);
    hdc->data.drive_head = (hdc->data.drive_head & 0xf0u) |
        (lib_u8)(lba >> 24u);
    return 1;
}

static lib_i32 x86_hdc_advance_sector(x86_hdc *hdc)
{
    return x86_hdc_lba_mode(hdc) ? x86_hdc_advance_lba(hdc) :
        x86_hdc_advance_chs(hdc);
}

static void x86_hdc_complete_data_sector(x86_hdc *hdc)
{
    if (hdc == LIB_NULL || hdc->data.sectors_remaining == 0u) return;
    --hdc->data.sectors_remaining;
    --hdc->data.sector_count;
}

static void x86_hdc_next_read_sector(x86_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    x86_hdc_complete_data_sector(hdc);
    if (hdc->data.sectors_remaining == 0u) {
        x86_hdc_complete(hdc);
        return;
    }
    if (!x86_hdc_advance_sector(hdc) || !x86_hdc_load_sector(hdc)) {
        return;
    }
    hdc->data.data_index = 0u;
    hdc->data.phase = X86_HDC_PHASE_DATA_READ;
    hdc->data.status = X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC |
        X86_HDC_STATUS_DRQ;
    x86_hdc_raise_irq(hdc);
}

static void x86_hdc_next_write_sector(x86_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    if (!x86_hdc_store_sector(hdc)) return;
    x86_hdc_complete_data_sector(hdc);
    if (hdc->data.sectors_remaining == 0u) {
        x86_hdc_complete(hdc);
        return;
    }
    if (!x86_hdc_advance_sector(hdc)) return;
    hdc->data.data_index = 0u;
    lib_memory_set(hdc->data.data, 0, sizeof(hdc->data.data));
    hdc->data.phase = X86_HDC_PHASE_DATA_WRITE;
    hdc->data.status = X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC |
        X86_HDC_STATUS_DRQ;
    x86_hdc_raise_irq(hdc);
}

static void x86_hdc_schedule_service(x86_hdc *hdc,
    lib_u32 service_ticks)
{
    if (hdc == LIB_NULL) return;
    hdc->data.next_service_tick = service_ticks == 0u ? hdc->data.elapsed_ticks :
        hdc->data.elapsed_ticks + service_ticks;
}

static void x86_hdc_capture_command(x86_hdc *hdc,
    lib_u8 command)
{
    if (hdc == LIB_NULL || hdc->data.reset_asserted ||
        !x86_hdc_task_file_is_writable(hdc)) return;
    hdc->data.pending_command = command;
    hdc->data.pending_features = hdc->data.features;
    hdc->data.pending_sector_count = hdc->data.sector_count;
    hdc->data.pending_sector_number = hdc->data.sector_number;
    hdc->data.pending_cylinder_low = hdc->data.cylinder_low;
    hdc->data.pending_cylinder_high = hdc->data.cylinder_high;
    hdc->data.pending_drive_head = hdc->data.drive_head;
    hdc->data.error = 0u;
    x86_hdc_clear_irq(hdc);
    hdc->data.phase = X86_HDC_PHASE_PENDING_COMMAND;
    hdc->data.status = X86_HDC_STATUS_BSY;
    x86_hdc_schedule_service(hdc, hdc->config.command_ticks);
}

static void x86_hdc_begin_read(x86_hdc *hdc)
{
    if (!x86_hdc_load_sector(hdc)) return;
    hdc->data.phase = X86_HDC_PHASE_DATA_READ;
    hdc->data.sectors_remaining = hdc->data.sector_count == 0u ? 256u :
        hdc->data.sector_count;
    hdc->data.data_index = 0u;
    hdc->data.status = X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC |
        X86_HDC_STATUS_DRQ;
    x86_hdc_raise_irq(hdc);
}

static void x86_hdc_begin_write(x86_hdc *hdc)
{
    lib_u64 sector;

    if (!x86_hdc_resolve_sector(hdc, LIB_TRUE, &sector)) return;
    hdc->data.phase = X86_HDC_PHASE_DATA_WRITE;
    hdc->data.sectors_remaining = hdc->data.sector_count == 0u ? 256u :
        hdc->data.sector_count;
    hdc->data.data_index = 0u;
    hdc->data.status = X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC |
        X86_HDC_STATUS_DRQ;
    x86_hdc_raise_irq(hdc);
}

static void x86_hdc_execute_command(x86_hdc *hdc, lib_u8 command)
{
    lib_u16 cylinder;

    if (hdc == LIB_NULL) return;
    hdc->data.last_command = command;
    ++hdc->data.command_count;
    if ((!x86_hdc_is_compaq_wd_40mb(hdc) &&
            !x86_hdc_selected_master(hdc)) ||
        (!x86_hdc_is_compaq_wd_40mb(hdc) && !x86_hdc_is_ibm_wd1003(hdc) &&
            (hdc->data.drive_head & 0x40u) != 0u &&
            !hdc->config.lba28_supported)) {
        x86_hdc_fail(hdc, X86_HDC_ERROR_ABORT);
        return;
    }
    if (x86_hdc_command_is_read(hdc, command)) {
        x86_hdc_begin_read(hdc);
        return;
    }
    if (x86_hdc_command_is_write(hdc, command)) {
        x86_hdc_begin_write(hdc);
        return;
    }
    if (x86_hdc_is_compaq_wd_40mb(hdc) || x86_hdc_is_ibm_wd1003(hdc)) {
        if (command == X86_HDC_COMMAND_INITIALIZE_DRIVE_PARAMETERS) {
            x86_hdc_complete(hdc);
            return;
        }
        if (command == X86_HDC_COMMAND_EXECUTE_DIAGNOSTICS) {
            x86_hdc_select_ibm_step_rate(hdc, 15u,
                X86_HDC_IBM_RESTORE_STEP_LIMIT);
            hdc->data.error = X86_HDC_ERROR_DIAGNOSTIC_OK;
            x86_hdc_complete(hdc);
            return;
        }
        if ((x86_hdc_is_ibm_wd1003(hdc) &&
                (command & 0xfeu) == X86_HDC_COMMAND_VERIFY_SECTORS) ||
            (!x86_hdc_is_ibm_wd1003(hdc) &&
                command == X86_HDC_COMMAND_VERIFY_SECTORS)) {
            if (x86_hdc_load_sector(hdc)) x86_hdc_complete(hdc);
            return;
        }
        if ((command & X86_HDC_COMMAND_RECALIBRATE_MASK) ==
            X86_HDC_COMMAND_RECALIBRATE_VALUE) {
            x86_hdc_select_ibm_step_rate(hdc, command & 0x0fu,
                X86_HDC_IBM_RESTORE_STEP_LIMIT);
            hdc->data.cylinder_low = 0u;
            hdc->data.cylinder_high = 0u;
            x86_hdc_complete(hdc);
            return;
        }
        if ((command & X86_HDC_COMMAND_SEEK_MASK) ==
            X86_HDC_COMMAND_SEEK_VALUE) {
            x86_hdc_select_ibm_step_rate(hdc, command & 0x0fu, 0u);
            cylinder = (lib_u16)hdc->data.cylinder_low |
                ((lib_u16)hdc->data.cylinder_high << 8u);
            if (cylinder > 1023u) {
                x86_hdc_fail(hdc, X86_HDC_ERROR_ID_NOT_FOUND);
            } else {
                x86_hdc_complete(hdc);
            }
            return;
        }
        x86_hdc_fail(hdc, X86_HDC_ERROR_ABORT);
        return;
    }
    if (command == X86_HDC_COMMAND_IDENTIFY_DEVICE) {
        x86_hdc_identify(hdc);
    } else {
        x86_hdc_fail(hdc, X86_HDC_ERROR_ABORT);
    }
}

static void x86_xebec_reset(x86_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    if (hdc->connect.drq != LIB_NULL)
        hdc->connect.drq(hdc->connect.context, LIB_FALSE);
    lib_memory_set(&hdc->xebec, 0, sizeof(hdc->xebec));
    hdc->xebec.phase = X86_XEBEC_PHASE_IDLE;
    x86_hdc_clear_irq(hdc);
}

static void x86_xebec_response(x86_hdc *hdc,
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
    hdc->xebec.phase = X86_XEBEC_PHASE_RESPONSE;
    if ((hdc->xebec.mask_pattern & X86_XEBEC_MASK_IRQ_ENABLE) != 0u) {
        x86_hdc_raise_irq(hdc);
    }
}

static void x86_xebec_request_dma(x86_hdc *hdc)
{
    if (hdc != LIB_NULL && hdc->connect.drq != LIB_NULL)
        hdc->connect.drq(hdc->connect.context, LIB_TRUE);
}

static void x86_xebec_release_dma(x86_hdc *hdc)
{
    if (hdc != LIB_NULL && hdc->connect.drq != LIB_NULL)
        hdc->connect.drq(hdc->connect.context, LIB_FALSE);
}

static void x86_xebec_sync_dma_request(x86_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    if ((hdc->xebec.mask_pattern & X86_XEBEC_MASK_DMA_ENABLE) != 0u &&
        (hdc->xebec.phase == X86_XEBEC_PHASE_DMA_READ ||
            hdc->xebec.phase == X86_XEBEC_PHASE_DMA_WRITE)) {
        x86_xebec_request_dma(hdc);
    } else {
        x86_xebec_release_dma(hdc);
    }
}

static lib_i32 x86_xebec_media_info(const x86_hdc *hdc,
    x86_hdc_medium *out_info, x86_hdc_record_result *out_result)
{
    const x86_hdc_geometry *expected;

    if (hdc == LIB_NULL || out_info == LIB_NULL || out_result == LIB_NULL ||
        hdc->connect.query == LIB_NULL || (hdc->xebec.dcb[1] & 0x20u) != 0u) return 0;
    lib_memory_set(out_info, 0, sizeof(*out_info));
    *out_result = hdc->connect.query(hdc->connect.context, 0u, out_info);
    if (*out_result != X86_HDC_RECORD_OK || !out_info->present ||
        !out_info->geometry_known) return 0;
    expected = &hdc->config.xebec_geometry;
    return out_info->geometry.logical_sector_count == expected->logical_sector_count &&
        out_info->geometry.bytes_per_sector == expected->bytes_per_sector &&
        out_info->geometry.cylinders == expected->cylinders &&
        out_info->geometry.heads == expected->heads &&
        out_info->geometry.sectors_per_track == expected->sectors_per_track;
}

static lib_i32 x86_xebec_sector(const x86_hdc *hdc,
    lib_u64 *out_sector)
{
    const x86_hdc_geometry *geometry;
    lib_u16 cylinder;
    lib_u8 head;
    lib_u8 sector;

    if (hdc == LIB_NULL || out_sector == LIB_NULL) return 0;
    geometry = &hdc->config.xebec_geometry;
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

static lib_i32 x86_xebec_transfer_sector(x86_hdc *hdc,
    lib_bool write_to_media)
{
    x86_hdc_medium info;
    x86_hdc_record_result result;
    lib_u64 sector;

    if (!x86_xebec_media_info(hdc, &info, &result) ||
        !x86_xebec_sector(hdc, &sector) ||
        info.geometry.bytes_per_sector != sizeof(hdc->data.data) ||
        sector >= info.geometry.logical_sector_count || (write_to_media && info.read_only))
        return 0;
    if (write_to_media) {
        result = hdc->connect.write == LIB_NULL ? X86_HDC_RECORD_FAILURE :
            hdc->connect.write(hdc->connect.context, 0u, sector, hdc->data.data);
    } else {
        result = hdc->connect.read == LIB_NULL ? X86_HDC_RECORD_FAILURE :
            hdc->connect.read(hdc->connect.context, 0u, sector, hdc->data.data);
    }
    return result == X86_HDC_RECORD_OK;
}

static lib_i32 x86_xebec_can_transfer(const x86_hdc *hdc,
    lib_u8 write_to_media)
{
    x86_hdc_medium info;
    x86_hdc_record_result result;
    lib_u64 sector;

    return x86_xebec_media_info(hdc, &info, &result) &&
        x86_xebec_sector(hdc, &sector) &&
        (!write_to_media ||
            !info.read_only);
}

static lib_i32 x86_xebec_next_sector(x86_hdc *hdc)
{
    const x86_hdc_geometry *geometry;
    lib_u16 cylinder;
    lib_u8 head;
    lib_u8 sector;

    if (hdc == LIB_NULL) return 0;
    geometry = &hdc->config.xebec_geometry;
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

static void x86_xebec_start_transfer(x86_hdc *hdc)
{
    lib_u8 sense[4] = {0x04u, 0u, 0u, 0u};

    if (hdc == LIB_NULL) return;
    sense[1] = hdc->xebec.dcb[1] & 0x20u;
    sense[2] = hdc->xebec.dcb[2];
    sense[3] = hdc->xebec.dcb[3];
    hdc->xebec.byte_index = 0u;
    if (hdc->xebec.dcb[0] == 0x08u) {
        if (hdc->xebec.dcb[4] == 0u || !x86_xebec_transfer_sector(hdc, LIB_FALSE)) {
            x86_xebec_response(hdc, 0x02u, sense);
            return;
        }
        hdc->xebec.sectors_remaining = hdc->xebec.dcb[4];
        hdc->xebec.phase = X86_XEBEC_PHASE_DMA_READ;
    } else {
        if (hdc->xebec.dcb[4] == 0u || !x86_xebec_can_transfer(hdc, LIB_TRUE)) {
            /* A Write Data block count is documented, but zero's meaning is
             * not; reject it rather than inventing an implicit 256-sector form. */
            x86_xebec_response(hdc, 0x02u, sense);
            return;
        }
        hdc->xebec.sectors_remaining = hdc->xebec.dcb[4];
        hdc->xebec.phase = X86_XEBEC_PHASE_DMA_WRITE;
    }
    x86_xebec_sync_dma_request(hdc);
}

static lib_i32 x86_xebec_command_is_defined(lib_u8 command)
{
    return (command <= 0x01u || (command >= 0x03u && command <= 0x08u) ||
        (command >= 0x0au && command <= 0x0fu) || command == 0xe0u ||
        (command >= 0xe3u && command <= 0xe6u));
}

static void x86_xebec_complete_dcb(x86_hdc *hdc)
{
    lib_u8 sense[4] = {0u, 0u, 0u, 0u};

    if (hdc == LIB_NULL) return;
    if (hdc->xebec.dcb[0] == 0x03u) {
        lib_memory_copy(hdc->xebec.response, hdc->xebec.last_sense,
            sizeof(hdc->xebec.last_sense));
        hdc->xebec.response_count = sizeof(hdc->xebec.last_sense);
        hdc->xebec.response_index = 0u;
        hdc->xebec.phase = X86_XEBEC_PHASE_RESPONSE;
        return;
    }
    if (!x86_xebec_command_is_defined(hdc->xebec.dcb[0])) {
        sense[0] = 0x20u;
        x86_xebec_response(hdc, 0x02u, sense);
        return;
    }
    if (hdc->xebec.dcb[0] == 0x08u || hdc->xebec.dcb[0] == 0x0au) {
        x86_xebec_start_transfer(hdc);
        return;
    }
    /* Other defined DCBs retain the existing drive-not-ready completion;
     * REQUEST SENSE and READ/WRITE are handled above. */
    sense[0] = 0x04u;
    sense[1] = (lib_u8)((hdc->xebec.dcb[1] >> 5u) & 1u) << 5u;
    sense[2] = hdc->xebec.dcb[2];
    sense[3] = hdc->xebec.dcb[3];
    x86_xebec_response(hdc, 0x02u, sense);
}

static void x86_xebec_schedule_dcb_completion(x86_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    hdc->xebec.phase = X86_XEBEC_PHASE_PENDING_COMMAND;
    x86_hdc_schedule_service(hdc, hdc->config.command_ticks);
}

static lib_status x86_xebec_port_read(x86_hdc *hdc,
    x86_hdc_register reg, lib_u32 *out_value)
{
    if (hdc == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (reg == X86_HDC_REGISTER_DATA) {
        if (hdc->xebec.phase != X86_XEBEC_PHASE_RESPONSE ||
            hdc->xebec.response_index >= hdc->xebec.response_count) return LIB_STATUS_OK;
        *out_value = hdc->xebec.response[hdc->xebec.response_index++];
        x86_hdc_clear_irq(hdc);
        if (hdc->xebec.response_index == hdc->xebec.response_count) {
            if (hdc->xebec.dcb[0] == 0x03u)
                lib_memory_set(hdc->xebec.last_sense, 0, sizeof(hdc->xebec.last_sense));
            hdc->xebec.phase = X86_XEBEC_PHASE_IDLE;
            hdc->xebec.dcb_count = 0u;
        }
        return LIB_STATUS_OK;
    }
    return LIB_STATUS_UNSUPPORTED;
}

static lib_status x86_xebec_port_write(x86_hdc *hdc,
    x86_hdc_register reg, lib_u32 value)
{
    if (hdc == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (reg == X86_HDC_REGISTER_XEBEC_RESET) {
        x86_xebec_reset(hdc);
        return LIB_STATUS_OK;
    }
    if (reg == X86_HDC_REGISTER_XEBEC_MASK) {
        hdc->xebec.mask_pattern = (lib_u8)value;
        x86_xebec_sync_dma_request(hdc);
        return LIB_STATUS_OK;
    }
    if (reg == X86_HDC_REGISTER_XEBEC_SELECT) {
        hdc->xebec.dcb_count = 0u;
        hdc->xebec.initialize_count = 0u;
        hdc->xebec.phase = X86_XEBEC_PHASE_DCB;
        return LIB_STATUS_OK;
    }
    if (reg != X86_HDC_REGISTER_DATA ||
        hdc->xebec.phase == X86_XEBEC_PHASE_IDLE ||
        hdc->xebec.phase == X86_XEBEC_PHASE_RESPONSE) return LIB_STATUS_OK;
    if (hdc->xebec.phase == X86_XEBEC_PHASE_DCB) {
        hdc->xebec.dcb[hdc->xebec.dcb_count++] = (lib_u8)value;
        if (hdc->xebec.dcb_count == sizeof(hdc->xebec.dcb)) {
            if (hdc->xebec.dcb[0] == 0x0cu) hdc->xebec.phase = X86_XEBEC_PHASE_INITIALIZE;
            else x86_xebec_schedule_dcb_completion(hdc);
        }
    } else if (hdc->xebec.phase == X86_XEBEC_PHASE_INITIALIZE) {
        hdc->xebec.initialize[hdc->xebec.initialize_count++] = (lib_u8)value;
        if (hdc->xebec.initialize_count == sizeof(hdc->xebec.initialize))
            x86_xebec_schedule_dcb_completion(hdc);
    }
    return LIB_STATUS_OK;
}

lib_status x86_hdc_read(x86_hdc *hdc, x86_hdc_register reg,
    lib_u32 *out_value)
{
    lib_u16 word;

    if (hdc == LIB_NULL || out_value == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_value = 0u;
    if (x86_hdc_is_xebec_xt(hdc)) return x86_xebec_port_read(hdc, reg, out_value);
    if (reg == X86_HDC_REGISTER_DATA) {
        if (hdc->data.phase != X86_HDC_PHASE_DATA_READ ||
            hdc->data.data_index >= sizeof(hdc->data.data)) {
            return LIB_STATUS_OK;
        }
        lib_memory_copy(&word, &hdc->data.data[hdc->data.data_index], sizeof(word));
        *out_value = word;
        hdc->data.data_index = (lib_u16)(hdc->data.data_index + sizeof(word));
        if (hdc->data.data_index == sizeof(hdc->data.data)) {
            hdc->data.phase = X86_HDC_PHASE_PENDING_READ_SECTOR;
            hdc->data.status = X86_HDC_STATUS_BSY;
            x86_hdc_schedule_service(hdc,
                hdc->config.next_sector_ticks);
        }
        return LIB_STATUS_OK;
    }
    if (reg == X86_HDC_REGISTER_ERROR_FEATURES) {
        *out_value = hdc->data.error;
    } else if (reg == X86_HDC_REGISTER_SECTOR_COUNT) {
        *out_value = hdc->data.sector_count;
    } else if (reg == X86_HDC_REGISTER_SECTOR_NUMBER) {
        *out_value = hdc->data.sector_number;
    } else if (reg == X86_HDC_REGISTER_CYLINDER_LOW) {
        *out_value = hdc->data.cylinder_low;
    } else if (reg == X86_HDC_REGISTER_CYLINDER_HIGH) {
        *out_value = hdc->data.cylinder_high;
    } else if (reg == X86_HDC_REGISTER_DRIVE_HEAD) {
        *out_value = hdc->data.drive_head;
    } else if (reg == X86_HDC_REGISTER_STATUS_COMMAND) {
        *out_value = hdc->data.status;
        x86_hdc_clear_irq(hdc);
    } else if (reg == X86_HDC_REGISTER_CONTROL) {
        if (x86_hdc_is_ibm_wd1003(hdc)) return LIB_STATUS_UNSUPPORTED;
        *out_value = hdc->data.status;
    } else if (reg == X86_HDC_REGISTER_DRIVE_ADDRESS &&
        x86_hdc_is_compaq_wd_40mb(hdc)) {
        *out_value = hdc->data.drive_head & 0x1fu;
    } else {
        return LIB_STATUS_UNSUPPORTED;
    }
    return LIB_STATUS_OK;
}

lib_status x86_hdc_write(x86_hdc *hdc, x86_hdc_register reg,
    lib_u32 value)
{

    if (hdc == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (x86_hdc_is_xebec_xt(hdc)) return x86_xebec_port_write(hdc, reg, value);
    if (reg == X86_HDC_REGISTER_DATA) {
        lib_u16 word = (lib_u16)value;
        if (hdc->data.phase != X86_HDC_PHASE_DATA_WRITE ||
            hdc->data.data_index >= sizeof(hdc->data.data)) {
            return LIB_STATUS_OK;
        }
        lib_memory_copy(&hdc->data.data[hdc->data.data_index], &word, sizeof(word));
        hdc->data.data_index = (lib_u16)(hdc->data.data_index + sizeof(word));
        if (hdc->data.data_index == sizeof(hdc->data.data)) {
            hdc->data.phase = X86_HDC_PHASE_PENDING_WRITE_SECTOR;
            hdc->data.status = X86_HDC_STATUS_BSY;
            x86_hdc_schedule_service(hdc,
                hdc->config.next_sector_ticks);
        }
        return LIB_STATUS_OK;
    }
    if (!x86_hdc_task_file_is_writable(hdc) &&
        (reg == X86_HDC_REGISTER_ERROR_FEATURES ||
            reg == X86_HDC_REGISTER_SECTOR_COUNT ||
            reg == X86_HDC_REGISTER_SECTOR_NUMBER ||
            reg == X86_HDC_REGISTER_CYLINDER_LOW ||
            reg == X86_HDC_REGISTER_CYLINDER_HIGH ||
            reg == X86_HDC_REGISTER_DRIVE_HEAD)) return LIB_STATUS_OK;
    if (reg == X86_HDC_REGISTER_ERROR_FEATURES) {
        hdc->data.features = (lib_u8)value;
    } else if (reg == X86_HDC_REGISTER_SECTOR_COUNT) {
        hdc->data.sector_count = (lib_u8)value;
    } else if (reg == X86_HDC_REGISTER_SECTOR_NUMBER) {
        hdc->data.sector_number = (lib_u8)value;
    } else if (reg == X86_HDC_REGISTER_CYLINDER_LOW) {
        hdc->data.cylinder_low = (lib_u8)value;
    } else if (reg == X86_HDC_REGISTER_CYLINDER_HIGH) {
        hdc->data.cylinder_high = (lib_u8)value;
    } else if (reg == X86_HDC_REGISTER_DRIVE_HEAD) {
        hdc->data.drive_head = (lib_u8)value;
        x86_hdc_refresh_compaq_selection_status(hdc);
    } else if (reg == X86_HDC_REGISTER_STATUS_COMMAND) {
        x86_hdc_capture_command(hdc, (lib_u8)value);
    } else if (reg == X86_HDC_REGISTER_CONTROL &&
        x86_hdc_is_ibm_wd1003(hdc)) {
        hdc->data.fixed_disk_register = (lib_u8)value & 0x08u;
    } else if (reg == X86_HDC_REGISTER_CONTROL) {
        lib_u8 device_control = (lib_u8)value;
        lib_u8 reset_asserted = (device_control &
            X86_HDC_DEVICE_CONTROL_SRST) != 0u;

        hdc->data.device_control = device_control;
        x86_hdc_sync_irq(hdc);
        if (reset_asserted && !hdc->data.reset_asserted) {
            hdc->data.reset_asserted = LIB_TRUE;
            hdc->data.phase = X86_HDC_PHASE_IDLE;
            hdc->data.data_index = 0u;
            hdc->data.sectors_remaining = 0u;
            hdc->data.error = 0u;
            hdc->data.status = X86_HDC_STATUS_BSY;
            x86_hdc_clear_irq(hdc);
        } else if (!reset_asserted && hdc->data.reset_asserted) {
            x86_hdc_reset_controller(hdc);
            hdc->data.device_control = device_control;
        }
    } else {
        return LIB_STATUS_UNSUPPORTED;
    }
    return LIB_STATUS_OK;
}



void x86_hdc_dma_read(x86_hdc *hdc, lib_u8 *io_byte)
{
    lib_u8 sense[4] = {0x04u, 0u, 0u, 0u};

    if (hdc == LIB_NULL || io_byte == LIB_NULL ||
        hdc->xebec.phase != X86_XEBEC_PHASE_DMA_READ ||
        hdc->xebec.byte_index >= sizeof(hdc->data.data)) return;
    *io_byte = hdc->data.data[hdc->xebec.byte_index++];
    if (hdc->xebec.byte_index == sizeof(hdc->data.data)) {
        if (--hdc->xebec.sectors_remaining == 0u) {
            x86_xebec_release_dma(hdc);
            x86_xebec_response(hdc, 0u, LIB_NULL);
        } else if (!x86_xebec_next_sector(hdc) ||
            !x86_xebec_transfer_sector(hdc, LIB_FALSE)) {
            sense[1] = hdc->xebec.dcb[1] & 0x20u;
            sense[2] = hdc->xebec.dcb[2];
            sense[3] = hdc->xebec.dcb[3];
            x86_xebec_release_dma(hdc);
            x86_xebec_response(hdc, 0x02u, sense);
        } else hdc->xebec.byte_index = 0u;
    }
}

void x86_hdc_dma_write(x86_hdc *hdc, lib_u8 byte)
{
    lib_u8 sense[4] = {0x04u, 0u, 0u, 0u};

    if (hdc == LIB_NULL ||
        hdc->xebec.phase != X86_XEBEC_PHASE_DMA_WRITE ||
        hdc->xebec.byte_index >= sizeof(hdc->data.data)) return;
    hdc->data.data[hdc->xebec.byte_index++] = byte;
    if (hdc->xebec.byte_index != sizeof(hdc->data.data)) return;
    sense[1] = hdc->xebec.dcb[1] & 0x20u;
    sense[2] = hdc->xebec.dcb[2];
    sense[3] = hdc->xebec.dcb[3];
    if (!x86_xebec_transfer_sector(hdc, LIB_TRUE) ||
        hdc->xebec.sectors_remaining == 0u) {
        x86_xebec_release_dma(hdc);
        x86_xebec_response(hdc, 0x02u, sense);
        return;
    }
    if (--hdc->xebec.sectors_remaining == 0u) {
        x86_xebec_release_dma(hdc);
        x86_xebec_response(hdc, 0u, LIB_NULL);
        return;
    }
    if (!x86_xebec_next_sector(hdc) ||
        !x86_xebec_can_transfer(hdc, LIB_TRUE)) {
        x86_xebec_release_dma(hdc);
        x86_xebec_response(hdc, 0x02u, sense);
        return;
    }
    hdc->xebec.byte_index = 0u;
}

void x86_hdc_terminal_count(x86_hdc *hdc)
{
    lib_u8 sense[4] = {0x04u, 0u, 0u, 0u};

    if (hdc == LIB_NULL || (hdc->xebec.phase != X86_XEBEC_PHASE_DMA_READ &&
        hdc->xebec.phase != X86_XEBEC_PHASE_DMA_WRITE)) return;
    sense[1] = hdc->xebec.dcb[1] & 0x20u;
    sense[2] = hdc->xebec.dcb[2];
    sense[3] = hdc->xebec.dcb[3];
    x86_xebec_release_dma(hdc);
    x86_xebec_response(hdc, 0x02u, sense);
}

lib_status x86_hdc_create(const x86_hdc_config *config,
    const x86_hdc_connection *connection, x86_hdc **out_hdc)
{
    x86_hdc *hdc;

    if (out_hdc == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_hdc = LIB_NULL;
    if (config == LIB_NULL || connection == LIB_NULL ||
        (lib_u32)config->protocol > X86_HDC_PROTOCOL_XEBEC_XT)
        return LIB_STATUS_INVALID_ARGUMENT;
    hdc = lib_allocate_zero(1u, sizeof(*hdc));
    if (hdc == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    hdc->config = *config;
    hdc->connect = *connection;
    x86_hdc_reset(hdc);
    *out_hdc = hdc;
    return LIB_STATUS_OK;
}

static void x86_hdc_reset_controller(x86_hdc *hdc)
{
    lib_u64 now;

    if (hdc == LIB_NULL) return;
    now = hdc->data.elapsed_ticks;
    lib_memory_set(&hdc->data, 0, sizeof(hdc->data));
    hdc->data.elapsed_ticks = now;
    if (x86_hdc_is_xebec_xt(hdc)) x86_xebec_reset(hdc);
    x86_hdc_clear_irq(hdc);
    hdc->data.error = X86_HDC_ERROR_DIAGNOSTIC_OK;
    hdc->data.sector_count = 1u;
    hdc->data.sector_number = 1u;
    hdc->data.status = X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC;
}

void x86_hdc_reset(x86_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    hdc->data.elapsed_ticks = 0u;
    x86_hdc_reset_controller(hdc);
}

void x86_hdc_advance_at(x86_hdc *hdc, lib_u64 now)
{
    if (hdc == LIB_NULL || now < hdc->data.elapsed_ticks) return;
    hdc->data.elapsed_ticks = now;
    if (hdc->data.next_service_tick != 0u &&
        hdc->data.elapsed_ticks < hdc->data.next_service_tick) return;
    hdc->data.next_service_tick = 0u;
    if (x86_hdc_is_xebec_xt(hdc) &&
        hdc->xebec.phase == X86_XEBEC_PHASE_PENDING_COMMAND) {
        x86_xebec_complete_dcb(hdc);
    } else if (hdc->data.phase == X86_HDC_PHASE_PENDING_COMMAND) {
        hdc->data.features = hdc->data.pending_features;
        hdc->data.sector_count = hdc->data.pending_sector_count;
        hdc->data.sector_number = hdc->data.pending_sector_number;
        hdc->data.cylinder_low = hdc->data.pending_cylinder_low;
        hdc->data.cylinder_high = hdc->data.pending_cylinder_high;
        hdc->data.drive_head = hdc->data.pending_drive_head;
        x86_hdc_execute_command(hdc, hdc->data.pending_command);
    } else if (hdc->data.phase == X86_HDC_PHASE_PENDING_READ_SECTOR) {
        x86_hdc_next_read_sector(hdc);
    } else if (hdc->data.phase == X86_HDC_PHASE_PENDING_WRITE_SECTOR) {
        x86_hdc_next_write_sector(hdc);
    }
}



lib_status x86_hdc_next_due_tick(const x86_hdc *hdc,
    lib_u64 *out_due_tick)
{
    if (hdc == LIB_NULL || out_due_tick == LIB_NULL) return LIB_STATUS_INVALID_STATE;
    if (x86_hdc_is_xebec_xt(hdc)) {
        if (hdc->xebec.phase != X86_XEBEC_PHASE_PENDING_COMMAND)
            return LIB_STATUS_INVALID_STATE;
    } else if (hdc->data.phase != X86_HDC_PHASE_PENDING_COMMAND &&
        hdc->data.phase != X86_HDC_PHASE_PENDING_READ_SECTOR &&
        hdc->data.phase != X86_HDC_PHASE_PENDING_WRITE_SECTOR) {
        return LIB_STATUS_INVALID_STATE;
    }
    *out_due_tick = hdc->data.next_service_tick;
    return LIB_STATUS_OK;
}

void x86_hdc_destroy(x86_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    if (hdc->connect.irq != LIB_NULL) hdc->connect.irq(hdc->connect.context, LIB_FALSE);
    if (hdc->connect.drq != LIB_NULL) hdc->connect.drq(hdc->connect.context, LIB_FALSE);
    lib_release(hdc);
}

lib_bool x86_hdc_irq_pending(const x86_hdc *hdc)
{
    return hdc != LIB_NULL && hdc->data.irq_pending &&
        (hdc->data.device_control & X86_HDC_DEVICE_CONTROL_NIEN) == 0u;
}


lib_status x86_hdc_capture(const x86_hdc *hdc, x86_hdc_observation *out)
{
    if (hdc == LIB_NULL || out == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out = (x86_hdc_observation){
        .elapsed_ticks = hdc->data.elapsed_ticks,
        .next_service_tick = hdc->data.next_service_tick,
        .command_count = hdc->data.command_count,
        .step_rate_ticks = hdc->data.step_rate_ticks,
        .data_index = hdc->data.data_index,
        .sectors_remaining = hdc->data.sectors_remaining,
        .step_pulse_limit = hdc->data.step_pulse_limit,
        .phase = (lib_u8)hdc->data.phase, .status = hdc->data.status,
        .error = hdc->data.error, .last_command = hdc->data.last_command,
        .sector_count = hdc->data.sector_count, .sector_number = hdc->data.sector_number,
        .cylinder_low = hdc->data.cylinder_low, .cylinder_high = hdc->data.cylinder_high,
        .drive_head = hdc->data.drive_head, .fixed_disk_register = hdc->data.fixed_disk_register,
        .step_rate_selector = hdc->data.step_rate_selector, .irq_pending = hdc->data.irq_pending,
        .xebec_phase = (lib_u8)hdc->xebec.phase, .xebec_dcb_count = hdc->xebec.dcb_count,
        .xebec_mask = hdc->xebec.mask_pattern
    };
    lib_memory_copy(out->xebec_initialize, hdc->xebec.initialize, sizeof(out->xebec_initialize));
    return LIB_STATUS_OK;
}
