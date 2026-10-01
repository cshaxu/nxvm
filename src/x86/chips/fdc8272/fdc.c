/* Copyright 2012-2026 Neko. */
#include "x86/chips/fdc8272/fdc.h"

#define x86_fdc_CMD_SPECIFY 0x03u
#define x86_fdc_CMD_SENSE_DRIVE_STATUS 0x04u
#define x86_fdc_CMD_RECALIBRATE 0x07u
#define x86_fdc_CMD_SENSE_INTERRUPT 0x08u
#define x86_fdc_CMD_SEEK 0x0fu
#define x86_fdc_CMD_READ_ID 0x0au
#define x86_fdc_CMD_WRITE_DATA 0x05u
#define x86_fdc_CMD_READ_DATA 0x06u
#define x86_fdc_CMD_WRITE_DELETED_DATA 0x09u
#define x86_fdc_CMD_READ_DELETED_DATA 0x0cu
#define x86_fdc_CMD_SCAN_EQUAL 0x11u
#define x86_fdc_CMD_SCAN_LOW_OR_EQUAL 0x19u
#define x86_fdc_CMD_SCAN_HIGH_OR_EQUAL 0x1du
#define x86_fdc_CMD_FORMAT_TRACK 0x0du
#define x86_fdc_CMD_READ_TRACK 0x02u

#define x86_fdc_ST1_NO_DATA 0x04u
#define x86_fdc_ST1_NOT_WRITABLE 0x02u
#define x86_fdc_ST1_END_OF_CYLINDER 0x80u
#define x86_fdc_ST2_WRONG_CYLINDER 0x10u
#define x86_fdc_ST2_BAD_CYLINDER 0x02u

#define x86_fdc_SCAN_EQUAL 0u
#define x86_fdc_SCAN_LOW_OR_EQUAL 1u
#define x86_fdc_SCAN_HIGH_OR_EQUAL 2u


static void x86_fdc_set_irq(x86_fdc *fdc, lib_bool asserted)
{
    fdc->irq = asserted;
    fdc->connect.irq(fdc->connect.context, asserted);
}

static lib_bool x86_fdc_track_info(const x86_fdc *fdc,
    x86_fdc_track *out_track)
{
    return fdc->connect.track(fdc->connect.context, fdc->data.selected_drive,
        (fdc->data.cmd[0] & 0x40u) != 0u, out_track);
}

static x86_fdc_record x86_fdc_record_at(const x86_fdc *fdc)
{
    return (x86_fdc_record) {fdc->data.cylinder, fdc->data.head,
        fdc->data.sector, fdc->data.byte_offset};
}

static lib_bool x86_fdc_record_valid(const x86_fdc *fdc,
    const x86_fdc_record *record)
{
    x86_fdc_track track = {0};
    return x86_fdc_track_info(fdc, &track) && record->head < track.heads &&
        record->cylinder < track.cylinders && record->sector != 0u &&
        record->sector <= track.sectors && track.bytes_per_sector == 512u &&
        record->offset < 512u;
}

static lib_u8 x86_fdc_sector_size(lib_u8 code)
{
    return code == 2u ? 0x02u : 0xffu;
}

static lib_u8 x86_fdc_msr(const x86_fdc *fdc)
{
    lib_u8 value;
    lib_u8 drive;

    switch (fdc->data.phase) {
    case x86_fdc_PHASE_PENDING_COMMAND:
    case x86_fdc_PHASE_PENDING_COMPLETE:
    case x86_fdc_PHASE_EXECUTION_WRITE_TAIL:
        value = VFDC_MSR_CB; break;
    case x86_fdc_PHASE_RESULT:
        value = VFDC_MSR_RQM | VFDC_MSR_DIO | VFDC_MSR_CB; break;
    case x86_fdc_PHASE_EXECUTION_READ:
        value = fdc->data.flagNDMA && !fdc->data.ndma_byte_gate_pending ? VFDC_MSR_RQM | VFDC_MSR_DIO |
            VFDC_MSR_NDM | VFDC_MSR_CB : VFDC_MSR_CB; break;
    case x86_fdc_PHASE_EXECUTION_WRITE:
    case x86_fdc_PHASE_EXECUTION_SCAN:
    case x86_fdc_PHASE_EXECUTION_FORMAT:
        value = fdc->data.flagNDMA && !fdc->data.ndma_byte_gate_pending ? VFDC_MSR_RQM | VFDC_MSR_NDM |
            VFDC_MSR_CB : VFDC_MSR_CB; break;
    default:
        value = VFDC_MSR_RQM; break;
    }
    for (drive = 0u; drive < X86_FDC_DRIVE_COUNT; ++drive) {
        if (fdc->data.seek_pending[drive]) value |= VFDC_MSR_DB(drive);
    }
    return value;
}

static lib_i32 x86_fdc_drive_status_ready(const x86_fdc *fdc,
    lib_u8 drive)
{
    return fdc->connect.sample(fdc->connect.context, drive).ready;
}

static void x86_fdc_sample_ready(x86_fdc *fdc)
{
    lib_u8 drive;

    for (drive = 0u; drive < X86_FDC_DRIVE_COUNT; ++drive) {
        fdc->data.observed_ready[drive] = x86_fdc_drive_status_ready(fdc, drive);
    }
}

static void x86_fdc_deassert_dma(x86_fdc *fdc)
{
    fdc->drq = LIB_FALSE;
    fdc->connect.drq(fdc->connect.context, LIB_FALSE);
}

static lib_i32 x86_fdc_execution_active(const x86_fdc *fdc);

static void x86_fdc_request_assert(x86_fdc *fdc)
{
    fdc->drq = LIB_TRUE;
    fdc->connect.drq(fdc->connect.context, LIB_TRUE);
}

static lib_u64 x86_fdc_dma_byte_ticks(const x86_fdc *fdc)
{
    return (fdc->data.cmd[0] & 0x40u) != 0u ?
        fdc->timing.mfm_byte_ticks : fdc->timing.fm_byte_ticks;
}

