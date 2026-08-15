/* Copyright 2012-2026 Neko. */

/* The neutral 8272A-compatible controller. Core owns DMA/PIC/RAM; this file
 * owns only the guest-visible floppy-controller state machine. */

#include "type.h"

#include "core/machine/dma.h"
#include "core/machine/media_interface.h"
#include "core/machine/pic.h"
#include "core/machine/port.h"
#include "core/machine/fdc.h"

#define core_machine_fdc_CMD_SPECIFY 0x03u
#define core_machine_fdc_CMD_SENSE_DRIVE_STATUS 0x04u
#define core_machine_fdc_CMD_RECALIBRATE 0x07u
#define core_machine_fdc_CMD_SENSE_INTERRUPT 0x08u
#define core_machine_fdc_CMD_SEEK 0x0fu
#define core_machine_fdc_CMD_READ_ID 0x0au
#define core_machine_fdc_CMD_WRITE_DATA 0x05u
#define core_machine_fdc_CMD_READ_DATA 0x06u
#define core_machine_fdc_CMD_FORMAT_TRACK 0x0du
#define core_machine_fdc_CMD_READ_TRACK 0x02u

#define core_machine_fdc_ST1_NO_DATA 0x04u
#define core_machine_fdc_ST1_NOT_WRITABLE 0x02u
#define core_machine_fdc_ST1_END_OF_CYLINDER 0x80u

static type_unsigned_8 core_machine_fdc_sector_size(type_unsigned_8 code)
{
    return code == 2u ? 0x02u : 0xffu;
}

static type_unsigned_8 core_machine_fdc_msr(const core_machine_fdc *fdc)
{
    switch (fdc->data.phase) {
    case core_machine_fdc_PHASE_PENDING_COMMAND:
    case core_machine_fdc_PHASE_PENDING_SEEK:
    case core_machine_fdc_PHASE_PENDING_COMPLETE:
        return VFDC_MSR_CB;
    case core_machine_fdc_PHASE_RESULT:
        return VFDC_MSR_RQM | VFDC_MSR_DIO | VFDC_MSR_CB;
    case core_machine_fdc_PHASE_EXECUTION_READ:
        return fdc->data.flagNDMA && !fdc->data.ndma_byte_gate_pending ? VFDC_MSR_RQM | VFDC_MSR_DIO |
            VFDC_MSR_NDM | VFDC_MSR_CB : VFDC_MSR_CB;
    case core_machine_fdc_PHASE_EXECUTION_WRITE:
    case core_machine_fdc_PHASE_EXECUTION_FORMAT:
        return fdc->data.flagNDMA && !fdc->data.ndma_byte_gate_pending ? VFDC_MSR_RQM | VFDC_MSR_NDM |
            VFDC_MSR_CB : VFDC_MSR_CB;
    default:
        return VFDC_MSR_RQM;
    }
}

static core_machine_media_id core_machine_fdc_drive_media_id(
    const core_machine_fdc *fdc, type_unsigned_8 drive)
{
    return fdc == STD_NULL || drive >= CORE_MACHINE_FDC_DRIVE_COUNT ?
        CORE_MACHINE_MEDIA_ID_INVALID :
        fdc->connect.drives.media_id[drive];
}

static core_machine_media_id core_machine_fdc_selected_media_id(
    const core_machine_fdc *fdc)
{
    return fdc == STD_NULL ? CORE_MACHINE_MEDIA_ID_INVALID :
        core_machine_fdc_drive_media_id(fdc, fdc->data.selected_drive);
}

static C_INT core_machine_fdc_drive_media_query(const core_machine_fdc *fdc,
    type_unsigned_8 drive, core_machine_media_info *out_info,
    core_machine_media_result *out_result)
{
    core_machine_media_id media_id = core_machine_fdc_drive_media_id(fdc, drive);

    return fdc != STD_NULL && fdc->connect.media_registry != STD_NULL &&
        media_id != CORE_MACHINE_MEDIA_ID_INVALID &&
        core_machine_media_query(fdc->connect.media_registry, media_id, out_info,
            out_result) == TYPE_STATUS_OK;
}

static C_INT core_machine_fdc_media_info(const core_machine_fdc *fdc,
    core_machine_media_info *out_info, core_machine_media_result *out_result)
{
    return core_machine_fdc_drive_media_query(fdc, fdc->data.selected_drive,
        out_info, out_result) &&
        *out_result == CORE_MACHINE_MEDIA_RESULT_OK;
}

