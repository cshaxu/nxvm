/* Copyright 2012-2026 Neko. */

/* The default-profile 8272A-compatible controller.  Core owns DMA/PIC/RAM;
 * this file owns only the guest-visible floppy-controller state machine. */

#include "type.h"

#include "core/machine/dma.h"
#include "core/machine/pic.h"
#include "core/machine/port.h"
#include "vm/machine/fdd.h"
#include "vm/machine/fdc.h"

#define VM_MACHINE_FDC_CMD_SPECIFY 0x03u
#define VM_MACHINE_FDC_CMD_SENSE_DRIVE_STATUS 0x04u
#define VM_MACHINE_FDC_CMD_RECALIBRATE 0x07u
#define VM_MACHINE_FDC_CMD_SENSE_INTERRUPT 0x08u
#define VM_MACHINE_FDC_CMD_SEEK 0x0fu
#define VM_MACHINE_FDC_CMD_VERSION 0x10u
#define VM_MACHINE_FDC_CMD_READ_ID 0x0au
#define VM_MACHINE_FDC_CMD_WRITE_DATA 0x05u
#define VM_MACHINE_FDC_CMD_READ_DATA 0x06u
#define VM_MACHINE_FDC_CMD_FORMAT_TRACK 0x0du
#define VM_MACHINE_FDC_CMD_READ_TRACK 0x02u

#define VM_MACHINE_FDC_ST1_NO_DATA 0x04u
#define VM_MACHINE_FDC_ST1_NOT_WRITABLE 0x02u
#define VM_MACHINE_FDC_ST1_END_OF_CYLINDER 0x80u

static type_unsigned_8 vm_machine_fdc_sector_size(type_unsigned_8 code)
{
    return code == 2u ? 0x02u : 0xffu;
}

static type_unsigned_8 vm_machine_fdc_msr(const t_fdc *fdc)
{
    switch (fdc->data.phase) {
    case VM_MACHINE_FDC_PHASE_RESULT:
        return VFDC_MSR_RQM | VFDC_MSR_DIO | VFDC_MSR_CB;
    case VM_MACHINE_FDC_PHASE_EXECUTION_READ:
        return fdc->data.flagNDMA ? VFDC_MSR_RQM | VFDC_MSR_DIO |
            VFDC_MSR_NDM | VFDC_MSR_CB : VFDC_MSR_CB;
    case VM_MACHINE_FDC_PHASE_EXECUTION_WRITE:
    case VM_MACHINE_FDC_PHASE_EXECUTION_FORMAT:
        return fdc->data.flagNDMA ? VFDC_MSR_RQM | VFDC_MSR_NDM |
            VFDC_MSR_CB : VFDC_MSR_CB;
    default:
        return VFDC_MSR_RQM;
    }
}

static C_VOID vm_machine_fdc_sync_position(t_fdc *fdc)
{
    t_fdd *fdd = fdc->connect.fdd;
    if (fdd == STD_NULL) return;
    fdd->data.cyl = fdc->data.cylinder;
    fdd->data.head = fdc->data.head;
    fdd->data.sector = fdc->data.sector;
}

static C_VOID vm_machine_fdc_deassert_dma(t_fdc *fdc)
{
    core_machine_dma_request_deassert(&fdc->connect.dma_request);
}

static C_VOID vm_machine_fdc_raise_irq(t_fdc *fdc)
{
    if ((fdc->data.dor & VFDC_DOR_ENRQ) == 0u) return;
    core_machine_pic_irq_source_assert(&fdc->connect.irq_source);
    fdc->data.flagINTR = TYPE_TRUE;
}

static C_VOID vm_machine_fdc_cancel_execution(t_fdc *fdc)
{
    vm_machine_fdc_deassert_dma(fdc);
    fdc->data.transfer_remaining = 0u;
    fdc->data.byte_offset = 0u;
    fdc->data.format_headers_remaining = 0u;
}

static C_VOID vm_machine_fdc_command_phase(t_fdc *fdc)
{
    fdc->data.phase = VM_MACHINE_FDC_PHASE_COMMAND;
    fdc->data.command_length = 0u;
    fdc->data.command_index = 0u;
    fdc->data.result_length = 0u;
    fdc->data.result_index = 0u;
}

