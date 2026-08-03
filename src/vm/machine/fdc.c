/* Copyright 2012-2014 Neko. */

/* VFDC implements Floppy Driver Controller: Intel 8272A. */

#include "type.h"

#include "core/product/utils.h"


#include "core/machine/dma.h"

#include "core/machine/pic.h"

#include "core/machine/port.h"

#include "vm/machine/fdd.h"


#include "vm/machine/fdc.h"

C_VOID vm_machine_fdc_connect(t_fdc *fdc, t_fdd *fdd, t_latch *dma_latch,
    t_dma *dma_primary, t_dma *dma_secondary, t_pic *pic_master,
    t_pic *pic_slave, t_port *port)
{
    if (fdc == STD_NULL) return;
    fdc->connect.fdd = fdd;
    fdc->connect.dma_latch = dma_latch;
    fdc->connect.dma_primary = dma_primary;
    fdc->connect.dma_secondary = dma_secondary;
    fdc->connect.pic_master = pic_master;
    fdc->connect.pic_slave = pic_slave;
    fdc->connect.port = port;
}

#define VFDC_RET_ERROR         0x80 /* Error Code */
/* Commands */
#define CMD_SPECIFY            0x03 /* set drive parameters */
#define CMD_SENSE_DRIVE_STATUS 0x04
#define CMD_RECALIBRATE        0x07 /* seek to cylinder 0 */
#define CMD_SENSE_INTERRUPT    0x08 /* acknowledge IRQ6 get status of last command */
#define CMD_SEEK               0x0f /* ! seek (both(?)) heads to cylinder X */
#define CMD_VERSION            0x10 /* ! used during initialization once */

#define CMD_PERPENDICULAR_MODE 0x12 /* ! used during initialization once maybe */
#define CMD_CONFIGURE          0x13 /* ! set controller parameters */
#define CMD_LOCK               0x14 /* ! protect controller params from a reset */
#define CMD_VERIFY             0x16
#define CMD_SCAN_LOW_OR_EQUAL  0x19
#define CMD_SCAN_HIGH_OR_EQUAL 0x1d
#define CMD_READ_TRACK         0x42 /* generates irq6 */
#define CMD_READ_ID            0x4a /* generates irq6 */
#define CMD_FORMAT_TRACK       0x4d /* generates irq6 */
#define CMD_WRITE_DATA         0xc5 /* write to the disk */
#define CMD_READ_DATA_ALL      0xc6 /* read all data, even if deleted */
#define CMD_WRITE_DELETED_DATA 0xc9
#define CMD_READ_DELETED_DATA  0xcc
#define CMD_SCAN_EQUAL_ALL     0xd1
#define CMD_READ_DATA          0xe6 /* ! read from the disk */
#define CMD_SCAN_EQUAL         0xf1

/* sector size code */
ntvdm64_type_unsigned_8 VFDC_GetBPSC(ntvdm64_type_unsigned_16 cb) {
    switch (cb) {
    case 0x0080:
        return 0x00;
    case 0x0100:
        return 0x01;
    case 0x0200:
        return 0x02;
    case 0x0400:
        return 0x03;
    case 0x0800:
        return 0x04;
    case 0x1000:
        return 0x05;
    case 0x2000:
        return 0x06;
    case 0x4000:
        return 0x07;
    default:
        return 0x00;
    }
}

#define IsRet(retl, count) (fdc->data.cmd[0] == (retl) && fdc->data.flagret == (count))
#define SetST0 (fdc->data.st0 = (0x00      << 7) | \
                           (0x00      << 6) | \
                           (0x00      << 5) | \
                           ((fdc->connect.fdd->data.cyl >= fdc->connect.fdd->data.ncyl) << 4) | \
                           (0x00      << 3) | \
                           (fdc->connect.fdd->data.head << 2) | \
                           ((fdc->data.cmd[1] & VFDC_ST0_DS) << 0))
