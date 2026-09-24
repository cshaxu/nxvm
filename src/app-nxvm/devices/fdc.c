/* Copyright 2012-2026 Neko. */

/* The neutral 8272A-compatible controller. Core owns DMA/PIC/RAM; this file
 * owns only the guest-visible floppy-controller state machine. */
#include "lib/types/types_interface.h"
#include "app-nxvm/devices/device_support.h"


#include "app-nxvm/devices/dma.h"
#include "app-nxvm/devices/media_interface.h"
#include "app-nxvm/devices/pic.h"
#include "app-nxvm/devices/port.h"
#include "app-nxvm/devices/fdc.h"

#define core_machine_fdc_CMD_SPECIFY 0x03u
#define core_machine_fdc_CMD_SENSE_DRIVE_STATUS 0x04u
#define core_machine_fdc_CMD_RECALIBRATE 0x07u
#define core_machine_fdc_CMD_SENSE_INTERRUPT 0x08u
#define core_machine_fdc_CMD_SEEK 0x0fu
#define core_machine_fdc_CMD_READ_ID 0x0au
#define core_machine_fdc_CMD_WRITE_DATA 0x05u
#define core_machine_fdc_CMD_READ_DATA 0x06u
#define core_machine_fdc_CMD_WRITE_DELETED_DATA 0x09u
#define core_machine_fdc_CMD_READ_DELETED_DATA 0x0cu
#define core_machine_fdc_CMD_SCAN_EQUAL 0x11u
#define core_machine_fdc_CMD_SCAN_LOW_OR_EQUAL 0x19u
#define core_machine_fdc_CMD_SCAN_HIGH_OR_EQUAL 0x1du
#define core_machine_fdc_CMD_FORMAT_TRACK 0x0du
#define core_machine_fdc_CMD_READ_TRACK 0x02u

#define core_machine_fdc_ST1_NO_DATA 0x04u
#define core_machine_fdc_ST1_NOT_WRITABLE 0x02u
#define core_machine_fdc_ST1_END_OF_CYLINDER 0x80u

#define core_machine_fdc_SCAN_EQUAL 0u
#define core_machine_fdc_SCAN_LOW_OR_EQUAL 1u
#define core_machine_fdc_SCAN_HIGH_OR_EQUAL 2u

static lib_u8 core_machine_fdc_sector_size(lib_u8 code)
{
    return code == 2u ? 0x02u : 0xffu;
}

static lib_u8 core_machine_fdc_msr(const core_machine_fdc *fdc)
{
    lib_u8 value;
    lib_u8 drive;

    switch (fdc->data.phase) {
    case core_machine_fdc_PHASE_PENDING_COMMAND:
    case core_machine_fdc_PHASE_PENDING_COMPLETE:
        value = VFDC_MSR_CB; break;
    case core_machine_fdc_PHASE_RESULT:
        value = VFDC_MSR_RQM | VFDC_MSR_DIO | VFDC_MSR_CB; break;
    case core_machine_fdc_PHASE_EXECUTION_READ:
        value = fdc->data.flagNDMA && !fdc->data.ndma_byte_gate_pending ? VFDC_MSR_RQM | VFDC_MSR_DIO |
            VFDC_MSR_NDM | VFDC_MSR_CB : VFDC_MSR_CB; break;
    case core_machine_fdc_PHASE_EXECUTION_WRITE:
    case core_machine_fdc_PHASE_EXECUTION_SCAN:
    case core_machine_fdc_PHASE_EXECUTION_FORMAT:
        value = fdc->data.flagNDMA && !fdc->data.ndma_byte_gate_pending ? VFDC_MSR_RQM | VFDC_MSR_NDM |
            VFDC_MSR_CB : VFDC_MSR_CB; break;
    default:
        value = VFDC_MSR_RQM; break;
    }
    for (drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive) {
        if (fdc->data.seek_pending[drive]) value |= VFDC_MSR_DB(drive);
    }
    return value;
}

static core_machine_media_id core_machine_fdc_drive_media_id(
    const core_machine_fdc *fdc, lib_u8 drive)
{
    return fdc == LIB_NULL || drive >= CORE_MACHINE_FDC_DRIVE_COUNT ?
        CORE_MACHINE_MEDIA_ID_INVALID :
        fdc->connect.drives.media_id[drive];
}

static core_machine_media_id core_machine_fdc_selected_media_id(
    const core_machine_fdc *fdc)
{
    return fdc == LIB_NULL ? CORE_MACHINE_MEDIA_ID_INVALID :
        core_machine_fdc_drive_media_id(fdc, fdc->data.selected_drive);
}

static lib_i32 core_machine_fdc_drive_media_query(const core_machine_fdc *fdc,
    lib_u8 drive, core_machine_media_info *out_info,
    core_machine_media_result *out_result)
{
    core_machine_media_id media_id = core_machine_fdc_drive_media_id(fdc, drive);

    return fdc != LIB_NULL && fdc->connect.media_registry != LIB_NULL &&
        media_id != CORE_MACHINE_MEDIA_ID_INVALID &&
        core_machine_media_query(fdc->connect.media_registry, media_id, out_info,
            out_result) == LIB_STATUS_OK;
}

static lib_i32 core_machine_fdc_media_info(const core_machine_fdc *fdc,
    core_machine_media_info *out_info, core_machine_media_result *out_result)
{
    return core_machine_fdc_drive_media_query(fdc, fdc->data.selected_drive,
        out_info, out_result) &&
        *out_result == CORE_MACHINE_MEDIA_RESULT_OK;
}

static lib_i32 core_machine_fdc_drive_media_ready(const core_machine_fdc *fdc,
    lib_u8 drive)
{
    core_machine_media_info info;
    core_machine_media_result result;

    return core_machine_fdc_drive_media_query(fdc, drive, &info, &result) &&
        result == CORE_MACHINE_MEDIA_RESULT_OK && info.present;
}

static void core_machine_fdc_sample_ready(core_machine_fdc *fdc)
{
    lib_u8 drive;

    for (drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive) {
        fdc->data.observed_ready[drive] = core_machine_fdc_drive_media_ready(fdc, drive);
    }
}

static void core_machine_fdc_update_dir(core_machine_fdc *fdc)
{
    lib_u8 drive;

    if (fdc == LIB_NULL) return;
    drive = fdc->data.dor & VFDC_DOR_DS;
    /* Disk Change is a signal from an installed mechanical unit.  An empty
     * fitted drive asserts it; an unpopulated select line cannot. */
    if ((fdc->data.dor & VFDC_DOR_ME(drive)) != 0u &&
        (fdc->connect.drives.installed_mask & (1u << drive)) != 0u &&
        (fdc->data.media_changed[drive] ||
         !core_machine_fdc_drive_media_ready(fdc, drive))) {
        fdc->data.dir |= VFDC_DIR_DC;
    } else {
        fdc->data.dir &= (lib_u8)~VFDC_DIR_DC;
    }
}

static void core_machine_fdc_observe_drive(core_machine_fdc *fdc,
    lib_u8 drive)
{
    core_machine_media_info info;
    core_machine_media_result result;

    if (!core_machine_fdc_drive_media_query(fdc, drive, &info, &result)) return;
    fdc->data.observed_media_generation[drive] = info.generation;
    fdc->data.media_changed[drive] = LIB_FALSE;
    core_machine_fdc_update_dir(fdc);
}

static void core_machine_fdc_observe_all_drives(core_machine_fdc *fdc)
{
    lib_u8 drive;

    for (drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive) {
        core_machine_fdc_observe_drive(fdc, drive);
    }
}

static lib_i32 core_machine_fdc_media_offset(const core_machine_fdc *fdc,
    const core_machine_media_info *info, lib_u64 *out_offset)
{
    lib_u64 sector;

    if (fdc == LIB_NULL || info == LIB_NULL || out_offset == LIB_NULL ||
        fdc->data.head >= info->geometry.heads ||
        fdc->data.cylinder >= info->geometry.cylinders ||
        fdc->data.sector == 0u ||
        fdc->data.sector > info->geometry.sectors_per_track ||
        info->geometry.bytes_per_sector != 512u) return LIB_FALSE;
    sector = ((lib_u64)fdc->data.cylinder * info->geometry.heads +
        fdc->data.head) * info->geometry.sectors_per_track +
        (fdc->data.sector - 1u);
    *out_offset = sector * info->geometry.bytes_per_sector + fdc->data.byte_offset;
    return LIB_TRUE;
}

