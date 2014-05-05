/* Copyright 2012-2014 Neko. */

/* VFDC implements Floppy Driver Controller: Intel 8272A. */

#include "../utils.h"

#include "vdma.h"
#include "vfdd.h"
#include "vpic.h"

#include "vbios.h"
#include "vport.h"
#include "vfdc.h"

t_fdc vfdc;

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
t_nubit8 VFDC_GetBPSC(t_nubit16 cb) {
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

#define IsRet(retl, count) (vfdc.data.cmd[0] == (retl) && vfdc.data.flagret == (count))
#define SetST0 (vfdc.data.st0 = (0x00      << 7) | \
                           (0x00      << 6) | \
                           (0x00      << 5) | \
                           ((vfdd.data.cyl >= vfdd.data.ncyl) << 4) | \
                           (0x00      << 3) | \
                           (vfdd.data.head << 2) | \
                           ((vfdc.data.cmd[1] & VFDC_ST0_DS) << 0))
#define SetST1 (vfdc.data.st1 = ((vfdd.data.sector >= (vfdd.data.nsector + 1)) << 7) | \
                           (0x00 << 6) | \
                           (0x00 << 5) | \
                           (0x00 << 4) | \
                           (0x00 << 3) | \
                           ((vfdd.data.cyl >= vfdd.data.ncyl) << 2) | \
                           (0x00 << 1) | \
                           (0x00 << 0))
#define SetST2 (vfdc.data.st2 = (0x00 << 7) | \
                           (0x00 << 6) | \
                           (0x00 << 5) | \
                           ((vfdd.data.cyl >= vfdd.data.ncyl) << 4) | \
                           (0x00 << 3) | \
                           (0x00 << 2) | \
                           (0x00 << 1) | \
                           (0x00 << 0))
#define SetST3 (vfdc.data.st3 = (0x00        << 7) | \
                           (vfdd.connect.flagReadOnly << 6) | \
                           (0x01        << 5) | \
                           ((!vfdd.data.cyl) << 4) | \
                           (0x01        << 3) | \
                           (vfdd.data.head   << 2) | \
                           ((vfdc.data.cmd[1] & VFDC_ST3_DS) << 0))
#define SetMSRReadyRead  (vfdc.data.msr = VFDC_MSR_ReadyRead, vfdc.data.rwCount = 0)
#define SetMSRReadyWrite (vfdc.data.msr = VFDC_MSR_ReadyWrite, vfdc.data.rwCount = 0)
#define SetMSRProcRead   (vfdc.data.msr = VFDC_MSR_ProcessRead)
#define SetMSRProcWrite  (vfdc.data.msr = VFDC_MSR_ProcessWrite)
#define SetMSRExecCmd    (vfdc.data.msr = VFDC_MSR_NDM)

#define GetMSRReadyRead  ((vfdc.data.msr & 0xc0) == VFDC_MSR_ReadyRead)
#define GetMSRReadyWrite ((vfdc.data.msr & 0xc0) == VFDC_MSR_ReadyWrite)
#define GetMSRProcRW     (GetBit(vfdc.data.msr, VFDC_MSR_CB))
#define GetMSRExecCmd    (GetBit(vfdc.data.msr, VFDC_MSR_NDM))

/* Resets FDC but keeps CCR */
static void doReset() {
    t_nubit8 ccr = vfdc.data.ccr;
    MEMSET((void *)(&vfdc.data), Zero8, sizeof(t_fdc_data));
    vfdc.data.ccr = ccr;
}

static void transRead() {
    /* NOTE: being called by DMA/PIO */
    vfddTransRead();
}
static void transWrite() {
    /* NOTE: being called by DMA/PIO */
    vfddTransWrite();
}
static void transInit() {
    /* NOTE: being called internally in vfdc */
    /* read parameters */
    vfdd.data.cyl       = vfdc.data.cmd[2];
    vfdd.data.head      = vfdc.data.cmd[3];
    vfdd.data.sector    = vfdc.data.cmd[4];
    vfdd.data.nbyte     = VFDC_GetBPS(vfdc.data.cmd[5]);
    vfdd.data.nsector   = vfdc.data.cmd[6];
    vfdd.data.gpl       = vfdc.data.cmd[7];
    if (!vfdc.data.cmd[5]) {
        vfdd.data.nbyte = vfdc.data.cmd[8];
    }
    vfdd.connect.transCount = Zero16;
    vfddSetPointer;
    /* send trans request */
    if (!vfdc.data.flagNDMA && GetBit(vfdc.data.dor, VFDC_DOR_ENRQ)) {
        vdmaSetDRQ(2);
    }
    SetMSRExecCmd;
}
static void transFinal() {
    /* NOTE: being called by DMA/PIO */
    SetST0;
    SetST1;
    SetST2;
    vfdc.data.ret[0] = vfdc.data.st0;
    vfdc.data.ret[1] = vfdc.data.st1;
    vfdc.data.ret[2] = vfdc.data.st2;
    vfdc.data.ret[3] = GetMax8(vfdd.data.cyl);
    vfdc.data.ret[4] = GetMax8(vfdd.data.head);
    vfdc.data.ret[5] = GetMax8(vfdd.data.sector);
    vfdc.data.ret[6] = VFDC_GetBPSC(vfdd.data.nbyte);
    if (GetBit(vfdc.data.dor, VFDC_DOR_ENRQ)) {
        vpicSetIRQ(0x06);
        vfdc.data.flagINTR = True;
    }
    SetMSRReadyRead;
}