#define SetST1 (fdc->data.st1 = ((fdc->connect.fdd->data.sector >= (fdc->connect.fdd->data.nsector + 1)) << 7) | \
                           (0x00 << 6) | \
                           (0x00 << 5) | \
                           (0x00 << 4) | \
                           (0x00 << 3) | \
                           ((fdc->connect.fdd->data.cyl >= fdc->connect.fdd->data.ncyl) << 2) | \
                           (0x00 << 1) | \
                           (0x00 << 0))
#define SetST2 (fdc->data.st2 = (0x00 << 7) | \
                           (0x00 << 6) | \
                           (0x00 << 5) | \
                           ((fdc->connect.fdd->data.cyl >= fdc->connect.fdd->data.ncyl) << 4) | \
                           (0x00 << 3) | \
                           (0x00 << 2) | \
                           (0x00 << 1) | \
                           (0x00 << 0))
#define SetST3 (fdc->data.st3 = (0x00        << 7) | \
                           (fdc->connect.fdd->connect.flagReadOnly << 6) | \
                           (0x01        << 5) | \
                           ((!fdc->connect.fdd->data.cyl) << 4) | \
                           (0x01        << 3) | \
                           (fdc->connect.fdd->data.head   << 2) | \
                           ((fdc->data.cmd[1] & VFDC_ST3_DS) << 0))
#define SetMSRReadyRead  (fdc->data.msr = VFDC_MSR_ReadyRead, fdc->data.rwCount = 0)
#define SetMSRReadyWrite (fdc->data.msr = VFDC_MSR_ReadyWrite, fdc->data.rwCount = 0)
#define SetMSRProcRead   (fdc->data.msr = VFDC_MSR_ProcessRead)
#define SetMSRProcWrite  (fdc->data.msr = VFDC_MSR_ProcessWrite)
#define SetMSRExecCmd    (fdc->data.msr = VFDC_MSR_NDM)

#define GetMSRReadyRead  ((fdc->data.msr & 0xc0) == VFDC_MSR_ReadyRead)
#define GetMSRReadyWrite ((fdc->data.msr & 0xc0) == VFDC_MSR_ReadyWrite)
#define GetMSRProcRW     (NTVDM64_TYPE_GET_BIT(fdc->data.msr, VFDC_MSR_CB))
#define GetMSRExecCmd    (NTVDM64_TYPE_GET_BIT(fdc->data.msr, VFDC_MSR_NDM))

/* Resets FDC but keeps CCR */
static C_VOID reset_controller(t_fdc *fdc) {
    ntvdm64_type_unsigned_8 ccr = fdc->data.ccr;
    STD_MEMSET((C_VOID *)(&fdc->data), NTVDM64_TYPE_ZERO_8, sizeof(t_fdc_data));
    fdc->data.ccr = ccr;
}