static void core_machine_fdc_deassert_dma(core_machine_fdc *fdc)
{
    if (fdc->connect.dma_request_deassert != LIB_NULL) {
        fdc->connect.dma_request_deassert(fdc->connect.dma_request_owner,
            &fdc->connect.dma_request);
    }
}

static lib_i32 core_machine_fdc_execution_active(const core_machine_fdc *fdc);

static void core_machine_fdc_request_assert(core_machine_fdc *fdc)
{
    if (fdc->connect.dma_request_assert != LIB_NULL) {
        fdc->connect.dma_request_assert(fdc->connect.dma_request_owner,
            &fdc->connect.dma_request);
    }
}

static lib_u64 core_machine_fdc_timing_ticks(const core_machine_fdc *fdc,
    lib_u64 microseconds)
{
    lib_u64 ticks_per_second;

    if (fdc == LIB_NULL) return 0u;
    ticks_per_second = fdc->connect.config.clock_ticks_per_second;
    if (ticks_per_second == 0u || microseconds > UINT64_MAX / ticks_per_second) {
        return 0u;
    }
    /* Round upward: a controller byte gate must not become visible before
     * the selected macro-axis duration has elapsed. */
    return (microseconds * ticks_per_second + 999999u) / 1000000u;
}

static lib_u64 core_machine_fdc_dma_byte_ticks(const core_machine_fdc *fdc)
{
    lib_u64 microseconds;

    if (fdc == LIB_NULL) return 0u;
    microseconds = (fdc->data.cmd[0] & 0x40u) != 0u ? 15u : 31u;
    switch (fdc->data.ccr) {
    case VFDC_CCR_RATE_500: break;
    case VFDC_CCR_RATE_300: microseconds = (microseconds * 5u + 2u) / 3u; break;
    /* The existing 250-kbps route has no sourced byte cadence.  It remains
     * an accepted logical transfer mode, whose next-Core-step gate is
     * selected below, rather than inventing a physical duration. */
    case VFDC_CCR_RATE_250: return 0u;
    default: return 0u;
    }
    return core_machine_fdc_timing_ticks(fdc, microseconds);
}

static lib_u8 core_machine_fdc_rate_supported(const core_machine_fdc *fdc)
{
    return fdc != LIB_NULL && (fdc->data.ccr == VFDC_CCR_RATE_500 ||
        fdc->data.ccr == VFDC_CCR_RATE_300 || fdc->data.ccr == VFDC_CCR_RATE_250);
}

static void core_machine_fdc_schedule_dma_byte(core_machine_fdc *fdc)
{
    lib_u64 ticks = core_machine_fdc_dma_byte_ticks(fdc);

    if (fdc == LIB_NULL || fdc->data.flagNDMA ||
        (fdc->data.phase != core_machine_fdc_PHASE_EXECUTION_READ &&
        fdc->data.phase != core_machine_fdc_PHASE_EXECUTION_WRITE &&
        fdc->data.phase != core_machine_fdc_PHASE_EXECUTION_SCAN &&
        fdc->data.phase != core_machine_fdc_PHASE_EXECUTION_FORMAT)) return;
    core_machine_fdc_deassert_dma(fdc);
    /* No selected service-time conversion retains the existing logical
     * handoff: each completed single-mode DMA service makes the next byte
     * eligible on the next Core progression point.  Reasserting inside the
     * DMA callback would be cleared by that same completed DMA service. */
    if (ticks == 0u) {
        fdc->data.next_dma_byte_tick = fdc->data.elapsed_ticks + 1u;
        fdc->data.dma_byte_gate_pending = LIB_TRUE;
        return;
    }
    fdc->data.next_dma_byte_tick = fdc->data.elapsed_ticks + ticks;
    fdc->data.dma_byte_gate_pending = LIB_TRUE;
}

static void core_machine_fdc_publish_due_dma_byte(core_machine_fdc *fdc)
{
    if (fdc == LIB_NULL || !fdc->data.dma_byte_gate_pending ||
        fdc->data.elapsed_ticks < fdc->data.next_dma_byte_tick ||
        !core_machine_fdc_execution_active(fdc)) return;
    fdc->data.dma_byte_gate_pending = LIB_FALSE;
    core_machine_fdc_request_assert(fdc);
}

static void core_machine_fdc_schedule_ndma_byte(core_machine_fdc *fdc)
{
    lib_u64 ticks = core_machine_fdc_dma_byte_ticks(fdc);

    if (fdc == LIB_NULL || !fdc->data.flagNDMA || ticks == 0u ||
        (fdc->data.phase != core_machine_fdc_PHASE_EXECUTION_READ &&
        fdc->data.phase != core_machine_fdc_PHASE_EXECUTION_WRITE &&
        fdc->data.phase != core_machine_fdc_PHASE_EXECUTION_SCAN &&
        fdc->data.phase != core_machine_fdc_PHASE_EXECUTION_FORMAT)) return;
    fdc->data.next_ndma_byte_tick = fdc->data.elapsed_ticks + ticks;
    fdc->data.ndma_byte_gate_pending = LIB_TRUE;
}

static void core_machine_fdc_publish_due_ndma_byte(core_machine_fdc *fdc)
{
    if (fdc == LIB_NULL || !fdc->data.ndma_byte_gate_pending ||
        fdc->data.elapsed_ticks < fdc->data.next_ndma_byte_tick ||
        !core_machine_fdc_execution_active(fdc)) return;
    fdc->data.ndma_byte_gate_pending = LIB_FALSE;
}

static void core_machine_fdc_raise_irq(core_machine_fdc *fdc)
{
    if ((fdc->data.dor & VFDC_DOR_ENRQ) == 0u) return;
    core_machine_pic_irq_source_assert(&fdc->connect.irq_source);
    fdc->data.flagINTR = LIB_TRUE;
}

static void core_machine_fdc_cancel_execution(core_machine_fdc *fdc)
{
    core_machine_fdc_deassert_dma(fdc);
    fdc->data.dma_byte_gate_pending = LIB_FALSE;
    fdc->data.next_dma_byte_tick = 0u;
    fdc->data.ndma_byte_gate_pending = LIB_FALSE;
    fdc->data.next_ndma_byte_tick = 0u;
    fdc->data.pending_st0 = 0u;
    fdc->data.pending_st1 = 0u;
    fdc->data.pending_st2 = 0u;
    fdc->data.transfer_remaining = 0u;
    fdc->data.byte_offset = 0u;
    fdc->data.format_headers_remaining = 0u;
}

static lib_i32 core_machine_fdc_execution_active(const core_machine_fdc *fdc)
{
    return fdc != LIB_NULL && (fdc->data.phase == core_machine_fdc_PHASE_PENDING_COMMAND ||
        fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_READ ||
        fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_WRITE ||
        fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_SCAN ||
        fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_FORMAT ||
        fdc->data.phase == core_machine_fdc_PHASE_PENDING_COMPLETE);
}

static void core_machine_fdc_command_phase(core_machine_fdc *fdc)
{
    fdc->data.phase = core_machine_fdc_PHASE_COMMAND;
    fdc->data.command_length = 0u;
    fdc->data.command_index = 0u;
    fdc->data.result_length = 0u;
    fdc->data.result_index = 0u;
    fdc->data.pending_st0 = 0u;
    fdc->data.pending_st1 = 0u;
    fdc->data.pending_st2 = 0u;
}

static void core_machine_fdc_result_phase(core_machine_fdc *fdc, lib_u8 length)
{
    core_machine_fdc_deassert_dma(fdc);
    fdc->data.phase = core_machine_fdc_PHASE_RESULT;
    fdc->data.result_length = length;
    fdc->data.result_index = 0u;
}