static C_INT core_machine_fdc_drive_media_ready(const core_machine_fdc *fdc,
    type_unsigned_8 drive)
{
    core_machine_media_info info;
    core_machine_media_result result;

    return core_machine_fdc_drive_media_query(fdc, drive, &info, &result) &&
        result == CORE_MACHINE_MEDIA_RESULT_OK && info.present;
}

static C_VOID core_machine_fdc_sample_ready(core_machine_fdc *fdc)
{
    type_unsigned_8 drive;

    for (drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive) {
        fdc->data.observed_ready[drive] = core_machine_fdc_drive_media_ready(fdc, drive);
    }
}

static C_VOID core_machine_fdc_update_dir(core_machine_fdc *fdc)
{
    type_unsigned_8 drive;

    if (fdc == STD_NULL) return;
    drive = fdc->data.dor & VFDC_DOR_DS;
    if (fdc->data.media_changed[drive]) {
        fdc->data.dir |= VFDC_DIR_DC;
    } else {
        fdc->data.dir &= (type_unsigned_8)~VFDC_DIR_DC;
    }
}

static C_VOID core_machine_fdc_observe_drive(core_machine_fdc *fdc,
    type_unsigned_8 drive)
{
    core_machine_media_info info;
    core_machine_media_result result;

    if (!core_machine_fdc_drive_media_query(fdc, drive, &info, &result)) return;
    fdc->data.observed_media_generation[drive] = info.generation;
    fdc->data.media_changed[drive] = TYPE_FALSE;
    core_machine_fdc_update_dir(fdc);
}

static C_VOID core_machine_fdc_observe_all_drives(core_machine_fdc *fdc)
{
    type_unsigned_8 drive;

    for (drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive) {
        core_machine_fdc_observe_drive(fdc, drive);
    }
}

static C_INT core_machine_fdc_media_offset(const core_machine_fdc *fdc,
    const core_machine_media_info *info, type_unsigned_64 *out_offset)
{
    type_unsigned_64 sector;

    if (fdc == STD_NULL || info == STD_NULL || out_offset == STD_NULL ||
        fdc->data.head >= info->geometry.heads ||
        fdc->data.cylinder >= info->geometry.cylinders ||
        fdc->data.sector == 0u ||
        fdc->data.sector > info->geometry.sectors_per_track ||
        info->geometry.bytes_per_sector != 512u) return TYPE_FALSE;
    sector = ((type_unsigned_64)fdc->data.cylinder * info->geometry.heads +
        fdc->data.head) * info->geometry.sectors_per_track +
        (fdc->data.sector - 1u);
    *out_offset = sector * info->geometry.bytes_per_sector + fdc->data.byte_offset;
    return TYPE_TRUE;
}

static C_VOID core_machine_fdc_deassert_dma(core_machine_fdc *fdc)
{
    if (fdc->connect.dma_request_deassert != STD_NULL) {
        fdc->connect.dma_request_deassert(fdc->connect.dma_request_owner,
            &fdc->connect.dma_request);
    }
}

static C_INT core_machine_fdc_execution_active(const core_machine_fdc *fdc);

static C_VOID core_machine_fdc_request_assert(core_machine_fdc *fdc)
{
    if (fdc->connect.dma_request_assert != STD_NULL) {
        fdc->connect.dma_request_assert(fdc->connect.dma_request_owner,
            &fdc->connect.dma_request);
    }
}

static type_unsigned_64 core_machine_fdc_dma_byte_ticks(const core_machine_fdc *fdc)
{
    return fdc != STD_NULL && fdc->data.ccr == 0u ?
        CORE_MACHINE_FDC_500K_BYTE_TICKS : 0u;
}

static C_VOID core_machine_fdc_schedule_dma_byte(core_machine_fdc *fdc)
{
    type_unsigned_64 ticks = core_machine_fdc_dma_byte_ticks(fdc);

    if (fdc == STD_NULL || fdc->data.flagNDMA || ticks == 0u ||
        (fdc->data.phase != core_machine_fdc_PHASE_EXECUTION_READ &&
        fdc->data.phase != core_machine_fdc_PHASE_EXECUTION_WRITE &&
        fdc->data.phase != core_machine_fdc_PHASE_EXECUTION_FORMAT)) return;
    core_machine_fdc_deassert_dma(fdc);
    fdc->data.next_dma_byte_tick = fdc->data.elapsed_ticks + ticks;
    fdc->data.dma_byte_gate_pending = TYPE_TRUE;
}

static C_VOID core_machine_fdc_publish_due_dma_byte(core_machine_fdc *fdc)
{
    if (fdc == STD_NULL || !fdc->data.dma_byte_gate_pending ||
        fdc->data.elapsed_ticks < fdc->data.next_dma_byte_tick ||
        !core_machine_fdc_execution_active(fdc)) return;
    fdc->data.dma_byte_gate_pending = TYPE_FALSE;
    core_machine_fdc_request_assert(fdc);
}