static C_VOID dma_read(C_VOID *owner, t_latch *latch) {
    t_fdc *fdc = owner;
    /* NOTE: being called by DMA/PIO */
    vm_machine_fdd_transfer_read(fdc->connect.fdd, latch);
}
static C_VOID dma_write(C_VOID *owner, t_latch *latch) {
    t_fdc *fdc = owner;
    /* NOTE: being called by DMA/PIO */
    vm_machine_fdd_transfer_write(fdc->connect.fdd, latch);
}
static C_VOID begin_transfer(t_fdc *fdc) {
    /* Called by the controller command path. */
    /* read parameters */
    fdc->connect.fdd->data.cyl       = fdc->data.cmd[2];
    fdc->connect.fdd->data.head      = fdc->data.cmd[3];
    fdc->connect.fdd->data.sector    = fdc->data.cmd[4];
    fdc->connect.fdd->data.nbyte     = VFDC_GetBPS(fdc->data.cmd[5]);
    fdc->connect.fdd->data.nsector   = fdc->data.cmd[6];
    fdc->connect.fdd->data.gpl       = fdc->data.cmd[7];
    if (!fdc->data.cmd[5]) {
        fdc->connect.fdd->data.nbyte = fdc->data.cmd[8];
    }
    fdc->connect.fdd->connect.transCount = NTVDM64_TYPE_ZERO_16;
    vm_machine_fdd_set_pointer(fdc->connect.fdd);
    /* send trans request */
    if (!fdc->data.flagNDMA && NTVDM64_TYPE_GET_BIT(fdc->data.dor, VFDC_DOR_ENRQ)) {
        core_machine_dma_set_drq(fdc->connect.dma_primary, fdc->connect.dma_secondary, 2);
    }
    SetMSRExecCmd;
}
static C_VOID finish_transfer(t_fdc *fdc) {
    /* NOTE: being called by DMA/PIO */
    SetST0;
    SetST1;
    SetST2;
    fdc->data.ret[0] = fdc->data.st0;
    fdc->data.ret[1] = fdc->data.st1;
    fdc->data.ret[2] = fdc->data.st2;
    fdc->data.ret[3] = NTVDM64_TYPE_MASK_UNSIGNED_8(fdc->connect.fdd->data.cyl);
    fdc->data.ret[4] = NTVDM64_TYPE_MASK_UNSIGNED_8(fdc->connect.fdd->data.head);
    fdc->data.ret[5] = NTVDM64_TYPE_MASK_UNSIGNED_8(fdc->connect.fdd->data.sector);
    fdc->data.ret[6] = VFDC_GetBPSC(fdc->connect.fdd->data.nbyte);
    if (NTVDM64_TYPE_GET_BIT(fdc->data.dor, VFDC_DOR_ENRQ)) {
        core_machine_pic_set_irq(fdc->connect.pic_master, fdc->connect.pic_slave, 0x06);
        fdc->data.flagINTR = NTVDM64_TYPE_TRUE;
    }
    SetMSRReadyRead;
}

