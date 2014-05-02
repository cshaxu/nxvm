/* Copyright 2012-2014 Neko. */

/* VFDC implements Floppy Driver Controller: Intel 8272A. */

#include "../utils.h"

#include "vdma.h"
#include "vfdd.h"
#include "vpic.h"

#include "vbios.h"
#include "vport.h"
#include "vmachine.h"
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

/* Resets FDC but keeps CCR */
static void doReset() {
	t_nubit8 ccr = vfdc.ccr;
	MEMSET(&vfdc, Zero8, sizeof(t_fdc));
	vfdc.ccr = ccr;
}

/* sector size code */
t_nubit8 VFDC_GetBPSC(t_nubit16 cb) {
	switch (cb) {
	case 0x0080: return 0x00;
	case 0x0100: return 0x01;
	case 0x0200: return 0x02;
	case 0x0400: return 0x03;
	case 0x0800: return 0x04;
	case 0x1000: return 0x05;
	case 0x2000: return 0x06;
	case 0x4000: return 0x07;
	default:     return 0x00;
	}
}

#define IsRet(retl, count) (vfdc.cmd[0] == (retl) && vfdc.flagret == (count))
#define SetST0 (vfdc.st0 = (0x00      << 7) | \
                           (0x00      << 6) | \
                           (0x00      << 5) | \
                           ((vfdd.cyl >= vfdd.ncyl) << 4) | \
                           (0x00      << 3) | \
                           (vfdd.head << 2) | \
                           ((vfdc.cmd[1] & VFDC_ST0_DS) << 0))
#define SetST1 (vfdc.st1 = ((vfdd.sector >= (vfdd.nsector + 1)) << 7) | \
                           (0x00 << 6) | \
                           (0x00 << 5) | \
                           (0x00 << 4) | \
                           (0x00 << 3) | \
                           ((vfdd.cyl >= vfdd.ncyl) << 2) | \
                           (0x00 << 1) | \
                           (0x00 << 0))
#define SetST2 (vfdc.st2 = (0x00 << 7) | \
                           (0x00 << 6) | \
                           (0x00 << 5) | \
                           ((vfdd.cyl >= vfdd.ncyl) << 4) | \
                           (0x00 << 3) | \
                           (0x00 << 2) | \
                           (0x00 << 1) | \
                           (0x00 << 0))
#define SetST3 (vfdc.st3 = (0x00        << 7) | \
                           (vfdd.flagro << 6) | \
                           (0x01        << 5) | \
                           ((!vfdd.cyl) << 4) | \
                           (0x01        << 3) | \
                           (vfdd.head   << 2) | \
                           ((vfdc.cmd[1] & VFDC_ST3_DS) << 0))
#define SetMSRReadyRead  (vfdc.msr = VFDC_MSR_ReadyRead, vfdc.rwid = 0)
#define SetMSRReadyWrite (vfdc.msr = VFDC_MSR_ReadyWrite, vfdc.rwid = 0)
#define SetMSRProcRead   (vfdc.msr = VFDC_MSR_ProcessRead)
#define SetMSRProcWrite  (vfdc.msr = VFDC_MSR_ProcessWrite)
#define SetMSRExecCmd    (vfdc.msr = VFDC_MSR_NDM)

#define GetMSRReadyRead  ((vfdc.msr & 0xc0) == VFDC_MSR_ReadyRead)
#define GetMSRReadyWrite ((vfdc.msr & 0xc0) == VFDC_MSR_ReadyWrite)
#define GetMSRProcRW     (GetBit(vfdc.msr, VFDC_MSR_CB))
#define GetMSRExecCmd    (GetBit(vfdc.msr, VFDC_MSR_NDM))