static void ExecCmdSpecify() {
    vfdc.data.hut      = VFDC_GetCMD_Specify1_HUT(vfdc.data.cmd[1]);
    vfdc.data.srt      = VFDC_GetCMD_Specify1_SRT(vfdc.data.cmd[1]);
    vfdc.data.hlt      = VFDC_GetCMD_Specify2_HLT(vfdc.data.cmd[2]);
    vfdc.data.flagNDMA = GetBit(vfdc.data.cmd[2], VFDC_CMD_Specify2_ND);
    SetMSRReadyWrite;
}
static void ExecCmdSenseDriveStatus() {
    vfdd.data.head = GetBit(vfdc.data.cmd[1], VFDC_CMD_SenseDriveStatus1_HD);
    vfddSetPointer;
    SetST3;
    vfdc.data.ret[0] = vfdc.data.st3;
    SetMSRReadyRead;
}
static void ExecCmdRecalibrate() {
    vfdd.data.cyl    = 0;
    vfdd.data.head   = 0;
    vfdd.data.sector = 1;
    vfddSetPointer;
    SetST0;
    SetBit(vfdc.data.st0, VFDC_ST0_SEEK_END);
    if (GetBit(vfdc.data.dor, VFDC_DOR_ENRQ)) {
        vpicSetIRQ(0x06);
        vfdc.data.flagINTR = True;
    }
    SetMSRReadyWrite;
}
static void ExecCmdSenseInterrupt() {
    if (vfdc.data.flagINTR) {
        vfdc.data.ret[0] = vfdc.data.st0;
        vfdc.data.ret[1] = (t_nubit8)vfdd.data.cyl;
        vfdc.data.flagINTR = False;
    } else {
        vfdc.data.ret[0] = vfdc.data.st0 = VFDC_RET_ERROR;
    }
    SetMSRReadyRead;
}
static void ExecCmdSeek() {
    vfdd.data.head = GetBit(vfdc.data.cmd[1], VFDC_CMD_Seek1_HD);
    vfdd.data.cyl  = vfdc.data.cmd[2];
    vfdd.data.sector = 1;
    vfddSetPointer;
    SetST0;
    SetBit(vfdc.data.st0, VFDC_ST0_SEEK_END);
    if (GetBit(vfdc.data.dor, VFDC_DOR_ENRQ)) {
        vpicSetIRQ(0x06);
        vfdc.data.flagINTR = True;
    }
    SetMSRReadyWrite;
}
#define     ExecCmdReadTrack        transInit
static void ExecCmdReadID() {
    vfdd.data.head = GetBit(vfdc.data.cmd[1], VFDC_CMD_ReadId1_HD);
    vfdd.data.sector = 1;
    vfddSetPointer;
    vfdc.data.dr = Zero8; /* data register: sector id info */
    transFinal();
}
static void ExecCmdFormatTrack() {
    /* NOTE: simplified procedure; dma not used */
    t_nubit8 fillByte;
    /* load parameters*/
    vfdd.data.head    = GetBit(vfdc.data.cmd[1], VFDC_CMD_FormatTrack1_HD);
    vfdd.data.sector  = 0x01;
    vfdd.data.nbyte   = VFDC_GetBPS(vfdc.data.cmd[2]);
    vfdd.data.nsector = vfdc.data.cmd[3];
    vfdd.data.gpl     = vfdc.data.cmd[4];
    fillByte     = vfdc.data.cmd[5];
    vfddSetPointer;
    /* execute format track*/
    vfddFormatTrack(fillByte);
    /* finish transaction */
    SetST0;
    SetST1;
    SetST2;
    vfdc.data.ret[0] = vfdc.data.st0;
    vfdc.data.ret[1] = vfdc.data.st1;
    vfdc.data.ret[2] = vfdc.data.st2;
    vfdc.data.ret[3] = Zero8;
    vfdc.data.ret[4] = Zero8;
    vfdc.data.ret[5] = Zero8;
    vfdc.data.ret[6] = Zero8;
    if (GetBit(vfdc.data.dor, VFDC_DOR_ENRQ)) {
        vpicSetIRQ(0x06);
        vfdc.data.flagINTR = True;
    }
    SetMSRReadyRead;
}
#define     ExecCmdWriteData        transInit
#define     ExecCmdReadDataAll      transInit
#define     ExecCmdWriteDeletedData transInit
#define     ExecCmdReadDeletedData  transInit
#define     ExecCmdScanEqualAll     ExecCmdScanEqual
#define     ExecCmdReadData         transInit
static void ExecCmdScanEqual() {
    /* NOTE: not fully implemented; lack of reference */
    SetST0;
    SetST1;
    SetST2;
    /* assume all data match */
    SetBit(vfdc.data.st2, VFDC_ST2_SCAN_MATCH);
    vfdc.data.ret[0] = vfdc.data.st0;
    vfdc.data.ret[1] = vfdc.data.st1;
    vfdc.data.ret[2] = vfdc.data.st2;
    vfdc.data.ret[3] = GetMax8(vfdd.data.cyl);
    vfdc.data.ret[4] = GetMax8(vfdd.data.head);
    vfdc.data.ret[5] = GetMax8(vfdd.data.sector); /* NOTE: eot not changed */
    vfdc.data.ret[6] = VFDC_GetBPSC(vfdd.data.nbyte);
    SetMSRReadyRead;
}
static void ExecCmdError() {
    vfdc.data.ret[0] = VFDC_RET_ERROR;
    SetMSRReadyRead;
}