static C_VOID execute_specify(t_fdc *fdc) {
    fdc->data.hut      = VFDC_GetCMD_Specify1_HUT(fdc->data.cmd[1]);
    fdc->data.srt      = VFDC_GetCMD_Specify1_SRT(fdc->data.cmd[1]);
    fdc->data.hlt      = VFDC_GetCMD_Specify2_HLT(fdc->data.cmd[2]);
    fdc->data.flagNDMA = NTVDM64_TYPE_GET_BIT(fdc->data.cmd[2], VFDC_CMD_Specify2_ND);
    SetMSRReadyWrite;
}
static C_VOID execute_sense_drive_status(t_fdc *fdc) {
    fdc->connect.fdd->data.head = NTVDM64_TYPE_GET_BIT(fdc->data.cmd[1], VFDC_CMD_SenseDriveStatus1_HD);
    vm_machine_fdd_set_pointer(fdc->connect.fdd);
    SetST3;
    fdc->data.ret[0] = fdc->data.st3;
    SetMSRReadyRead;
}
static C_VOID execute_recalibrate(t_fdc *fdc) {
    fdc->connect.fdd->data.cyl    = 0;
    fdc->connect.fdd->data.head   = 0;
    fdc->connect.fdd->data.sector = 1;
    vm_machine_fdd_set_pointer(fdc->connect.fdd);
    SetST0;
    NTVDM64_TYPE_SET_BIT(fdc->data.st0, VFDC_ST0_SEEK_END);
    if (NTVDM64_TYPE_GET_BIT(fdc->data.dor, VFDC_DOR_ENRQ)) {
        core_machine_pic_set_irq(fdc->connect.pic_master, fdc->connect.pic_slave, 0x06);
        fdc->data.flagINTR = NTVDM64_TYPE_TRUE;
    }
    SetMSRReadyWrite;
}
static C_VOID execute_sense_interrupt(t_fdc *fdc) {
    if (fdc->data.flagINTR) {
        fdc->data.ret[0] = fdc->data.st0;
        fdc->data.ret[1] = (ntvdm64_type_unsigned_8)fdc->connect.fdd->data.cyl;
        fdc->data.flagINTR = NTVDM64_TYPE_FALSE;
    } else {
        fdc->data.ret[0] = fdc->data.st0 = VFDC_RET_ERROR;
    }
    SetMSRReadyRead;
}
static C_VOID execute_seek(t_fdc *fdc) {
    fdc->connect.fdd->data.head = NTVDM64_TYPE_GET_BIT(fdc->data.cmd[1], VFDC_CMD_Seek1_HD);
    fdc->connect.fdd->data.cyl  = fdc->data.cmd[2];
    fdc->connect.fdd->data.sector = 1;
    vm_machine_fdd_set_pointer(fdc->connect.fdd);
    SetST0;
    NTVDM64_TYPE_SET_BIT(fdc->data.st0, VFDC_ST0_SEEK_END);
    if (NTVDM64_TYPE_GET_BIT(fdc->data.dor, VFDC_DOR_ENRQ)) {
        core_machine_pic_set_irq(fdc->connect.pic_master, fdc->connect.pic_slave, 0x06);
        fdc->data.flagINTR = NTVDM64_TYPE_TRUE;
    }
    SetMSRReadyWrite;
}
#define execute_read_track(fdc) begin_transfer((fdc))
static C_VOID execute_read_id(t_fdc *fdc) {
    fdc->connect.fdd->data.head = NTVDM64_TYPE_GET_BIT(fdc->data.cmd[1], VFDC_CMD_ReadId1_HD);
    fdc->connect.fdd->data.sector = 1;
    vm_machine_fdd_set_pointer(fdc->connect.fdd);
    fdc->data.dr = NTVDM64_TYPE_ZERO_8; /* data register: sector id info */
    finish_transfer(fdc);
}
static C_VOID execute_format_track(t_fdc *fdc) {
    /* NOTE: simplified procedure; dma not used */
    ntvdm64_type_unsigned_8 fillByte;
    /* load parameters*/
    fdc->connect.fdd->data.head    = NTVDM64_TYPE_GET_BIT(fdc->data.cmd[1], VFDC_CMD_FormatTrack1_HD);
    fdc->connect.fdd->data.sector  = 0x01;
    fdc->connect.fdd->data.nbyte   = VFDC_GetBPS(fdc->data.cmd[2]);
    fdc->connect.fdd->data.nsector = fdc->data.cmd[3];
    fdc->connect.fdd->data.gpl     = fdc->data.cmd[4];
    fillByte     = fdc->data.cmd[5];
    vm_machine_fdd_set_pointer(fdc->connect.fdd);
    /* execute format track*/
    vm_machine_fdd_format_track(fdc->connect.fdd, fillByte);
    /* finish transaction */
    SetST0;
    SetST1;
    SetST2;
    fdc->data.ret[0] = fdc->data.st0;
    fdc->data.ret[1] = fdc->data.st1;
    fdc->data.ret[2] = fdc->data.st2;
    fdc->data.ret[3] = NTVDM64_TYPE_ZERO_8;
    fdc->data.ret[4] = NTVDM64_TYPE_ZERO_8;
    fdc->data.ret[5] = NTVDM64_TYPE_ZERO_8;
    fdc->data.ret[6] = NTVDM64_TYPE_ZERO_8;
    if (NTVDM64_TYPE_GET_BIT(fdc->data.dor, VFDC_DOR_ENRQ)) {
        core_machine_pic_set_irq(fdc->connect.pic_master, fdc->connect.pic_slave, 0x06);
        fdc->data.flagINTR = NTVDM64_TYPE_TRUE;
    }
    SetMSRReadyRead;
}
#define execute_write_data(fdc) begin_transfer((fdc))
#define execute_read_data_all(fdc) begin_transfer((fdc))
#define execute_write_deleted_data(fdc) begin_transfer((fdc))
#define execute_read_deleted_data(fdc) begin_transfer((fdc))
#define execute_scan_equal_all(fdc) execute_scan_equal((fdc))
#define execute_read_data(fdc) begin_transfer((fdc))
static C_VOID execute_scan_equal(t_fdc *fdc) {
    /* NOTE: not fully implemented; lack of reference */
    SetST0;
    SetST1;
    SetST2;
    /* assume all data match */
    NTVDM64_TYPE_SET_BIT(fdc->data.st2, VFDC_ST2_SCAN_MATCH);
    fdc->data.ret[0] = fdc->data.st0;
    fdc->data.ret[1] = fdc->data.st1;
    fdc->data.ret[2] = fdc->data.st2;
    fdc->data.ret[3] = NTVDM64_TYPE_MASK_UNSIGNED_8(fdc->connect.fdd->data.cyl);
    fdc->data.ret[4] = NTVDM64_TYPE_MASK_UNSIGNED_8(fdc->connect.fdd->data.head);
    fdc->data.ret[5] = NTVDM64_TYPE_MASK_UNSIGNED_8(fdc->connect.fdd->data.sector); /* NOTE: eot not changed */
    fdc->data.ret[6] = VFDC_GetBPSC(fdc->connect.fdd->data.nbyte);
    SetMSRReadyRead;
}
static C_VOID execute_error(t_fdc *fdc) {
    fdc->data.ret[0] = VFDC_RET_ERROR;
    SetMSRReadyRead;
}

