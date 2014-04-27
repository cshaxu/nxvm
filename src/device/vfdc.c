/* Copyright 2012-2014 Neko. */

/* VFDC implements Floppy Driver Controller: Intel 8272A. */

#include "vmachine.h"
#include "vport.h"
#include "vbios.h"
#include "vpic.h"
#include "vdma.h"
#include "vfdd.h"
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
	memset(&vfdc, 0, sizeof(t_fdc));
	vfdc.ccr = ccr;
}

/* sector size code */
t_nubit8 VFDC_GetBPSC(t_nubit16 cbyte) {
	switch (cbyte) {
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
#define SetST0 (vfdc.st0 = (0x00                    << 7) |                   \
                           (0x00                    << 6) |                   \
                           (0x00                    << 5) |                   \
                           ((vfdd.cyl >= vfdd.ncyl) << 4) |                   \
                           (0x00                    << 3) |                   \
                           (vfdd.head               << 2) |                   \
                           (VFDC_GetDS(vfdc.cmd[1])      << 0))
#define SetST1 (vfdc.st1 = ((vfdd.sector >= (vfdd.nsector + 1)) << 7) |       \
                           (0x00                    << 6) |                   \
                           (0x00                    << 5) |                   \
                           (0x00                    << 4) |                   \
                           (0x00                    << 3) |                   \
                           ((vfdd.cyl >= vfdd.ncyl) << 2) |                   \
                           (0x00                    << 1) |                   \
                           (0x00                    << 0))
#define SetST2 (vfdc.st2 = (0x00                    << 7) |                   \
                           (0x00                    << 6) |                   \
                           (0x00                    << 5) |                   \
                           ((vfdd.cyl >= vfdd.ncyl) << 4) |                   \
                           (0x00                    << 3) |                   \
                           (0x00                    << 2) |                   \
                           (0x00                    << 1) |                   \
                           (0x00                    << 0))
#define SetST3 (vfdc.st3 = (0x00                    << 7) |                   \
                           (vfdd.flagro             << 6) |                   \
                           (0x01                    << 5) |                   \
                           ((!vfdd.cyl)             << 4) |                   \
                           (0x01                    << 3) |                   \
                           (vfdd.head               << 2) |                   \
                           (VFDC_GetDS(vfdc.cmd[1])      << 0))
#define SetMSRReadyRead  (vfdc.msr = 0xc0, vfdc.rwid = 0x00)
#define SetMSRReadyWrite (vfdc.msr = 0x80, vfdc.rwid = 0x00)
#define SetMSRProcRead   (vfdc.msr = 0xd0)
#define SetMSRProcWrite  (vfdc.msr = 0x90)
#define SetMSRExecCmd    (vfdc.msr = 0x20)

#define GetMSRReadyRead  ((vfdc.msr & 0xc0) == 0xc0)
#define GetMSRReadyWrite ((vfdc.msr & 0xc0) == 0x80)
#define GetMSRProcRW     (!!(vfdc.msr & 0x10))
#define GetMSRExecCmd    (!!(vfdc.msr & 0x20))