static C_VOID core_machine_fdc_schedule_ndma_byte(core_machine_fdc *fdc)
{
    type_unsigned_64 ticks = core_machine_fdc_dma_byte_ticks(fdc);

    if (fdc == STD_NULL || !fdc->data.flagNDMA || ticks == 0u ||
        (fdc->data.phase != core_machine_fdc_PHASE_EXECUTION_READ &&
        fdc->data.phase != core_machine_fdc_PHASE_EXECUTION_WRITE &&
        fdc->data.phase != core_machine_fdc_PHASE_EXECUTION_FORMAT)) return;
    fdc->data.next_ndma_byte_tick = fdc->data.elapsed_ticks + ticks;
    fdc->data.ndma_byte_gate_pending = TYPE_TRUE;
}

static C_VOID core_machine_fdc_publish_due_ndma_byte(core_machine_fdc *fdc)
{
    if (fdc == STD_NULL || !fdc->data.ndma_byte_gate_pending ||
        fdc->data.elapsed_ticks < fdc->data.next_ndma_byte_tick ||
        !core_machine_fdc_execution_active(fdc)) return;
    fdc->data.ndma_byte_gate_pending = TYPE_FALSE;
}

static C_VOID core_machine_fdc_raise_irq(core_machine_fdc *fdc)
{
    if ((fdc->data.dor & VFDC_DOR_ENRQ) == 0u) return;
    core_machine_pic_irq_source_assert(&fdc->connect.irq_source);
    fdc->data.flagINTR = TYPE_TRUE;
}

static C_VOID core_machine_fdc_cancel_execution(core_machine_fdc *fdc)
{
    core_machine_fdc_deassert_dma(fdc);
    fdc->data.dma_byte_gate_pending = TYPE_FALSE;
    fdc->data.next_dma_byte_tick = 0u;
    fdc->data.ndma_byte_gate_pending = TYPE_FALSE;
    fdc->data.next_ndma_byte_tick = 0u;
    fdc->data.pending_st1 = 0u;
    fdc->data.transfer_remaining = 0u;
    fdc->data.byte_offset = 0u;
    fdc->data.format_headers_remaining = 0u;
}

static C_INT core_machine_fdc_execution_active(const core_machine_fdc *fdc)
{
    return fdc != STD_NULL && (fdc->data.phase == core_machine_fdc_PHASE_PENDING_COMMAND ||
        fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_READ ||
        fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_WRITE ||
        fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_FORMAT ||
        fdc->data.phase == core_machine_fdc_PHASE_PENDING_COMPLETE);
}

static C_VOID core_machine_fdc_command_phase(core_machine_fdc *fdc)
{
    fdc->data.phase = core_machine_fdc_PHASE_COMMAND;
    fdc->data.command_length = 0u;
    fdc->data.command_index = 0u;
    fdc->data.result_length = 0u;
    fdc->data.result_index = 0u;
    fdc->data.pending_st1 = 0u;
}

static C_VOID core_machine_fdc_result_phase(core_machine_fdc *fdc, type_unsigned_8 length)
{
    core_machine_fdc_deassert_dma(fdc);
    fdc->data.phase = core_machine_fdc_PHASE_RESULT;
    fdc->data.result_length = length;
    fdc->data.result_index = 0u;
}

static C_VOID core_machine_fdc_set_result(core_machine_fdc *fdc, type_unsigned_8 st0,
    type_unsigned_8 st1, type_unsigned_8 st2)
{
    fdc->data.st0 = st0;
    fdc->data.st1 = st1;
    fdc->data.st2 = st2;
    fdc->data.ret[0] = st0;
    fdc->data.ret[1] = st1;
    fdc->data.ret[2] = st2;
    fdc->data.ret[3] = (type_unsigned_8)fdc->data.cylinder;
    fdc->data.ret[4] = (type_unsigned_8)fdc->data.head;
    fdc->data.ret[5] = (type_unsigned_8)fdc->data.sector;
    fdc->data.ret[6] = 0x02u;
}

static C_VOID core_machine_fdc_complete_transfer(core_machine_fdc *fdc,
    type_unsigned_8 st1)
{
    core_machine_fdc_deassert_dma(fdc);
    fdc->data.pending_st1 = st1;
    fdc->data.phase = core_machine_fdc_PHASE_PENDING_COMPLETE;
}