/* read main status register */
static C_VOID read_03f4(t_port *port, ntvdm64_type_unsigned_16 port_id, C_VOID *owner) {
    t_fdc *fdc = owner;
    (C_VOID)port;
    (C_VOID)port_id;
    fdc->connect.port->data.ioByte = fdc->data.msr;
}
/* read standard results */
static C_VOID read_03f5(t_port *port, ntvdm64_type_unsigned_16 port_id, C_VOID *owner) {
    t_fdc *fdc = owner;
    (C_VOID)port;
    (C_VOID)port_id;
    if (!GetMSRReadyRead) {
        return;
    } else {
        SetMSRProcRead;
    }
    fdc->connect.port->data.ioByte = fdc->data.ret[fdc->data.rwCount++];
    switch (fdc->data.cmd[0]) {
    case CMD_SPECIFY:
        if (fdc->data.rwCount >= 0) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_SENSE_DRIVE_STATUS:
        if (fdc->data.rwCount >= 1) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_RECALIBRATE:
        if (fdc->data.rwCount >= 0) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_SENSE_INTERRUPT:
        if (fdc->data.rwCount >= 2) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_SEEK:
        if (fdc->data.rwCount >= 0) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_READ_TRACK:
        if (fdc->data.rwCount >= 7) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_READ_ID:
        if (fdc->data.rwCount >= 7) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_FORMAT_TRACK:
        if (fdc->data.rwCount >= 7) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_WRITE_DATA:
        if (fdc->data.rwCount >= 7) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_READ_DATA_ALL:
        if (fdc->data.rwCount >= 7) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_WRITE_DELETED_DATA:
        if (fdc->data.rwCount >= 7) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_READ_DELETED_DATA:
        if (fdc->data.rwCount >= 7) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_SCAN_EQUAL_ALL:
        if (fdc->data.rwCount >= 7) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_READ_DATA:
        if (fdc->data.rwCount >= 7) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_SCAN_EQUAL:
        if (fdc->data.rwCount >= 7) {
            SetMSRReadyWrite;
        }
        break;
    default:
        if (fdc->data.rwCount >= 1) {
            SetMSRReadyWrite;
        }
        break;
    }
}
/* read digital input register */
static C_VOID read_03f7(t_port *port, ntvdm64_type_unsigned_16 port_id, C_VOID *owner) {
    t_fdc *fdc = owner;
    (C_VOID)port;
    (C_VOID)port_id;
    fdc->connect.port->data.ioByte = fdc->data.dir;
}

