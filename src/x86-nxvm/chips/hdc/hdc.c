#include "x86-nxvm/chips/hdc/hdc.h"


#define X86_HDC_COMMAND_READ_SECTORS 0x20u
#define X86_HDC_COMMAND_WRITE_SECTORS 0x30u
#define X86_HDC_COMMAND_VERIFY_SECTORS 0x40u
#define X86_HDC_COMMAND_FORMAT_TRACK 0x50u
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
#define X86_HDC_ECC_CORRECTED 0x04u
#define X86_HDC_ECC_UNCORRECTABLE 0x40u
static void x86_hdc_reset_controller(x86_hdc *hdc);

/* Reviewed L2 record encoding, not a claim about physical disk wire order. */
static lib_u32 x86_hdc_ecc_polynomial(const x86_hdc *hdc)
{
    return hdc->config.protocol == X86_HDC_PROTOCOL_IBM_WD1003_ST506 ?
        0x140a0445u : 0x00a00805u;
}

static lib_u32 x86_hdc_ecc_remainder(const x86_hdc_record *record,
    lib_u32 polynomial, lib_bool include_ecc)
{
    lib_u32 remainder = 0u;
    for (lib_size byte = 0u; byte < sizeof(record->data) + sizeof(record->ecc); ++byte) {
        const lib_u8 value = byte < sizeof(record->data) ? record->data[byte] :
            include_ecc ? record->ecc[byte - sizeof(record->data)] : 0u;
        for (lib_u32 bit = 0u; bit < 8u; ++bit) {
            const lib_bool carry = (remainder >> 31u) != 0u;
            remainder = (remainder << 1u) | ((value >> (7u - bit)) & 1u);
            if (carry) remainder ^= polynomial;
        }
    }
    return remainder;
}

static void x86_hdc_ecc_encode(x86_hdc_record *record, lib_u32 polynomial)
{
    const lib_u32 remainder = x86_hdc_ecc_remainder(record, polynomial, LIB_FALSE);
    for (lib_u32 byte = 0u; byte < sizeof(record->ecc); ++byte)
        record->ecc[byte] = (lib_u8)(remainder >> (24u - byte * 8u));
    record->ecc_present = LIB_TRUE;
}

static lib_bool x86_hdc_ecc_check(x86_hdc_record *record, lib_u32 polynomial,
    lib_u8 limit, lib_u8 *out_burst)
{
    const lib_u32 bits = (lib_u32)(sizeof(record->data) + sizeof(record->ecc)) * 8u;
    lib_u32 syndrome, candidate_mask = 0u, candidate_position = 0u;
    lib_u8 candidate_span = 0u;
    x86_hdc_record candidate;

    *out_burst = 0u;
    if (!record->ecc_present) {
        /* Raw payloads denote healthy logical records, not stored disk ECC. */
        x86_hdc_ecc_encode(record, polynomial);
        return LIB_TRUE;
    }
    syndrome = x86_hdc_ecc_remainder(record, polynomial, LIB_TRUE);
    if (syndrome == 0u) return LIB_TRUE;
    if (limit == 0u) return LIB_FALSE;
    for (lib_u32 position = 0u; position < bits; ++position) {
        if ((syndrome & 1u) != 0u && syndrome < (1u << limit)) {
            lib_u8 span = 0u;
            for (lib_u32 value = syndrome; value != 0u; value >>= 1u) ++span;
            if (position + span <= bits) {
                if (candidate_mask != 0u) return LIB_FALSE;
                candidate_mask = syndrome;
                candidate_position = position;
                candidate_span = span;
            }
        }
        syndrome = (syndrome >> 1u) ^ ((syndrome & 1u) != 0u ?
            (0x80000000u | (polynomial >> 1u)) : 0u);
    }
    if (candidate_mask == 0u) return LIB_FALSE;
    candidate = *record;
    for (lib_u32 bit = 0u; bit < candidate_span; ++bit) {
        if ((candidate_mask & (1u << bit)) != 0u) {
            const lib_u32 position = candidate_position + bit;
            const lib_u32 byte = bits / 8u - 1u - position / 8u;
            lib_u8 *value = byte < sizeof(candidate.data) ? &candidate.data[byte] :
                &candidate.ecc[byte - sizeof(candidate.data)];
            *value ^= (lib_u8)(1u << (position & 7u));
        }
    }
    if (x86_hdc_ecc_remainder(&candidate, polynomial, LIB_TRUE) != 0u)
        return LIB_FALSE;
    *record = candidate;
    *out_burst = candidate_span;
    return LIB_TRUE;
}
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

static lib_i32 x86_hdc_is_ata_pio(const x86_hdc *hdc)
{
    return hdc != LIB_NULL && hdc->config.protocol ==
        X86_HDC_PROTOCOL_ATA_PIO;
}

static lib_i32 x86_hdc_task_file_is_writable(const x86_hdc *hdc)
{
    return hdc != LIB_NULL && (hdc->data.status &
        (X86_HDC_STATUS_BSY | X86_HDC_STATUS_DRQ)) == 0u;
}

static lib_i32 x86_hdc_selected_master(const x86_hdc *hdc)
{
    return hdc != LIB_NULL && (hdc->data.drive_head & 0x10u) == 0u;
}

static lib_u8 x86_hdc_current_head(const x86_hdc *hdc)
{
    if (hdc == LIB_NULL) return 0u;
    if (x86_hdc_is_ibm_wd1003(hdc) || x86_hdc_is_compaq_wd_40mb(hdc)) {
        return hdc->data.drive_head &
            ((hdc->data.device_control & 0x08u) != 0u ? 0x0fu : 0x07u);
    }
    return hdc->data.drive_head & 0x0fu;
}

static void x86_hdc_set_current_head(x86_hdc *hdc,
    lib_u8 head)
{
    if (hdc == LIB_NULL) return;
    hdc->data.drive_head = (lib_u8)((hdc->data.drive_head & 0xf0u) | head);
}

static lib_i32 x86_hdc_lba_mode(const x86_hdc *hdc)
{
    return hdc != LIB_NULL && !x86_hdc_is_compaq_wd_40mb(hdc) &&
        !x86_hdc_is_ibm_wd1003(hdc) &&
        hdc->config.lba28_supported &&
        (hdc->data.drive_head & 0x40u) != 0u;
}

static lib_i32 x86_hdc_command_is_read(const x86_hdc *hdc,
    lib_u8 command)
{
    if (x86_hdc_is_compaq_wd_40mb(hdc)) {
        return (command & 0xfcu) == X86_HDC_COMMAND_READ_SECTORS;
    }
    if (x86_hdc_is_ibm_wd1003(hdc)) {
        return (command & 0xfcu) == X86_HDC_COMMAND_READ_SECTORS;
    }
    return (command & 0xfeu) == X86_HDC_COMMAND_READ_SECTORS;
}