static void ExecCmdSpecify() {
	vfdc.hut      = VFDC_GetCMD_Specify1_HUT(vfdc.cmd[1]);
	vfdc.srt      = VFDC_GetCMD_Specify1_SRT(vfdc.cmd[1]);
	vfdc.hlt      = VFDC_GetCMD_Specify2_HLT(vfdc.cmd[2]);
	vfdc.flagndma = GetBit(vfdc.cmd[2], VFDC_CMD_Specify2_ND);
	SetMSRReadyWrite;
}
static void ExecCmdSenseDriveStatus() {
	vfdd.head = GetBit(vfdc.cmd[1], VFDC_CMD_SenseDriveStatus1_HD);
	vfddSetPointer;
	SetST3;
	vfdc.ret[0] = vfdc.st3;
	SetMSRReadyRead;
}
static void ExecCmdRecalibrate() {
	vfdd.cyl    = 0;
	vfdd.head   = 0;
	vfdd.sector = 1;
	vfddSetPointer;
	SetST0;
	SetBit(vfdc.st0, VFDC_ST0_SEEK_END);
	if (GetBit(vfdc.dor, VFDC_DOR_ENRQ)) {
		vpicSetIRQ(0x06);
		vfdc.flagintr = True;
	}
	SetMSRReadyWrite;
}
static void ExecCmdSenseInterrupt() {
	if (vfdc.flagintr) {
		vfdc.ret[0] = vfdc.st0;
		vfdc.ret[1] = (t_nubit8)vfdd.cyl;
		vfdc.flagintr = False;
	} else {
		vfdc.ret[0] = vfdc.st0 = VFDC_RET_ERROR;
	}
	SetMSRReadyRead;
}
static void ExecCmdSeek() {
	vfdd.head = GetBit(vfdc.cmd[1], VFDC_CMD_Seek1_HD);
	vfdd.cyl  = vfdc.cmd[2];
	vfdd.sector = 1;
	vfddSetPointer;
	SetST0;
	SetBit(vfdc.st0, VFDC_ST0_SEEK_END);
	if (GetBit(vfdc.dor, VFDC_DOR_ENRQ)) {
		vpicSetIRQ(0x06);
		vfdc.flagintr = True;
	}
	SetMSRReadyWrite;
}
#define     ExecCmdReadTrack        vfdcTransInit
static void ExecCmdReadID() {
	vfdd.head = GetBit(vfdc.cmd[1], VFDC_CMD_ReadId1_HD);
	vfdd.sector = 1;
	vfddSetPointer;
	vfdc.dr = Zero8; /* data register: sector id info */
	vfdcTransFinal();
}
static void ExecCmdFormatTrack() {
	/* NOTE: simplified procedure; dma not used */
	t_nubit8 fillbyte;
	/* load parameters*/
	vfdd.head    = GetBit(vfdc.cmd[1], VFDC_CMD_FormatTrack1_HD);
	vfdd.sector  = 0x01;
	vfdd.nbyte   = VFDC_GetBPS(vfdc.cmd[2]);
	vfdd.nsector = vfdc.cmd[3];
	vfdd.gaplen  = vfdc.cmd[4];
	fillbyte     = vfdc.cmd[5];
	vfddSetPointer;
	/* execute format track*/
	vfddFormatTrack(fillbyte);
	/* finish transaction */
	SetST0;
	SetST1;
	SetST2;
	vfdc.ret[0] = vfdc.st0;
	vfdc.ret[1] = vfdc.st1;
	vfdc.ret[2] = vfdc.st2;
	vfdc.ret[3] = Zero8;
	vfdc.ret[4] = Zero8;
	vfdc.ret[5] = Zero8;
	vfdc.ret[6] = Zero8;
	if (GetBit(vfdc.dor, VFDC_DOR_ENRQ)) {
		vpicSetIRQ(0x06);
		vfdc.flagintr = True;
	}
	SetMSRReadyRead;
}
#define     ExecCmdWriteData        vfdcTransInit
#define     ExecCmdReadDataAll      vfdcTransInit
#define     ExecCmdWriteDeletedData vfdcTransInit
#define     ExecCmdReadDeletedData  vfdcTransInit
#define     ExecCmdScanEqualAll     ExecCmdScanEqual
#define     ExecCmdReadData         vfdcTransInit
static void ExecCmdScanEqual() {
	/* NOTE: not fully implemented; lack of reference */
	SetST0;
	SetST1;
	SetST2;
	/* assume all data match */
	SetBit(vfdc.st2, VFDC_ST2_SCAN_MATCH);
	vfdc.ret[0] = vfdc.st0;
	vfdc.ret[1] = vfdc.st1;
	vfdc.ret[2] = vfdc.st2;
	vfdc.ret[3] = GetMax8(vfdd.cyl);
	vfdc.ret[4] = GetMax8(vfdd.head);
	vfdc.ret[5] = GetMax8(vfdd.sector); /* NOTE: eot not changed */
	vfdc.ret[6] = VFDC_GetBPSC(vfdd.nbyte);
	SetMSRReadyRead;
}
static void ExecCmdError() {
	vfdc.ret[0] = VFDC_RET_ERROR;
	SetMSRReadyRead;
}