/* read main status register */
static void io_read_03F4() {
    vport.data.ioByte = vfdc.data.msr;
}
/* read standard results */
static void io_read_03F5() {
    if (!GetMSRReadyRead) {
        return;
    } else {
        SetMSRProcRead;
    }
    vport.data.ioByte = vfdc.data.ret[vfdc.data.rwCount++];
    switch (vfdc.data.cmd[0]) {
    case CMD_SPECIFY:
        if (vfdc.data.rwCount >= 0) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_SENSE_DRIVE_STATUS:
        if (vfdc.data.rwCount >= 1) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_RECALIBRATE:
        if (vfdc.data.rwCount >= 0) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_SENSE_INTERRUPT:
        if (vfdc.data.rwCount >= 2) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_SEEK:
        if (vfdc.data.rwCount >= 0) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_READ_TRACK:
        if (vfdc.data.rwCount >= 7) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_READ_ID:
        if (vfdc.data.rwCount >= 7) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_FORMAT_TRACK:
        if (vfdc.data.rwCount >= 7) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_WRITE_DATA:
        if (vfdc.data.rwCount >= 7) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_READ_DATA_ALL:
        if (vfdc.data.rwCount >= 7) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_WRITE_DELETED_DATA:
        if (vfdc.data.rwCount >= 7) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_READ_DELETED_DATA:
        if (vfdc.data.rwCount >= 7) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_SCAN_EQUAL_ALL:
        if (vfdc.data.rwCount >= 7) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_READ_DATA:
        if (vfdc.data.rwCount >= 7) {
            SetMSRReadyWrite;
        }
        break;
    case CMD_SCAN_EQUAL:
        if (vfdc.data.rwCount >= 7) {
            SetMSRReadyWrite;
        }
        break;
    default:
        if (vfdc.data.rwCount >= 1) {
            SetMSRReadyWrite;
        }
        break;
    }
}
/* read digital input register */
static void io_read_03F7() {
    vport.data.ioByte = vfdc.data.dir;
}