static C_VOID vm_machine_fdc_result_phase(t_fdc *fdc, type_unsigned_8 length)
{
    vm_machine_fdc_deassert_dma(fdc);
    fdc->data.phase = VM_MACHINE_FDC_PHASE_RESULT;
    fdc->data.result_length = length;
    fdc->data.result_index = 0u;
}

static C_VOID vm_machine_fdc_set_result(t_fdc *fdc, type_unsigned_8 st0,
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

static C_VOID vm_machine_fdc_complete_transfer(t_fdc *fdc,
    type_unsigned_8 st1)
{
    vm_machine_fdc_sync_position(fdc);
    vm_machine_fdc_set_result(fdc, st1 == 0u ? VM_MACHINE_FDC_ST0_NORMAL :
        VM_MACHINE_FDC_ST0_ABNORMAL, st1, 0u);
    vm_machine_fdc_result_phase(fdc, 7u);
    vm_machine_fdc_raise_irq(fdc);
}

static C_VOID vm_machine_fdc_complete_simple(t_fdc *fdc, type_unsigned_8 st0,
    type_unsigned_8 cylinder)
{
    fdc->data.ret[0] = st0;
    fdc->data.ret[1] = cylinder;
    fdc->data.st0 = st0;
    vm_machine_fdc_result_phase(fdc, 2u);
}

static C_INT vm_machine_fdc_drive_ready(const t_fdc *fdc)
{
    return fdc->connect.fdd != STD_NULL && fdc->data.selected_drive == 0u &&
        (fdc->data.dor & VFDC_DOR_NRS) != 0u &&
        (fdc->data.dor & VFDC_DOR_ME(fdc->data.selected_drive)) != 0u &&
        fdc->connect.fdd->connect.flagDiskExist;
}

static C_VOID vm_machine_fdc_advance_position(t_fdc *fdc)
{
    fdc->data.byte_offset++;
    if (fdc->data.byte_offset < 512u) return;
    fdc->data.byte_offset = 0u;
    fdc->data.sector++;
}

static C_INT vm_machine_fdc_transfer_byte(t_fdc *fdc, t_latch *latch,
    C_INT write_to_media)
{
    type_unsigned_8 byte;
    if (fdc->data.transfer_remaining == 0u || !vm_machine_fdc_drive_ready(fdc)) {
        vm_machine_fdc_complete_transfer(fdc, VM_MACHINE_FDC_ST1_NO_DATA);
        return TYPE_TRUE;
    }
    if (fdc->data.sector > fdc->data.eot || fdc->data.sector >
        fdc->connect.fdd->data.nsector) {
        vm_machine_fdc_complete_transfer(fdc, VM_MACHINE_FDC_ST1_END_OF_CYLINDER);
        return TYPE_TRUE;
    }
    if (write_to_media) {
        if (vm_machine_fdd_write_byte(fdc->connect.fdd, fdc->data.cylinder,
            fdc->data.head, fdc->data.sector, fdc->data.byte_offset,
            latch->data.byte)) {
            vm_machine_fdc_complete_transfer(fdc, fdc->connect.fdd->connect.flagReadOnly ?
                VM_MACHINE_FDC_ST1_NOT_WRITABLE : VM_MACHINE_FDC_ST1_NO_DATA);
            return TYPE_TRUE;
        }
    } else if (vm_machine_fdd_read_byte(fdc->connect.fdd, fdc->data.cylinder,
        fdc->data.head, fdc->data.sector, fdc->data.byte_offset, &byte)) {
        vm_machine_fdc_complete_transfer(fdc, VM_MACHINE_FDC_ST1_NO_DATA);
        return TYPE_TRUE;
    } else {
        latch->data.byte = byte;
    }
    fdc->data.transfer_remaining--;
    vm_machine_fdc_advance_position(fdc);
    if (fdc->data.transfer_remaining == 0u) vm_machine_fdc_complete_transfer(fdc, 0u);
    return TYPE_FALSE;
}

static C_VOID vm_machine_fdc_format_byte(t_fdc *fdc, type_unsigned_8 byte)
{
    if (fdc->data.format_headers_remaining == 0u) return;
    fdc->data.format_id[fdc->data.format_id_index++] = byte;
    if (fdc->data.format_id_index != 4u) return;
    fdc->data.format_id_index = 0u;
    if (fdc->data.format_id[0] != fdc->data.cylinder ||
        fdc->data.format_id[1] != fdc->data.head ||
        fdc->data.format_id[3] != 2u || vm_machine_fdd_format_sector(
        fdc->connect.fdd, fdc->data.cylinder, fdc->data.head,
        fdc->data.format_id[2], fdc->data.cmd[5])) {
        vm_machine_fdc_complete_transfer(fdc, fdc->connect.fdd->connect.flagReadOnly ?
            VM_MACHINE_FDC_ST1_NOT_WRITABLE : VM_MACHINE_FDC_ST1_NO_DATA);
        return;
    }
    fdc->data.format_headers_remaining--;
    fdc->data.sector = fdc->data.format_id[2];
    if (fdc->data.format_headers_remaining == 0u) vm_machine_fdc_complete_transfer(fdc, 0u);
}

static C_VOID vm_machine_fdc_dma_read(C_VOID *owner, t_latch *latch)
{
    t_fdc *fdc = owner;
    if (fdc != STD_NULL && fdc->data.phase == VM_MACHINE_FDC_PHASE_EXECUTION_READ) {
        (C_VOID)vm_machine_fdc_transfer_byte(fdc, latch, TYPE_FALSE);
    }
}

static C_VOID vm_machine_fdc_dma_write(C_VOID *owner, t_latch *latch)
{
    t_fdc *fdc = owner;
    if (fdc == STD_NULL) return;
    if (fdc->data.phase == VM_MACHINE_FDC_PHASE_EXECUTION_WRITE) {
        (C_VOID)vm_machine_fdc_transfer_byte(fdc, latch, TYPE_TRUE);
    } else if (fdc->data.phase == VM_MACHINE_FDC_PHASE_EXECUTION_FORMAT) {
        vm_machine_fdc_format_byte(fdc, latch->data.byte);
    }
}

static C_VOID vm_machine_fdc_dma_terminal(C_VOID *owner, t_latch *latch)
{
    t_fdc *fdc = owner;
    (C_VOID)latch;
    if (fdc != STD_NULL && (fdc->data.phase == VM_MACHINE_FDC_PHASE_EXECUTION_READ ||
        fdc->data.phase == VM_MACHINE_FDC_PHASE_EXECUTION_WRITE ||
        fdc->data.phase == VM_MACHINE_FDC_PHASE_EXECUTION_FORMAT)) {
        vm_machine_fdc_complete_transfer(fdc, fdc->data.transfer_remaining == 0u &&
            fdc->data.format_headers_remaining == 0u ? 0u : VM_MACHINE_FDC_ST1_NO_DATA);
    }
}

static const core_machine_dma_channel_provider vm_machine_fdc_dma_channel = {
    vm_machine_fdc_dma_read, vm_machine_fdc_dma_write, vm_machine_fdc_dma_terminal
};

const core_machine_dma_channel_provider *vm_machine_fdc_dma_provider(C_VOID)
{ return &vm_machine_fdc_dma_channel; }

static type_unsigned_8 vm_machine_fdc_command_length(type_unsigned_8 opcode)
{
    switch (opcode & 0x1fu) {
    case VM_MACHINE_FDC_CMD_SPECIFY: return 3u;
    case VM_MACHINE_FDC_CMD_SENSE_DRIVE_STATUS: return 2u;
    case VM_MACHINE_FDC_CMD_RECALIBRATE: return 2u;
    case VM_MACHINE_FDC_CMD_SENSE_INTERRUPT: return 1u;
    case VM_MACHINE_FDC_CMD_SEEK: return 3u;
    case VM_MACHINE_FDC_CMD_VERSION: return 1u;
    case VM_MACHINE_FDC_CMD_READ_ID: return 2u;
    case VM_MACHINE_FDC_CMD_WRITE_DATA:
    case VM_MACHINE_FDC_CMD_READ_DATA:
    case VM_MACHINE_FDC_CMD_READ_TRACK: return 9u;
    case VM_MACHINE_FDC_CMD_FORMAT_TRACK: return 6u;
    default: return 1u;
    }
}

static C_VOID vm_machine_fdc_start_transfer(t_fdc *fdc, C_INT write_to_media)
{
    type_unsigned_8 size = vm_machine_fdc_sector_size(fdc->data.cmd[5]);
    fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
    fdc->data.cylinder = fdc->data.cmd[2];
    fdc->data.head = fdc->data.cmd[3];
    fdc->data.sector = fdc->data.cmd[4];
    fdc->data.eot = fdc->data.cmd[6];
    fdc->data.byte_offset = 0u;
    if (size != 2u || (fdc->data.ccr != 0u && fdc->data.ccr != VFDC_CCR_DRC) ||
        !vm_machine_fdc_drive_ready(fdc) ||
        fdc->data.head >= fdc->connect.fdd->data.nhead || fdc->data.sector == 0u ||
        fdc->data.sector > fdc->data.eot || fdc->data.eot > fdc->connect.fdd->data.nsector) {
        vm_machine_fdc_complete_transfer(fdc, VM_MACHINE_FDC_ST1_NO_DATA);
        return;
    }
    fdc->data.transfer_remaining = (type_unsigned_32)(fdc->data.eot -
        fdc->data.sector + 1u) * 512u;
    fdc->data.phase = write_to_media ? VM_MACHINE_FDC_PHASE_EXECUTION_WRITE :
        VM_MACHINE_FDC_PHASE_EXECUTION_READ;
    if (!fdc->data.flagNDMA && (fdc->data.dor & VFDC_DOR_ENRQ) != 0u) {
        core_machine_dma_request_assert(&fdc->connect.dma_request);
    }
}

static C_VOID vm_machine_fdc_start_read_track(t_fdc *fdc)
{
    fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
    fdc->data.cylinder = fdc->data.cmd[2];
    fdc->data.head = fdc->data.cmd[3];
    fdc->data.sector = fdc->data.cmd[4];
    fdc->data.eot = fdc->data.cmd[6];
    fdc->data.byte_offset = 0u;
    if (fdc->data.cmd[0] != 0x42u || fdc->data.flagNDMA ||
        vm_machine_fdc_sector_size(fdc->data.cmd[5]) != 2u ||
        (fdc->data.ccr != 0u && fdc->data.ccr != VFDC_CCR_DRC) ||
        !vm_machine_fdc_drive_ready(fdc) || fdc->data.selected_drive != 0u ||
        fdc->data.head >= fdc->connect.fdd->data.nhead ||
        fdc->data.cylinder >= fdc->connect.fdd->data.ncyl ||
        fdc->data.sector != 1u ||
        fdc->data.eot != fdc->connect.fdd->data.nsector) {
        vm_machine_fdc_complete_transfer(fdc, VM_MACHINE_FDC_ST1_NO_DATA);
        return;
    }
    fdc->data.transfer_remaining = (type_unsigned_32)
        fdc->connect.fdd->data.nsector * 512u;
    fdc->data.phase = VM_MACHINE_FDC_PHASE_EXECUTION_READ;
    if ((fdc->data.dor & VFDC_DOR_ENRQ) != 0u) {
        core_machine_dma_request_assert(&fdc->connect.dma_request);
    }
}

static C_VOID vm_machine_fdc_execute(t_fdc *fdc)
{
    type_unsigned_8 opcode = fdc->data.cmd[0] & 0x1fu;
    t_fdd *fdd = fdc->connect.fdd;
    switch (opcode) {
    case VM_MACHINE_FDC_CMD_SPECIFY:
        fdc->data.hut = fdc->data.cmd[1] & 0x0fu;
        fdc->data.srt = fdc->data.cmd[1] >> 4u;
        fdc->data.hlt = fdc->data.cmd[2] >> 1u;
        fdc->data.flagNDMA = (fdc->data.cmd[2] & 1u) != 0u;
        vm_machine_fdc_command_phase(fdc);
        break;
    case VM_MACHINE_FDC_CMD_SENSE_DRIVE_STATUS:
        fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
        fdc->data.head = (fdc->data.cmd[1] >> 2u) & 1u;
        fdc->data.st3 = (fdc->data.selected_drive & 3u) |
            (fdc->data.head << 2u) | (fdd != STD_NULL &&
            fdd->connect.flagReadOnly ? 0x40u : 0u) | (vm_machine_fdc_drive_ready(fdc) ?
            0x20u : 0u) | (fdc->data.cylinder == 0u ? 0x10u : 0u);
        fdc->data.ret[0] = fdc->data.st3;
        vm_machine_fdc_result_phase(fdc, 1u);
        break;
    case VM_MACHINE_FDC_CMD_RECALIBRATE:
        fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
        fdc->data.cylinder = 0u; fdc->data.head = 0u; fdc->data.sector = 1u;
        fdc->data.observed_media_generation = fdd != STD_NULL ?
            fdd->connect.media_generation : 0u;
        vm_machine_fdc_complete_simple(fdc, VM_MACHINE_FDC_ST0_NORMAL |
            VFDC_ST0_SEEK_END | fdc->data.selected_drive, 0u);
        vm_machine_fdc_raise_irq(fdc);
        vm_machine_fdc_command_phase(fdc);
        break;
    case VM_MACHINE_FDC_CMD_SENSE_INTERRUPT:
        if (fdc->data.flagINTR) {
            fdc->data.ret[0] = fdc->data.st0;
            fdc->data.ret[1] = (type_unsigned_8)fdc->data.cylinder;
            fdc->data.flagINTR = TYPE_FALSE;
            core_machine_pic_irq_source_deassert(&fdc->connect.irq_source);
        } else {
            fdc->data.ret[0] = 0x80u;
            fdc->data.ret[1] = 0u;
        }
        vm_machine_fdc_result_phase(fdc, 2u);
        break;
    case VM_MACHINE_FDC_CMD_SEEK:
        fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
        fdc->data.head = (fdc->data.cmd[1] >> 2u) & 1u;
        fdc->data.cylinder = fdc->data.cmd[2]; fdc->data.sector = 1u;
        fdc->data.observed_media_generation = fdd != STD_NULL ?
            fdd->connect.media_generation : 0u;
        vm_machine_fdc_complete_simple(fdc, (vm_machine_fdc_drive_ready(fdc) &&
            fdc->data.cylinder < fdd->data.ncyl ? VM_MACHINE_FDC_ST0_NORMAL :
            VM_MACHINE_FDC_ST0_ABNORMAL) | VFDC_ST0_SEEK_END |
            fdc->data.selected_drive, (type_unsigned_8)fdc->data.cylinder);
        vm_machine_fdc_raise_irq(fdc);
        vm_machine_fdc_command_phase(fdc);
        break;
    case VM_MACHINE_FDC_CMD_VERSION:
        fdc->data.ret[0] = 0x90u;
        vm_machine_fdc_result_phase(fdc, 1u);
        break;
    case VM_MACHINE_FDC_CMD_READ_ID:
        fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
        fdc->data.head = (fdc->data.cmd[1] >> 2u) & 1u;
        if (!vm_machine_fdc_drive_ready(fdc)) {
            vm_machine_fdc_complete_transfer(fdc, VM_MACHINE_FDC_ST1_NO_DATA);
        } else {
            if (fdc->data.sector == 0u) fdc->data.sector = 1u;
            vm_machine_fdc_set_result(fdc, VM_MACHINE_FDC_ST0_NORMAL, 0u, 0u);
            vm_machine_fdc_result_phase(fdc, 7u);
            vm_machine_fdc_raise_irq(fdc);
        }
        break;
    case VM_MACHINE_FDC_CMD_READ_DATA:
        vm_machine_fdc_start_transfer(fdc, TYPE_FALSE);
        break;
    case VM_MACHINE_FDC_CMD_READ_TRACK:
        vm_machine_fdc_start_read_track(fdc);
        break;
    case VM_MACHINE_FDC_CMD_WRITE_DATA:
        vm_machine_fdc_start_transfer(fdc, TYPE_TRUE);
        break;
    case VM_MACHINE_FDC_CMD_FORMAT_TRACK:
        fdc->data.selected_drive = fdc->data.cmd[1] & 0x03u;
        fdc->data.head = (fdc->data.cmd[1] >> 2u) & 1u;
        fdc->data.sector = 1u;
        fdc->data.eot = fdc->data.cmd[3];
        fdc->data.format_headers_remaining = fdc->data.cmd[3];
        fdc->data.format_id_index = 0u;
        if (vm_machine_fdc_sector_size(fdc->data.cmd[2]) != 2u ||
            (fdc->data.ccr != 0u && fdc->data.ccr != VFDC_CCR_DRC) ||
            !vm_machine_fdc_drive_ready(fdc) || fdc->data.eot == 0u ||
            fdc->data.eot > fdd->data.nsector) {
            vm_machine_fdc_complete_transfer(fdc, VM_MACHINE_FDC_ST1_NO_DATA);
        } else {
            fdc->data.phase = VM_MACHINE_FDC_PHASE_EXECUTION_FORMAT;
            if (!fdc->data.flagNDMA && (fdc->data.dor & VFDC_DOR_ENRQ) != 0u) {
                core_machine_dma_request_assert(&fdc->connect.dma_request);
            }
        }
        break;
    default:
        fdc->data.ret[0] = 0x80u;
        vm_machine_fdc_result_phase(fdc, 1u);
        break;
    }
}

static C_VOID vm_machine_fdc_reset_controller(t_fdc *fdc)
{
    type_unsigned_8 ccr = fdc->data.ccr;
    uint32_t observed_media_generation = fdc->data.observed_media_generation;
    vm_machine_fdc_cancel_execution(fdc);
    STD_MEMSET(&fdc->data, TYPE_ZERO_8, sizeof(fdc->data));
    fdc->data.ccr = ccr;
    fdc->data.observed_media_generation = observed_media_generation;
    vm_machine_fdc_command_phase(fdc);
}

static C_VOID vm_machine_fdc_read_status(t_port *port, type_unsigned_16 id,
    C_VOID *owner)
{
    t_fdc *fdc = owner; (C_VOID)port; (C_VOID)id;
    fdc->connect.port->data.ioByte = vm_machine_fdc_msr(fdc);
}

static C_VOID vm_machine_fdc_read_data(t_port *port, type_unsigned_16 id,
    C_VOID *owner)
{
    t_fdc *fdc = owner; t_latch latch;
    (C_VOID)port; (C_VOID)id;
    if (fdc->data.phase == VM_MACHINE_FDC_PHASE_RESULT) {
        fdc->connect.port->data.ioByte = fdc->data.ret[fdc->data.result_index++];
        if (fdc->data.result_index >= fdc->data.result_length) vm_machine_fdc_command_phase(fdc);
    } else if (fdc->data.phase == VM_MACHINE_FDC_PHASE_EXECUTION_READ &&
        fdc->data.flagNDMA) {
        latch.data.byte = 0u;
        (C_VOID)vm_machine_fdc_transfer_byte(fdc, &latch, TYPE_FALSE);
        fdc->connect.port->data.ioByte = latch.data.byte;
    }
}

static C_VOID vm_machine_fdc_read_direction(t_port *port, type_unsigned_16 id,
    C_VOID *owner)
{
    t_fdc *fdc = owner; (C_VOID)port; (C_VOID)id;
    fdc->connect.port->data.ioByte = fdc->data.dir;
}

static C_VOID vm_machine_fdc_write_dor(t_port *port, type_unsigned_16 id,
    C_VOID *owner)
{
    t_fdc *fdc = owner;
    type_unsigned_8 dor = fdc->connect.port->data.ioByte;
    (C_VOID)port; (C_VOID)id;
    if ((dor & VFDC_DOR_NRS) == 0u) {
        core_machine_pic_irq_source_deassert(&fdc->connect.irq_source);
        fdc->data.flagINTR = TYPE_FALSE;
        vm_machine_fdc_reset_controller(fdc);
    }
    fdc->data.dor = dor;
}

static C_VOID vm_machine_fdc_write_data(t_port *port, type_unsigned_16 id,
    C_VOID *owner)
{
    t_fdc *fdc = owner; t_latch latch;
    (C_VOID)port; (C_VOID)id;
    if (fdc->data.phase == VM_MACHINE_FDC_PHASE_EXECUTION_WRITE && fdc->data.flagNDMA) {
        latch.data.byte = fdc->connect.port->data.ioByte;
        (C_VOID)vm_machine_fdc_transfer_byte(fdc, &latch, TYPE_TRUE);
        return;
    }
    if (fdc->data.phase == VM_MACHINE_FDC_PHASE_EXECUTION_FORMAT && fdc->data.flagNDMA) {
        vm_machine_fdc_format_byte(fdc, fdc->connect.port->data.ioByte);
        return;
    }
    if (fdc->data.phase != VM_MACHINE_FDC_PHASE_COMMAND) return;
    if (fdc->data.command_index == 0u) {
        fdc->data.command_length = vm_machine_fdc_command_length(
            fdc->connect.port->data.ioByte);
    }
    if (fdc->data.command_index >= sizeof(fdc->data.cmd)) return;
    fdc->data.cmd[fdc->data.command_index++] = fdc->connect.port->data.ioByte;
    if (fdc->data.command_index == fdc->data.command_length) vm_machine_fdc_execute(fdc);
}

static C_VOID vm_machine_fdc_write_control(t_port *port, type_unsigned_16 id,
    C_VOID *owner)
{
    t_fdc *fdc = owner; (C_VOID)port; (C_VOID)id;
    fdc->data.ccr = fdc->connect.port->data.ioByte & 0x03u;
}

C_VOID vm_machine_fdc_connect(t_fdc *fdc, t_fdd *fdd,
    const core_machine_dma_request_binding *dma_request, t_pic *pic_master,
    t_pic *pic_slave, t_port *port, const vm_machine_fdc_config *config)
{
    if (fdc == STD_NULL || dma_request == STD_NULL || config == STD_NULL) return;
    fdc->connect.fdd = fdd;
    fdc->connect.dma_request = *dma_request;
    core_machine_pic_irq_source_bind(&fdc->connect.irq_source, pic_master,
        pic_slave, config->irq);
    fdc->connect.port = port;
    fdc->connect.config = *config;
}

C_VOID vm_machine_fdc_initialize(t_fdc *fdc)
{
    if (fdc == STD_NULL || fdc->connect.port == STD_NULL) return;
    STD_MEMSET(&fdc->data, TYPE_ZERO_8, sizeof(fdc->data));
    fdc->data.ccr = VFDC_CCR_DRC;
    vm_machine_fdc_command_phase(fdc);
    core_machine_port_add_read(fdc->connect.port, fdc->connect.config.status_port,
        vm_machine_fdc_read_status, fdc);
    core_machine_port_add_read(fdc->connect.port, fdc->connect.config.data_port,
        vm_machine_fdc_read_data, fdc);
    core_machine_port_add_read(fdc->connect.port, fdc->connect.config.direction_port,
        vm_machine_fdc_read_direction, fdc);
    core_machine_port_add_write(fdc->connect.port, fdc->connect.config.dor_port,
        vm_machine_fdc_write_dor, fdc);
    core_machine_port_add_write(fdc->connect.port, fdc->connect.config.data_port,
        vm_machine_fdc_write_data, fdc);
    core_machine_port_add_write(fdc->connect.port, fdc->connect.config.control_port,
        vm_machine_fdc_write_control, fdc);
}

C_VOID vm_machine_fdc_reset(t_fdc *fdc)
{
    if (fdc == STD_NULL) return;
    vm_machine_fdc_deassert_dma(fdc);
    core_machine_pic_irq_source_deassert(&fdc->connect.irq_source);
    vm_machine_fdc_reset_controller(fdc);
}

C_VOID vm_machine_fdc_refresh(t_fdc *fdc)
{
    if (fdc == STD_NULL || fdc->connect.fdd == STD_NULL) return;
    if (fdc->data.observed_media_generation !=
        fdc->connect.fdd->connect.media_generation) {
        fdc->data.dir |= VFDC_DIR_DC;
    } else {
        fdc->data.dir &= (type_unsigned_8)~VFDC_DIR_DC;
    }
}

C_VOID vm_machine_fdc_finalize(t_fdc *fdc)
{
    if (fdc == STD_NULL) return;
    vm_machine_fdc_deassert_dma(fdc);
    core_machine_pic_irq_source_deassert(&fdc->connect.irq_source);
}

C_VOID vm_machine_fdc_print(const t_fdc *fdc)
{
    if (fdc == STD_NULL) return;
    STD_PRINTF("FDC phase=%d msr=%02x dor=%02x ccr=%02x command=%u/%u result=%u/%u\\n",
        fdc->data.phase, vm_machine_fdc_msr(fdc), fdc->data.dor, fdc->data.ccr,
        fdc->data.command_index, fdc->data.command_length, fdc->data.result_index,
        fdc->data.result_length);
}