static void x86_fdc_schedule_dma_byte(x86_fdc *fdc)
{
    lib_u64 ticks = x86_fdc_dma_byte_ticks(fdc);

    if (fdc == LIB_NULL || fdc->data.flagNDMA ||
        (fdc->data.phase != x86_fdc_PHASE_EXECUTION_READ &&
        fdc->data.phase != x86_fdc_PHASE_EXECUTION_WRITE &&
        fdc->data.phase != x86_fdc_PHASE_EXECUTION_WRITE_TAIL &&
        fdc->data.phase != x86_fdc_PHASE_EXECUTION_SCAN &&
        fdc->data.phase != x86_fdc_PHASE_EXECUTION_FORMAT)) return;
    x86_fdc_deassert_dma(fdc);
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

static lib_i32 x86_fdc_prepare_read_sector(x86_fdc *fdc);
static lib_i32 x86_fdc_transfer_byte(x86_fdc *fdc, lib_u8 *byte,
    lib_bool write_to_media);

static void x86_fdc_publish_due_dma_byte(x86_fdc *fdc)
{
    if (fdc == LIB_NULL || !fdc->data.dma_byte_gate_pending ||
        fdc->data.elapsed_ticks < fdc->data.next_dma_byte_tick ||
        !x86_fdc_execution_active(fdc)) return;
    fdc->data.dma_byte_gate_pending = LIB_FALSE;
    if (fdc->data.phase == x86_fdc_PHASE_EXECUTION_WRITE_TAIL) {
        lib_u8 zero = 0u;
        (void)x86_fdc_transfer_byte(fdc, &zero, LIB_TRUE);
        x86_fdc_schedule_dma_byte(fdc);
        return;
    }
    /* The completed DMA byte may assert TC before another sector is needed.
     * Inspect the next address mark only when publishing its first DRQ. */
    if (fdc->data.byte_offset == 0u &&
        (fdc->data.phase == x86_fdc_PHASE_EXECUTION_READ ||
        fdc->data.phase == x86_fdc_PHASE_EXECUTION_SCAN) &&
        !x86_fdc_prepare_read_sector(fdc)) return;
    x86_fdc_request_assert(fdc);
}

static void x86_fdc_schedule_ndma_byte(x86_fdc *fdc)
{
    lib_u64 ticks = x86_fdc_dma_byte_ticks(fdc);

    if (fdc == LIB_NULL || !fdc->data.flagNDMA || ticks == 0u ||
        (fdc->data.phase != x86_fdc_PHASE_EXECUTION_READ &&
        fdc->data.phase != x86_fdc_PHASE_EXECUTION_WRITE &&
        fdc->data.phase != x86_fdc_PHASE_EXECUTION_SCAN &&
        fdc->data.phase != x86_fdc_PHASE_EXECUTION_FORMAT)) return;
    fdc->data.next_ndma_byte_tick = fdc->data.elapsed_ticks + ticks;
    fdc->data.ndma_byte_gate_pending = LIB_TRUE;
}

static void x86_fdc_publish_due_ndma_byte(x86_fdc *fdc)
{
    if (fdc == LIB_NULL || !fdc->data.ndma_byte_gate_pending ||
        fdc->data.elapsed_ticks < fdc->data.next_ndma_byte_tick ||
        !x86_fdc_execution_active(fdc)) return;
    fdc->data.ndma_byte_gate_pending = LIB_FALSE;
}

static void x86_fdc_raise_irq(x86_fdc *fdc)
{
    if (!fdc->service_enabled) return;
    x86_fdc_set_irq(fdc, LIB_TRUE);
    fdc->data.flagINTR = LIB_TRUE;
}

static void x86_fdc_cancel_execution(x86_fdc *fdc)
{
    x86_fdc_deassert_dma(fdc);
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

static lib_i32 x86_fdc_execution_active(const x86_fdc *fdc)
{
    return fdc != LIB_NULL && (fdc->data.phase == x86_fdc_PHASE_EXECUTION_READ ||
        fdc->data.phase == x86_fdc_PHASE_EXECUTION_WRITE ||
        fdc->data.phase == x86_fdc_PHASE_EXECUTION_WRITE_TAIL ||
        fdc->data.phase == x86_fdc_PHASE_EXECUTION_SCAN ||
        fdc->data.phase == x86_fdc_PHASE_EXECUTION_FORMAT);
}

static void x86_fdc_command_phase(x86_fdc *fdc)
{
    fdc->data.phase = x86_fdc_PHASE_COMMAND;
    fdc->data.command_length = 0u;
    fdc->data.command_index = 0u;
    fdc->data.result_length = 0u;
    fdc->data.result_index = 0u;
    fdc->data.pending_st0 = 0u;
    fdc->data.pending_st1 = 0u;
    fdc->data.pending_st2 = 0u;
}

static void x86_fdc_result_phase(x86_fdc *fdc, lib_u8 length)
{
    x86_fdc_deassert_dma(fdc);
    fdc->data.phase = x86_fdc_PHASE_RESULT;
    fdc->data.result_length = length;
    fdc->data.result_index = 0u;
}

static void x86_fdc_set_result(x86_fdc *fdc, lib_u8 st0,
    lib_u8 st1, lib_u8 st2)
{
    /* Table 12: IC is separate from Seek End and from HD/US identity. */
    st0 |= fdc->data.cmd[1] & 0x07u;
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

static void x86_fdc_publish_terminal_result(x86_fdc *fdc)
{
    x86_fdc_terminal observation;
    if (fdc->connect.terminal == LIB_NULL) return;
    observation.command = fdc->data.cmd[0];
    observation.drive = fdc->data.selected_drive;
    lib_memory_copy(observation.result, fdc->data.ret, sizeof(observation.result));
    observation.successful = (fdc->data.st0 & 0xc0u) == x86_fdc_ST0_NORMAL &&
        fdc->data.st1 == 0u;
    fdc->connect.terminal(fdc->connect.context, &observation);
}

static void x86_fdc_complete_transfer_with_status(x86_fdc *fdc,
    lib_u8 st0, lib_u8 st1)
{
    if (st1 != 0u || (st0 & 0xc0u) != 0u)
        fdc->data.pending_st2 &= (lib_u8)~VFDC_ST2_SCAN_MATCH;
    x86_fdc_deassert_dma(fdc);
    /* A terminal DMA service may finish a command between byte gates.  The
     * result phase owns no future byte transfer, so it must not retain the
     * previous execution gate as a phantom deadline. */
    fdc->data.dma_byte_gate_pending = LIB_FALSE;
    fdc->data.next_dma_byte_tick = 0u;
    fdc->data.ndma_byte_gate_pending = LIB_FALSE;
    fdc->data.next_ndma_byte_tick = 0u;
    fdc->data.pending_st0 = st0;
    fdc->data.pending_st1 = st1;
    fdc->data.phase = x86_fdc_PHASE_PENDING_COMPLETE;
}

static void x86_fdc_complete_transfer(x86_fdc *fdc,
    lib_u8 st1)
{
    x86_fdc_complete_transfer_with_status(fdc, 0u, st1);
}

static void x86_fdc_complete_missing_input(x86_fdc *fdc)
{
    if (!x86_fdc_drive_status_ready(fdc, fdc->data.selected_drive)) {
        x86_fdc_complete_transfer_with_status(fdc,
            x86_fdc_ST0_ABNORMAL | x86_fdc_ST0_NOT_READY, 0u);
        return;
    }
    x86_fdc_complete_transfer(fdc, x86_fdc_ST1_NO_DATA);
}

static lib_bool x86_fdc_track_zero(const x86_fdc *fdc, lib_u8 unit)
{
    return fdc->connect.sample(fdc->connect.context,
        unit).track_zero;
}

static lib_u64 x86_fdc_step_ticks(const x86_fdc *fdc)
{
    lib_u64 value = (16u - fdc->data.srt) * fdc->timing.step_numerator;
    return value / fdc->timing.step_denominator +
        (value % fdc->timing.step_denominator != 0u);
}

static void x86_fdc_begin_seek(x86_fdc *fdc, lib_u16 target)
{
    lib_u8 drive = fdc->data.selected_drive;
    lib_bool complete;

    fdc->data.seek_target[drive] = target;
    fdc->data.seek_recalibrate[drive] =
        (fdc->data.cmd[0] & 0x1fu) == x86_fdc_CMD_RECALIBRATE;
    fdc->data.seek_head[drive] = fdc->data.cmd[1] & 0x04u;
    fdc->data.seek_steps[drive] = 0u;
    if (fdc->data.seek_recalibrate[drive]) fdc->data.pcn[drive] = 0u;
    complete = fdc->data.seek_recalibrate[drive] ?
        x86_fdc_track_zero(fdc, fdc->data.selected_drive) : fdc->data.pcn[drive] == target;
    fdc->data.seek_due_tick[drive] = fdc->data.elapsed_ticks +
        (complete || !x86_fdc_drive_status_ready(fdc, drive) ?
            0u : x86_fdc_step_ticks(fdc));
    fdc->data.seek_pending[drive] = LIB_TRUE;
    x86_fdc_command_phase(fdc);
}

static lib_i32 x86_fdc_drive_ready_for(const x86_fdc *fdc,
    lib_u8 drive)
{
    x86_fdc_pins pins = fdc->connect.sample(fdc->connect.context, drive);
    return fdc->reset_released && pins.ready && pins.data_available;
}

static lib_i32 x86_fdc_drive_ready(const x86_fdc *fdc)
{
    return x86_fdc_drive_ready_for(fdc, fdc->data.selected_drive);
}

static lib_u8 x86_fdc_sector_step(const x86_fdc *fdc)
{
    return fdc->data.phase == x86_fdc_PHASE_EXECUTION_SCAN &&
        fdc->data.cmd[8] == 2u ? 2u : 1u;
}

static void x86_fdc_advance_position(x86_fdc *fdc)
{
    fdc->data.byte_offset++;
    if (fdc->data.byte_offset < 512u) return;
    fdc->data.byte_offset = 0u;
    fdc->data.sector += x86_fdc_sector_step(fdc);
}

static void x86_fdc_complete_dma_data(x86_fdc *fdc)
{
    /* Intel Table 8 describes the ID after the final transferred sector,
     * including TC within that sector; it does not move the physical head. */
    if (fdc->data.byte_offset != 0u) ++fdc->data.sector;
    fdc->data.byte_offset = 0u;
    if (fdc->data.sector > fdc->data.eot) {
        fdc->data.sector = 1u;
        if ((fdc->data.cmd[0] & 0x80u) != 0u) {
            if ((fdc->data.head & 1u) != 0u) ++fdc->data.cylinder;
            fdc->data.head ^= 1u;
        } else ++fdc->data.cylinder;
    }
    x86_fdc_complete_transfer(fdc, 0u);
}

static lib_i32 x86_fdc_transfer_byte(x86_fdc *fdc, lib_u8 *byte,
    lib_bool write_to_media)
{
    x86_fdc_record record = x86_fdc_record_at(fdc);
    x86_fdc_record_result result;
    if (fdc->data.transfer_remaining == 0u) {
        x86_fdc_complete_transfer(fdc, x86_fdc_ST1_NO_DATA);
        return LIB_TRUE;
    }
    if (!x86_fdc_drive_ready(fdc)) {
        x86_fdc_complete_missing_input(fdc);
        return LIB_TRUE;
    }
    if (fdc->data.sector > fdc->data.eot || !x86_fdc_record_valid(fdc, &record)) {
        x86_fdc_complete_transfer(fdc, x86_fdc_ST1_END_OF_CYLINDER);
        return LIB_TRUE;
    }
    if (fdc->data.byte_offset == 0u && write_to_media) {
        result = fdc->connect.write_mark(fdc->connect.context, fdc->data.selected_drive,
            &record, fdc->data.transfer_write_deleted);
        if (result != X86_FDC_RECORD_OK) {
            x86_fdc_complete_transfer(fdc, result == X86_FDC_RECORD_PROTECTED ?
                x86_fdc_ST1_NOT_WRITABLE : x86_fdc_ST1_NO_DATA);
            return LIB_TRUE;
        }
    }
    result = write_to_media ?
        fdc->connect.write(fdc->connect.context, fdc->data.selected_drive, &record, *byte) :
        fdc->connect.read(fdc->connect.context, fdc->data.selected_drive, &record, byte);
    if (result != X86_FDC_RECORD_OK) {
        x86_fdc_complete_transfer(fdc, write_to_media && result == X86_FDC_RECORD_PROTECTED ?
            x86_fdc_ST1_NOT_WRITABLE : x86_fdc_ST1_NO_DATA);
        return LIB_TRUE;
    }
    fdc->data.transfer_remaining--;
    x86_fdc_advance_position(fdc);
    if (fdc->data.transfer_remaining == 0u) {
        if (fdc->data.phase == x86_fdc_PHASE_EXECUTION_WRITE_TAIL)
            x86_fdc_complete_dma_data(fdc);
        else x86_fdc_complete_transfer(fdc, 0u);
    } else if (!write_to_media && fdc->data.flagNDMA && fdc->data.byte_offset == 0u)
        (void)x86_fdc_prepare_read_sector(fdc);
    return LIB_FALSE;
}

static lib_i32 x86_fdc_prepare_read_sector(x86_fdc *fdc)
{
    x86_fdc_record record;
    lib_bool deleted;

    while (fdc->data.transfer_remaining != 0u) {
        record = x86_fdc_record_at(fdc);
        if (fdc->data.sector > fdc->data.eot || !x86_fdc_record_valid(fdc, &record)) {
            x86_fdc_complete_transfer(fdc, x86_fdc_ST1_END_OF_CYLINDER);
            return LIB_FALSE;
        }
        deleted = LIB_FALSE;
        if (fdc->connect.read_mark(fdc->connect.context, fdc->data.selected_drive,
                &record, &deleted) != X86_FDC_RECORD_OK) {
            x86_fdc_complete_transfer(fdc, x86_fdc_ST1_NO_DATA);
            return LIB_FALSE;
        }
        if (deleted ==
            fdc->data.transfer_expect_deleted) return LIB_TRUE;
        if (fdc->data.phase == x86_fdc_PHASE_EXECUTION_SCAN ||
            (fdc->data.cmd[0] & 0x20u) == 0u ||
            (fdc->data.cmd[0] & 0x1fu) == x86_fdc_CMD_READ_TRACK)
            fdc->data.pending_st2 |= VFDC_ST2_CONTROL_MARK;
        if ((fdc->data.cmd[0] & 0x1fu) == x86_fdc_CMD_READ_TRACK) return LIB_TRUE;
        if ((fdc->data.cmd[0] & 0x20u) == 0u) {
            /* READ/READ DELETED and SCAN end after this sector with SK=0. */
            fdc->data.transfer_remaining = 512u;
            return LIB_TRUE;
        }
        /* Skip without transferring payload or requesting a host byte. */
        fdc->data.transfer_remaining -= 512u;
        fdc->data.sector += x86_fdc_sector_step(fdc);
        fdc->data.scan_sector_satisfies = LIB_TRUE;
    }
    if (fdc->data.phase != x86_fdc_PHASE_EXECUTION_SCAN) {
        x86_fdc_complete_transfer(fdc, x86_fdc_ST1_END_OF_CYLINDER);
        return LIB_FALSE;
    }
    fdc->data.pending_st2 = (fdc->data.pending_st2 & ~VFDC_ST2_SCAN_MATCH) |
        VFDC_ST2_SCAN_MISMATCH;
    x86_fdc_complete_transfer(fdc,
        fdc->data.sector - x86_fdc_sector_step(fdc) == fdc->data.eot ?
        0u : x86_fdc_ST1_END_OF_CYLINDER);
    return LIB_FALSE;
}

static void x86_fdc_scan_byte(x86_fdc *fdc,
    lib_u8 compare_byte)
{
    lib_u8 media_byte;
    x86_fdc_record record = x86_fdc_record_at(fdc);
    lib_i32 byte_satisfies;

    if (!x86_fdc_drive_ready(fdc)) {
        x86_fdc_complete_missing_input(fdc);
        return;
    }
    if (fdc->data.transfer_remaining == 0u) {
        x86_fdc_complete_transfer(fdc, x86_fdc_ST1_NO_DATA);
        return;
    }
    if (fdc->data.sector > fdc->data.eot || !x86_fdc_record_valid(fdc, &record)) {
        x86_fdc_complete_transfer(fdc, x86_fdc_ST1_END_OF_CYLINDER);
        return;
    }
    if (fdc->connect.read(fdc->connect.context, fdc->data.selected_drive,
            &record, &media_byte) != X86_FDC_RECORD_OK) {
        x86_fdc_complete_transfer(fdc, x86_fdc_ST1_NO_DATA);
        return;
    }
    byte_satisfies = compare_byte == 0xffu;
    if (!byte_satisfies) {
        /* Table 10 compares disk against processor data. SH means equality,
         * not merely satisfaction of a Low/High-or-Equal command. */
        if (compare_byte != media_byte)
            fdc->data.pending_st2 &= (lib_u8)~VFDC_ST2_SCAN_MATCH;
        switch (fdc->data.scan_mode) {
        case x86_fdc_SCAN_EQUAL:
            byte_satisfies = compare_byte == media_byte;
            break;
        case x86_fdc_SCAN_LOW_OR_EQUAL:
            byte_satisfies = media_byte <= compare_byte;
            break;
        default:
            byte_satisfies = media_byte >= compare_byte;
            break;
        }
    }
    if (!byte_satisfies) fdc->data.scan_sector_satisfies = LIB_FALSE;
    fdc->data.transfer_remaining--;
    x86_fdc_advance_position(fdc);
    if (fdc->data.byte_offset != 0u) return;
    if (fdc->data.scan_sector_satisfies) {
        x86_fdc_complete_transfer(fdc, 0u);
    } else if (fdc->data.transfer_remaining == 0u) {
        fdc->data.pending_st2 |= VFDC_ST2_SCAN_MISMATCH;
        x86_fdc_complete_transfer(fdc,
            ((fdc->data.pending_st2 & VFDC_ST2_CONTROL_MARK) != 0u &&
                (fdc->data.cmd[0] & 0x20u) == 0u) ||
            fdc->data.sector - x86_fdc_sector_step(fdc) == fdc->data.eot ?
            0u : x86_fdc_ST1_END_OF_CYLINDER);
    } else {
        fdc->data.scan_sector_satisfies = LIB_TRUE;
        fdc->data.pending_st2 |= VFDC_ST2_SCAN_MATCH;
        if (fdc->data.flagNDMA) (void)x86_fdc_prepare_read_sector(fdc);
    }
}

static void x86_fdc_format_byte(x86_fdc *fdc, lib_u8 byte)
{
    x86_fdc_record record;
    x86_fdc_record_result result;
    if (fdc->data.format_headers_remaining == 0u) return;
    if (!x86_fdc_drive_ready(fdc)) {
        x86_fdc_complete_missing_input(fdc);
        return;
    }
    fdc->data.format_id[fdc->data.format_id_index++] = byte;
    if (fdc->data.format_id_index != 4u) return;
    fdc->data.format_id_index = 0u;
    record = (x86_fdc_record) {fdc->data.cylinder, fdc->data.head, fdc->data.format_id[2], 0u};
    if (fdc->data.format_id[0] != fdc->data.cylinder ||
        fdc->data.format_id[1] != fdc->data.head || fdc->data.format_id[3] != 2u ||
        !x86_fdc_record_valid(fdc, &record)) {
        x86_fdc_complete_transfer(fdc, x86_fdc_ST1_NO_DATA);
        return;
    }
    result = fdc->connect.format(fdc->connect.context, fdc->data.selected_drive,
        &record, fdc->data.cmd[5]);
    if (result != X86_FDC_RECORD_OK) {
        x86_fdc_complete_transfer(fdc, result == X86_FDC_RECORD_PROTECTED ?
            x86_fdc_ST1_NOT_WRITABLE : x86_fdc_ST1_NO_DATA);
        return;
    }
    fdc->data.format_headers_remaining--;
    fdc->data.sector = fdc->data.format_id[2];
    if (fdc->data.format_headers_remaining == 0u) x86_fdc_complete_transfer(fdc, 0u);
}

void x86_fdc_dma_read(x86_fdc *fdc, lib_u8 *byte)
{
    if (fdc != LIB_NULL && byte != LIB_NULL && fdc->data.phase == x86_fdc_PHASE_EXECUTION_READ) {
        (void)x86_fdc_transfer_byte(fdc, byte, LIB_FALSE);
        x86_fdc_schedule_dma_byte(fdc);
    }
}

void x86_fdc_dma_write(x86_fdc *fdc, lib_u8 value)
{
    if (fdc == LIB_NULL) return;
    if (fdc->data.phase == x86_fdc_PHASE_EXECUTION_WRITE) {
        (void)x86_fdc_transfer_byte(fdc, &value, LIB_TRUE);
        x86_fdc_schedule_dma_byte(fdc);
    } else if (fdc->data.phase == x86_fdc_PHASE_EXECUTION_SCAN) {
        x86_fdc_scan_byte(fdc, value);
        x86_fdc_schedule_dma_byte(fdc);
    } else if (fdc->data.phase == x86_fdc_PHASE_EXECUTION_FORMAT) {
        x86_fdc_format_byte(fdc, value);
        x86_fdc_schedule_dma_byte(fdc);
    }
}

void x86_fdc_terminal_count(x86_fdc *fdc)
{
    lib_u8 opcode;
    if (fdc == LIB_NULL) return;
    if (fdc->data.phase == x86_fdc_PHASE_EXECUTION_WRITE &&
        fdc->data.byte_offset != 0u) {
        /* Intel 6-232: TC stops host transfers, not the current data field.
         * Finish its zero tail through the same byte service, without DRQ. */
        fdc->data.phase = x86_fdc_PHASE_EXECUTION_WRITE_TAIL;
        fdc->data.transfer_remaining = 512u - fdc->data.byte_offset;
        x86_fdc_schedule_dma_byte(fdc);
        return;
    }
    opcode = fdc->data.cmd[0] & 0x1fu;
    if ((opcode == x86_fdc_CMD_READ_DATA ||
        opcode == x86_fdc_CMD_READ_DELETED_DATA ||
        opcode == x86_fdc_CMD_WRITE_DATA ||
        opcode == x86_fdc_CMD_WRITE_DELETED_DATA) &&
        (fdc->data.phase == x86_fdc_PHASE_EXECUTION_READ ||
        fdc->data.phase == x86_fdc_PHASE_EXECUTION_WRITE ||
        (fdc->data.phase == x86_fdc_PHASE_PENDING_COMPLETE &&
        fdc->data.transfer_remaining == 0u && fdc->data.pending_st0 == 0u &&
        fdc->data.pending_st1 == 0u && fdc->data.pending_st2 == 0u))) {
        x86_fdc_complete_dma_data(fdc);
        return;
    }
    if (fdc->data.phase == x86_fdc_PHASE_EXECUTION_READ ||
        fdc->data.phase == x86_fdc_PHASE_EXECUTION_SCAN ||
        fdc->data.phase == x86_fdc_PHASE_EXECUTION_FORMAT) {
        x86_fdc_complete_transfer(fdc, 0u);
    }
}

static lib_u8 x86_fdc_command_length(lib_u8 opcode)
{
    switch (opcode & 0x1fu) {
    case x86_fdc_CMD_SPECIFY: return 3u;
    case x86_fdc_CMD_SENSE_DRIVE_STATUS: return 2u;
    case x86_fdc_CMD_RECALIBRATE: return 2u;
    case x86_fdc_CMD_SENSE_INTERRUPT: return 1u;
    case x86_fdc_CMD_SEEK: return 3u;
    case x86_fdc_CMD_READ_ID: return 2u;
    case x86_fdc_CMD_WRITE_DATA:
    case x86_fdc_CMD_READ_DATA:
    case x86_fdc_CMD_WRITE_DELETED_DATA:
    case x86_fdc_CMD_READ_DELETED_DATA:
    case x86_fdc_CMD_SCAN_EQUAL:
    case x86_fdc_CMD_SCAN_LOW_OR_EQUAL:
    case x86_fdc_CMD_SCAN_HIGH_OR_EQUAL:
    case x86_fdc_CMD_READ_TRACK: return 9u;
    case x86_fdc_CMD_FORMAT_TRACK: return 6u;
    default: return 1u;
    }
}

static void x86_fdc_start_transfer(x86_fdc *fdc,
    x86_fdc_phase phase, lib_i32 deleted_data, lib_u8 scan_mode)
{
    lib_u8 size = x86_fdc_sector_size(fdc->data.cmd[5]);
    x86_fdc_track info = {0};
    fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
    fdc->data.cylinder = fdc->data.cmd[2];
    fdc->data.head = fdc->data.cmd[3];
    fdc->data.sector = fdc->data.cmd[4];
    fdc->data.eot = fdc->data.cmd[6];
    fdc->data.byte_offset = 0u;
    fdc->data.transfer_expect_deleted = phase == x86_fdc_PHASE_EXECUTION_READ &&
        deleted_data;
    fdc->data.transfer_write_deleted = phase == x86_fdc_PHASE_EXECUTION_WRITE &&
        deleted_data;
    fdc->data.scan_mode = scan_mode;
    fdc->data.scan_sector_satisfies = LIB_TRUE;
    if (!x86_fdc_drive_ready(fdc)) {
        x86_fdc_complete_missing_input(fdc);
        return;
    }
    if (size != 2u ||
        !x86_fdc_track_info(fdc, &info) || !info.id_readable ||
        fdc->data.head >= info.heads || fdc->data.sector == 0u ||
        fdc->data.sector > fdc->data.eot) {
        x86_fdc_complete_transfer(fdc, x86_fdc_ST1_NO_DATA);
        return;
    }
    /* Flat media IDs follow the physical track, not the command's C or PCN.
     * Intel 6-234: data commands do not perform an implied seek. */
    if (fdc->data.cylinder != info.cylinder) {
        fdc->data.cylinder = info.cylinder;
        fdc->data.pending_st2 |= x86_fdc_ST2_WRONG_CYLINDER;
        if (fdc->data.cylinder == 0xffu)
            fdc->data.pending_st2 |= x86_fdc_ST2_BAD_CYLINDER;
        x86_fdc_complete_transfer(fdc, x86_fdc_ST1_NO_DATA);
        return;
    }
    fdc->data.phase = phase;
    fdc->data.transfer_remaining = (lib_u32)((fdc->data.eot -
        fdc->data.sector) / x86_fdc_sector_step(fdc) + 1u) * 512u;
    if (phase == x86_fdc_PHASE_EXECUTION_SCAN) {
        fdc->data.pending_st2 |= VFDC_ST2_SCAN_MATCH;
    }
    if ((phase == x86_fdc_PHASE_EXECUTION_SCAN ||
        phase == x86_fdc_PHASE_EXECUTION_READ) &&
        !x86_fdc_prepare_read_sector(fdc)) return;
    if (!fdc->data.flagNDMA && fdc->service_enabled) {
        x86_fdc_request_assert(fdc);
    }
}

static void x86_fdc_start_read_track(x86_fdc *fdc)
{
    x86_fdc_track info = {0};
    fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
    fdc->data.cylinder = fdc->data.cmd[2];
    fdc->data.head = fdc->data.cmd[3];
    fdc->data.sector = fdc->data.cmd[4];
    fdc->data.eot = fdc->data.cmd[6];
    fdc->data.byte_offset = 0u;
    if (!x86_fdc_drive_ready(fdc)) {
        x86_fdc_complete_missing_input(fdc);
        return;
    }
    if (fdc->data.cmd[0] != 0x42u || fdc->data.flagNDMA ||
        x86_fdc_sector_size(fdc->data.cmd[5]) != 2u ||
        fdc->data.selected_drive != 0u ||
        !x86_fdc_track_info(fdc, &info) || !info.id_readable ||
        fdc->data.head >= info.heads ||
        fdc->data.cylinder >= info.cylinders ||
        fdc->data.cylinder != info.cylinder ||
        fdc->data.sector != 1u ||
        fdc->data.eot != info.sectors) {
        x86_fdc_complete_transfer(fdc, x86_fdc_ST1_NO_DATA);
        return;
    }
    fdc->data.transfer_remaining = (lib_u32)
        info.sectors * 512u;
    fdc->data.transfer_expect_deleted = LIB_FALSE;
    fdc->data.phase = x86_fdc_PHASE_EXECUTION_READ;
    if (!x86_fdc_prepare_read_sector(fdc)) return;
    if (fdc->service_enabled) {
        x86_fdc_request_assert(fdc);
    }
}

static void x86_fdc_execute(x86_fdc *fdc)
{
    lib_u8 opcode = fdc->data.cmd[0] & 0x1fu;
    x86_fdc_track info = {0};
    lib_i32 media_ok;

    /* Intel requires SIS after seek/recalibrate completion. Until then no
     * new command may overwrite a result. While seeking, each unit has one
     * operation: reject a duplicate rather than replacing its target/deadline.
     * Together these admission rules bound outstanding results to four. */
    if ((opcode != x86_fdc_CMD_SENSE_INTERRUPT &&
            fdc->data.seek_result_count != 0u) ||
        ((opcode == x86_fdc_CMD_SEEK ||
            opcode == x86_fdc_CMD_RECALIBRATE) &&
            fdc->data.seek_pending[fdc->data.cmd[1] & 3u])) {
        fdc->data.ret[0] = 0x80u;
        x86_fdc_result_phase(fdc, 1u);
        return;
    }

    /* A command that changes controller activity supersedes reset's stale
     * Sense Interrupt notifications.  Sense Interrupt itself is the sole
     * command allowed to drain them. */
    if (opcode != x86_fdc_CMD_SENSE_INTERRUPT &&
        fdc->data.reset_sense_mask != 0u) {
        fdc->data.reset_sense_mask = 0u;
        fdc->data.reset_pending = LIB_FALSE;
        x86_fdc_set_irq(fdc, LIB_FALSE);
    }
    switch (opcode) {
    case x86_fdc_CMD_SPECIFY:
        fdc->data.hut = fdc->data.cmd[1] & 0x0fu;
        fdc->data.srt = fdc->data.cmd[1] >> 4u;
        fdc->data.hlt = fdc->data.cmd[2] >> 1u;
        fdc->data.flagNDMA = (fdc->data.cmd[2] & 1u) != 0u;
        fdc->data.ready_poll_enabled = LIB_TRUE;
        x86_fdc_command_phase(fdc);
        break;
    case x86_fdc_CMD_SENSE_DRIVE_STATUS:
        fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
        fdc->data.head = (fdc->data.cmd[1] >> 2u) & 1u;
        {
            x86_fdc_pins pins = fdc->connect.sample(fdc->connect.context, fdc->data.selected_drive);
            fdc->data.st3 = (fdc->data.selected_drive & 3u) | (fdc->data.head << 2u) |
                (pins.write_protected ? 0x40u : 0u) | (pins.ready ? 0x20u : 0u) |
                (pins.two_sided ? 0x08u : 0u) | (pins.track_zero ? 0x10u : 0u) |
                (pins.fault ? 0x80u : 0u);
        }
        fdc->data.ret[0] = fdc->data.st3;
        x86_fdc_result_phase(fdc, 1u);
        break;
    case x86_fdc_CMD_RECALIBRATE:
        fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
        fdc->data.head = 0u; fdc->data.sector = 1u;
        x86_fdc_begin_seek(fdc, 0u);
        break;
    case x86_fdc_CMD_SENSE_INTERRUPT:
        if (fdc->data.reset_sense_mask != 0u) {
            lib_u8 drive = 0u;

            while ((fdc->data.reset_sense_mask & (1u << drive)) == 0u) ++drive;

            fdc->data.ret[0] = x86_fdc_ST0_READY_CHANGE | drive;
            fdc->data.ret[1] = fdc->data.pcn[drive];
            fdc->data.reset_sense_mask &= (lib_u8)~(1u << drive);
            fdc->data.flagINTR = LIB_FALSE;
            x86_fdc_set_irq(fdc, LIB_FALSE);
        } else if (fdc->data.seek_result_count != 0u) {
            lib_u8 index;

            fdc->data.ret[0] = fdc->data.seek_result_st0[0u];
            fdc->data.ret[1] = fdc->data.seek_result_cylinder[0u];
            for (index = 1u; index < fdc->data.seek_result_count; ++index) {
                fdc->data.seek_result_st0[index - 1u] = fdc->data.seek_result_st0[index];
                fdc->data.seek_result_cylinder[index - 1u] = fdc->data.seek_result_cylinder[index];
            }
            fdc->data.seek_result_count--;
            fdc->data.flagINTR = fdc->data.seek_result_count != 0u ||
                fdc->data.ready_sense_mask != 0u;
            if (!fdc->data.flagINTR) x86_fdc_set_irq(fdc, LIB_FALSE);
        } else if (fdc->data.ready_sense_mask != 0u) {
            lib_u8 drive = 0u;
            while ((fdc->data.ready_sense_mask & (1u << drive)) == 0u) ++drive;
            fdc->data.ret[0] = x86_fdc_ST0_READY_CHANGE | drive |
                (fdc->data.observed_ready[drive] ? 0u : x86_fdc_ST0_NOT_READY);
            fdc->data.ret[1] = fdc->data.pcn[drive];
            fdc->data.ready_sense_mask &= (lib_u8)~(1u << drive);
            fdc->data.flagINTR = fdc->data.ready_sense_mask != 0u;
            if (!fdc->data.flagINTR) x86_fdc_set_irq(fdc, LIB_FALSE);
        } else {
            fdc->data.ret[0] = 0x80u;
            /* No pending cause: reference-model invalid-command result. */
            x86_fdc_result_phase(fdc, 1u);
            break;
        }
        x86_fdc_result_phase(fdc, 2u);
        break;
    case x86_fdc_CMD_SEEK:
        fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
        fdc->data.head = (fdc->data.cmd[1] >> 2u) & 1u;
        fdc->data.sector = 1u;
        x86_fdc_begin_seek(fdc, fdc->data.cmd[2]);
        break;
    case x86_fdc_CMD_READ_ID:
        fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
        media_ok = x86_fdc_track_info(fdc, &info);
        fdc->data.cylinder = info.cylinder;
        fdc->data.head = (fdc->data.cmd[1] >> 2u) & 1u;
        if (!x86_fdc_drive_ready(fdc) || !media_ok) {
            x86_fdc_complete_missing_input(fdc);
        } else if (!info.id_readable || fdc->data.head >= info.heads) {
            x86_fdc_complete_transfer(fdc, x86_fdc_ST1_NO_DATA);
        } else {
            if (fdc->data.sector == 0u) fdc->data.sector = 1u;
            x86_fdc_set_result(fdc, x86_fdc_ST0_NORMAL, 0u, 0u);
            x86_fdc_result_phase(fdc, 7u);
            x86_fdc_publish_terminal_result(fdc);
            x86_fdc_raise_irq(fdc);
        }
        break;
    case x86_fdc_CMD_READ_DATA:
        x86_fdc_start_transfer(fdc, x86_fdc_PHASE_EXECUTION_READ,
            LIB_FALSE, 0u);
        break;
    case x86_fdc_CMD_READ_TRACK:
        x86_fdc_start_read_track(fdc);
        break;
    case x86_fdc_CMD_WRITE_DATA:
        x86_fdc_start_transfer(fdc, x86_fdc_PHASE_EXECUTION_WRITE,
            LIB_FALSE, 0u);
        break;
    case x86_fdc_CMD_READ_DELETED_DATA:
        x86_fdc_start_transfer(fdc, x86_fdc_PHASE_EXECUTION_READ,
            LIB_TRUE, 0u);
        break;
    case x86_fdc_CMD_WRITE_DELETED_DATA:
        x86_fdc_start_transfer(fdc, x86_fdc_PHASE_EXECUTION_WRITE,
            LIB_TRUE, 0u);
        break;
    case x86_fdc_CMD_SCAN_EQUAL:
        x86_fdc_start_transfer(fdc, x86_fdc_PHASE_EXECUTION_SCAN,
            LIB_FALSE, x86_fdc_SCAN_EQUAL);
        break;
    case x86_fdc_CMD_SCAN_LOW_OR_EQUAL:
        x86_fdc_start_transfer(fdc, x86_fdc_PHASE_EXECUTION_SCAN,
            LIB_FALSE, x86_fdc_SCAN_LOW_OR_EQUAL);
        break;
    case x86_fdc_CMD_SCAN_HIGH_OR_EQUAL:
        x86_fdc_start_transfer(fdc, x86_fdc_PHASE_EXECUTION_SCAN,
            LIB_FALSE, x86_fdc_SCAN_HIGH_OR_EQUAL);
        break;
    case x86_fdc_CMD_FORMAT_TRACK:
        fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
        media_ok = x86_fdc_track_info(fdc, &info);
        fdc->data.cylinder = info.cylinder;
        fdc->data.head = (fdc->data.cmd[1] >> 2u) & 1u;
        fdc->data.sector = 1u;
        fdc->data.eot = fdc->data.cmd[3];
        fdc->data.format_headers_remaining = fdc->data.cmd[3];
        fdc->data.format_id_index = 0u;
        if (!x86_fdc_drive_ready(fdc)) {
            x86_fdc_complete_missing_input(fdc);
            break;
        }
        if (x86_fdc_sector_size(fdc->data.cmd[2]) != 2u ||
            fdc->data.eot == 0u ||
            !media_ok || !info.id_readable ||
            fdc->data.eot > info.sectors) {
            x86_fdc_complete_transfer(fdc, x86_fdc_ST1_NO_DATA);
        } else {
            fdc->data.phase = x86_fdc_PHASE_EXECUTION_FORMAT;
            if (!fdc->data.flagNDMA && fdc->service_enabled) {
                x86_fdc_request_assert(fdc);
            }
        }
        break;
    default:
        fdc->data.ret[0] = 0x80u;
        x86_fdc_result_phase(fdc, 1u);
        break;
    }
}

static void x86_fdc_reset_controller(x86_fdc *fdc)
{
    lib_u8 hut = fdc->data.hut, hlt = fdc->data.hlt, srt = fdc->data.srt;
    lib_u64 elapsed_ticks = fdc->data.elapsed_ticks;
    x86_fdc_cancel_execution(fdc);
    lib_memory_set(&fdc->data, 0u, sizeof(fdc->data));
    fdc->data.hut = hut;
    fdc->data.hlt = hlt;
    fdc->data.srt = srt;
    fdc->data.elapsed_ticks = elapsed_ticks;
    x86_fdc_sample_ready(fdc);
    x86_fdc_command_phase(fdc);
}

static void x86_fdc_publish_due_reset(x86_fdc *fdc)
{
    if (fdc == LIB_NULL || !fdc->data.reset_pending ||
        fdc->data.elapsed_ticks < fdc->data.reset_due_tick) return;
    fdc->data.reset_pending = LIB_FALSE;
    if (fdc->data.reset_sense_mask != 0u) x86_fdc_raise_irq(fdc);
}

static void x86_fdc_schedule_reset_completion(x86_fdc *fdc)
{
    /* Reset completion queues one Sense Interrupt Status result for every
     * controller drive select.  This is controller state, not a sample of
     * the board READY inputs: system firmware drains all four results even
     * when fewer mechanical drives are fitted. */
    fdc->data.reset_sense_mask = !(x86_fdc_drive_status_ready(fdc, 0u) ||
        x86_fdc_drive_status_ready(fdc, 1u) ||
        x86_fdc_drive_status_ready(fdc, 2u) ||
        x86_fdc_drive_status_ready(fdc, 3u)) ? 0u :
        (lib_u8)((1u << X86_FDC_DRIVE_COUNT) - 1u);
    fdc->data.reset_due_tick = fdc->data.elapsed_ticks +
        fdc->timing.reset_ticks;
    fdc->data.reset_pending = fdc->data.reset_sense_mask != 0u;
    x86_fdc_publish_due_reset(fdc);
}

lib_u8 x86_fdc_read_status(const x86_fdc *fdc)
{
    return fdc == LIB_NULL ? 0u : x86_fdc_msr(fdc);
}

void x86_fdc_read_data(x86_fdc *fdc, lib_u8 *io_byte)
{
    lib_u8 value;
    if (fdc == LIB_NULL || io_byte == LIB_NULL) return;
    if (fdc->data.phase == x86_fdc_PHASE_RESULT) {
        if (fdc->data.result_index == 0u && fdc->data.flagINTR &&
            (fdc->data.cmd[0] & 0x1fu) != x86_fdc_CMD_SENSE_INTERRUPT &&
            (fdc->data.cmd[0] & 0x1fu) != x86_fdc_CMD_SENSE_DRIVE_STATUS) {
            fdc->data.flagINTR = fdc->data.seek_result_count != 0u ||
                fdc->data.ready_sense_mask != 0u;
            if (!fdc->data.flagINTR) x86_fdc_set_irq(fdc, LIB_FALSE);
        }
        *io_byte = fdc->data.ret[fdc->data.result_index++];
        if (fdc->data.result_index >= fdc->data.result_length) x86_fdc_command_phase(fdc);
    } else if (fdc->data.phase == x86_fdc_PHASE_EXECUTION_READ &&
        fdc->data.flagNDMA && !fdc->data.ndma_byte_gate_pending) {
        value = 0u;
        (void)x86_fdc_transfer_byte(fdc, &value, LIB_FALSE);
        x86_fdc_schedule_ndma_byte(fdc);
        *io_byte = value;
    }
}

void x86_fdc_write_data(x86_fdc *fdc, lib_u8 byte)
{
    if (fdc == LIB_NULL) return;
    if (fdc->data.phase == x86_fdc_PHASE_EXECUTION_WRITE && fdc->data.flagNDMA &&
        !fdc->data.ndma_byte_gate_pending) {
        (void)x86_fdc_transfer_byte(fdc, &byte, LIB_TRUE);
        x86_fdc_schedule_ndma_byte(fdc);
        return;
    }
    if (fdc->data.phase == x86_fdc_PHASE_EXECUTION_SCAN && fdc->data.flagNDMA &&
        !fdc->data.ndma_byte_gate_pending) {
        x86_fdc_scan_byte(fdc, byte);
        x86_fdc_schedule_ndma_byte(fdc);
        return;
    }
    if (fdc->data.phase == x86_fdc_PHASE_EXECUTION_FORMAT && fdc->data.flagNDMA &&
        !fdc->data.ndma_byte_gate_pending) {
        x86_fdc_format_byte(fdc, byte);
        x86_fdc_schedule_ndma_byte(fdc);
        return;
    }
    if (fdc->data.phase != x86_fdc_PHASE_COMMAND) return;
    if (fdc->data.command_index == 0u) {
        fdc->data.command_length = x86_fdc_command_length(
            byte);
    }
    if (fdc->data.command_index >= sizeof(fdc->data.cmd)) return;
    fdc->data.cmd[fdc->data.command_index++] = byte;
    if (fdc->data.command_index == fdc->data.command_length) {
        fdc->data.phase = x86_fdc_PHASE_PENDING_COMMAND;
    }
}

void x86_fdc_reset(x86_fdc *fdc)
{
    if (fdc == LIB_NULL) return;
    x86_fdc_set_irq(fdc, LIB_FALSE);
    x86_fdc_reset_controller(fdc);
    fdc->reset_released = LIB_FALSE;
    fdc->service_enabled = LIB_FALSE;
}

static void x86_fdc_finish_seek(x86_fdc *fdc,
    lib_u8 drive, lib_u8 status)
{
    const lib_u8 index = fdc->data.seek_result_count++;
    fdc->data.seek_pending[drive] = LIB_FALSE;
    fdc->data.seek_result_st0[index] = status | VFDC_ST0_SEEK_END |
        fdc->data.seek_head[drive] | drive;
    fdc->data.seek_result_cylinder[index] = fdc->data.pcn[drive];
    x86_fdc_raise_irq(fdc);
}

static void x86_fdc_step_drive(x86_fdc *fdc,
    lib_u8 unit, lib_bool outward)
{
    fdc->connect.step(fdc->connect.context, unit, outward);
}

static void x86_fdc_advance_seeks(x86_fdc *fdc)
{
    for (lib_u8 drive = 0u; drive < X86_FDC_DRIVE_COUNT; ++drive) {
        if (fdc->data.seek_pending[drive] &&
            !x86_fdc_drive_status_ready(fdc, drive)) {
            x86_fdc_finish_seek(fdc, drive,
                x86_fdc_ST0_ABNORMAL | x86_fdc_ST0_NOT_READY);
        }
    }
    for (;;) {
        lib_u8 next = X86_FDC_DRIVE_COUNT;
        lib_u64 due = LIB_UINT64_MAX;
        lib_bool complete;
        for (lib_u8 drive = 0u; drive < X86_FDC_DRIVE_COUNT; ++drive) {
            if (fdc->data.seek_pending[drive] && fdc->data.seek_due_tick[drive] <=
                fdc->data.elapsed_ticks && (next == X86_FDC_DRIVE_COUNT ||
                fdc->data.seek_due_tick[drive] < due)) {
                next = drive;
                due = fdc->data.seek_due_tick[drive];
            }
        }
        if (next == X86_FDC_DRIVE_COUNT) return;
        complete = fdc->data.seek_recalibrate[next] ? x86_fdc_track_zero(fdc, next) :
            fdc->data.pcn[next] == fdc->data.seek_target[next];
        if (!complete) {
            const lib_bool outward = fdc->data.seek_recalibrate[next] ||
                fdc->data.pcn[next] > fdc->data.seek_target[next];
            x86_fdc_step_drive(fdc, next, outward);
            ++fdc->data.seek_steps[next];
            if (!fdc->data.seek_recalibrate[next]) {
                if (outward) --fdc->data.pcn[next];
                else ++fdc->data.pcn[next];
            }
            complete = fdc->data.seek_recalibrate[next] ? x86_fdc_track_zero(fdc, next) :
                fdc->data.pcn[next] == fdc->data.seek_target[next];
        }
        if (complete) x86_fdc_finish_seek(fdc, next, x86_fdc_ST0_NORMAL);
        else if (fdc->data.seek_recalibrate[next] && fdc->data.seek_steps[next] == 77u)
            x86_fdc_finish_seek(fdc, next,
                x86_fdc_ST0_ABNORMAL | VFDC_ST0_EQUIPMENT_CHECK);
        else fdc->data.seek_due_tick[next] = due + x86_fdc_step_ticks(fdc);
    }
}

void x86_fdc_advance_at(x86_fdc *fdc,
    lib_u64 elapsed_ticks)
{
    if (fdc == LIB_NULL) return;
    fdc->data.elapsed_ticks = elapsed_ticks;
    if (fdc->data.phase == x86_fdc_PHASE_PENDING_COMMAND) {
        x86_fdc_execute(fdc);
    } else if (fdc->data.phase == x86_fdc_PHASE_PENDING_COMPLETE) {
        x86_fdc_set_result(fdc, fdc->data.pending_st0 != 0u ?
            fdc->data.pending_st0 : (fdc->data.pending_st1 == 0u ?
            x86_fdc_ST0_NORMAL : x86_fdc_ST0_ABNORMAL),
            fdc->data.pending_st1, fdc->data.pending_st2);
        x86_fdc_result_phase(fdc, 7u);
        x86_fdc_publish_terminal_result(fdc);
        x86_fdc_raise_irq(fdc);
    }
    x86_fdc_publish_due_reset(fdc);
    if (x86_fdc_execution_active(fdc) && !x86_fdc_drive_ready(fdc))
        x86_fdc_complete_missing_input(fdc);
    x86_fdc_advance_seeks(fdc);
    x86_fdc_publish_due_dma_byte(fdc);
    x86_fdc_publish_due_ndma_byte(fdc);
}

lib_status x86_fdc_next_due_tick(const x86_fdc *fdc,
    lib_u64 *out_due_tick)
{
    lib_u64 due_tick = LIB_UINT64_MAX;
    lib_u8 drive;

    if (fdc == LIB_NULL || out_due_tick == LIB_NULL) {
        return LIB_STATUS_INVALID_ARGUMENT;
    }
    if (fdc->data.phase == x86_fdc_PHASE_PENDING_COMMAND ||
        fdc->data.phase == x86_fdc_PHASE_PENDING_COMPLETE) {
        *out_due_tick = fdc->data.elapsed_ticks;
        return LIB_STATUS_OK;
    }
    if (fdc->data.reset_pending && fdc->data.reset_due_tick < due_tick) {
        due_tick = fdc->data.reset_due_tick;
    }
    for (drive = 0u; drive < X86_FDC_DRIVE_COUNT; ++drive) {
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
    if (due_tick == LIB_UINT64_MAX) return LIB_STATUS_INVALID_STATE;
    *out_due_tick = due_tick;
    return LIB_STATUS_OK;
}

void x86_fdc_poll_ready(x86_fdc *fdc)
{
    if (fdc == LIB_NULL) return;
    for (lib_u8 drive = 0u; drive < X86_FDC_DRIVE_COUNT; ++drive) {
        lib_bool ready = x86_fdc_drive_status_ready(fdc, drive);
        if (fdc->data.ready_poll_enabled && ready != fdc->data.observed_ready[drive] &&
            (fdc->data.ready_sense_mask & (1u << drive)) == 0u) {
            fdc->data.observed_ready[drive] = ready;
            fdc->data.ready_sense_mask |= (lib_u8)(1u << drive);
            x86_fdc_raise_irq(fdc);
        }
    }
}

void x86_fdc_refresh(x86_fdc *fdc)
{
    if (fdc != LIB_NULL && fdc->reset_released && x86_fdc_execution_active(fdc) &&
        !x86_fdc_drive_ready(fdc)) x86_fdc_complete_missing_input(fdc);
}

lib_status x86_fdc_set_timing(x86_fdc *fdc, const x86_fdc_timing *timing)
{
    if (fdc == LIB_NULL || timing == LIB_NULL || timing->step_denominator == 0u ||
        timing->step_numerator > LIB_UINT64_MAX / 16u) return LIB_STATUS_INVALID_ARGUMENT;
    fdc->timing = *timing;
    return LIB_STATUS_OK;
}

lib_status x86_fdc_create(const x86_fdc_connection *connection,
    const x86_fdc_timing *timing, x86_fdc **out_fdc)
{
    x86_fdc *fdc;
    lib_status status;
    if (out_fdc == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_fdc = LIB_NULL;
    if (connection == LIB_NULL || connection->sample == LIB_NULL ||
        connection->step == LIB_NULL || connection->track == LIB_NULL ||
        connection->read == LIB_NULL || connection->write == LIB_NULL ||
        connection->read_mark == LIB_NULL || connection->write_mark == LIB_NULL ||
        connection->format == LIB_NULL || connection->irq == LIB_NULL ||
        connection->drq == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    fdc = lib_allocate_zero(1u, sizeof(*fdc));
    if (fdc == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    status = x86_fdc_set_timing(fdc, timing);
    if (status != LIB_STATUS_OK) {
        lib_release(fdc);
        return status;
    }
    fdc->connect = *connection;
    x86_fdc_sample_ready(fdc);
    x86_fdc_command_phase(fdc);
    *out_fdc = fdc;
    return LIB_STATUS_OK;
}

void x86_fdc_set_reset(x86_fdc *fdc, lib_bool released)
{
    lib_bool old;
    if (fdc == LIB_NULL) return;
    old = fdc->reset_released;
    if (!released || !old) {
        x86_fdc_set_irq(fdc, LIB_FALSE);
        x86_fdc_reset_controller(fdc);
    }
    fdc->reset_released = released;
    if (released && !old) x86_fdc_schedule_reset_completion(fdc);
}

void x86_fdc_set_service_enabled(x86_fdc *fdc, lib_bool enabled)
{
    if (fdc != LIB_NULL) fdc->service_enabled = enabled;
}

void x86_fdc_destroy(x86_fdc *fdc)
{
    if (fdc == LIB_NULL) return;
    x86_fdc_deassert_dma(fdc);
    x86_fdc_set_irq(fdc, LIB_FALSE);
    lib_release(fdc);
}

lib_status x86_fdc_capture(const x86_fdc *fdc, x86_fdc_observation *out_observation)
{
    if (fdc == LIB_NULL || out_observation == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_observation = (x86_fdc_observation) {
        .elapsed_ticks = fdc->data.elapsed_ticks,
        .reset_due_tick = fdc->data.reset_due_tick,
        .next_dma_byte_tick = fdc->data.next_dma_byte_tick,
        .transfer_remaining = fdc->data.transfer_remaining,
        .cylinder = fdc->data.cylinder, .head = fdc->data.head,
        .sector = fdc->data.sector, .eot = fdc->data.eot,
        .phase = (lib_u8)fdc->data.phase, .msr = x86_fdc_msr(fdc),
        .command_index = fdc->data.command_index,
        .result_length = fdc->data.result_length, .result_index = fdc->data.result_index,
        .st0 = fdc->data.st0, .st1 = fdc->data.st1,
        .st2 = fdc->data.st2, .st3 = fdc->data.st3,
        .reset_sense_mask = fdc->data.reset_sense_mask,
        .seek_result_count = fdc->data.seek_result_count,
        .reset_pending = fdc->data.reset_pending,
        .dma_byte_gate_pending = fdc->data.dma_byte_gate_pending,
        .interrupt_pending = fdc->data.flagINTR,
        .irq = fdc->irq, .drq = fdc->drq
    };
    lib_memory_copy(out_observation->command, fdc->data.cmd, sizeof(out_observation->command));
    lib_memory_copy(out_observation->result, fdc->data.ret, sizeof(out_observation->result));
    lib_memory_copy(out_observation->pcn, fdc->data.pcn, sizeof(out_observation->pcn));
    return LIB_STATUS_OK;
}