static C_VOID core_machine_fdc_complete_simple(core_machine_fdc *fdc, type_unsigned_8 st0,
    type_unsigned_8 cylinder)
{
    fdc->data.ret[0] = st0;
    fdc->data.ret[1] = cylinder;
    fdc->data.st0 = st0;
    core_machine_fdc_result_phase(fdc, 2u);
}

static C_VOID core_machine_fdc_begin_seek(core_machine_fdc *fdc, type_unsigned_16 target)
{
    type_unsigned_16 current = fdc->data.drive_cylinder[fdc->data.selected_drive];
    type_unsigned_16 distance = current > target ? current - target : target - current;

    fdc->data.seek_target = target;
    fdc->data.seek_due_tick = fdc->data.elapsed_ticks +
        (type_unsigned_64)distance * CORE_MACHINE_FDC_SEEK_TRACK_TICKS;
    fdc->data.phase = core_machine_fdc_PHASE_PENDING_SEEK;
}

static C_INT core_machine_fdc_drive_ready(const core_machine_fdc *fdc)
{
    return fdc->data.selected_drive == (fdc->data.dor & VFDC_DOR_DS) &&
        (fdc->data.dor & VFDC_DOR_NRS) != 0u &&
        (fdc->data.dor & VFDC_DOR_ME(fdc->data.selected_drive)) != 0u &&
        core_machine_fdc_drive_media_ready(fdc, fdc->data.selected_drive);
}

static C_VOID core_machine_fdc_advance_position(core_machine_fdc *fdc)
{
    fdc->data.byte_offset++;
    if (fdc->data.byte_offset < 512u) return;
    fdc->data.byte_offset = 0u;
    fdc->data.sector++;
}

static C_INT core_machine_fdc_transfer_byte(core_machine_fdc *fdc, t_latch *latch,
    C_INT write_to_media)
{
    type_unsigned_8 byte;
    core_machine_media_info info;
    core_machine_media_result result;
    core_machine_media_id media_id;
    type_unsigned_64 offset;
    if (fdc->data.transfer_remaining == 0u || !core_machine_fdc_drive_ready(fdc)) {
        core_machine_fdc_complete_transfer(fdc, core_machine_fdc_ST1_NO_DATA);
        return TYPE_TRUE;
    }
    if (!core_machine_fdc_media_info(fdc, &info, &result) ||
        fdc->data.sector > fdc->data.eot ||
        !core_machine_fdc_media_offset(fdc, &info, &offset)) {
        core_machine_fdc_complete_transfer(fdc, core_machine_fdc_ST1_END_OF_CYLINDER);
        return TYPE_TRUE;
    }
    media_id = core_machine_fdc_selected_media_id(fdc);
    if (write_to_media) {
        if (core_machine_media_write_bytes(fdc->connect.media_registry,
            media_id, offset, &latch->data.byte, 1u, &result) !=
            TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK) {
            core_machine_fdc_complete_transfer(fdc,
                result == CORE_MACHINE_MEDIA_RESULT_READ_ONLY ?
                core_machine_fdc_ST1_NOT_WRITABLE : core_machine_fdc_ST1_NO_DATA);
            return TYPE_TRUE;
        }
    } else if (core_machine_media_read_bytes(fdc->connect.media_registry,
        media_id, offset, &byte, 1u, &result) != TYPE_STATUS_OK ||
        result != CORE_MACHINE_MEDIA_RESULT_OK) {
        core_machine_fdc_complete_transfer(fdc, core_machine_fdc_ST1_NO_DATA);
        return TYPE_TRUE;
    } else {
        latch->data.byte = byte;
    }
    fdc->data.transfer_remaining--;
    core_machine_fdc_advance_position(fdc);
    if (fdc->data.transfer_remaining == 0u) core_machine_fdc_complete_transfer(fdc, 0u);
    return TYPE_FALSE;
}

static C_VOID core_machine_fdc_format_byte(core_machine_fdc *fdc, type_unsigned_8 byte)
{
    core_machine_media_info info;
    core_machine_media_result result;
    core_machine_media_id media_id;
    type_unsigned_64 logical_sector;
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
    logical_sector = ((type_unsigned_64)fdc->data.cylinder * info.geometry.heads +
        fdc->data.head) * info.geometry.sectors_per_track +
        fdc->data.format_id[2] - 1u;
    media_id = core_machine_fdc_selected_media_id(fdc);
    if (core_machine_media_format_sectors(fdc->connect.media_registry,
        media_id, logical_sector, 1u, fdc->data.cmd[5], &result) !=
        TYPE_STATUS_OK || result != CORE_MACHINE_MEDIA_RESULT_OK) {
        core_machine_fdc_complete_transfer(fdc,
            result == CORE_MACHINE_MEDIA_RESULT_READ_ONLY ?
            core_machine_fdc_ST1_NOT_WRITABLE : core_machine_fdc_ST1_NO_DATA);
        return;
    }
    fdc->data.format_headers_remaining--;
    fdc->data.sector = fdc->data.format_id[2];
    if (fdc->data.format_headers_remaining == 0u) core_machine_fdc_complete_transfer(fdc, 0u);
}