static void core_machine_fdc_set_result(core_machine_fdc *fdc, lib_u8 st0,
    lib_u8 st1, lib_u8 st2)
{
    fdc->data.st0 = st0;
    fdc->data.st1 = st1;
    fdc->data.st2 = st2;
    fdc->data.ret[0] = st0;
    fdc->data.ret[1] = st1;
    fdc->data.ret[2] = st2;
    fdc->data.ret[3] = (lib_u8)fdc->data.cylinder;
    fdc->data.ret[4] = (lib_u8)fdc->data.head;
    fdc->data.ret[5] = (lib_u8)fdc->data.sector;
    fdc->data.ret[6] = 0x02u;
}

static void core_machine_fdc_publish_terminal_result(core_machine_fdc *fdc)
{
    core_machine_fdc_terminal_observation observation;

    if (fdc == LIB_NULL || fdc->connect.observation_provider.callback == LIB_NULL) return;
    observation.sequence = ++fdc->connect.observation_sequence;
    observation.command = fdc->data.cmd[0];
    observation.drive = fdc->data.selected_drive;
    lib_memory_copy(observation.result, fdc->data.ret, sizeof(observation.result));
    observation.successful = fdc->data.st0 == core_machine_fdc_ST0_NORMAL &&
        fdc->data.st1 == 0u;
    fdc->connect.observation_provider.callback(fdc->connect.observation_provider.context,
        &observation);
}

static void core_machine_fdc_complete_transfer_with_status(core_machine_fdc *fdc,
    lib_u8 st0, lib_u8 st1)
{
    core_machine_fdc_deassert_dma(fdc);
    /* A terminal DMA service may finish a command between byte gates.  The
     * result phase owns no future byte transfer, so it must not retain the
     * previous execution gate as a phantom deadline. */
    fdc->data.dma_byte_gate_pending = LIB_FALSE;
    fdc->data.next_dma_byte_tick = 0u;
    fdc->data.ndma_byte_gate_pending = LIB_FALSE;
    fdc->data.next_ndma_byte_tick = 0u;
    fdc->data.pending_st0 = st0;
    fdc->data.pending_st1 = st1;
    fdc->data.phase = core_machine_fdc_PHASE_PENDING_COMPLETE;
}

static void core_machine_fdc_complete_transfer(core_machine_fdc *fdc,
    lib_u8 st1)
{
    core_machine_fdc_complete_transfer_with_status(fdc, 0u, st1);
}

static void core_machine_fdc_complete_unready_read(core_machine_fdc *fdc,
    core_machine_fdc_phase phase)
{
    if (fdc->connect.config.unready_read_policy ==
        CORE_MACHINE_FDC_UNREADY_READ_DESKPRO_REFERENCE &&
        phase == core_machine_fdc_PHASE_EXECUTION_READ) {
        core_machine_fdc_complete_transfer_with_status(fdc,
            core_machine_fdc_ST0_ABNORMAL | core_machine_fdc_ST0_NOT_READY, 0u);
        return;
    }
    core_machine_fdc_complete_transfer(fdc, core_machine_fdc_ST1_NO_DATA);
}

static void core_machine_fdc_begin_seek(core_machine_fdc *fdc, lib_u16 target)
{
    lib_u8 drive = fdc->data.selected_drive;
    lib_u16 current = fdc->data.drive_cylinder[drive];
    lib_u16 cylinder_count = fdc->connect.drives.cylinder_count[drive];
    lib_u16 physical;
    lib_u16 distance;

    if (cylinder_count != 0u && target >= cylinder_count)
        physical = (lib_u16)(cylinder_count - 1u);
    else physical = target;

    distance = current > physical ? current - physical : physical - current;
    fdc->data.seek_target[drive] = physical;
    fdc->data.seek_due_tick[drive] = fdc->data.elapsed_ticks +
        (lib_u64)distance * core_machine_fdc_timing_ticks(fdc,
            (lib_u64)(16u - fdc->data.srt) * 1000u);
    fdc->data.seek_pending[drive] = LIB_TRUE;
    core_machine_fdc_command_phase(fdc);
}

static lib_i32 core_machine_fdc_drive_ready_for(const core_machine_fdc *fdc,
    lib_u8 drive)
{
    return drive == (fdc->data.dor & VFDC_DOR_DS) &&
        (fdc->data.dor & VFDC_DOR_NRS) != 0u &&
        (fdc->data.dor & VFDC_DOR_ME(drive)) != 0u &&
        core_machine_fdc_drive_media_ready(fdc, drive);
}

/* A mounted medium is required for data transfer, but not for head motion.
 * The 8272A's seek completion reports the selected mechanical unit, whose
 * ready path is determined by its wiring and motor state. */
static lib_i32 core_machine_fdc_drive_mechanical_ready_for(const core_machine_fdc *fdc,
    lib_u8 drive)
{
    return fdc != LIB_NULL && drive < CORE_MACHINE_FDC_DRIVE_COUNT &&
        drive == (fdc->data.dor & VFDC_DOR_DS) &&
        (fdc->data.dor & VFDC_DOR_NRS) != 0u &&
        (fdc->connect.drives.installed_mask & (1u << drive)) != 0u;
}

static lib_i32 core_machine_fdc_drive_status_ready(const core_machine_fdc *fdc,
    lib_u8 drive)
{
    /* The ordinary PC FDC's Sense Drive Status samples the board READY line,
     * which is pulled ready for every unit select; media availability remains
     * exclusively a transfer-path condition.  Board personalities with a
     * different electrical READY source require an explicit topology input. */
    return fdc != LIB_NULL && drive < CORE_MACHINE_FDC_DRIVE_COUNT &&
        (fdc->connect.config.ready_mask & (1u << drive)) != 0u;
}

static lib_i32 core_machine_fdc_drive_ready(const core_machine_fdc *fdc)
{
    return core_machine_fdc_drive_ready_for(fdc, fdc->data.selected_drive);
}

static void core_machine_fdc_advance_position(core_machine_fdc *fdc)
{
    fdc->data.byte_offset++;
    if (fdc->data.byte_offset < 512u) return;
    fdc->data.byte_offset = 0u;
    fdc->data.sector++;
}