/* write digital output register */
static void io_write_03F2() {
    if (!GetBit(vfdc.data.dor, VFDC_DOR_NRS) && GetBit(vport.data.ioByte, VFDC_DOR_NRS)) {
        SetMSRReadyWrite;
    }
    vfdc.data.dor = vport.data.ioByte;
    if (!GetBit(vfdc.data.dor, VFDC_DOR_NRS)) {
        doReset();
    }
}
/* write standard commands */
static void io_write_03F5() {
    if (!GetMSRReadyWrite) {
        return;
    } else {
        SetMSRProcWrite;
    }
    vfdc.data.cmd[vfdc.data.rwCount++] = vport.data.ioByte;
    switch (vfdc.data.cmd[0]) {
    case CMD_SPECIFY:
        if (vfdc.data.rwCount == 3) {
            ExecCmdSpecify();
        }
        break;
    case CMD_SENSE_DRIVE_STATUS:
        if (vfdc.data.rwCount == 2) {
            ExecCmdSenseDriveStatus();
        }
        break;
    case CMD_RECALIBRATE:
        if (vfdc.data.rwCount == 2) {
            ExecCmdRecalibrate();
        }
        break;
    case CMD_SENSE_INTERRUPT:
        if (vfdc.data.rwCount == 1) {
            ExecCmdSenseInterrupt();
        }
        break;
    case CMD_SEEK:
        if (vfdc.data.rwCount == 3) {
            ExecCmdSeek();
        }
        break;
    case CMD_READ_TRACK:
        if (vfdc.data.rwCount == 9) {
            ExecCmdReadTrack();
        }
        break;
    case CMD_READ_ID:
        if (vfdc.data.rwCount == 2) {
            ExecCmdReadID();
        }
        break;
    case CMD_FORMAT_TRACK:
        if (vfdc.data.rwCount == 6) {
            ExecCmdFormatTrack();
        }
        break;
    case CMD_WRITE_DATA:
        if (vfdc.data.rwCount == 9) {
            ExecCmdWriteData();
        }
        break;
    case CMD_READ_DATA_ALL:
        if (vfdc.data.rwCount == 9) {
            ExecCmdReadDataAll();
        }
        break;
    case CMD_WRITE_DELETED_DATA:
        if (vfdc.data.rwCount == 9) {
            ExecCmdWriteDeletedData();
        }
        break;
    case CMD_READ_DELETED_DATA:
        if (vfdc.data.rwCount == 9) {
            ExecCmdReadDeletedData();
        }
        break;
    case CMD_SCAN_EQUAL_ALL:
        if (vfdc.data.rwCount == 9) {
            ExecCmdScanEqualAll();
        }
        break;
    case CMD_READ_DATA:
        if (vfdc.data.rwCount == 9) {
            ExecCmdReadData();
        }
        break;
    case CMD_SCAN_EQUAL:
        if (vfdc.data.rwCount == 9) {
            ExecCmdScanEqual();
        }
        break;
    default:
        ExecCmdError();
        break;
    }
}
static void io_write_03F7() {
    vfdc.data.ccr = vport.data.ioByte;
}

#define dmaReadMe   transRead
#define dmaWriteMe  transWrite
#define dmaCloseMe  transFinal

void vfdcInit() {
    MEMSET((void *)(&vfdc), Zero8, sizeof(t_fdc));
    vfdc.data.ccr = VFDC_CCR_DRC;
    vportAddRead(0x03f4, (t_faddrcc) io_read_03F4);
    vportAddRead(0x03f5, (t_faddrcc) io_read_03F5);
    vportAddRead(0x03f7, (t_faddrcc) io_read_03F7);
    vportAddWrite(0x03f2, (t_faddrcc) io_write_03F2);
    vportAddWrite(0x03f5, (t_faddrcc) io_write_03F5);
    vportAddWrite(0x03f7, (t_faddrcc) io_write_03F7);

    /* connect vfdc to dma request 2 (on vdma1) */
    vdmaAddMe(2);

    vbiosAddPost(VFDC_POST);
    vbiosAddInt(VFDC_INT_HARD_FDD_0E, 0x0e);
    /* overridden by VHDC_INT_SOFT_HDD_13 */
    vbiosAddInt(VFDC_INT_SOFT_FDD_40, 0x13);
    vbiosAddInt(VFDC_INT_SOFT_FDD_40, 0x40);
}

void vfdcReset() {
    doReset();
}

void vfdcRefresh() {
    if (!vfdd.connect.flagDiskExist) {
        SetBit(vfdc.data.dir, VFDC_DIR_DC);
    } else {
        ClrBit(vfdc.data.dir, VFDC_DIR_DC);
    }
}

void vfdcFinal() {}

/* Prints FDC status */
void devicePrintFdc() {
    t_nubit8 i;
    PRINTF("FDC INFO\n========\n");
    PRINTF("msr = %x, dir = %x, dor = %x, ccr = %x, dr = %x\n",
           vfdc.data.msr,vfdc.data.dir,vfdc.data.dor,vfdc.data.ccr,vfdc.data.dr);
    PRINTF("hut = %x, hlt = %x, srt = %x, Non-DMA = %x, INTR = %x\n",
           vfdc.data.hut,vfdc.data.hlt,vfdc.data.srt,vfdc.data.flagNDMA,vfdc.data.flagINTR);
    PRINTF("rwCount = %x, st0 = %x, st1 = %x, st2 = %x, st3 = %x\n",
           vfdc.data.rwCount,vfdc.data.st0,vfdc.data.st1,vfdc.data.st2,vfdc.data.st3);
    for (i = 0; i < 9; ++i) {
        PRINTF("cmd[%d] = %x, ", i, vfdc.data.cmd[i]);
    }
    PRINTF("\n");
    for (i = 0; i < 7; ++i) {
        PRINTF("ret[%d] = %x, ", i, vfdc.data.ret[i]);
    }
    PRINTF("\n");
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