static lib_i32 x86_hdc_command_is_write(const x86_hdc *hdc,
    lib_u8 command)
{
    if (x86_hdc_is_compaq_wd_40mb(hdc)) {
        return (command & 0xfcu) == X86_HDC_COMMAND_WRITE_SECTORS;
    }
    if (x86_hdc_is_ibm_wd1003(hdc)) {
        return (command & 0xfcu) == X86_HDC_COMMAND_WRITE_SECTORS;
    }
    return (command & 0xfeu) == X86_HDC_COMMAND_WRITE_SECTORS;
}

static lib_bool x86_hdc_command_is_long(const x86_hdc *hdc, lib_u8 command)
{
    return (x86_hdc_is_compaq_wd_40mb(hdc) || x86_hdc_is_ibm_wd1003(hdc)) &&
        ((command & 0xfeu) == 0x22u || (command & 0xfeu) == 0x32u);
}

static lib_u16 x86_hdc_transfer_size(const x86_hdc *hdc)
{
    const lib_bool long_transfer = x86_hdc_is_xebec_xt(hdc) ?
        (hdc->xebec.dcb[0] == 0xe5u || hdc->xebec.dcb[0] == 0xe6u) :
        x86_hdc_command_is_long(hdc, hdc->data.last_command);
    return (lib_u16)(sizeof(hdc->data.record.data) +
        (long_transfer ? sizeof(hdc->data.record.ecc) : 0u));
}