/* read main status register */
static void io_read_03F4() {
	vport.iobyte = vfdc.msr;
}
/* read standard results */
static void io_read_03F5() {
	if (!GetMSRReadyRead) {
		return;
	} else {
		SetMSRProcRead;
	}
	vport.iobyte = vfdc.ret[vfdc.rwid++];
	switch (vfdc.cmd[0]) {
	case CMD_SPECIFY:
		if (vfdc.rwid >= 0) {
			SetMSRReadyWrite;
		}
		break;
	case CMD_SENSE_DRIVE_STATUS:
		if (vfdc.rwid >= 1) {
			SetMSRReadyWrite;
		}
		break;
	case CMD_RECALIBRATE:
		if (vfdc.rwid >= 0) {
			SetMSRReadyWrite;
		}
		break;
	case CMD_SENSE_INTERRUPT:
		if (vfdc.rwid >= 2) {
			SetMSRReadyWrite;
		}
		break;
	case CMD_SEEK:
		if (vfdc.rwid >= 0) {
			SetMSRReadyWrite;
		}
		break;
	case CMD_READ_TRACK:
		if (vfdc.rwid >= 7) {
			SetMSRReadyWrite;
		}
		break;
	case CMD_READ_ID:
		if (vfdc.rwid >= 7) {
			SetMSRReadyWrite;
		}
		break;
	case CMD_FORMAT_TRACK:
		if (vfdc.rwid >= 7) {
			SetMSRReadyWrite;
		}
		break;
	case CMD_WRITE_DATA:
		if (vfdc.rwid >= 7) {
			SetMSRReadyWrite;
		}
		break;
	case CMD_READ_DATA_ALL:
		if (vfdc.rwid >= 7) {
			SetMSRReadyWrite;
		}
		break;
	case CMD_WRITE_DELETED_DATA:
		if (vfdc.rwid >= 7) {
			SetMSRReadyWrite;
		}
		break;
	case CMD_READ_DELETED_DATA:
		if (vfdc.rwid >= 7) {
			SetMSRReadyWrite;
		}
		break;
	case CMD_SCAN_EQUAL_ALL:
		if (vfdc.rwid >= 7) {
			SetMSRReadyWrite;
		}
		break;
	case CMD_READ_DATA:
		if (vfdc.rwid >= 7) {
			SetMSRReadyWrite;
		}
		break;
	case CMD_SCAN_EQUAL:
		if (vfdc.rwid >= 7) {
			SetMSRReadyWrite;
		}
		break;
	default: 
		if (vfdc.rwid >= 1) {
			SetMSRReadyWrite;
		}
		break;
	}
}
/* read digital input register */
static void io_read_03F7() {
	vport.iobyte = vfdc.dir;
}