static void ExecCmdSpecify() {
	vfdc.hut      = VFDC_GetHUT (vfdc.cmd[1]);
	vfdc.srt      = VFDC_GetSRT (vfdc.cmd[1]);
	vfdc.hlt      = VFDC_GetHLT (vfdc.cmd[2]);
	vfdc.flagndma = VFDC_GetNDMA(vfdc.cmd[2]);
	SetMSRReadyWrite;
}
static void ExecCmdSenseDriveStatus() {
	vfdd.head = VFDC_GetHDS(vfdc.cmd[1]);
	vfddSetPointer;
	SetST3;
	vfdc.ret[0] = vfdc.st3;
	SetMSRReadyRead;
}
static void ExecCmdRecalibrate() {
	vfdd.cyl    = 0x00;
	vfdd.head   = 0x00;
	vfdd.sector = 0x01;
	vfddSetPointer;
	SetST0;
	vfdc.st0 |= 0x20;
	if (VFDC_GetENRQ(vfdc.dor)) {
		vpicSetIRQ(0x06);
		vfdc.flagintr = 1;
	}
	SetMSRReadyWrite;
}
static void ExecCmdSenseInterrupt() {
	if (vfdc.flagintr) {
		vfdc.ret[0] = vfdc.st0;
		vfdc.ret[1] = (t_nubit8)vfdd.cyl;
		vfdc.flagintr = 0;
	} else {
		vfdc.ret[0] = vfdc.st0 = VFDC_RET_ERROR;
	}
	SetMSRReadyRead;
}
static void ExecCmdSeek() {
	vfdd.head = VFDC_GetHDS(vfdc.cmd[1]);
	vfdd.cyl  = vfdc.cmd[2];
	vfdd.sector = 0x01;
	vfddSetPointer;
	SetST0;
	vfdc.st0 |= 0x20;
	if (VFDC_GetENRQ(vfdc.dor)) {
		vpicSetIRQ(0x06);
		vfdc.flagintr = 1;
	}
	SetMSRReadyWrite;
}
#define     ExecCmdReadTrack        vfdcTransInit
static void ExecCmdReadID() {
	vfdd.head = VFDC_GetHDS(vfdc.cmd[1]);
	vfdd.sector = 0x01;
	vfddSetPointer;
	vfdc.dr = 0x00; /* data register: sector id info */
	vfdcTransFinal();
}
static void ExecCmdFormatTrack() {
	/* NOTE: simplified procedure; dma not used */
	t_nubit8 fillbyte;
	/* load parameters*/
	vfdd.head    = VFDC_GetHDS(vfdc.cmd[1]);
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
	vfdc.ret[3] = 0x00;
	vfdc.ret[4] = 0x00;
	vfdc.ret[5] = 0x00;
	vfdc.ret[6] = 0x00;
	if (VFDC_GetENRQ(vfdc.dor)) {
		vpicSetIRQ(0x06);
		vfdc.flagintr = 1;
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
	vfdc.st2 |= 0x04; /* assume all data match; otherwise use 0x08 */
	vfdc.ret[0] = vfdc.st0;
	vfdc.ret[1] = vfdc.st1;
	vfdc.ret[2] = vfdc.st2;
	vfdc.ret[3] = (t_nubit8)vfdd.cyl;
	vfdc.ret[4] = (t_nubit8)vfdd.head;
	vfdc.ret[5] = (t_nubit8)vfdd.sector; /* NOTE: eot not changed */
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
	if (!(vfdc.dor & 0x04) && (vport.iobyte & 0x04)) {
		SetMSRReadyWrite;
	}
	vfdc.dor = vport.iobyte;
	if (!(vfdc.dor & 0x04)) {
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
	vfdd.count     = 0x00;
	vfddSetPointer;
	/* send trans request */
	if (!vfdc.flagndma && VFDC_GetENRQ(vfdc.dor)) {
		vdmaSetDRQ(0x02);
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
	vfdc.ret[3] = (t_nubit8)vfdd.cyl;
	vfdc.ret[4] = (t_nubit8)vfdd.head;
	vfdc.ret[5] = (t_nubit8)vfdd.sector;
	vfdc.ret[6] = VFDC_GetBPSC(vfdd.nbyte);
	if (VFDC_GetENRQ(vfdc.dor)) {
		vpicSetIRQ(0x06);
		vfdc.flagintr = 1;
	}
	SetMSRReadyRead;
}

static void init() {
	memset(&vfdc, 0, sizeof(t_fdc));
	vfdc.ccr = 0x02;
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

static void reset() {
	doReset();
}

static void refresh() {
	if (!vfdd.flagexist) {
		vfdc.dir |= 0x80;
	} else {
		vfdc.dir &= 0x7f;
	}
}

static void final() {}

void vfdcRegister() {
	vmachine.deviceTable[VMACHINE_DEVICE_INIT][vmachine.numDevices] = (t_faddrcc) init;
	vmachine.deviceTable[VMACHINE_DEVICE_RESET][vmachine.numDevices] = (t_faddrcc) reset;
	vmachine.deviceTable[VMACHINE_DEVICE_REFRESH][vmachine.numDevices] = (t_faddrcc) refresh;
	vmachine.deviceTable[VMACHINE_DEVICE_FINAL][vmachine.numDevices] = (t_faddrcc) final;
	vmachine.numDevices++;
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