static void x86_hdc_select_wd_step_rate(x86_hdc *hdc,
    lib_u8 selector, lib_u16 pulse_limit)
{
    if (!x86_hdc_is_ibm_wd1003(hdc) && !x86_hdc_is_compaq_wd_40mb(hdc)) return;
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

static x86_hdc_geometry x86_hdc_address_geometry(const x86_hdc *hdc,
    const x86_hdc_medium *info, lib_u8 unit)
{
    x86_hdc_geometry geometry = info->geometry;
    if (hdc->data.drive_heads[unit] != 0u) {
        geometry.heads = hdc->data.drive_heads[unit];
        geometry.sectors_per_track = hdc->data.drive_sectors_per_track[unit];
        if (hdc->config.protocol == X86_HDC_PROTOCOL_ATA_PIO &&
            geometry.sectors_per_track != 0u) {
            const lib_u64 cylinders = info->geometry.logical_sector_count /
                ((lib_u64)geometry.heads * geometry.sectors_per_track);
            geometry.cylinders = (lib_u32)(cylinders > 65535u ? 65535u : cylinders);
        }
    }
    return geometry;
}

static void x86_hdc_complete(x86_hdc *hdc, lib_bool interrupt)
{
    if (hdc == LIB_NULL) return;
    hdc->data.phase = X86_HDC_PHASE_IDLE;
    hdc->data.next_service_tick = 0u;
    hdc->data.data_index = 0u;
    hdc->data.status = X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC |
        (hdc->data.status & X86_HDC_ECC_CORRECTED);
    if (interrupt) x86_hdc_raise_irq(hdc);
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
    x86_hdc_geometry geometry;

    if (hdc == LIB_NULL || out_sector == LIB_NULL ||
        !x86_hdc_media_info(hdc, &info, &media_result) || !info.present ||
        (write_to_media && info.read_only)) {
        x86_hdc_fail(hdc, X86_HDC_ERROR_ABORT);
        return 0;
    }
    if (x86_hdc_lba_mode(hdc)) {
        if (hdc->data.drive_heads[x86_hdc_selected_unit(hdc)] != 0u &&
            hdc->data.drive_sectors_per_track[x86_hdc_selected_unit(hdc)] == 0u) {
            x86_hdc_fail(hdc, X86_HDC_ERROR_ID_NOT_FOUND);
            return 0;
        }
        lba = x86_hdc_lba(hdc);
        if ((!x86_hdc_selected_master(hdc) && !x86_hdc_is_ata_pio(hdc)) ||
            (lib_u64)lba >= x86_hdc_sector_capacity(&info) ||
            info.geometry.bytes_per_sector != sizeof(hdc->data.record.data)) {
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
    geometry = x86_hdc_address_geometry(hdc, &info, x86_hdc_selected_unit(hdc));
    if ((!x86_hdc_is_ata_pio(hdc) && !x86_hdc_is_compaq_wd_40mb(hdc) && !x86_hdc_is_ibm_wd1003(hdc) &&
            !x86_hdc_selected_master(hdc)) ||
        sector == 0u || cylinder >= geometry.cylinders ||
        head >= geometry.heads || sector > geometry.sectors_per_track ||
        info.geometry.bytes_per_sector != sizeof(hdc->data.record.data)) {
        x86_hdc_fail(hdc, X86_HDC_ERROR_ID_NOT_FOUND);
        return 0;
    }
    *out_sector = ((lib_u64)cylinder * geometry.heads + head) *
        geometry.sectors_per_track + (sector - 1u);
    if (*out_sector >= x86_hdc_sector_capacity(&info)) {
        x86_hdc_fail(hdc, X86_HDC_ERROR_ID_NOT_FOUND);
        return 0;
    }
    return 1;
}

static lib_i32 x86_hdc_load_sector(x86_hdc *hdc)
{
    x86_hdc_record_result media_result;
    x86_hdc_record record = {0};
    lib_u64 sector;

    if (!x86_hdc_resolve_sector(hdc, LIB_FALSE, &sector)) return 0;
    media_result = hdc->connect.read == LIB_NULL ? X86_HDC_RECORD_FAILURE :
        hdc->connect.read(hdc->connect.context, x86_hdc_selected_unit(hdc),
            sector, &record);
    if (media_result != X86_HDC_RECORD_OK) {
        x86_hdc_fail(hdc, media_result == X86_HDC_RECORD_BAD_TRACK ? 0x80u :
            media_result == X86_HDC_RECORD_RANGE ?
            X86_HDC_ERROR_ID_NOT_FOUND : X86_HDC_ERROR_ABORT);
        return 0;
    }
    if (hdc->config.protocol != X86_HDC_PROTOCOL_ATA_PIO) {
        const lib_u32 polynomial = x86_hdc_ecc_polynomial(hdc);
        lib_u8 burst;
        if (x86_hdc_command_is_long(hdc, hdc->data.last_command)) {
            if (!record.ecc_present) x86_hdc_ecc_encode(&record, polynomial);
        } else if (!x86_hdc_ecc_check(&record, polynomial,
                (hdc->data.drive_head & 0x80u) == 0u ? 0u :
                x86_hdc_is_ibm_wd1003(hdc) ? 5u : 11u, &burst)) {
            if (!x86_hdc_is_compaq_wd_40mb(hdc) ||
                (hdc->data.last_command != 0x20u && hdc->data.last_command != 0x21u)) {
                x86_hdc_fail(hdc, X86_HDC_ECC_UNCORRECTABLE);
                return 0;
            }
            /* This selected Compaq read delivers flawed payload before stopping. */
            hdc->data.error = X86_HDC_ECC_UNCORRECTABLE;
            hdc->data.status |= X86_HDC_STATUS_ERR;
        } else if (burst != 0u) {
            hdc->data.status |= X86_HDC_ECC_CORRECTED;
        }
    }
    hdc->data.record = record;
    return 1;
}

static lib_i32 x86_hdc_store_sector(x86_hdc *hdc)
{
    x86_hdc_record_result media_result;
    x86_hdc_record record = hdc->data.record;
    lib_u64 sector;

    if (!x86_hdc_resolve_sector(hdc, LIB_TRUE, &sector)) return 0;
    if (!x86_hdc_command_is_long(hdc, hdc->data.last_command)) {
        if (hdc->config.protocol == X86_HDC_PROTOCOL_ATA_PIO) record.ecc_present = LIB_FALSE;
        else x86_hdc_ecc_encode(&record, x86_hdc_ecc_polynomial(hdc));
    }
    media_result = hdc->connect.write == LIB_NULL ? X86_HDC_RECORD_FAILURE :
        hdc->connect.write(hdc->connect.context, x86_hdc_selected_unit(hdc),
            sector, &record);
    if (media_result != X86_HDC_RECORD_OK) {
        x86_hdc_fail(hdc, media_result == X86_HDC_RECORD_BAD_TRACK ? 0x80u :
            media_result == X86_HDC_RECORD_RANGE ?
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
        (!x86_hdc_selected_master(hdc) && !x86_hdc_is_ata_pio(hdc)) ||
        !info.present || info.geometry.bytes_per_sector != sizeof(hdc->data.record.data)) {
        x86_hdc_fail(hdc, X86_HDC_ERROR_ABORT);
        return;
    }
    lib_memory_set(&hdc->data.record, 0, sizeof(hdc->data.record));
    word = 0x0040u;
    lib_memory_copy(&hdc->data.record.data[0], &word, sizeof(word));
    word = info.geometry.cylinders;
    lib_memory_copy(&hdc->data.record.data[2], &word, sizeof(word));
    word = info.geometry.heads;
    lib_memory_copy(&hdc->data.record.data[6], &word, sizeof(word));
    word = info.geometry.sectors_per_track;
    lib_memory_copy(&hdc->data.record.data[12], &word, sizeof(word));
    {
        const x86_hdc_geometry current = x86_hdc_address_geometry(hdc, &info,
            x86_hdc_selected_unit(hdc));
        const lib_u64 capacity = (lib_u64)current.cylinders * current.heads *
            current.sectors_per_track;
        word = current.sectors_per_track != 0u ? 1u : 0u;
        lib_memory_copy(&hdc->data.record.data[106], &word, sizeof(word));
        word = (lib_u16)current.cylinders;
        lib_memory_copy(&hdc->data.record.data[108], &word, sizeof(word));
        word = (lib_u16)current.heads;
        lib_memory_copy(&hdc->data.record.data[110], &word, sizeof(word));
        word = (lib_u16)current.sectors_per_track;
        lib_memory_copy(&hdc->data.record.data[112], &word, sizeof(word));
        word = (lib_u16)capacity;
        lib_memory_copy(&hdc->data.record.data[114], &word, sizeof(word));
        word = (lib_u16)(capacity >> 16u);
        lib_memory_copy(&hdc->data.record.data[116], &word, sizeof(word));
    }
    word = hdc->config.lba28_supported ? 0x0200u : 0u;
    lib_memory_copy(&hdc->data.record.data[98], &word, sizeof(word));
    word = (lib_u16)x86_hdc_sector_capacity(&info);
    lib_memory_copy(&hdc->data.record.data[120], &word, sizeof(word));
    word = (lib_u16)(x86_hdc_sector_capacity(&info) >> 16u);
    lib_memory_copy(&hdc->data.record.data[122], &word, sizeof(word));
    hdc->data.phase = X86_HDC_PHASE_DATA_READ;
    hdc->data.sectors_remaining = 0u;
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
    x86_hdc_geometry geometry;

    if (hdc == LIB_NULL || !x86_hdc_media_info(hdc, &info, &media_result) ||
        !info.present) {
        x86_hdc_fail(hdc, X86_HDC_ERROR_ABORT);
        return 0;
    }
    cylinder = (lib_u16)hdc->data.cylinder_low |
        ((lib_u16)hdc->data.cylinder_high << 8u);
    head = x86_hdc_current_head(hdc);
    geometry = x86_hdc_address_geometry(hdc, &info, x86_hdc_selected_unit(hdc));
    sector = (lib_u8)(hdc->data.sector_number + 1u);
    if (sector == 0u || sector > geometry.sectors_per_track) {
        sector = 1u;
        ++head;
        if (head >= geometry.heads) {
            head = 0u;
            ++cylinder;
        }
    }
    if (cylinder >= geometry.cylinders) {
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
    if (hdc->data.error == X86_HDC_ECC_UNCORRECTABLE) {
        x86_hdc_complete(hdc, LIB_FALSE);
        hdc->data.status |= X86_HDC_STATUS_ERR;
        return;
    }
    x86_hdc_complete_data_sector(hdc);
    if (hdc->data.sectors_remaining == 0u) {
        /* The read-buffer IRQ already notified this final host block. */
        x86_hdc_complete(hdc, LIB_FALSE);
        return;
    }
    if (!x86_hdc_advance_sector(hdc) || !x86_hdc_load_sector(hdc)) {
        return;
    }
    hdc->data.data_index = 0u;
    hdc->data.phase = X86_HDC_PHASE_DATA_READ;
    hdc->data.status = X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC |
        X86_HDC_STATUS_DRQ | (hdc->data.status &
            (X86_HDC_ECC_CORRECTED | X86_HDC_STATUS_ERR));
    x86_hdc_raise_irq(hdc);
}

static void x86_hdc_next_write_sector(x86_hdc *hdc)
{
    if (hdc == LIB_NULL) return;
    if (!x86_hdc_store_sector(hdc)) return;
    x86_hdc_complete_data_sector(hdc);
    if (hdc->data.sectors_remaining == 0u) {
        x86_hdc_complete(hdc, LIB_TRUE);
        return;
    }
    if (!x86_hdc_advance_sector(hdc)) return;
    hdc->data.data_index = 0u;
    lib_memory_set(&hdc->data.record, 0, sizeof(hdc->data.record));
    hdc->data.phase = X86_HDC_PHASE_DATA_WRITE;
    hdc->data.status = X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC |
        X86_HDC_STATUS_DRQ;
    x86_hdc_raise_irq(hdc);
}

static void x86_hdc_schedule_service(x86_hdc *hdc,
    lib_u32 service_ticks)
{
    if (hdc == LIB_NULL) return;
    hdc->data.next_service_tick = service_ticks > LIB_UINT64_MAX - hdc->data.elapsed_ticks ?
        LIB_UINT64_MAX : hdc->data.elapsed_ticks + service_ticks;
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
        X86_HDC_STATUS_DRQ | (hdc->data.status &
            (X86_HDC_ECC_CORRECTED | X86_HDC_STATUS_ERR));
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
    hdc->data.record.ecc_present = LIB_FALSE;
    hdc->data.status = X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC |
        X86_HDC_STATUS_DRQ;
    /* The first write buffer is exposed through DRQ, not completion IRQ. */
}

static void x86_hdc_verify_sectors(x86_hdc *hdc)
{
    hdc->data.sectors_remaining = hdc->data.sector_count == 0u ? 256u :
        hdc->data.sector_count;
    while (hdc->data.sectors_remaining != 0u) {
        if (!x86_hdc_load_sector(hdc)) return;
        x86_hdc_complete_data_sector(hdc);
        if (hdc->data.sectors_remaining != 0u && !x86_hdc_advance_sector(hdc)) return;
    }
    x86_hdc_complete(hdc, LIB_TRUE);
}

static void x86_hdc_begin_format(x86_hdc *hdc)
{
    x86_hdc_medium info;
    x86_hdc_record_result result;

    if (!x86_hdc_media_info(hdc, &info, &result) || !info.present || info.read_only) {
        x86_hdc_fail(hdc, X86_HDC_ERROR_ABORT);
        return;
    }
    hdc->data.phase = X86_HDC_PHASE_DATA_WRITE;
    hdc->data.sectors_remaining = 0u;
    hdc->data.data_index = 0u;
    hdc->data.record.ecc_present = LIB_FALSE;
    hdc->data.status = X86_HDC_STATUS_DRDY | X86_HDC_STATUS_DSC | X86_HDC_STATUS_DRQ;
}

static void x86_hdc_format_track(x86_hdc *hdc)
{
    const lib_u8 unit = (hdc->data.pending_drive_head >> 4u) & 1u;
    const lib_u8 head = hdc->data.pending_drive_head &
        ((hdc->data.device_control & 0x08u) != 0u ? 0x0fu : 0x07u);
    const lib_u16 cylinder = (lib_u16)hdc->data.pending_cylinder_low |
        ((lib_u16)hdc->data.pending_cylinder_high << 8u);
    x86_hdc_medium info = {0};
    x86_hdc_geometry geometry;
    x86_hdc_record_result result;
    x86_hdc_track track = {.sector_count = hdc->data.pending_sector_count,
        .fill_payload = LIB_TRUE};
    lib_bool seen[256] = {0};

    result = hdc->connect.query == LIB_NULL ? X86_HDC_RECORD_ABSENT :
        hdc->connect.query(hdc->connect.context, unit, &info);
    if (result != X86_HDC_RECORD_OK || !info.present || info.read_only) {
        x86_hdc_fail(hdc, X86_HDC_ERROR_ABORT);
        return;
    }
    geometry = x86_hdc_address_geometry(hdc, &info, unit);
    if (!info.geometry_known || info.geometry.bytes_per_sector != 512u ||
        track.sector_count == 0u || track.sector_count != geometry.sectors_per_track ||
        cylinder >= geometry.cylinders || head >= geometry.heads) {
        x86_hdc_fail(hdc, X86_HDC_ERROR_ID_NOT_FOUND);
        return;
    }
    track.first_sector = ((lib_u64)cylinder * geometry.heads + head) * geometry.sectors_per_track;
    if (track.first_sector >= info.geometry.logical_sector_count ||
        track.sector_count > info.geometry.logical_sector_count - track.first_sector) {
        x86_hdc_fail(hdc, X86_HDC_ERROR_ID_NOT_FOUND);
        return;
    }
    for (lib_u32 index = 0u; index < track.sector_count; ++index) {
        const lib_u8 flag = hdc->data.record.data[index * 2u];
        const lib_u8 id = hdc->data.record.data[index * 2u + 1u];
        if ((flag != 0u && flag != 0x80u) || id == 0u ||
            id > track.sector_count || seen[id - 1u]) {
            x86_hdc_fail(hdc, X86_HDC_ERROR_ABORT);
            return;
        }
        seen[id - 1u] = LIB_TRUE;
        track.ids[index] = id - 1u;
        track.bad[index] = flag == 0x80u;
    }
    result = hdc->connect.format_track == LIB_NULL ? X86_HDC_RECORD_FAILURE :
        hdc->connect.format_track(hdc->connect.context, unit, &track);
    if (result != X86_HDC_RECORD_OK) {
        x86_hdc_fail(hdc, result == X86_HDC_RECORD_RANGE ?
            X86_HDC_ERROR_ID_NOT_FOUND : X86_HDC_ERROR_ABORT);
        return;
    }
    x86_hdc_complete(hdc, LIB_TRUE);
}

static void x86_hdc_execute_command(x86_hdc *hdc, lib_u8 command)
{
    lib_u16 cylinder;

    if (hdc == LIB_NULL) return;
    hdc->data.last_command = command;
    ++hdc->data.command_count;
    if ((!x86_hdc_is_ata_pio(hdc) && !x86_hdc_is_compaq_wd_40mb(hdc) && !x86_hdc_is_ibm_wd1003(hdc) &&
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
    if ((command & 0xfeu) == X86_HDC_COMMAND_VERIFY_SECTORS) {
        x86_hdc_verify_sectors(hdc);
        return;
    }
    if (command == X86_HDC_COMMAND_INITIALIZE_DRIVE_PARAMETERS) {
        const lib_u8 unit = x86_hdc_selected_unit(hdc);
        const lib_u8 heads = (hdc->data.drive_head & 0x0fu) + 1u;
        const lib_bool ata = hdc->config.protocol == X86_HDC_PROTOCOL_ATA_PIO;
        x86_hdc_medium info;
        x86_hdc_record_result result;
        if (hdc->data.sector_count == 0u || (ata &&
            (!x86_hdc_media_info(hdc, &info, &result) || !info.present ||
                info.geometry.logical_sector_count <
                    (lib_u32)heads * hdc->data.sector_count))) {
            if (ata) {
                hdc->data.drive_heads[unit] = heads;
                hdc->data.drive_sectors_per_track[unit] = 0u;
            }
            x86_hdc_fail(hdc, X86_HDC_ERROR_ABORT);
            return;
        }
        hdc->data.drive_heads[unit] = heads;
        hdc->data.drive_sectors_per_track[unit] = hdc->data.sector_count;
        x86_hdc_complete(hdc, LIB_TRUE);
        return;
    }
    if (x86_hdc_is_ata_pio(hdc) &&
        (command & X86_HDC_COMMAND_RECALIBRATE_MASK) == X86_HDC_COMMAND_RECALIBRATE_VALUE) {
        x86_hdc_medium info;
        x86_hdc_record_result result;
        if (!x86_hdc_media_info(hdc, &info, &result) || !info.present) {
            x86_hdc_fail(hdc, X86_HDC_ERROR_ABORT);
            return;
        }
        hdc->data.cylinder_low = 0u;
        hdc->data.cylinder_high = 0u;
        hdc->data.sector_number = (hdc->data.drive_head & 0x40u) != 0u ? 0u : 1u;
        hdc->data.drive_head &= 0xf0u;
        x86_hdc_complete(hdc, LIB_TRUE);
        return;
    }
    if (x86_hdc_is_compaq_wd_40mb(hdc) || x86_hdc_is_ibm_wd1003(hdc)) {
        if (command == X86_HDC_COMMAND_FORMAT_TRACK) {
            x86_hdc_begin_format(hdc);
            return;
        }
        if (command == X86_HDC_COMMAND_EXECUTE_DIAGNOSTICS) {
            if (x86_hdc_is_ibm_wd1003(hdc))
                x86_hdc_select_wd_step_rate(hdc, 15u,
                    X86_HDC_IBM_RESTORE_STEP_LIMIT);
            hdc->data.error = X86_HDC_ERROR_DIAGNOSTIC_OK;
            x86_hdc_complete(hdc, LIB_TRUE);
            return;
        }
        if ((command & X86_HDC_COMMAND_RECALIBRATE_MASK) ==
            X86_HDC_COMMAND_RECALIBRATE_VALUE) {
            x86_hdc_select_wd_step_rate(hdc, command & 0x0fu,
                X86_HDC_IBM_RESTORE_STEP_LIMIT);
            hdc->data.cylinder_low = 0u;
            hdc->data.cylinder_high = 0u;
            x86_hdc_complete(hdc, LIB_TRUE);
            return;
        }
        if ((command & X86_HDC_COMMAND_SEEK_MASK) ==
            X86_HDC_COMMAND_SEEK_VALUE) {
            x86_hdc_select_wd_step_rate(hdc, command & 0x0fu, 0u);
            cylinder = (lib_u16)hdc->data.cylinder_low |
                ((lib_u16)hdc->data.cylinder_high << 8u);
            if (cylinder > 1023u) {
                x86_hdc_fail(hdc, X86_HDC_ERROR_ID_NOT_FOUND);
            } else {
                x86_hdc_complete(hdc, LIB_TRUE);
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
    hdc->xebec.ecc_limit = 11u;
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
    x86_hdc_raise_irq(hdc);
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

static x86_hdc_geometry x86_xebec_address_geometry(const x86_hdc *hdc)
{
    x86_hdc_geometry geometry = hdc->config.xebec_geometry;
    if (hdc->xebec.cylinders != 0u) {
        geometry.cylinders = hdc->xebec.cylinders;
        geometry.heads = hdc->xebec.heads;
    }
    return geometry;
}

static lib_i32 x86_xebec_sector(const x86_hdc *hdc,
    lib_u64 *out_sector)
{
    x86_hdc_geometry geometry;
    lib_u16 cylinder;
    lib_u8 head;
    lib_u8 sector;

    if (hdc == LIB_NULL || out_sector == LIB_NULL) return 0;
    geometry = x86_xebec_address_geometry(hdc);
    cylinder = (lib_u16)hdc->xebec.dcb[3] |
        ((lib_u16)(hdc->xebec.dcb[2] & 0xc0u) << 2u);
    head = hdc->xebec.dcb[1] & 0x1fu;
    sector = hdc->xebec.dcb[2] & 0x3fu;
    if (geometry.cylinders > 1024u || geometry.heads > 32u ||
        geometry.sectors_per_track > 64u ||
        cylinder >= geometry.cylinders || head >= geometry.heads ||
        sector >= geometry.sectors_per_track ||
        cylinder >= hdc->config.xebec_geometry.cylinders ||
        head >= hdc->config.xebec_geometry.heads) return 0;
    /* Programmed limits do not resize the physical recording channel. */
    *out_sector = ((lib_u64)cylinder * hdc->config.xebec_geometry.heads + head) *
        geometry.sectors_per_track + sector;
    return 1;
}

static lib_u8 x86_xebec_prepare_sector(const x86_hdc *hdc,
    lib_bool write_to_media, lib_u64 *out_sector)
{
    x86_hdc_medium info;
    x86_hdc_record_result result;
    const x86_hdc_geometry *expected = &hdc->config.xebec_geometry;

    lib_memory_set(&info, 0, sizeof(info));
    result = hdc->connect.query == LIB_NULL ? X86_HDC_RECORD_ABSENT :
        hdc->connect.query(hdc->connect.context, (hdc->xebec.dcb[1] >> 5u) & 1u, &info);
    if (result != X86_HDC_RECORD_OK || !info.present) return 0x04u;
    if (!info.geometry_known ||
        info.geometry.logical_sector_count != expected->logical_sector_count ||
        info.geometry.bytes_per_sector != sizeof(hdc->data.record.data) ||
        info.geometry.cylinders != expected->cylinders ||
        info.geometry.heads != expected->heads ||
        info.geometry.sectors_per_track != expected->sectors_per_track ||
        !x86_xebec_sector(hdc, out_sector) ||
        *out_sector >= info.geometry.logical_sector_count) return 0x21u;
    return write_to_media && info.read_only ? 0x03u : 0u;
}

static lib_u8 x86_xebec_transfer_sector(x86_hdc *hdc,
    lib_bool write_to_media)
{
    lib_u64 sector;
    lib_u8 error = x86_xebec_prepare_sector(hdc, write_to_media, &sector);
    x86_hdc_record_result result;
    x86_hdc_record record = {0};

    if (error != 0u) return error;
    if (write_to_media) {
        record = hdc->data.record;
        if (hdc->xebec.dcb[0] != 0xe6u)
            x86_hdc_ecc_encode(&record, x86_hdc_ecc_polynomial(hdc));
        result = hdc->connect.write == LIB_NULL ? X86_HDC_RECORD_FAILURE :
            hdc->connect.write(hdc->connect.context,
                (hdc->xebec.dcb[1] >> 5u) & 1u, sector, &record);
    } else {
        result = hdc->connect.read == LIB_NULL ? X86_HDC_RECORD_FAILURE :
            hdc->connect.read(hdc->connect.context,
                (hdc->xebec.dcb[1] >> 5u) & 1u, sector, &record);
        if (result == X86_HDC_RECORD_OK) {
            lib_u8 burst;
            hdc->data.error = 0u;
            if (hdc->xebec.dcb[0] == 0xe5u) {
                if (!record.ecc_present)
                    x86_hdc_ecc_encode(&record, x86_hdc_ecc_polynomial(hdc));
            } else if (!x86_hdc_ecc_check(&record, x86_hdc_ecc_polynomial(hdc),
                    hdc->xebec.ecc_limit, &burst)) {
                return 0x11u;
            } else if (burst != 0u) {
                hdc->xebec.corrected_burst = burst;
                hdc->data.error = 0x18u;
            }
            hdc->data.record = record;
        }
    }
    if (result == X86_HDC_RECORD_OK)
        return !write_to_media ? hdc->data.error : 0u;
    if (result == X86_HDC_RECORD_ABSENT) return 0x04u;
    if (result == X86_HDC_RECORD_RANGE) return 0x21u;
    if (result == X86_HDC_RECORD_BAD_TRACK) return 0x19u;
    /* Provider failures are logical data/write errors, not physical ECC proof. */
    return write_to_media ? 0x03u : 0x11u;
}

static lib_u8 x86_xebec_format_track(x86_hdc *hdc)
{
    const x86_hdc_geometry geometry = x86_xebec_address_geometry(hdc);
    const lib_u8 interleave = hdc->xebec.dcb[4];
    x86_hdc_track track = {0};
    lib_bool occupied[256] = {0};
    lib_u64 sector;
    lib_u32 position = 0u;
    lib_u8 error;
    x86_hdc_record_result result;

    if (interleave == 0u || interleave > 16u ||
        geometry.cylinders == 0u || geometry.cylinders > 1024u ||
        geometry.heads == 0u || geometry.heads > 32u ||
        geometry.sectors_per_track == 0u || geometry.sectors_per_track > 64u)
        return 0x20u;
    /* Format addresses a whole track; the DCB's sector bits are reserved. */
    hdc->xebec.dcb[2] &= 0xc0u;
    error = x86_xebec_prepare_sector(hdc, LIB_TRUE, &sector);
    if (error != 0u) return error;
    track.first_sector = sector;
    track.sector_count = geometry.sectors_per_track;
    /* L2 flat-image fill; ID order and bad flags are installed-lease facts. */
    track.fill_payload = hdc->xebec.dcb[0] != 0x07u;
    for (lib_u32 id = 0u; id < track.sector_count; ++id) {
        while (occupied[position]) position = (position + 1u) % track.sector_count;
        occupied[position] = LIB_TRUE;
        track.ids[position] = (lib_u8)id;
        track.bad[position] = hdc->xebec.dcb[0] == 0x07u;
        position = (position + interleave) % track.sector_count;
    }
    result = hdc->connect.format_track == LIB_NULL ? X86_HDC_RECORD_FAILURE :
        hdc->connect.format_track(hdc->connect.context,
            (hdc->xebec.dcb[1] >> 5u) & 1u, &track);
    if (result == X86_HDC_RECORD_OK) return 0u;
    if (result == X86_HDC_RECORD_ABSENT) return 0x04u;
    if (result == X86_HDC_RECORD_RANGE) return 0x21u;
    return 0x03u;
}

static lib_u8 x86_xebec_format_drive(x86_hdc *hdc)
{
    const x86_hdc_geometry geometry = x86_xebec_address_geometry(hdc);
    const lib_u16 first_cylinder = (lib_u16)hdc->xebec.dcb[3] |
        ((lib_u16)(hdc->xebec.dcb[2] & 0xc0u) << 2u);
    const lib_u8 first_head = hdc->xebec.dcb[1] & 0x1fu;
    const lib_u8 unit = hdc->xebec.dcb[1] & 0x20u;
    lib_u8 error;

    /* Validate the requested starting track before any mutation. */
    error = x86_xebec_format_track(hdc);
    if (error != 0u) return error;
    for (lib_u16 cylinder = first_cylinder; cylinder < geometry.cylinders; ++cylinder) {
        for (lib_u8 head = cylinder == first_cylinder ? first_head + 1u : 0u;
            head < geometry.heads; ++head) {
            hdc->xebec.dcb[1] = unit | head;
            hdc->xebec.dcb[2] = (lib_u8)((cylinder >> 2u) & 0xc0u);
            hdc->xebec.dcb[3] = (lib_u8)cylinder;
            error = x86_xebec_format_track(hdc);
            if (error != 0u) return error;
        }
    }
    return 0u;
}

static void x86_xebec_fail(x86_hdc *hdc, lib_u8 error, lib_bool addressed)
{
    lib_u8 sense[4] = {error, hdc->xebec.dcb[1] & 0x3fu,
        hdc->xebec.dcb[2], hdc->xebec.dcb[3]};

    if (addressed) sense[0] |= 0x80u;
    x86_xebec_release_dma(hdc);
    x86_xebec_response(hdc, 0x02u, sense);
}

static lib_i32 x86_xebec_next_sector(x86_hdc *hdc)
{
    x86_hdc_geometry geometry;
    lib_u16 cylinder;
    lib_u8 head;
    lib_u8 sector;

    if (hdc == LIB_NULL) return 0;
    geometry = x86_xebec_address_geometry(hdc);
    cylinder = (lib_u16)hdc->xebec.dcb[3] |
        ((lib_u16)(hdc->xebec.dcb[2] & 0xc0u) << 2u);
    head = hdc->xebec.dcb[1] & 0x1fu;
    sector = (lib_u8)((hdc->xebec.dcb[2] & 0x3fu) + 1u);
    if (sector == geometry.sectors_per_track) {
        sector = 0u;
        if (++head == geometry.heads) {
            head = 0u;
            ++cylinder;
        }
    }
    hdc->xebec.dcb[1] = (hdc->xebec.dcb[1] & 0xe0u) | head;
    hdc->xebec.dcb[2] = (hdc->xebec.dcb[2] & 0x3fu) |
        (lib_u8)((cylinder >> 2u) & 0xc0u);
    hdc->xebec.dcb[2] = (hdc->xebec.dcb[2] & 0xc0u) | sector;
    hdc->xebec.dcb[3] = (lib_u8)cylinder;
    return cylinder < geometry.cylinders;
}

static void x86_xebec_start_transfer(x86_hdc *hdc)
{
    const lib_bool read = hdc->xebec.dcb[0] == 0x08u || hdc->xebec.dcb[0] == 0xe5u;
    lib_u8 error;
    lib_u64 sector;

    if (hdc == LIB_NULL) return;
    hdc->xebec.byte_index = 0u;
    if (hdc->xebec.dcb[4] == 0u) {
        /* Zero count has no defined encoding in the selected original. */
        x86_xebec_fail(hdc, 0x20u, LIB_TRUE);
        return;
    }
    if (read) {
        error = x86_xebec_transfer_sector(hdc, LIB_FALSE);
    } else {
        error = x86_xebec_prepare_sector(hdc, LIB_TRUE, &sector);
    }
    if (error != 0u && error != 0x18u) {
        x86_xebec_fail(hdc, error, LIB_TRUE);
        return;
    }
    hdc->xebec.sectors_remaining = hdc->xebec.dcb[4];
    if (!read) hdc->data.record.ecc_present = LIB_FALSE;
    hdc->xebec.phase = read ?
        X86_XEBEC_PHASE_DMA_READ : X86_XEBEC_PHASE_DMA_WRITE;
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
    x86_hdc_medium info;
    x86_hdc_record_result result;
    lib_u64 sector;

    if (hdc == LIB_NULL) return;
    hdc->data.error = 0u;
    if (hdc->xebec.dcb[0] == 0x0du) {
        hdc->xebec.response[0] = hdc->xebec.corrected_burst;
        hdc->xebec.response[1] = hdc->xebec.dcb[1] & 0x20u;
        hdc->xebec.response_count = 2u;
        hdc->xebec.response_index = 0u;
        hdc->xebec.phase = X86_XEBEC_PHASE_RESPONSE;
        return;
    }
    if (hdc->xebec.dcb[0] == 0x03u) {
        lib_memory_copy(hdc->xebec.response, hdc->xebec.last_sense,
            sizeof(hdc->xebec.last_sense));
        hdc->xebec.response[4] = hdc->xebec.dcb[1] & 0x20u;
        hdc->xebec.response_count = sizeof(hdc->xebec.response);
        hdc->xebec.response_index = 0u;
        hdc->xebec.phase = X86_XEBEC_PHASE_RESPONSE;
        return;
    }
    if (!x86_xebec_command_is_defined(hdc->xebec.dcb[0])) {
        sense[0] = 0x20u;
        x86_xebec_response(hdc, 0x02u, sense);
        return;
    }
    if (hdc->xebec.dcb[0] == 0x0cu) {
        const lib_u16 cylinders = ((lib_u16)hdc->xebec.initialize[0] << 8u) |
            hdc->xebec.initialize[1];
        const lib_u8 heads = hdc->xebec.initialize[2];
        /* DCB selection is ignored: one characteristics set serves both drives.
         * Zero/out-of-domain counts are rejected by the flat-record model. */
        hdc->xebec.dcb[1] = 0u;
        if (cylinders == 0u || cylinders > 1024u || heads == 0u || heads > 32u ||
            hdc->xebec.initialize[7] > 11u) {
            sense[0] = 0x20u;
        } else {
            hdc->xebec.cylinders = cylinders;
            hdc->xebec.heads = heads;
            hdc->xebec.ecc_limit = hdc->xebec.initialize[7];
        }
        x86_xebec_response(hdc, sense[0] != 0u ? 0x02u : 0u, sense);
        return;
    }
    if (hdc->xebec.dcb[0] == 0xe0u || hdc->xebec.dcb[0] == 0xe4u) {
        /* E4 uses the L2 healthy-controller model also used by PCjs/86Box.
         * Only buffer RAM is exercised here; program-ROM checksum and physical
         * ECC circuit faults are not modeled. Neither command accesses a drive.
         * These patterns and the configured duration are not firmware proof. */
        static const lib_u8 patterns[] = {0u, 0xffu, 0x55u, 0xaau};
        hdc->data.record.ecc_present = LIB_FALSE;
        for (lib_size pattern = 0u; pattern < sizeof(patterns); ++pattern) {
            lib_memory_set(hdc->data.record.data, patterns[pattern], sizeof(hdc->data.record.data));
            for (lib_size byte = 0u; byte < sizeof(hdc->data.record.data); ++byte) {
                if (hdc->data.record.data[byte] != patterns[pattern]) {
                    sense[0] = 0x30u;
                    break;
                }
            }
            if (sense[0] != 0u) break;
        }
        x86_xebec_response(hdc, sense[0] != 0u ? 0x02u : 0u, sense);
        return;
    }
    if (hdc->xebec.dcb[0] == 0x04u || hdc->xebec.dcb[0] == 0x06u ||
        hdc->xebec.dcb[0] == 0x07u) {
        const lib_u8 error = hdc->xebec.dcb[0] == 0x04u ?
            x86_xebec_format_drive(hdc) : x86_xebec_format_track(hdc);
        if (error != 0u) x86_xebec_fail(hdc, error, LIB_TRUE);
        else x86_xebec_response(hdc, 0u, sense);
        return;
    }
    if (hdc->xebec.dcb[0] == 0x08u || hdc->xebec.dcb[0] == 0x0au ||
        hdc->xebec.dcb[0] == 0xe5u || hdc->xebec.dcb[0] == 0xe6u) {
        x86_xebec_start_transfer(hdc);
        return;
    }
    if (hdc->xebec.dcb[0] == 0x0eu || hdc->xebec.dcb[0] == 0x0fu) {
        if (hdc->xebec.dcb[0] == 0x0fu) hdc->data.record.ecc_present = LIB_FALSE;
        hdc->xebec.byte_index = 0u;
        hdc->xebec.sectors_remaining = 1u;
        hdc->xebec.phase = hdc->xebec.dcb[0] == 0x0eu ?
            X86_XEBEC_PHASE_DMA_READ : X86_XEBEC_PHASE_DMA_WRITE;
        x86_xebec_sync_dma_request(hdc);
        return;
    }
    if (hdc->xebec.dcb[0] == 0x00u || hdc->xebec.dcb[0] == 0x01u ||
        hdc->xebec.dcb[0] == 0x0bu || hdc->xebec.dcb[0] == 0x05u ||
        hdc->xebec.dcb[0] == 0xe3u) {
        lib_u8 unit = (hdc->xebec.dcb[1] >> 5u) & 1u;
        lib_bool addressed = hdc->xebec.dcb[0] == 0x0bu ||
            hdc->xebec.dcb[0] == 0x05u || hdc->xebec.dcb[0] == 0xe3u;

        if (hdc->xebec.dcb[0] == 0xe3u) {
            hdc->xebec.dcb[1] = unit << 5u;
            hdc->xebec.dcb[2] = 0u;
            hdc->xebec.dcb[3] = 0u;
        }

        lib_memory_set(&info, 0, sizeof(info));
        result = hdc->connect.query == LIB_NULL ? X86_HDC_RECORD_ABSENT :
            hdc->connect.query(hdc->connect.context, unit, &info);
        if (result != X86_HDC_RECORD_OK || !info.present) {
            sense[0] = 0x04u;
        } else if (addressed &&
            ((hdc->xebec.dcb[0] == 0x0bu &&
                (hdc->xebec.dcb[2] & 0x3fu) != 0u) ||
            !x86_xebec_sector(hdc, &sector) ||
            sector >= info.geometry.logical_sector_count)) {
            sense[0] = 0x21u;
        } else if (hdc->xebec.dcb[0] == 0xe3u) {
            /* Logical Drive Diagnostic reads sector zero of every track;
             * no physical STEP/ECC circuit or diagnostic duration is modeled. */
            const x86_hdc_geometry geometry = x86_xebec_address_geometry(hdc);
            for (lib_u16 cylinder = 0u; cylinder < geometry.cylinders; ++cylinder) {
                hdc->xebec.dcb[2] = (lib_u8)((cylinder >> 2u) & 0xc0u);
                hdc->xebec.dcb[3] = (lib_u8)cylinder;
                for (lib_u8 head = 0u; head < geometry.heads; ++head) {
                    hdc->xebec.dcb[1] = (unit << 5u) | head;
                    sense[0] = x86_xebec_transfer_sector(hdc, LIB_FALSE);
                    if (sense[0] != 0u) break;
                }
                if (sense[0] != 0u) break;
            }
        } else if (hdc->xebec.dcb[0] == 0x05u) {
            if (hdc->xebec.dcb[4] == 0u) {
                sense[0] = 0x20u;
            } else {
                for (lib_u16 count = 0u; count < hdc->xebec.dcb[4]; ++count) {
                    sense[0] = x86_xebec_transfer_sector(hdc, LIB_FALSE);
                    if (sense[0] != 0u) break;
                    if (count + 1u < hdc->xebec.dcb[4] &&
                        !x86_xebec_next_sector(hdc)) {
                        sense[0] = 0x21u;
                        break;
                    }
                }
            }
        }
        if (sense[0] != 0u) {
            if (addressed) sense[0] |= 0x80u;
            sense[1] = hdc->xebec.dcb[1] & 0x3fu;
            sense[2] = hdc->xebec.dcb[2];
            sense[3] = hdc->xebec.dcb[3];
        }
        x86_xebec_response(hdc, sense[0] != 0u ? 0x02u : 0u, sense);
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
        if (hdc->xebec.phase == X86_XEBEC_PHASE_DMA_READ) {
            lib_u8 byte = 0u;
            x86_hdc_dma_read(hdc, &byte);
            *out_value = byte;
            return LIB_STATUS_OK;
        }
        if (hdc->xebec.phase != X86_XEBEC_PHASE_RESPONSE ||
            hdc->xebec.response_index >= hdc->xebec.response_count) return LIB_STATUS_OK;
        *out_value = hdc->xebec.response[hdc->xebec.response_index++];
        x86_hdc_clear_irq(hdc);
        if (hdc->xebec.dcb[0] == 0x03u && hdc->xebec.response_index == 4u)
            x86_hdc_raise_irq(hdc);
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
        x86_hdc_sync_irq(hdc);
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
    if (hdc->xebec.phase == X86_XEBEC_PHASE_DMA_WRITE) {
        x86_hdc_dma_write(hdc, (lib_u8)value);
        return LIB_STATUS_OK;
    }
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
            hdc->data.data_index >= x86_hdc_transfer_size(hdc)) {
            return LIB_STATUS_OK;
        }
        if (hdc->data.data_index < sizeof(hdc->data.record.data)) {
            lib_memory_copy(&word, &hdc->data.record.data[hdc->data.data_index], sizeof(word));
            hdc->data.data_index = (lib_u16)(hdc->data.data_index + sizeof(word));
        } else {
            word = hdc->data.record.ecc[hdc->data.data_index - sizeof(hdc->data.record.data)];
            ++hdc->data.data_index;
        }
        *out_value = word;
        if (hdc->data.data_index == x86_hdc_transfer_size(hdc)) {
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
        /* Flat records do not model the physical WRITE GATE pulse. Bit 7
         * belongs to the board's wired-OR multipurpose-controller input. */
        *out_value = 0x40u | ((~x86_hdc_current_head(hdc) & 0x0fu) << 2u) |
            (x86_hdc_selected_unit(hdc) == 0u ? 0x02u : 0x01u);
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
            hdc->data.data_index >= x86_hdc_transfer_size(hdc)) {
            return LIB_STATUS_OK;
        }
        if (hdc->data.data_index < sizeof(hdc->data.record.data)) {
            lib_memory_copy(&hdc->data.record.data[hdc->data.data_index], &word, sizeof(word));
            hdc->data.data_index = (lib_u16)(hdc->data.data_index + sizeof(word));
        } else {
            hdc->data.record.ecc[hdc->data.data_index - sizeof(hdc->data.record.data)] = (lib_u8)value;
            ++hdc->data.data_index;
        }
        if (hdc->data.data_index == x86_hdc_transfer_size(hdc)) {
            hdc->data.record.ecc_present = x86_hdc_command_is_long(hdc, hdc->data.last_command);
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
    lib_u8 error;

    if (hdc == LIB_NULL || io_byte == LIB_NULL ||
        hdc->xebec.phase != X86_XEBEC_PHASE_DMA_READ ||
        hdc->xebec.byte_index >= x86_hdc_transfer_size(hdc)) return;
    *io_byte = hdc->xebec.byte_index < sizeof(hdc->data.record.data) ?
        hdc->data.record.data[hdc->xebec.byte_index] :
        hdc->data.record.ecc[hdc->xebec.byte_index - sizeof(hdc->data.record.data)];
    ++hdc->xebec.byte_index;
    if (hdc->xebec.byte_index == x86_hdc_transfer_size(hdc)) {
        if (hdc->data.error == 0x18u) {
            x86_xebec_fail(hdc, 0x18u, LIB_TRUE);
        } else if (--hdc->xebec.sectors_remaining == 0u) {
            x86_xebec_release_dma(hdc);
            x86_xebec_response(hdc, 0u, LIB_NULL);
        } else {
            error = x86_xebec_next_sector(hdc) ?
                x86_xebec_transfer_sector(hdc, LIB_FALSE) : 0x21u;
            if (error != 0u && error != 0x18u) x86_xebec_fail(hdc, error, LIB_TRUE);
            else hdc->xebec.byte_index = 0u;
        }
    }
}

void x86_hdc_dma_write(x86_hdc *hdc, lib_u8 byte)
{
    lib_u8 error;
    lib_u64 sector;

    if (hdc == LIB_NULL ||
        hdc->xebec.phase != X86_XEBEC_PHASE_DMA_WRITE ||
        hdc->xebec.byte_index >= x86_hdc_transfer_size(hdc)) return;
    if (hdc->xebec.byte_index < sizeof(hdc->data.record.data))
        hdc->data.record.data[hdc->xebec.byte_index] = byte;
    else hdc->data.record.ecc[hdc->xebec.byte_index - sizeof(hdc->data.record.data)] = byte;
    ++hdc->xebec.byte_index;
    if (hdc->xebec.byte_index != x86_hdc_transfer_size(hdc)) return;
    hdc->data.record.ecc_present = hdc->xebec.dcb[0] == 0xe6u;
    error = hdc->xebec.dcb[0] == 0x0fu ? 0u :
        x86_xebec_transfer_sector(hdc, LIB_TRUE);
    if (error != 0u || hdc->xebec.sectors_remaining == 0u) {
        x86_xebec_fail(hdc, error != 0u ? error : 0x20u,
            hdc->xebec.dcb[0] != 0x0fu);
        return;
    }
    if (--hdc->xebec.sectors_remaining == 0u) {
        x86_xebec_release_dma(hdc);
        x86_xebec_response(hdc, 0u, LIB_NULL);
        return;
    }
    error = x86_xebec_next_sector(hdc) ?
        x86_xebec_prepare_sector(hdc, LIB_TRUE, &sector) : 0x21u;
    if (error != 0u) {
        x86_xebec_fail(hdc, error, LIB_TRUE);
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
    if (x86_hdc_is_compaq_wd_40mb(hdc)) {
        hdc->data.drive_heads[0] = hdc->data.drive_heads[1] = 4u;
        hdc->data.drive_sectors_per_track[0] = hdc->data.drive_sectors_per_track[1] = 17u;
    }
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
        if (hdc->data.last_command == X86_HDC_COMMAND_FORMAT_TRACK)
            x86_hdc_format_track(hdc);
        else x86_hdc_next_write_sector(hdc);
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
        (!x86_hdc_is_xebec_xt(hdc) ||
            (hdc->xebec.mask_pattern & X86_XEBEC_MASK_IRQ_ENABLE) != 0u) &&
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
        .drive_head = hdc->data.drive_head, .fixed_disk_register = hdc->data.device_control & 0x08u,
        .step_rate_selector = hdc->data.step_rate_selector, .irq_pending = hdc->data.irq_pending,
        .xebec_phase = (lib_u8)hdc->xebec.phase, .xebec_dcb_count = hdc->xebec.dcb_count,
        .xebec_mask = hdc->xebec.mask_pattern
    };
    lib_memory_copy(out->xebec_initialize, hdc->xebec.initialize, sizeof(out->xebec_initialize));
    return LIB_STATUS_OK;
}