/* write digital output register */
static void io_write_03F2() {
	if (!GetBit(vfdc.dor, VFDC_DOR_NRS) && GetBit(vport.iobyte, VFDC_DOR_NRS)) {
		SetMSRReadyWrite;
	}
	vfdc.dor = vport.iobyte;
	if (!GetBit(vfdc.dor, VFDC_DOR_NRS)) {
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
	vfdc.cmd[vfdc.rwid++] = vport.iobyte;
	switch (vfdc.cmd[0]) {
	case CMD_SPECIFY:
		if (vfdc.rwid == 3) {
			ExecCmdSpecify();
		}
		break;
	case CMD_SENSE_DRIVE_STATUS:
		if (vfdc.rwid == 2) {
			ExecCmdSenseDriveStatus();
		}
		break;
	case CMD_RECALIBRATE:
		if (vfdc.rwid == 2) {
			ExecCmdRecalibrate();
		}
		break;
	case CMD_SENSE_INTERRUPT:
		if (vfdc.rwid == 1) {
			ExecCmdSenseInterrupt();
		}
		break;
	case CMD_SEEK:
		if (vfdc.rwid == 3) {
			ExecCmdSeek();
		}
		break;
	case CMD_READ_TRACK:
		if (vfdc.rwid == 9) {
			ExecCmdReadTrack();
		}
		break;
	case CMD_READ_ID:
		if (vfdc.rwid == 2) {
			ExecCmdReadID();
		}
		break;
	case CMD_FORMAT_TRACK:
		if (vfdc.rwid == 6) {
			ExecCmdFormatTrack();
		}
		break;
	case CMD_WRITE_DATA:
		if (vfdc.rwid == 9) {
			ExecCmdWriteData();
		}
		break;
	case CMD_READ_DATA_ALL:
		if (vfdc.rwid == 9) {
			ExecCmdReadDataAll();
		}
		break;
	case CMD_WRITE_DELETED_DATA:
		if (vfdc.rwid == 9) {
			ExecCmdWriteDeletedData();
		}
		break;
	case CMD_READ_DELETED_DATA:
		if (vfdc.rwid == 9) {
			ExecCmdReadDeletedData();
		}
		break;
	case CMD_SCAN_EQUAL_ALL:
		if (vfdc.rwid == 9) {
			ExecCmdScanEqualAll();
		}
		break;
	case CMD_READ_DATA:
		if (vfdc.rwid == 9) {
			ExecCmdReadData();
		}
		break;
	case CMD_SCAN_EQUAL:
		if (vfdc.rwid == 9) {
			ExecCmdScanEqual();
		}
		break;
	default:
		ExecCmdError();
		break;
	}
}
static void io_write_03F7() {
	vfdc.ccr = vport.iobyte;
}

void vfdcTransRead() {
	/* NOTE: being called by DMA/PIO */
	vfddTransRead();
}
void vfdcTransWrite() {
	/* NOTE: being called by DMA/PIO */
	vfddTransWrite();
}
void vfdcTransInit() {
	/* NOTE: being called internally in vfdc */
	/* read parameters */
	vfdd.cyl       = vfdc.cmd[2];
	vfdd.head      = vfdc.cmd[3];
	vfdd.sector    = vfdc.cmd[4];
	vfdd.nbyte     = VFDC_GetBPS(vfdc.cmd[5]);
	vfdd.nsector   = vfdc.cmd[6];
	vfdd.gaplen    = vfdc.cmd[7];
	if (!vfdc.cmd[5]) {
		vfdd.nbyte = vfdc.cmd[8];
	}
	vfdd.count     = Zero16;
	vfddSetPointer;
	/* send trans request */
	if (!vfdc.flagndma && GetBit(vfdc.dor, VFDC_DOR_ENRQ)) {
		vdmaSetDRQ(2);
	}
	SetMSRExecCmd;
}
void vfdcTransFinal() {
	/* NOTE: being called by DMA/PIO */
	SetST0;
	SetST1;
	SetST2;
	vfdc.ret[0] = vfdc.st0;
	vfdc.ret[1] = vfdc.st1;
	vfdc.ret[2] = vfdc.st2;
	vfdc.ret[3] = GetMax8(vfdd.cyl);
	vfdc.ret[4] = GetMax8(vfdd.head);
	vfdc.ret[5] = GetMax8(vfdd.sector);
	vfdc.ret[6] = VFDC_GetBPSC(vfdd.nbyte);
	if (GetBit(vfdc.dor, VFDC_DOR_ENRQ)) {
		vpicSetIRQ(0x06);
		vfdc.flagintr = True;
	}
	SetMSRReadyRead;
}

static void init() {
	MEMSET(&vfdc, Zero8, sizeof(t_fdc));
	vfdc.ccr = VFDC_CCR_DRC;
	vport.in[0x03f4] = (t_faddrcc) io_read_03F4;
	vport.in[0x03f5] = (t_faddrcc) io_read_03F5;
	vport.in[0x03f7] = (t_faddrcc) io_read_03F7;
	vport.out[0x03f2] = (t_faddrcc) io_write_03F2;
	vport.out[0x03f5] = (t_faddrcc) io_write_03F5;
	vport.out[0x03f7] = (t_faddrcc) io_write_03F7;
    
    vbiosAddPost(VFDC_POST);
	vbiosAddInt(VFDC_INT_HARD_FDD_0E, 0x0e);
	vbiosAddInt(VFDC_INT_SOFT_FDD_40, 0x13); /* overridden by VHDC_INT_SOFT_HDD_13 */
	vbiosAddInt(VFDC_INT_SOFT_FDD_40, 0x40);
}

static void reset() {doReset();}

static void refresh() {
	if (!vfdd.flagexist) {
		SetBit(vfdc.dir, VFDC_DIR_DC);
	} else {
		ClrBit(vfdc.dir, VFDC_DIR_DC);
	}
}

static void final() {}

void vfdcRegister() {vmachineAddMe;}

/* Prints FDC status */
void devicePrintFdc() {
	t_nubit8 i;
	PRINTF("FDC INFO\n========\n");
	PRINTF("msr = %x, dir = %x, dor = %x, ccr = %x, dr = %x\n",
		vfdc.msr,vfdc.dir,vfdc.dor,vfdc.ccr,vfdc.dr);
	PRINTF("hut = %x, hlt = %x, srt = %x, Non-DMA = %x, INTR = %x\n",
		vfdc.hut,vfdc.hlt,vfdc.srt,vfdc.flagndma,vfdc.flagintr);
	PRINTF("rwid = %x, st0 = %x, st1 = %x, st2 = %x, st3 = %x\n",
		vfdc.rwid,vfdc.st0,vfdc.st1,vfdc.st2,vfdc.st3);
	for (i = 0;i < 9;++i) {
		PRINTF("cmd[%d] = %x, ", i,vfdc.cmd[i]);
	}
	PRINTF("\n");
	for (i = 0;i < 7;++i) {
		PRINTF("ret[%d] = %x, ", i,vfdc.ret[i]);
	}
	PRINTF("\n");
	PRINTF("FDD INFO\n========\n");
	PRINTF("cyl = %x, head = %x, sector = %x\n",
		vfdd.cyl,vfdd.head,vfdd.sector);
	PRINTF("nsector = %x, nbyte = %x, gaplen = %x\n",
		vfdd.nsector,vfdd.nbyte,vfdd.gaplen);
	PRINTF("ReadOnly = %x, Exist = %x\n",
		vfdd.flagro,vfdd.flagexist);
	PRINTF("base = %x, curr = %x, count = %x\n",
		vfdd.base,vfdd.curr,vfdd.count);
}

/*
FOR FDD READ
of3f1 00  refresh
o03f5 0f  seek command
o03f5 00  drv 0 head 0
o03f5 00  cyl 0
if3f0     show status: flagintr=1,ReadyWrite
o03f5 08  sense interrupt
if3f0     show status: flagintr=0,ReadyRead
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
if3f0     show status: flagintr=1,ReadyWrite
o03f5 08  sense interrupt
if3f0     show status: flagintr=0,ReadyRead
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