/* write digital output register */
static C_VOID write_03f2(t_port *port, ntvdm64_type_unsigned_16 port_id, C_VOID *owner) {
    t_fdc *fdc = owner;
    (C_VOID)port;
    (C_VOID)port_id;
    if (!NTVDM64_TYPE_GET_BIT(fdc->data.dor, VFDC_DOR_NRS) && NTVDM64_TYPE_GET_BIT(fdc->connect.port->data.ioByte, VFDC_DOR_NRS)) {
        SetMSRReadyWrite;
    }
    fdc->data.dor = fdc->connect.port->data.ioByte;
    if (!NTVDM64_TYPE_GET_BIT(fdc->data.dor, VFDC_DOR_NRS)) {
        reset_controller(fdc);
    }
}
/* write standard commands */
static C_VOID write_03f5(t_port *port, ntvdm64_type_unsigned_16 port_id, C_VOID *owner) {
    t_fdc *fdc = owner;
    (C_VOID)port;
    (C_VOID)port_id;
    if (!GetMSRReadyWrite) {
        return;
    } else {
        SetMSRProcWrite;
    }
    fdc->data.cmd[fdc->data.rwCount++] = fdc->connect.port->data.ioByte;
    switch (fdc->data.cmd[0]) {
    case CMD_SPECIFY:
        if (fdc->data.rwCount == 3) {
            execute_specify(fdc);
        }
        break;
    case CMD_SENSE_DRIVE_STATUS:
        if (fdc->data.rwCount == 2) {
            execute_sense_drive_status(fdc);
        }
        break;
    case CMD_RECALIBRATE:
        if (fdc->data.rwCount == 2) {
            execute_recalibrate(fdc);
        }
        break;
    case CMD_SENSE_INTERRUPT:
        if (fdc->data.rwCount == 1) {
            execute_sense_interrupt(fdc);
        }
        break;
    case CMD_SEEK:
        if (fdc->data.rwCount == 3) {
            execute_seek(fdc);
        }
        break;
    case CMD_READ_TRACK:
        if (fdc->data.rwCount == 9) {
            execute_read_track(fdc);
        }
        break;
    case CMD_READ_ID:
        if (fdc->data.rwCount == 2) {
            execute_read_id(fdc);
        }
        break;
    case CMD_FORMAT_TRACK:
        if (fdc->data.rwCount == 6) {
            execute_format_track(fdc);
        }
        break;
    case CMD_WRITE_DATA:
        if (fdc->data.rwCount == 9) {
            execute_write_data(fdc);
        }
        break;
    case CMD_READ_DATA_ALL:
        if (fdc->data.rwCount == 9) {
            execute_read_data_all(fdc);
        }
        break;
    case CMD_WRITE_DELETED_DATA:
        if (fdc->data.rwCount == 9) {
            execute_write_deleted_data(fdc);
        }
        break;
    case CMD_READ_DELETED_DATA:
        if (fdc->data.rwCount == 9) {
            execute_read_deleted_data(fdc);
        }
        break;
    case CMD_SCAN_EQUAL_ALL:
        if (fdc->data.rwCount == 9) {
            execute_scan_equal_all(fdc);
        }
        break;
    case CMD_READ_DATA:
        if (fdc->data.rwCount == 9) {
            execute_read_data(fdc);
        }
        break;
    case CMD_SCAN_EQUAL:
        if (fdc->data.rwCount == 9) {
            execute_scan_equal(fdc);
        }
        break;
    default:
        execute_error(fdc);
        break;
    }
}
static C_VOID write_03f7(t_port *port, ntvdm64_type_unsigned_16 port_id, C_VOID *owner) {
    t_fdc *fdc = owner;
    (C_VOID)port;
    (C_VOID)port_id;
    fdc->data.ccr = fdc->connect.port->data.ioByte;
}

static C_VOID dma_close(C_VOID *owner, t_latch *latch)
{
    t_fdc *fdc = owner;
    (C_VOID)latch;
    finish_transfer(fdc);
}

C_VOID vm_machine_fdc_initialize(t_fdc *fdc)
{
    if (fdc == STD_NULL || fdc->connect.port == STD_NULL) return;
    STD_MEMSET((C_VOID *)&fdc->data, NTVDM64_TYPE_ZERO_8, sizeof(fdc->data));
    fdc->data.ccr = VFDC_CCR_DRC;
    core_machine_port_add_read(fdc->connect.port, 0x03f4, read_03f4, fdc);
    core_machine_port_add_read(fdc->connect.port, 0x03f5, read_03f5, fdc);
    core_machine_port_add_read(fdc->connect.port, 0x03f7, read_03f7, fdc);
    core_machine_port_add_write(fdc->connect.port, 0x03f2, write_03f2, fdc);
    core_machine_port_add_write(fdc->connect.port, 0x03f5, write_03f5, fdc);
    core_machine_port_add_write(fdc->connect.port, 0x03f7, write_03f7, fdc);
    core_machine_dma_bind_device(fdc->connect.dma_primary,
        fdc->connect.dma_secondary, 2, dma_read, dma_write, dma_close, fdc);
}