static C_VOID core_machine_fdc_dma_read(C_VOID *owner, t_latch *latch)
{
    core_machine_fdc *fdc = owner;
    if (fdc != STD_NULL && fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_READ) {
        (C_VOID)core_machine_fdc_transfer_byte(fdc, latch, TYPE_FALSE);
        core_machine_fdc_schedule_dma_byte(fdc);
    }
}

static C_VOID core_machine_fdc_dma_write(C_VOID *owner, t_latch *latch)
{
    core_machine_fdc *fdc = owner;
    if (fdc == STD_NULL) return;
    if (fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_WRITE) {
        (C_VOID)core_machine_fdc_transfer_byte(fdc, latch, TYPE_TRUE);
        core_machine_fdc_schedule_dma_byte(fdc);
    } else if (fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_FORMAT) {
        core_machine_fdc_format_byte(fdc, latch->data.byte);
        core_machine_fdc_schedule_dma_byte(fdc);
    }
}

static C_VOID core_machine_fdc_dma_terminal(C_VOID *owner, t_latch *latch)
{
    core_machine_fdc *fdc = owner;
    (C_VOID)latch;
    if (fdc != STD_NULL && (fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_READ ||
        fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_WRITE ||
        fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_FORMAT)) {
        core_machine_fdc_complete_transfer(fdc, fdc->data.transfer_remaining == 0u &&
            fdc->data.format_headers_remaining == 0u ? 0u : core_machine_fdc_ST1_NO_DATA);
    }
}

static const core_machine_dma_channel_provider core_machine_fdc_dma_channel = {
    core_machine_fdc_dma_read, core_machine_fdc_dma_write, core_machine_fdc_dma_terminal
};

const core_machine_dma_channel_provider *core_machine_fdc_dma_provider(C_VOID)
{ return &core_machine_fdc_dma_channel; }

static type_unsigned_8 core_machine_fdc_command_length(type_unsigned_8 opcode)
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
    case core_machine_fdc_CMD_READ_TRACK: return 9u;
    case core_machine_fdc_CMD_FORMAT_TRACK: return 6u;
    default: return 1u;
    }
}

static C_VOID core_machine_fdc_start_transfer(core_machine_fdc *fdc, C_INT write_to_media)
{
    type_unsigned_8 size = core_machine_fdc_sector_size(fdc->data.cmd[5]);
    core_machine_media_info info;
    core_machine_media_result result;
    fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
    fdc->data.cylinder = fdc->data.cmd[2];
    fdc->data.head = fdc->data.cmd[3];
    fdc->data.sector = fdc->data.cmd[4];
    fdc->data.eot = fdc->data.cmd[6];
    fdc->data.byte_offset = 0u;
    if (size != 2u || (fdc->data.ccr != 0u && fdc->data.ccr != VFDC_CCR_DRC) ||
        !core_machine_fdc_drive_ready(fdc) ||
        !core_machine_fdc_media_info(fdc, &info, &result) ||
        fdc->data.head >= info.geometry.heads || fdc->data.sector == 0u ||
        fdc->data.sector > fdc->data.eot || fdc->data.eot > info.geometry.sectors_per_track) {
        core_machine_fdc_complete_transfer(fdc, core_machine_fdc_ST1_NO_DATA);
        return;
    }
    fdc->data.transfer_remaining = (type_unsigned_32)(fdc->data.eot -
        fdc->data.sector + 1u) * 512u;
    fdc->data.phase = write_to_media ? core_machine_fdc_PHASE_EXECUTION_WRITE :
        core_machine_fdc_PHASE_EXECUTION_READ;
    if (!fdc->data.flagNDMA && (fdc->data.dor & VFDC_DOR_ENRQ) != 0u) {
        core_machine_fdc_request_assert(fdc);
    }
}