static lib_i32 core_machine_fdc_transfer_byte(core_machine_fdc *fdc, t_latch *latch,
    lib_i32 write_to_media)
{
    lib_u8 byte;
    core_machine_media_info info;
    core_machine_media_result result;
    core_machine_media_id media_id;
    lib_u64 offset;
    lib_u64 logical_sector;
    core_machine_media_address_mark mark;
    if (fdc->data.transfer_remaining == 0u) {
        core_machine_fdc_complete_transfer(fdc, core_machine_fdc_ST1_NO_DATA);
        return LIB_TRUE;
    }
    if (!core_machine_fdc_drive_ready(fdc)) {
        core_machine_fdc_complete_unready_read(fdc, fdc->data.phase);
        return LIB_TRUE;
    }
    if (!core_machine_fdc_media_info(fdc, &info, &result) ||
        fdc->data.sector > fdc->data.eot ||
        !core_machine_fdc_media_offset(fdc, &info, &offset)) {
        core_machine_fdc_complete_transfer(fdc, core_machine_fdc_ST1_END_OF_CYLINDER);
        return LIB_TRUE;
    }
    media_id = core_machine_fdc_selected_media_id(fdc);
    logical_sector = offset / info.geometry.bytes_per_sector;
    if (fdc->data.byte_offset == 0u && write_to_media) {
        if (core_machine_media_set_address_mark(fdc->connect.media_registry, media_id,
            logical_sector, fdc->data.transfer_write_deleted ?
            CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA :
            CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA, &result) != LIB_STATUS_OK ||
            result != CORE_MACHINE_MEDIA_RESULT_OK) {
            core_machine_fdc_complete_transfer(fdc, result == CORE_MACHINE_MEDIA_RESULT_READ_ONLY ?
                core_machine_fdc_ST1_NOT_WRITABLE : core_machine_fdc_ST1_NO_DATA);
            return LIB_TRUE;
        }
    }
    /* Address marks are optional media metadata.  A provider which does not
     * advertise them describes conventional Data-mark sectors, not an
     * unreadable medium. */
    mark = CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
    if (fdc->data.byte_offset == 0u && !write_to_media &&
        (info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_ADDRESS_MARKS) != 0u &&
        (core_machine_media_get_address_mark(fdc->connect.media_registry, media_id,
            logical_sector, &mark, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK)) {
        core_machine_fdc_complete_transfer(fdc, core_machine_fdc_ST1_NO_DATA);
        return LIB_TRUE;
    }
    if (fdc->data.byte_offset == 0u && !write_to_media &&
        ((mark == CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA) !=
        fdc->data.transfer_expect_deleted)) fdc->data.pending_st2 |= VFDC_ST2_CONTROL_MARK;
    if (write_to_media) {
        if (core_machine_media_write_bytes(fdc->connect.media_registry,
            media_id, offset, &latch->data.byte, 1u, &result) !=
            LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK) {
            core_machine_fdc_complete_transfer(fdc,
                result == CORE_MACHINE_MEDIA_RESULT_READ_ONLY ?
                core_machine_fdc_ST1_NOT_WRITABLE : core_machine_fdc_ST1_NO_DATA);
            return LIB_TRUE;
        }
    } else if (core_machine_media_read_bytes(fdc->connect.media_registry,
        media_id, offset, &byte, 1u, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK) {
        core_machine_fdc_complete_transfer(fdc, core_machine_fdc_ST1_NO_DATA);
        return LIB_TRUE;
    } else {
        latch->data.byte = byte;
    }
    fdc->data.transfer_remaining--;
    core_machine_fdc_advance_position(fdc);
    if (fdc->data.transfer_remaining == 0u) core_machine_fdc_complete_transfer(fdc, 0u);
    return LIB_FALSE;
}

static lib_i32 core_machine_fdc_prepare_scan_sector(core_machine_fdc *fdc)
{
    core_machine_media_info info;
    core_machine_media_result result;
    core_machine_media_id media_id;
    lib_u64 offset;
    lib_u64 logical_sector;
    core_machine_media_address_mark mark;

    while (fdc->data.transfer_remaining != 0u) {
        if (!core_machine_fdc_media_info(fdc, &info, &result) ||
            fdc->data.sector > fdc->data.eot ||
            !core_machine_fdc_media_offset(fdc, &info, &offset)) {
            core_machine_fdc_complete_transfer(fdc, core_machine_fdc_ST1_END_OF_CYLINDER);
            return LIB_FALSE;
        }
        media_id = core_machine_fdc_selected_media_id(fdc);
        logical_sector = offset / info.geometry.bytes_per_sector;
        mark = CORE_MACHINE_MEDIA_ADDRESS_MARK_DATA;
        if ((info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_ADDRESS_MARKS) != 0u &&
            (core_machine_media_get_address_mark(fdc->connect.media_registry, media_id,
                logical_sector, &mark, &result) != LIB_STATUS_OK ||
            result != CORE_MACHINE_MEDIA_RESULT_OK)) {
            core_machine_fdc_complete_transfer(fdc, core_machine_fdc_ST1_NO_DATA);
            return LIB_FALSE;
        }
        if (mark != CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA ||
            (fdc->data.cmd[0] & 0x20u) == 0u) {
            if (mark == CORE_MACHINE_MEDIA_ADDRESS_MARK_DELETED_DATA) {
                fdc->data.pending_st2 |= VFDC_ST2_CONTROL_MARK;
            }
            return LIB_TRUE;
        }
        /* SK skips a Deleted-Data sector before it requests comparison bytes
           from the host, but still advances through the selected EOT range. */
        fdc->data.transfer_remaining -= 512u;
        fdc->data.sector++;
        fdc->data.scan_sector_satisfies = LIB_TRUE;
    }
    fdc->data.pending_st2 |= VFDC_ST2_SCAN_MISMATCH;
    core_machine_fdc_complete_transfer(fdc, 0u);
    return LIB_FALSE;
}

static void core_machine_fdc_scan_byte(core_machine_fdc *fdc,
    lib_u8 compare_byte)
{
    lib_u8 media_byte;
    core_machine_media_info info;
    core_machine_media_result result;
    core_machine_media_id media_id;
    lib_u64 offset;
    lib_i32 byte_satisfies;

    if (fdc->data.transfer_remaining == 0u || !core_machine_fdc_drive_ready(fdc)) {
        core_machine_fdc_complete_transfer(fdc, core_machine_fdc_ST1_NO_DATA);
        return;
    }
    if (!core_machine_fdc_media_info(fdc, &info, &result) ||
        fdc->data.sector > fdc->data.eot ||
        !core_machine_fdc_media_offset(fdc, &info, &offset)) {
        core_machine_fdc_complete_transfer(fdc, core_machine_fdc_ST1_END_OF_CYLINDER);
        return;
    }
    media_id = core_machine_fdc_selected_media_id(fdc);
    if (core_machine_media_read_bytes(fdc->connect.media_registry, media_id, offset,
        &media_byte, 1u, &result) != LIB_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK) {
        core_machine_fdc_complete_transfer(fdc, core_machine_fdc_ST1_NO_DATA);
        return;
    }
    byte_satisfies = compare_byte == 0xffu;
    if (!byte_satisfies) {
        switch (fdc->data.scan_mode) {
        case core_machine_fdc_SCAN_EQUAL:
            byte_satisfies = compare_byte == media_byte;
            break;
        case core_machine_fdc_SCAN_LOW_OR_EQUAL:
            byte_satisfies = compare_byte <= media_byte;
            break;
        default:
            byte_satisfies = compare_byte >= media_byte;
            break;
        }
    }
    if (!byte_satisfies) fdc->data.scan_sector_satisfies = LIB_FALSE;
    fdc->data.transfer_remaining--;
    core_machine_fdc_advance_position(fdc);
    if (fdc->data.byte_offset != 0u) return;
    if (fdc->data.scan_sector_satisfies) {
        fdc->data.pending_st2 |= VFDC_ST2_SCAN_MATCH;
        core_machine_fdc_complete_transfer(fdc, 0u);
    } else if (fdc->data.transfer_remaining == 0u) {
        fdc->data.pending_st2 |= VFDC_ST2_SCAN_MISMATCH;
        core_machine_fdc_complete_transfer(fdc, 0u);
    } else {
        fdc->data.scan_sector_satisfies = LIB_TRUE;
        (void)core_machine_fdc_prepare_scan_sector(fdc);
    }
}

static void core_machine_fdc_format_byte(core_machine_fdc *fdc, lib_u8 byte)
{
    core_machine_media_info info;
    core_machine_media_result result;
    core_machine_media_id media_id;
    lib_u64 logical_sector;
    if (fdc->data.format_headers_remaining == 0u) return;
    fdc->data.format_id[fdc->data.format_id_index++] = byte;
    if (fdc->data.format_id_index != 4u) return;
    fdc->data.format_id_index = 0u;
    if (fdc->data.format_id[0] != fdc->data.cylinder ||
        fdc->data.format_id[1] != fdc->data.head ||
        fdc->data.format_id[3] != 2u ||
        !core_machine_fdc_media_info(fdc, &info, &result) ||
        fdc->data.format_id[2] == 0u ||
        fdc->data.format_id[2] > info.geometry.sectors_per_track) {
        core_machine_fdc_complete_transfer(fdc, core_machine_fdc_ST1_NO_DATA);
        return;
    }
    logical_sector = ((lib_u64)fdc->data.cylinder * info.geometry.heads +
        fdc->data.head) * info.geometry.sectors_per_track +
        fdc->data.format_id[2] - 1u;
    media_id = core_machine_fdc_selected_media_id(fdc);
    if (core_machine_media_format_sectors(fdc->connect.media_registry,
        media_id, logical_sector, 1u, fdc->data.cmd[5], &result) !=
        LIB_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK) {
        core_machine_fdc_complete_transfer(fdc,
            result == CORE_MACHINE_MEDIA_RESULT_READ_ONLY ?
            core_machine_fdc_ST1_NOT_WRITABLE : core_machine_fdc_ST1_NO_DATA);
        return;
    }
    fdc->data.format_headers_remaining--;
    fdc->data.sector = fdc->data.format_id[2];
    if (fdc->data.format_headers_remaining == 0u) core_machine_fdc_complete_transfer(fdc, 0u);
}

static void core_machine_fdc_dma_read(void *owner, t_latch *latch)
{
    core_machine_fdc *fdc = owner;
    if (fdc != LIB_NULL && fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_READ) {
        (void)core_machine_fdc_transfer_byte(fdc, latch, LIB_FALSE);
        core_machine_fdc_schedule_dma_byte(fdc);
    }
}

static void core_machine_fdc_dma_write(void *owner, t_latch *latch)
{
    core_machine_fdc *fdc = owner;
    if (fdc == LIB_NULL) return;
    if (fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_WRITE) {
        (void)core_machine_fdc_transfer_byte(fdc, latch, LIB_TRUE);
        core_machine_fdc_schedule_dma_byte(fdc);
    } else if (fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_SCAN) {
        core_machine_fdc_scan_byte(fdc, latch->data.byte);
        core_machine_fdc_schedule_dma_byte(fdc);
    } else if (fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_FORMAT) {
        core_machine_fdc_format_byte(fdc, latch->data.byte);
        core_machine_fdc_schedule_dma_byte(fdc);
    }
}

static void core_machine_fdc_dma_terminal(void *owner, t_latch *latch)
{
    core_machine_fdc *fdc = owner;
    (void)latch;
    if (fdc != LIB_NULL && (fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_READ ||
        fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_WRITE ||
        fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_SCAN ||
        fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_FORMAT)) {
        core_machine_fdc_complete_transfer(fdc, 0u);
    }
}

static const core_machine_dma_channel_provider core_machine_fdc_dma_channel = {
    core_machine_fdc_dma_read, core_machine_fdc_dma_write, core_machine_fdc_dma_terminal
};

const core_machine_dma_channel_provider *core_machine_fdc_dma_provider(void)
{ return &core_machine_fdc_dma_channel; }

static lib_u8 core_machine_fdc_command_length(lib_u8 opcode)
{
    switch (opcode & 0x1fu) {
    case core_machine_fdc_CMD_SPECIFY: return 3u;
    case core_machine_fdc_CMD_SENSE_DRIVE_STATUS: return 2u;
    case core_machine_fdc_CMD_RECALIBRATE: return 2u;
    case core_machine_fdc_CMD_SENSE_INTERRUPT: return 1u;
    case core_machine_fdc_CMD_SEEK: return 3u;
    case core_machine_fdc_CMD_READ_ID: return 2u;
    case core_machine_fdc_CMD_WRITE_DATA:
    case core_machine_fdc_CMD_READ_DATA:
    case core_machine_fdc_CMD_WRITE_DELETED_DATA:
    case core_machine_fdc_CMD_READ_DELETED_DATA:
    case core_machine_fdc_CMD_SCAN_EQUAL:
    case core_machine_fdc_CMD_SCAN_LOW_OR_EQUAL:
    case core_machine_fdc_CMD_SCAN_HIGH_OR_EQUAL:
    case core_machine_fdc_CMD_READ_TRACK: return 9u;
    case core_machine_fdc_CMD_FORMAT_TRACK: return 6u;
    default: return 1u;
    }
}

static void core_machine_fdc_start_transfer(core_machine_fdc *fdc,
    core_machine_fdc_phase phase, lib_i32 deleted_data, lib_u8 scan_mode)
{
    lib_u8 size = core_machine_fdc_sector_size(fdc->data.cmd[5]);
    core_machine_media_info info;
    core_machine_media_result result;
    fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
    fdc->data.cylinder = fdc->data.cmd[2];
    fdc->data.head = fdc->data.cmd[3];
    fdc->data.sector = fdc->data.cmd[4];
    fdc->data.eot = fdc->data.cmd[6];
    fdc->data.byte_offset = 0u;
    fdc->data.transfer_expect_deleted = phase == core_machine_fdc_PHASE_EXECUTION_READ &&
        deleted_data;
    fdc->data.transfer_write_deleted = phase == core_machine_fdc_PHASE_EXECUTION_WRITE &&
        deleted_data;
    fdc->data.scan_mode = scan_mode;
    fdc->data.scan_sector_satisfies = LIB_TRUE;
    if (!core_machine_fdc_drive_ready(fdc)) {
        core_machine_fdc_complete_unready_read(fdc, phase);
        return;
    }
    if (size != 2u || !core_machine_fdc_rate_supported(fdc) ||
        !core_machine_fdc_media_info(fdc, &info, &result) ||
        fdc->data.head >= info.geometry.heads || fdc->data.sector == 0u ||
        fdc->data.sector > fdc->data.eot) {
        core_machine_fdc_complete_transfer(fdc, core_machine_fdc_ST1_NO_DATA);
        return;
    }
    fdc->data.transfer_remaining = (lib_u32)(fdc->data.eot -
        fdc->data.sector + 1u) * 512u;
    fdc->data.phase = phase;
    if (phase == core_machine_fdc_PHASE_EXECUTION_SCAN &&
        !core_machine_fdc_prepare_scan_sector(fdc)) return;
    if (!fdc->data.flagNDMA && (fdc->data.dor & VFDC_DOR_ENRQ) != 0u) {
        core_machine_fdc_request_assert(fdc);
    }
}

static void core_machine_fdc_start_read_track(core_machine_fdc *fdc)
{
    core_machine_media_info info;
    core_machine_media_result result;
    fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
    fdc->data.cylinder = fdc->data.cmd[2];
    fdc->data.head = fdc->data.cmd[3];
    fdc->data.sector = fdc->data.cmd[4];
    fdc->data.eot = fdc->data.cmd[6];
    fdc->data.byte_offset = 0u;
    if (fdc->data.cmd[0] != 0x42u || fdc->data.flagNDMA ||
        core_machine_fdc_sector_size(fdc->data.cmd[5]) != 2u ||
        !core_machine_fdc_rate_supported(fdc) ||
        !core_machine_fdc_drive_ready(fdc) || fdc->data.selected_drive != 0u ||
        !core_machine_fdc_media_info(fdc, &info, &result) ||
        fdc->data.head >= info.geometry.heads ||
        fdc->data.cylinder >= info.geometry.cylinders ||
        fdc->data.sector != 1u ||
        fdc->data.eot != info.geometry.sectors_per_track) {
        core_machine_fdc_complete_transfer(fdc, core_machine_fdc_ST1_NO_DATA);
        return;
    }
    fdc->data.transfer_remaining = (lib_u32)
        info.geometry.sectors_per_track * 512u;
    fdc->data.phase = core_machine_fdc_PHASE_EXECUTION_READ;
    if ((fdc->data.dor & VFDC_DOR_ENRQ) != 0u) {
        core_machine_fdc_request_assert(fdc);
    }
}

static void core_machine_fdc_execute(core_machine_fdc *fdc)
{
    lib_u8 opcode = fdc->data.cmd[0] & 0x1fu;
    core_machine_media_info info;
    core_machine_media_result media_result;
    lib_i32 media_ok;

    /* A command that changes controller activity supersedes reset's stale
     * Sense Interrupt notifications.  Sense Interrupt itself is the sole
     * command allowed to drain them. */
    if (opcode != core_machine_fdc_CMD_SENSE_INTERRUPT &&
        fdc->data.reset_sense_mask != 0u) {
        fdc->data.reset_sense_mask = 0u;
        fdc->data.reset_pending = LIB_FALSE;
        core_machine_pic_irq_source_deassert(&fdc->connect.irq_source);
    }
    switch (opcode) {
    case core_machine_fdc_CMD_SPECIFY:
        fdc->data.hut = fdc->data.cmd[1] & 0x0fu;
        fdc->data.srt = fdc->data.cmd[1] >> 4u;
        fdc->data.hlt = fdc->data.cmd[2] >> 1u;
        fdc->data.flagNDMA = (fdc->data.cmd[2] & 1u) != 0u;
        fdc->data.ready_poll_enabled = LIB_TRUE;
        core_machine_fdc_command_phase(fdc);
        break;
    case core_machine_fdc_CMD_SENSE_DRIVE_STATUS:
        fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
        fdc->data.head = (fdc->data.cmd[1] >> 2u) & 1u;
        media_ok = core_machine_fdc_media_info(fdc, &info, &media_result);
        fdc->data.st3 = (fdc->data.selected_drive & 3u) |
            (fdc->data.head << 2u) | (media_ok &&
            (info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_READ_ONLY) != 0u ? 0x40u : 0u) | (core_machine_fdc_drive_status_ready(fdc,
                fdc->data.selected_drive) ?
            0x20u : 0u) | ((fdc->connect.drives.double_sided_mask &
            (1u << fdc->data.selected_drive)) != 0u ? 0x08u : 0u) |
            ((fdc->connect.drives.installed_mask &
                (1u << fdc->data.selected_drive)) != 0u &&
            fdc->data.drive_cylinder[fdc->data.selected_drive] == 0u &&
            (fdc->connect.drives.track_zero_active_low_mask &
                (1u << fdc->data.selected_drive)) == 0u ? 0x10u : 0u);
        fdc->data.ret[0] = fdc->data.st3;
        core_machine_fdc_result_phase(fdc, 1u);
        break;
    case core_machine_fdc_CMD_RECALIBRATE:
        fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
        fdc->data.head = 0u; fdc->data.sector = 1u;
        core_machine_fdc_begin_seek(fdc, 0u);
        break;
    case core_machine_fdc_CMD_SENSE_INTERRUPT:
        if (fdc->data.reset_sense_mask != 0u) {
            lib_u8 drive = 0u;

            while ((fdc->data.reset_sense_mask & (1u << drive)) == 0u) ++drive;

            fdc->data.ret[0] = core_machine_fdc_ST0_READY_CHANGE | drive;
            fdc->data.ret[1] = (lib_u8)fdc->data.drive_cylinder[drive];
            fdc->data.reset_sense_mask &= (lib_u8)~(1u << drive);
            fdc->data.flagINTR = LIB_FALSE;
            core_machine_pic_irq_source_deassert(&fdc->connect.irq_source);
        } else if (fdc->data.seek_result_count != 0u) {
            lib_u8 index;

            fdc->data.ret[0] = fdc->data.seek_result_st0[0u];
            fdc->data.ret[1] = fdc->data.seek_result_cylinder[0u];
            for (index = 1u; index < fdc->data.seek_result_count; ++index) {
                fdc->data.seek_result_st0[index - 1u] = fdc->data.seek_result_st0[index];
                fdc->data.seek_result_cylinder[index - 1u] = fdc->data.seek_result_cylinder[index];
            }
            fdc->data.seek_result_count--;
            fdc->data.flagINTR = fdc->data.seek_result_count != 0u;
            if (!fdc->data.flagINTR) core_machine_pic_irq_source_deassert(&fdc->connect.irq_source);
        } else if (fdc->data.flagINTR) {
            fdc->data.ret[0] = fdc->data.st0;
            fdc->data.ret[1] = (lib_u8)fdc->data.cylinder;
            fdc->data.flagINTR = LIB_FALSE;
            core_machine_pic_irq_source_deassert(&fdc->connect.irq_source);
        } else {
            fdc->data.ret[0] = 0x80u;
            fdc->data.ret[1] = 0u;
        }
        core_machine_fdc_result_phase(fdc, 2u);
        break;
    case core_machine_fdc_CMD_SEEK:
        fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
        fdc->data.head = (fdc->data.cmd[1] >> 2u) & 1u;
        fdc->data.sector = 1u;
        core_machine_fdc_begin_seek(fdc, fdc->data.cmd[2]);
        break;
    case core_machine_fdc_CMD_READ_ID:
        fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
        fdc->data.head = (fdc->data.cmd[1] >> 2u) & 1u;
        media_ok = core_machine_fdc_media_info(fdc, &info, &media_result);
        if (!core_machine_fdc_drive_ready(fdc) || !media_ok) {
            core_machine_fdc_complete_transfer(fdc, core_machine_fdc_ST1_NO_DATA);
        } else {
            if (fdc->data.sector == 0u) fdc->data.sector = 1u;
            core_machine_fdc_set_result(fdc, core_machine_fdc_ST0_NORMAL, 0u, 0u);
            core_machine_fdc_result_phase(fdc, 7u);
            core_machine_fdc_publish_terminal_result(fdc);
            core_machine_fdc_raise_irq(fdc);
        }
        break;
    case core_machine_fdc_CMD_READ_DATA:
        core_machine_fdc_start_transfer(fdc, core_machine_fdc_PHASE_EXECUTION_READ,
            LIB_FALSE, 0u);
        break;
    case core_machine_fdc_CMD_READ_TRACK:
        core_machine_fdc_start_read_track(fdc);
        break;
    case core_machine_fdc_CMD_WRITE_DATA:
        core_machine_fdc_start_transfer(fdc, core_machine_fdc_PHASE_EXECUTION_WRITE,
            LIB_FALSE, 0u);
        break;
    case core_machine_fdc_CMD_READ_DELETED_DATA:
        core_machine_fdc_start_transfer(fdc, core_machine_fdc_PHASE_EXECUTION_READ,
            LIB_TRUE, 0u);
        break;
    case core_machine_fdc_CMD_WRITE_DELETED_DATA:
        core_machine_fdc_start_transfer(fdc, core_machine_fdc_PHASE_EXECUTION_WRITE,
            LIB_TRUE, 0u);
        break;
    case core_machine_fdc_CMD_SCAN_EQUAL:
        core_machine_fdc_start_transfer(fdc, core_machine_fdc_PHASE_EXECUTION_SCAN,
            LIB_FALSE, core_machine_fdc_SCAN_EQUAL);
        break;
    case core_machine_fdc_CMD_SCAN_LOW_OR_EQUAL:
        core_machine_fdc_start_transfer(fdc, core_machine_fdc_PHASE_EXECUTION_SCAN,
            LIB_FALSE, core_machine_fdc_SCAN_LOW_OR_EQUAL);
        break;
    case core_machine_fdc_CMD_SCAN_HIGH_OR_EQUAL:
        core_machine_fdc_start_transfer(fdc, core_machine_fdc_PHASE_EXECUTION_SCAN,
            LIB_FALSE, core_machine_fdc_SCAN_HIGH_OR_EQUAL);
        break;
    case core_machine_fdc_CMD_FORMAT_TRACK:
        fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
        fdc->data.head = (fdc->data.cmd[1] >> 2u) & 1u;
        fdc->data.sector = 1u;
        fdc->data.eot = fdc->data.cmd[3];
        fdc->data.format_headers_remaining = fdc->data.cmd[3];
        fdc->data.format_id_index = 0u;
        media_ok = core_machine_fdc_media_info(fdc, &info, &media_result);
        if (core_machine_fdc_sector_size(fdc->data.cmd[2]) != 2u ||
            !core_machine_fdc_rate_supported(fdc) ||
            !core_machine_fdc_drive_ready(fdc) || fdc->data.eot == 0u ||
            !media_ok || fdc->data.eot > info.geometry.sectors_per_track) {
            core_machine_fdc_complete_transfer(fdc, core_machine_fdc_ST1_NO_DATA);
        } else {
            fdc->data.phase = core_machine_fdc_PHASE_EXECUTION_FORMAT;
            if (!fdc->data.flagNDMA && (fdc->data.dor & VFDC_DOR_ENRQ) != 0u) {
                core_machine_fdc_request_assert(fdc);
            }
        }
        break;
    default:
        fdc->data.ret[0] = 0x80u;
        core_machine_fdc_result_phase(fdc, 1u);
        break;
    }
}

static void core_machine_fdc_reset_controller(core_machine_fdc *fdc)
{
    lib_u8 ccr = fdc->data.ccr;
    lib_u8 hut = fdc->data.hut;
    lib_u8 hlt = fdc->data.hlt;
    lib_u8 srt = fdc->data.srt;
    lib_u64 elapsed_ticks = fdc->data.elapsed_ticks;
    lib_u8 initial_media_baseline_pending = fdc->data.initial_media_baseline_pending;
    lib_u64 observed_media_generation[CORE_MACHINE_FDC_DRIVE_COUNT];
    lib_memory_copy(observed_media_generation, fdc->data.observed_media_generation,
        sizeof(observed_media_generation));
    core_machine_fdc_cancel_execution(fdc);
    lib_memory_set(&fdc->data, 0u, sizeof(fdc->data));
    fdc->data.ccr = ccr;
    fdc->data.hut = hut;
    fdc->data.hlt = hlt;
    fdc->data.srt = srt;
    fdc->data.elapsed_ticks = elapsed_ticks;
    fdc->data.initial_media_baseline_pending = initial_media_baseline_pending;
    lib_memory_copy(fdc->data.observed_media_generation, observed_media_generation,
        sizeof(observed_media_generation));
    core_machine_fdc_sample_ready(fdc);
    core_machine_fdc_command_phase(fdc);
}

static void core_machine_fdc_publish_due_reset(core_machine_fdc *fdc)
{
    if (fdc == LIB_NULL || !fdc->data.reset_pending ||
        fdc->data.elapsed_ticks < fdc->data.reset_due_tick) return;
    fdc->data.reset_pending = LIB_FALSE;
    if (fdc->data.reset_sense_mask != 0u) core_machine_fdc_raise_irq(fdc);
}

static void core_machine_fdc_schedule_reset_completion(core_machine_fdc *fdc)
{
    /* Reset completion queues one Sense Interrupt Status result for every
     * controller drive select.  This is controller state, not a sample of
     * the board READY inputs: system firmware drains all four results even
     * when fewer mechanical drives are fitted. */
    fdc->data.reset_sense_mask = fdc->connect.config.ready_mask == 0u ? 0u :
        (lib_u8)((1u << CORE_MACHINE_FDC_DRIVE_COUNT) - 1u);
    fdc->data.reset_due_tick = fdc->data.elapsed_ticks +
        core_machine_fdc_timing_ticks(fdc, 1024u);
    fdc->data.reset_pending = fdc->data.reset_sense_mask != 0u;
    core_machine_fdc_publish_due_reset(fdc);
}

static void core_machine_fdc_read_status(t_port *port, lib_u16 id,
    void *owner)
{
    core_machine_fdc *fdc = owner; (void)port; (void)id;
    fdc->connect.port->data.ioByte = core_machine_fdc_msr(fdc);
}

static void core_machine_fdc_read_data(t_port *port, lib_u16 id,
    void *owner)
{
    core_machine_fdc *fdc = owner; t_latch latch;
    (void)port; (void)id;
    if (fdc->data.phase == core_machine_fdc_PHASE_RESULT) {
        if (fdc->data.result_index == 0u && fdc->data.flagINTR) {
            fdc->data.flagINTR = LIB_FALSE;
            core_machine_pic_irq_source_deassert(&fdc->connect.irq_source);
        }
        fdc->connect.port->data.ioByte = fdc->data.ret[fdc->data.result_index++];
        if (fdc->data.result_index >= fdc->data.result_length) core_machine_fdc_command_phase(fdc);
    } else if (fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_READ &&
        fdc->data.flagNDMA && !fdc->data.ndma_byte_gate_pending) {
        latch.data.byte = 0u;
        (void)core_machine_fdc_transfer_byte(fdc, &latch, LIB_FALSE);
        core_machine_fdc_schedule_ndma_byte(fdc);
        fdc->connect.port->data.ioByte = latch.data.byte;
    }
}

static void core_machine_fdc_read_direction(t_port *port, lib_u16 id,
    void *owner)
{
    core_machine_fdc *fdc = owner; (void)port; (void)id;

    /* Media insertion/removal is an external input, not a controller clock.
     * Sample it only when the guest observes the change-latch port. */
    core_machine_fdc_refresh(fdc);
    fdc->connect.port->data.ioByte = fdc->data.dir;
}

static void core_machine_fdc_write_dor(t_port *port, lib_u16 id,
    void *owner)
{
    core_machine_fdc *fdc = owner;
    lib_u8 dor = fdc->connect.port->data.ioByte;
    lib_u8 old_dor = fdc->data.dor;
    (void)port; (void)id;
    if ((dor & VFDC_DOR_NRS) == 0u) {
        core_machine_pic_irq_source_deassert(&fdc->connect.irq_source);
        fdc->data.flagINTR = LIB_FALSE;
        core_machine_fdc_reset_controller(fdc);
    }
    fdc->data.dor = dor;
    if ((dor & VFDC_DOR_NRS) != 0u && (old_dor & VFDC_DOR_NRS) == 0u) {
        core_machine_fdc_reset_controller(fdc);
        fdc->data.dor = dor;
        core_machine_fdc_schedule_reset_completion(fdc);
    }
    core_machine_fdc_update_dir(fdc);
    if ((dor & VFDC_DOR_NRS) != 0u && core_machine_fdc_execution_active(fdc) &&
        !core_machine_fdc_drive_ready(fdc)) {
        core_machine_fdc_cancel_execution(fdc);
        core_machine_fdc_command_phase(fdc);
    }
}

static void core_machine_fdc_write_data(t_port *port, lib_u16 id,
    void *owner)
{
    core_machine_fdc *fdc = owner; t_latch latch;
    (void)port; (void)id;
    if (fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_WRITE && fdc->data.flagNDMA &&
        !fdc->data.ndma_byte_gate_pending) {
        latch.data.byte = fdc->connect.port->data.ioByte;
        (void)core_machine_fdc_transfer_byte(fdc, &latch, LIB_TRUE);
        core_machine_fdc_schedule_ndma_byte(fdc);
        return;
    }
    if (fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_SCAN && fdc->data.flagNDMA &&
        !fdc->data.ndma_byte_gate_pending) {
        core_machine_fdc_scan_byte(fdc, fdc->connect.port->data.ioByte);
        core_machine_fdc_schedule_ndma_byte(fdc);
        return;
    }
    if (fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_FORMAT && fdc->data.flagNDMA &&
        !fdc->data.ndma_byte_gate_pending) {
        core_machine_fdc_format_byte(fdc, fdc->connect.port->data.ioByte);
        core_machine_fdc_schedule_ndma_byte(fdc);
        return;
    }
    if (fdc->data.phase != core_machine_fdc_PHASE_COMMAND) return;
    if (fdc->data.command_index == 0u) {
        fdc->data.command_length = core_machine_fdc_command_length(
            fdc->connect.port->data.ioByte);
    }
    if (fdc->data.command_index >= sizeof(fdc->data.cmd)) return;
    fdc->data.cmd[fdc->data.command_index++] = fdc->connect.port->data.ioByte;
    if (fdc->data.command_index == fdc->data.command_length) {
        fdc->data.phase = core_machine_fdc_PHASE_PENDING_COMMAND;
    }
}

static void core_machine_fdc_write_control(t_port *port, lib_u16 id,
    void *owner)
{
    core_machine_fdc *fdc = owner; (void)port; (void)id;
    fdc->data.ccr = fdc->connect.port->data.ioByte & VFDC_CCR_RATE_MASK;
}

static void core_machine_fdc_read_diagnostic(t_port *port, lib_u16 id,
    void *owner)
{
    const core_machine_fdc *fdc = owner;

    (void)id;
    if (fdc == LIB_NULL || fdc->connect.port == LIB_NULL) return;
    port->data.ioByte = fdc->connect.config.diagnostic_read_value;
}

void core_machine_fdc_connect(core_machine_fdc *fdc,
    const core_machine_media_registry *media_registry,
    const core_machine_fdc_drive_bindings *drives,
    const core_machine_dma_request_binding *dma_request,
    core_machine_fdc_dma_request_operation dma_request_assert,
    core_machine_fdc_dma_request_operation dma_request_deassert,
    void *dma_request_owner, t_pic *pic_master, t_pic *pic_slave,
    t_port *port, const core_machine_fdc_config *config,
    const core_machine_fdc_terminal_observation_provider *observation_provider)
{
    if (fdc == LIB_NULL || drives == LIB_NULL || dma_request == LIB_NULL ||
        dma_request_assert == LIB_NULL || dma_request_deassert == LIB_NULL ||
        dma_request_owner == LIB_NULL || config == LIB_NULL) return;
    fdc->connect.media_registry = media_registry;
    fdc->connect.drives = *drives;
    fdc->connect.dma_request = *dma_request;
    fdc->connect.dma_request_assert = dma_request_assert;
    fdc->connect.dma_request_deassert = dma_request_deassert;
    fdc->connect.dma_request_owner = dma_request_owner;
    core_machine_pic_irq_source_bind(&fdc->connect.irq_source, pic_master,
        pic_slave, config->irq);
    fdc->connect.port = port;
    fdc->connect.config = *config;
    if (observation_provider != LIB_NULL) {
        fdc->connect.observation_provider = *observation_provider;
    }
}

void core_machine_fdc_initialize(core_machine_fdc *fdc)
{
    if (fdc == LIB_NULL || fdc->connect.port == LIB_NULL) return;
    lib_memory_set(&fdc->data, 0u, sizeof(fdc->data));
    fdc->data.ccr = VFDC_CCR_RATE_250;
    core_machine_fdc_observe_all_drives(fdc);
    core_machine_fdc_sample_ready(fdc);
    fdc->data.initial_media_baseline_pending = LIB_TRUE;
    core_machine_fdc_update_dir(fdc);
    core_machine_fdc_command_phase(fdc);
    core_machine_port_add_read(fdc->connect.port, fdc->connect.config.status_port,
        core_machine_fdc_read_status, fdc);
    core_machine_port_add_read(fdc->connect.port, fdc->connect.config.data_port,
        core_machine_fdc_read_data, fdc);
    if (fdc->connect.config.direction_port != 0u) {
        core_machine_port_add_read(fdc->connect.port, fdc->connect.config.direction_port,
            core_machine_fdc_read_direction, fdc);
    }
    if (fdc->connect.config.diagnostic_port != 0u) {
        core_machine_port_add_read(fdc->connect.port,
            fdc->connect.config.diagnostic_port, core_machine_fdc_read_diagnostic, fdc);
    }
    core_machine_port_add_write(fdc->connect.port, fdc->connect.config.dor_port,
        core_machine_fdc_write_dor, fdc);
    core_machine_port_add_write(fdc->connect.port, fdc->connect.config.data_port,
        core_machine_fdc_write_data, fdc);
    if (fdc->connect.config.control_port != 0u) {
        core_machine_port_add_write(fdc->connect.port, fdc->connect.config.control_port,
            core_machine_fdc_write_control, fdc);
    }
}

void core_machine_fdc_reset(core_machine_fdc *fdc)
{
    if (fdc == LIB_NULL) return;
    core_machine_fdc_deassert_dma(fdc);
    core_machine_pic_irq_source_deassert(&fdc->connect.irq_source);
    core_machine_fdc_reset_controller(fdc);
    if (fdc->data.initial_media_baseline_pending) {
        /* A session attaches its selected boot medium before its first
         * machine reset.  That construction-time state is not a hot swap. */
        core_machine_fdc_observe_all_drives(fdc);
        fdc->data.initial_media_baseline_pending = LIB_FALSE;
    }
}

void core_machine_fdc_advance(core_machine_fdc *fdc)
{
    if (fdc == LIB_NULL || fdc->data.elapsed_ticks == UINT64_MAX) return;
    core_machine_fdc_advance_at(fdc, fdc->data.elapsed_ticks + 1u);
}

void core_machine_fdc_advance_at(core_machine_fdc *fdc,
    lib_u64 elapsed_ticks)
{
    if (fdc == LIB_NULL) return;
    fdc->data.elapsed_ticks = elapsed_ticks;
    if (fdc->data.phase == core_machine_fdc_PHASE_PENDING_COMMAND) {
        core_machine_fdc_execute(fdc);
    } else if (fdc->data.phase == core_machine_fdc_PHASE_PENDING_COMPLETE) {
        core_machine_fdc_set_result(fdc, fdc->data.pending_st0 != 0u ?
            fdc->data.pending_st0 : (fdc->data.pending_st1 == 0u ?
            core_machine_fdc_ST0_NORMAL : core_machine_fdc_ST0_ABNORMAL),
            fdc->data.pending_st1, fdc->data.pending_st2);
        core_machine_fdc_result_phase(fdc, 7u);
        core_machine_fdc_publish_terminal_result(fdc);
        core_machine_fdc_raise_irq(fdc);
    }
    core_machine_fdc_publish_due_reset(fdc);
    for (lib_u8 drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive) {
        if (fdc->data.seek_pending[drive] && elapsed_ticks >= fdc->data.seek_due_tick[drive]) {
            fdc->data.seek_pending[drive] = LIB_FALSE;
            fdc->data.drive_cylinder[drive] = fdc->data.seek_target[drive];
            fdc->data.cylinder = fdc->data.seek_target[drive];
            core_machine_fdc_observe_drive(fdc, drive);
            /* uPD765 ST0 reserves Not Ready for Read/Write.  A SEEK still
             * completes with SEEK END when its selected unit is absent; the
             * controller records the requested PCN rather than inventing an
             * abnormal completion.  RECALIBRATE is the mechanical check
             * path: a missing Track 0 signal is Equipment Check. */
            fdc->data.seek_result_st0[fdc->data.seek_result_count] =
                ((fdc->data.cmd[0] & 0x1fu) == core_machine_fdc_CMD_SEEK ||
                core_machine_fdc_drive_mechanical_ready_for(fdc, drive)) ?
                core_machine_fdc_ST0_NORMAL |
                VFDC_ST0_SEEK_END | drive : core_machine_fdc_ST0_ABNORMAL |
                VFDC_ST0_SEEK_END | VFDC_ST0_EQUIPMENT_CHECK | drive;
            fdc->data.seek_result_cylinder[fdc->data.seek_result_count++] =
                (lib_u8)fdc->data.drive_cylinder[drive];
            core_machine_fdc_raise_irq(fdc);
        }
    }
    core_machine_fdc_publish_due_dma_byte(fdc);
    core_machine_fdc_publish_due_ndma_byte(fdc);
}

lib_status core_machine_fdc_next_due_tick(const core_machine_fdc *fdc,
    lib_u64 *out_due_tick)
{
    lib_u64 due_tick = UINT64_MAX;
    lib_u8 drive;

    if (fdc == LIB_NULL || out_due_tick == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (fdc->data.phase == core_machine_fdc_PHASE_PENDING_COMMAND ||
        fdc->data.phase == core_machine_fdc_PHASE_PENDING_COMPLETE) {
        *out_due_tick = fdc->data.elapsed_ticks;
        return LIB_STATUS_OK;
    }
    if (fdc->data.reset_pending && fdc->data.reset_due_tick < due_tick) {
        due_tick = fdc->data.reset_due_tick;
    }
    for (drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive) {
        if (fdc->data.seek_pending[drive] && fdc->data.seek_due_tick[drive] < due_tick) {
            due_tick = fdc->data.seek_due_tick[drive];
        }
    }
    if (fdc->data.dma_byte_gate_pending && fdc->data.next_dma_byte_tick < due_tick) {
        due_tick = fdc->data.next_dma_byte_tick;
    }
    if (fdc->data.ndma_byte_gate_pending && fdc->data.next_ndma_byte_tick < due_tick) {
        due_tick = fdc->data.next_ndma_byte_tick;
    }
    if (due_tick == UINT64_MAX) return LIB_STATUS_INVALID_STATE;
    *out_due_tick = due_tick;
    return LIB_STATUS_OK;
}

void core_machine_fdc_refresh(core_machine_fdc *fdc)
{
    core_machine_media_info info;
    core_machine_media_result result;
    lib_u8 drive;
    lib_u8 ready;

    if (fdc == LIB_NULL) return;
    for (drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive) {
        if (core_machine_fdc_drive_media_query(fdc, drive, &info, &result) &&
            fdc->data.observed_media_generation[drive] != info.generation) {
            fdc->data.media_changed[drive] = LIB_TRUE;
        }
        ready = core_machine_fdc_drive_media_ready(fdc, drive);
        if (fdc->data.ready_poll_enabled && ready != fdc->data.observed_ready[drive] &&
            !fdc->data.flagINTR) {
            fdc->data.observed_ready[drive] = ready;
            fdc->data.st0 = core_machine_fdc_ST0_READY_CHANGE |
                core_machine_fdc_ST0_NOT_READY | drive;
            core_machine_fdc_raise_irq(fdc);
        }
    }
    core_machine_fdc_update_dir(fdc);
}

void core_machine_fdc_finalize(core_machine_fdc *fdc)
{
    if (fdc == LIB_NULL) return;
    core_machine_fdc_deassert_dma(fdc);
    core_machine_pic_irq_source_deassert(&fdc->connect.irq_source);
    lib_memory_set(&fdc->data, 0u, sizeof(fdc->data));
    lib_memory_set(&fdc->connect, 0u, sizeof(fdc->connect));
}