C_VOID vm_machine_fdc_reset(t_fdc *fdc)
{
    if (fdc != STD_NULL) reset_controller(fdc);
}

C_VOID vm_machine_fdc_refresh(t_fdc *fdc)
{
    if (fdc == STD_NULL || fdc->connect.fdd == STD_NULL) return;
    if (!fdc->connect.fdd->connect.flagDiskExist) {
        NTVDM64_TYPE_SET_BIT(fdc->data.dir, VFDC_DIR_DC);
    } else {
        NTVDM64_TYPE_CLEAR_BIT(fdc->data.dir, VFDC_DIR_DC);
    }
}

C_VOID vm_machine_fdc_finalize(t_fdc *fdc) { (C_VOID)fdc; }

/* Prints FDC status */
C_VOID vm_machine_fdc_print(const t_fdc *fdc) {
    ntvdm64_type_native_unsigned i;
    STD_PRINTF("FDC INFO\n========\n");
    STD_PRINTF("msr = %x, dir = %x, dor = %x, ccr = %x, dr = %x\n",
           fdc->data.msr,fdc->data.dir,fdc->data.dor,fdc->data.ccr,fdc->data.dr);
    STD_PRINTF("hut = %x, hlt = %x, srt = %x, Non-DMA = %x, INTR = %x\n",
           fdc->data.hut,fdc->data.hlt,fdc->data.srt,fdc->data.flagNDMA,fdc->data.flagINTR);
    STD_PRINTF("rwCount = %x, st0 = %x, st1 = %x, st2 = %x, st3 = %x\n",
           fdc->data.rwCount,fdc->data.st0,fdc->data.st1,fdc->data.st2,fdc->data.st3);
    for (i = 0; i < 9; ++i) {
        STD_PRINTF("cmd[%d] = %x, ", i, fdc->data.cmd[i]);
    }
    STD_PRINTF("\n");
    for (i = 0; i < 7; ++i) {
        STD_PRINTF("ret[%d] = %x, ", i, fdc->data.ret[i]);
    }
    STD_PRINTF("\n");
}

/*
FOR FDD READ
of3f1 00  refresh
o03f5 0f  seek command
o03f5 00  drv 0 head 0
o03f5 00  cyl 0
if3f0     show status: flagINTR=1,ReadyWrite
o03f5 08  sense interrupt
if3f0     show status: flagINTR=0,ReadyRead
i03f5     show st0: 0x20
i03f5     show cyl: 0x00
if3f0     show status: ReadyWrite
o03f5 e6  read data
o03f5 00  dev 0 head 0
o03f5 00  cyl 0
o03f5 00  head 0
o03f5 02  sector 2 (start)
o03f5 02  sector size 512B
o03f5 12  end sector id 18
o03f5 1b  gap length 1b
o03f5 ff  customized sector size not used
if3f0     show status: ExecCmd

FOR FDD WRITE
of3f1 00  refresh
o03f5 0f  seek command
o03f5 00  drv 0 head 0
o03f5 00  cyl 0
if3f0     show status: flagINTR=1,ReadyWrite
o03f5 08  sense interrupt
if3f0     show status: flagINTR=0,ReadyRead
i03f5     show st0: 0x20
i03f5     show cyl: 0x01
if3f0     show status: ReadyWrite
o03f5 c5  write data
o03f5 00  dev 0 head 0
o03f5 00  cyl 0
o03f5 00  head 0
o03f5 02  sector 2 (start)
o03f5 02  sector size 512B
o03f5 12  end sector id 18
o03f5 1b  gap length 1b
o03f5 ff  customized sector size not used
if3f0     show status: ExecCmd
*/