static C_VOID core_machine_fdc_start_read_track(core_machine_fdc *fdc)
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
        (fdc->data.ccr != 0u && fdc->data.ccr != VFDC_CCR_DRC) ||
        !core_machine_fdc_drive_ready(fdc) || fdc->data.selected_drive != 0u ||
        !core_machine_fdc_media_info(fdc, &info, &result) ||
        fdc->data.head >= info.geometry.heads ||
        fdc->data.cylinder >= info.geometry.cylinders ||
        fdc->data.sector != 1u ||
        fdc->data.eot != info.geometry.sectors_per_track) {
        core_machine_fdc_complete_transfer(fdc, core_machine_fdc_ST1_NO_DATA);
        return;
    }
    fdc->data.transfer_remaining = (type_unsigned_32)
        info.geometry.sectors_per_track * 512u;
    fdc->data.phase = core_machine_fdc_PHASE_EXECUTION_READ;
    if ((fdc->data.dor & VFDC_DOR_ENRQ) != 0u) {
        core_machine_fdc_request_assert(fdc);
    }
}

static C_VOID core_machine_fdc_execute(core_machine_fdc *fdc)
{
    type_unsigned_8 opcode = fdc->data.cmd[0] & 0x1fu;
    core_machine_media_info info;
    core_machine_media_result media_result;
    C_INT media_ok;
    switch (opcode) {
    case core_machine_fdc_CMD_SPECIFY:
        fdc->data.hut = fdc->data.cmd[1] & 0x0fu;
        fdc->data.srt = fdc->data.cmd[1] >> 4u;
        fdc->data.hlt = fdc->data.cmd[2] >> 1u;
        fdc->data.flagNDMA = (fdc->data.cmd[2] & 1u) != 0u;
        fdc->data.ready_poll_enabled = TYPE_TRUE;
        core_machine_fdc_command_phase(fdc);
        break;
    case core_machine_fdc_CMD_SENSE_DRIVE_STATUS:
        fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
        fdc->data.head = (fdc->data.cmd[1] >> 2u) & 1u;
        media_ok = core_machine_fdc_media_info(fdc, &info, &media_result);
        fdc->data.st3 = (fdc->data.selected_drive & 3u) |
            (fdc->data.head << 2u) | (media_ok &&
            (info.capabilities & CORE_MACHINE_MEDIA_CAPABILITY_READ_ONLY) != 0u ? 0x40u : 0u) | (core_machine_fdc_drive_ready(fdc) ?
            0x20u : 0u) | (fdc->data.cylinder == 0u ? 0x10u : 0u);
        fdc->data.ret[0] = fdc->data.st3;
        core_machine_fdc_result_phase(fdc, 1u);
        break;
    case core_machine_fdc_CMD_RECALIBRATE:
        fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
        fdc->data.head = 0u; fdc->data.sector = 1u;
        core_machine_fdc_begin_seek(fdc, 0u);
        break;
    case core_machine_fdc_CMD_SENSE_INTERRUPT:
        if (fdc->data.flagINTR) {
            fdc->data.ret[0] = fdc->data.st0;
            fdc->data.ret[1] = (type_unsigned_8)fdc->data.cylinder;
            fdc->data.flagINTR = TYPE_FALSE;
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
            core_machine_fdc_raise_irq(fdc);
        }
        break;
    case core_machine_fdc_CMD_READ_DATA:
        core_machine_fdc_start_transfer(fdc, TYPE_FALSE);
        break;
    case core_machine_fdc_CMD_READ_TRACK:
        core_machine_fdc_start_read_track(fdc);
        break;
    case core_machine_fdc_CMD_WRITE_DATA:
        core_machine_fdc_start_transfer(fdc, TYPE_TRUE);
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
            (fdc->data.ccr != 0u && fdc->data.ccr != VFDC_CCR_DRC) ||
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

static C_VOID core_machine_fdc_reset_controller(core_machine_fdc *fdc)
{
    type_unsigned_8 ccr = fdc->data.ccr;
    type_unsigned_64 observed_media_generation[CORE_MACHINE_FDC_DRIVE_COUNT];
    STD_MEMCPY(observed_media_generation, fdc->data.observed_media_generation,
        sizeof(observed_media_generation));
    core_machine_fdc_cancel_execution(fdc);
    STD_MEMSET(&fdc->data, TYPE_ZERO_8, sizeof(fdc->data));
    fdc->data.ccr = ccr;
    STD_MEMCPY(fdc->data.observed_media_generation, observed_media_generation,
        sizeof(observed_media_generation));
    core_machine_fdc_sample_ready(fdc);
    core_machine_fdc_command_phase(fdc);
}

static C_VOID core_machine_fdc_read_status(t_port *port, type_unsigned_16 id,
    C_VOID *owner)
{
    core_machine_fdc *fdc = owner; (C_VOID)port; (C_VOID)id;
    fdc->connect.port->data.ioByte = core_machine_fdc_msr(fdc);
}

static C_VOID core_machine_fdc_read_data(t_port *port, type_unsigned_16 id,
    C_VOID *owner)
{
    core_machine_fdc *fdc = owner; t_latch latch;
    (C_VOID)port; (C_VOID)id;
    if (fdc->data.phase == core_machine_fdc_PHASE_RESULT) {
        fdc->connect.port->data.ioByte = fdc->data.ret[fdc->data.result_index++];
        if (fdc->data.result_index >= fdc->data.result_length) core_machine_fdc_command_phase(fdc);
    } else if (fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_READ &&
        fdc->data.flagNDMA && !fdc->data.ndma_byte_gate_pending) {
        latch.data.byte = 0u;
        (C_VOID)core_machine_fdc_transfer_byte(fdc, &latch, TYPE_FALSE);
        core_machine_fdc_schedule_ndma_byte(fdc);
        fdc->connect.port->data.ioByte = latch.data.byte;
    }
}

static C_VOID core_machine_fdc_read_direction(t_port *port, type_unsigned_16 id,
    C_VOID *owner)
{
    core_machine_fdc *fdc = owner; (C_VOID)port; (C_VOID)id;
    fdc->connect.port->data.ioByte = fdc->data.dir;
}

static C_VOID core_machine_fdc_write_dor(t_port *port, type_unsigned_16 id,
    C_VOID *owner)
{
    core_machine_fdc *fdc = owner;
    type_unsigned_8 dor = fdc->connect.port->data.ioByte;
    (C_VOID)port; (C_VOID)id;
    if ((dor & VFDC_DOR_NRS) == 0u) {
        core_machine_pic_irq_source_deassert(&fdc->connect.irq_source);
        fdc->data.flagINTR = TYPE_FALSE;
        core_machine_fdc_reset_controller(fdc);
    }
    fdc->data.dor = dor;
    core_machine_fdc_update_dir(fdc);
    if ((dor & VFDC_DOR_NRS) != 0u && core_machine_fdc_execution_active(fdc) &&
        !core_machine_fdc_drive_ready(fdc)) {
        core_machine_fdc_cancel_execution(fdc);
        core_machine_fdc_command_phase(fdc);
    }
}

static C_VOID core_machine_fdc_write_data(t_port *port, type_unsigned_16 id,
    C_VOID *owner)
{
    core_machine_fdc *fdc = owner; t_latch latch;
    (C_VOID)port; (C_VOID)id;
    if (fdc->data.phase == core_machine_fdc_PHASE_EXECUTION_WRITE && fdc->data.flagNDMA &&
        !fdc->data.ndma_byte_gate_pending) {
        latch.data.byte = fdc->connect.port->data.ioByte;
        (C_VOID)core_machine_fdc_transfer_byte(fdc, &latch, TYPE_TRUE);
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

static C_VOID core_machine_fdc_write_control(t_port *port, type_unsigned_16 id,
    C_VOID *owner)
{
    core_machine_fdc *fdc = owner; (C_VOID)port; (C_VOID)id;
    fdc->data.ccr = fdc->connect.port->data.ioByte & 0x03u;
}

C_VOID core_machine_fdc_connect(core_machine_fdc *fdc,
    const core_machine_media_registry *media_registry,
    const core_machine_fdc_drive_bindings *drives,
    const core_machine_dma_request_binding *dma_request,
    core_machine_fdc_dma_request_operation dma_request_assert,
    core_machine_fdc_dma_request_operation dma_request_deassert,
    C_VOID *dma_request_owner, t_pic *pic_master, t_pic *pic_slave,
    t_port *port, const core_machine_fdc_config *config)
{
    if (fdc == STD_NULL || drives == STD_NULL || dma_request == STD_NULL ||
        dma_request_assert == STD_NULL || dma_request_deassert == STD_NULL ||
        dma_request_owner == STD_NULL || config == STD_NULL) return;
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
}

C_VOID core_machine_fdc_initialize(core_machine_fdc *fdc)
{
    if (fdc == STD_NULL || fdc->connect.port == STD_NULL) return;
    STD_MEMSET(&fdc->data, TYPE_ZERO_8, sizeof(fdc->data));
    fdc->data.ccr = VFDC_CCR_DRC;
    core_machine_fdc_observe_all_drives(fdc);
    core_machine_fdc_sample_ready(fdc);
    core_machine_fdc_update_dir(fdc);
    core_machine_fdc_command_phase(fdc);
    core_machine_port_add_read(fdc->connect.port, fdc->connect.config.status_port,
        core_machine_fdc_read_status, fdc);
    core_machine_port_add_read(fdc->connect.port, fdc->connect.config.data_port,
        core_machine_fdc_read_data, fdc);
    core_machine_port_add_read(fdc->connect.port, fdc->connect.config.direction_port,
        core_machine_fdc_read_direction, fdc);
    core_machine_port_add_write(fdc->connect.port, fdc->connect.config.dor_port,
        core_machine_fdc_write_dor, fdc);
    core_machine_port_add_write(fdc->connect.port, fdc->connect.config.data_port,
        core_machine_fdc_write_data, fdc);
    core_machine_port_add_write(fdc->connect.port, fdc->connect.config.control_port,
        core_machine_fdc_write_control, fdc);
}

C_VOID core_machine_fdc_reset(core_machine_fdc *fdc)
{
    if (fdc == STD_NULL) return;
    core_machine_fdc_deassert_dma(fdc);
    core_machine_pic_irq_source_deassert(&fdc->connect.irq_source);
    core_machine_fdc_reset_controller(fdc);
}

C_VOID core_machine_fdc_advance(core_machine_fdc *fdc)
{
    if (fdc == STD_NULL || fdc->data.elapsed_ticks == UINT64_MAX) return;
    core_machine_fdc_advance_at(fdc, fdc->data.elapsed_ticks + 1u);
}

C_VOID core_machine_fdc_advance_at(core_machine_fdc *fdc,
    type_unsigned_64 elapsed_ticks)
{
    if (fdc == STD_NULL) return;
    fdc->data.elapsed_ticks = elapsed_ticks;
    if (fdc->data.phase == core_machine_fdc_PHASE_PENDING_COMMAND) {
        core_machine_fdc_execute(fdc);
    } else if (fdc->data.phase == core_machine_fdc_PHASE_PENDING_SEEK &&
        elapsed_ticks >= fdc->data.seek_due_tick) {
        fdc->data.cylinder = fdc->data.seek_target;
        fdc->data.drive_cylinder[fdc->data.selected_drive] = fdc->data.seek_target;
        core_machine_fdc_observe_drive(fdc, fdc->data.selected_drive);
        core_machine_fdc_complete_simple(fdc, core_machine_fdc_drive_ready(fdc) ?
            core_machine_fdc_ST0_NORMAL | VFDC_ST0_SEEK_END |
            fdc->data.selected_drive : core_machine_fdc_ST0_ABNORMAL |
            VFDC_ST0_SEEK_END | fdc->data.selected_drive,
            (type_unsigned_8)fdc->data.cylinder);
        core_machine_fdc_raise_irq(fdc);
        core_machine_fdc_command_phase(fdc);
    } else if (fdc->data.phase == core_machine_fdc_PHASE_PENDING_COMPLETE) {
        core_machine_fdc_set_result(fdc, fdc->data.pending_st1 == 0u ?
            core_machine_fdc_ST0_NORMAL : core_machine_fdc_ST0_ABNORMAL,
            fdc->data.pending_st1, 0u);
        core_machine_fdc_result_phase(fdc, 7u);
        core_machine_fdc_raise_irq(fdc);
    }
    core_machine_fdc_publish_due_dma_byte(fdc);
    core_machine_fdc_publish_due_ndma_byte(fdc);
}

C_VOID core_machine_fdc_refresh(core_machine_fdc *fdc)
{
    core_machine_media_info info;
    core_machine_media_result result;
    type_unsigned_8 drive;
    type_bool ready;

    if (fdc == STD_NULL) return;
    for (drive = 0u; drive < CORE_MACHINE_FDC_DRIVE_COUNT; ++drive) {
        if (core_machine_fdc_drive_media_query(fdc, drive, &info, &result) &&
            fdc->data.observed_media_generation[drive] != info.generation) {
            fdc->data.media_changed[drive] = TYPE_TRUE;
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

C_VOID core_machine_fdc_finalize(core_machine_fdc *fdc)
{
    if (fdc == STD_NULL) return;
    core_machine_fdc_deassert_dma(fdc);
    core_machine_pic_irq_source_deassert(&fdc->connect.irq_source);
    STD_MEMSET(&fdc->data, TYPE_ZERO_8, sizeof(fdc->data));
    STD_MEMSET(&fdc->connect, TYPE_ZERO_8, sizeof(fdc->connect));
}

C_VOID core_machine_fdc_print(const core_machine_fdc *fdc)
{
    if (fdc == STD_NULL) return;
    STD_PRINTF("FDC phase=%d msr=%02x dor=%02x ccr=%02x command=%u/%u result=%u/%u\\n",
        fdc->data.phase, core_machine_fdc_msr(fdc), fdc->data.dor, fdc->data.ccr,
        fdc->data.command_index, fdc->data.command_length, fdc->data.result_index,
        fdc->data.result_length);
}
