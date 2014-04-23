/* This file is a part of NXVM project. */

#include "memory.h"

#include "vcpu.h"
#include "vcpuins.h"
#include "vpic.h"
#include "vram.h"
#include "vdma.h"
#include "vfdd.h"
#include "vfdc.h"

t_fdc vfdc;

#define VFDC_RET_ERROR 0x80                                        /* Error Code */

/* Commands */
#define CMD_SPECIFY            0x03                  /* set drive parameters */
#define CMD_SENSE_DRIVE_STATUS 0x04
#define CMD_RECALIBRATE        0x07                    /* seek to cylinder 0 */
#define CMD_SENSE_INTERRUPT    0x08
                              /* acknowledge IRQ6 get status of last command */
#define CMD_SEEK               0x0f  /* ! seek (both(?)) heads to cylinder X */
#define CMD_VERSION            0x10     /* ! used during initialization once */

#define CMD_PERPENDICULAR_MODE 0x12
                                  /* ! used during initialization once maybe */
#define CMD_CONFIGURE          0x13           /* ! set controller parameters */
#define CMD_LOCK               0x14
                                 /* ! protect controller params from a reset */
#define CMD_VERIFY             0x16
#define CMD_SCAN_LOW_OR_EQUAL  0x19
#define CMD_SCAN_HIGH_OR_EQUAL 0x1d
#define CMD_READ_TRACK         0x42                        /* generates irq6 */
#define CMD_READ_ID            0x4a                        /* generates irq6 */
#define CMD_FORMAT_TRACK       0x4d                        /* generates irq6 */
#define CMD_WRITE_DATA         0xc5                     /* write to the disk */
#define CMD_READ_DATA_ALL      0xc6        /* read all data, even if deleted */
#define CMD_WRITE_DELETED_DATA 0xc9
#define CMD_READ_DELETED_DATA  0xcc
#define CMD_SCAN_EQUAL_ALL     0xd1
#define CMD_READ_DATA          0xe6                  /* ! read from the disk */
#define CMD_SCAN_EQUAL         0xf1

/* Command Bytes */
//#define GET_A0(cbyte)                                    /* Address Line 0 */
//#define GET_C(cbyte)                             /* Cylinder Number (0-79) */
//#define GET_D(cbyte)                                               /* Data */
//#define GET_DB(cbyte)                                  /* Data Bus (D7-D0) */
//#define GET_DTL(cbyte)                                      /* Data Length */
//#define GET_EOT(cbyte)                                     /* End Of Track */
//#define GET_GPL(cbyte)                                       /* Gap Length */
//#define GET_H(cbyte)                                 /* Head Address (0-1) */
//#define GET_MFM(cbyte)                                   /* fm or mfm mode */
//#define GET_MT(cbyte)                                       /* multi-track */
//#define GET_N(cbyte)                                             /* number */
//#define GET_NCN(cbyte)                              /* new cylinder number */
//#define GET_R(cbyte)                                             /* record */
//#define GET_RW(cbyte)                                 /* read/write signal */
//#define GET_SC(cbyte)                                            /* sector */
//#define GET_SK(cbyte)                                              /* skip */
//#define GET_STP(cbyte)                                             /* step */

#define GetENRQ(cbyte) ((cbyte) & 0x08)               /* enable dma and intr */
#define GetDS(cbyte)   ((cbyte) & 0x03)            /* drive select (ds0,ds1) */
#define GetHUT(cbyte)  ((cbyte) & 0x0f)                  /* head unload time */
#define GetSRT(cbyte)  ((cbyte) >> 4)                      /* step rate time */
#define GetHLT(cbyte)  ((cbyte) >> 1)                      /* head load time */
#define GetNDMA(cbyte) ((cbyte) & 0x01)                      /* non-dma mode */
#define GetHDS(cbyte)  (!!((cbyte) & 0x04))          /* head select (0 or 1) */
#define GetBPS(cbyte)  (0x0080 << (cbyte))               /* bytes per sector */
static t_nubit8 GetBPSC(t_nubit8 cbyte)                  /* sector size code */
{
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
                           (0x01                    << 5) |                   \
                           ((vfdd.cyl >= VFDD_NCYL) << 4) |                   \
                           (0x00                    << 3) |                   \
                           (vfdd.head               << 2) |                   \
                           (GetDS(vfdc.cmd[1])      << 0))
#define SetST1 (vfdc.st1 = (((vfdd.sector - 0x01) >= vfdd.nsector) << 7) |    \
                           (0x00                    << 6) |                   \
                           (0x00                    << 5) |                   \
                           (0x00                    << 4) |                   \
                           (0x00                    << 3) |                   \
                           ((vfdd.cyl >= VFDD_NCYL) << 2) |                   \
                           (0x00                    << 1) |                   \
                           (0x00                    << 0))
#define SetST2 (vfdc.st2 = (0x00                    << 7) |                   \
                           (0x00                    << 6) |                   \
                           (0x00                    << 5) |                   \
                           ((vfdd.cyl >= VFDD_NCYL) << 4) |                   \
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
                           (GetDS(vfdc.cmd[1])      << 0))
#define SetMSRReadyRead  (vfdc.msr = 0xc0, vfdc.rwid = 0x00)
#define SetMSRReadyWrite (vfdc.msr = 0x80, vfdc.rwid = 0x00)
#define SetMSRProcRead   (vfdc.msr = 0xd0)
#define SetMSRProcWrite  (vfdc.msr = 0x90)
#define SetMSRExecCmd    (vfdc.msr = 0x20)

#define GetMSRReadyRead  ((vfdc.msr & 0xc0) == 0xc0)
#define GetMSRReadyWrite ((vfdc.msr & 0xc0) == 0x80)
#define GetMSRProcRW     (!!(vfdc.msr & 0x10))
#define GetMSRExecCmd    (!!(vfdc.msr & 0x20))

static void ExecCmdSpecify()
{
	vfdc.hut      = GetHUT (vfdc.cmd[1]);
	vfdc.srt      = GetSRT (vfdc.cmd[1]);
	vfdc.hlt      = GetHLT (vfdc.cmd[2]);
	vfdc.flagndma = GetNDMA(vfdc.cmd[2]);
	SetMSRReadyWrite;
}
static void ExecCmdSenseDriveStatus()
{
	vfdd.head    = GetHDS(vfdc.cmd[1]);
	vfddResetCURR;
	SetST3;
	vfdc.ret[0]  = vfdc.st3;
	SetMSRReadyRead;
}
static void ExecCmdRecalibrate()
{
	vfdd.cyl      = 0x00;
	vfdd.head     = 0x00;
	vfdd.sector   = 0x01;
	vfddResetCURR;
	SetST0;
	if (GetENRQ(vfdc.dor)) {
		vpicSetIRQ(0x06);     /* TODO: BIOS: INT 0EH Should Call Command 08H */
		vfdc.flagintr = 0x01;
	}
	SetMSRReadyWrite;
}
static void ExecCmdSenseInterrupt()
{
	if (vfdc.flagintr) {
		vfdc.ret[0]    = vfdc.st0;
		vfdc.ret[1]    = vfdd.cyl;
		vfdc.flagintr  = 0x00;
	} else vfdc.ret[0] = vfdc.st0 = VFDC_RET_ERROR;
	SetMSRReadyRead;
}
static void ExecCmdSeek()
{
	vfdd.head     = GetHDS(vfdc.cmd[1]);
	vfdd.cyl      = vfdc.cmd[2];
	vfdd.sector   = 0x01;
	vfddResetCURR;
	SetST0;
	if (GetENRQ(vfdc.dor)) {
		vpicSetIRQ(0x06);     /* TODO: BIOS: INT 0EH Should Call Command 08H */
		vfdc.flagintr = 0x01;
	}
	SetMSRReadyWrite;
}
#define     ExecCmdReadTrack        vfdcTransInit
static void ExecCmdReadID()
{
	vfdd.head     = GetHDS(vfdc.cmd[1]);
	vfdd.sector   = 0x01;
	vfddResetCURR;
	vfdc.dr       = 0x00;                   /* data register: sector id info */
	vfdcTransFinal();
}
static void ExecCmdFormatTrack()
{
	/* NOTE: simplified procedure; dma not used */
	t_nubit8 fillbyte;
	/* load parameters*/
	vfdd.head    = GetHDS(vfdc.cmd[1]);
	vfdd.sector  = 0x01;
	vfdd.nbyte   = GetBPS(vfdc.cmd[2]);
	vfdd.nsector = vfdc.cmd[3];
	vfdd.gaplen  = vfdc.cmd[4];
	fillbyte     = vfdc.cmd[5];
	vfddResetCURR;
	/* execute format track*/
	vfddFormatTrack(fillbyte);
	/* finish transaction */
	SetST0;
	SetST1;
	SetST2;
	vfdc.ret[0] = vfdc.st0;
	vfdc.ret[1] = vfdc.st1;
	vfdc.ret[2] = vfdc.st2;
	if (GetENRQ(vfdc.dor)) {
		vpicSetIRQ(0x06);     /* TODO: BIOS: INT 0EH Should Call Command 08H */
		vfdc.flagintr = 0x01;
	}
	SetMSRReadyRead;
}
#define     ExecCmdWriteData        vfdcTransInit
#define     ExecCmdReadDataAll      vfdcTransInit
#define     ExecCmdWriteDeletedData vfdcTransInit
#define     ExecCmdReadDeletedData  vfdcTransInit
#define     ExecCmdScanEqualAll     ExecCmdScanEqual
#define     ExecCmdReadData         vfdcTransInit
static void ExecCmdScanEqual()
{
	/* NOTE: not fully implemented; lack of reference */
	SetST0;
	SetST1;
	SetST2;
	vfdc.st2   |= 0x04;         /* assume all data match; otherwise use 0x08 */
	vfdc.ret[0] = vfdc.st0;
	vfdc.ret[1] = vfdc.st1;
	vfdc.ret[2] = vfdc.st2;
	vfdc.ret[3] = vfdd.cyl;
	vfdc.ret[4] = vfdd.head;
	vfdc.ret[5] = vfdd.sector;                      /* TODO: correct the EOT */
	vfdc.ret[6] = GetBPSC(vfdd.nbyte);
	SetMSRReadyRead;
}
static void ExecCmdError()
{
	vfdc.ret[0] = VFDC_RET_ERROR;
	SetMSRReadyRead;
}

void IO_Read_03F4()
{
	vcpu.al = vfdc.msr;
}
void IO_Read_03F5()
{
	if (!GetMSRReadyRead) return;
	else SetMSRProcRead;
	vcpu.al = vfdc.ret[vfdc.rwid++];
	switch (vfdc.cmd[0]) {
	case CMD_SPECIFY:
		if (vfdc.rwid >= 0) SetMSRReadyWrite;break;
	case CMD_SENSE_DRIVE_STATUS:
		if (vfdc.rwid >= 1) SetMSRReadyWrite;break;
	case CMD_RECALIBRATE:
		if (vfdc.rwid >= 0) SetMSRReadyWrite;break;
	case CMD_SENSE_INTERRUPT:
		if (vfdc.rwid >= 2) SetMSRReadyWrite;break;
	case CMD_SEEK:
		if (vfdc.rwid >= 0) SetMSRReadyWrite;break;
	case CMD_READ_TRACK:
		if (vfdc.rwid >= 7) SetMSRReadyWrite;break;
	case CMD_READ_ID:
		if (vfdc.rwid >= 7) SetMSRReadyWrite;break;
	case CMD_FORMAT_TRACK:
		if (vfdc.rwid >= 3) SetMSRReadyWrite;break;
	case CMD_WRITE_DATA:
		if (vfdc.rwid >= 7) SetMSRReadyWrite;break;
	case CMD_READ_DATA_ALL:
		if (vfdc.rwid >= 7) SetMSRReadyWrite;break;
	case CMD_WRITE_DELETED_DATA:
		if (vfdc.rwid >= 7) SetMSRReadyWrite;break;
	case CMD_READ_DELETED_DATA:
		if (vfdc.rwid >= 7) SetMSRReadyWrite;break;
	case CMD_SCAN_EQUAL_ALL:
		if (vfdc.rwid >= 7) SetMSRReadyWrite;break;
	case CMD_READ_DATA:
		if (vfdc.rwid >= 7) SetMSRReadyWrite;break;
	case CMD_SCAN_EQUAL:
		if (vfdc.rwid >= 7) SetMSRReadyWrite;break;
	default: 
		if (vfdc.rwid >= 1) SetMSRReadyWrite;break;
		break;
	}
}
void IO_Read_03F7()
{
	vcpu.al = vfdc.dir;
}

void IO_Write_03F2()
{
	if (!(vfdc.dor & 0x04) && (vcpu.al & 0x04)) SetMSRReadyWrite; 
	vfdc.dor = vcpu.al;
	if (!(vfdc.dor & 0x04)) vfdcReset();
}
void IO_Write_03F5()
{
	if (!GetMSRReadyWrite) return;
	else SetMSRProcWrite;
	vfdc.cmd[vfdc.rwid++] = vcpu.al;
	switch (vfdc.cmd[0]) {
	case CMD_SPECIFY:
		if (vfdc.rwid == 3) ExecCmdSpecify();break;
	case CMD_SENSE_DRIVE_STATUS:
		if (vfdc.rwid == 2) ExecCmdSenseDriveStatus();break;
	case CMD_RECALIBRATE:
		if (vfdc.rwid == 2) ExecCmdRecalibrate();break;
	case CMD_SENSE_INTERRUPT:
		if (vfdc.rwid == 1) ExecCmdSenseInterrupt();break;
	case CMD_SEEK:
		if (vfdc.rwid == 3) ExecCmdSeek();break;
	case CMD_READ_TRACK:
		if (vfdc.rwid == 9) ExecCmdReadTrack();break;
	case CMD_READ_ID:
		if (vfdc.rwid == 2) ExecCmdReadID();break;
	case CMD_FORMAT_TRACK:
		if (vfdc.rwid == 6) ExecCmdFormatTrack();break;
	case CMD_WRITE_DATA:
		if (vfdc.rwid == 9) ExecCmdWriteData();break;
	case CMD_READ_DATA_ALL:
		if (vfdc.rwid == 9) ExecCmdReadDataAll();break;
	case CMD_WRITE_DELETED_DATA:
		if (vfdc.rwid == 9) ExecCmdWriteDeletedData();break;
	case CMD_READ_DELETED_DATA:
		if (vfdc.rwid == 9) ExecCmdReadDeletedData();break;
	case CMD_SCAN_EQUAL_ALL:
		if (vfdc.rwid == 9) ExecCmdScanEqualAll();break;
	case CMD_READ_DATA:
		if (vfdc.rwid == 9) ExecCmdReadData();break;
	case CMD_SCAN_EQUAL:
		if (vfdc.rwid == 9) ExecCmdScanEqual();break;
	default:
		                    ExecCmdError();break;
	}
}
void IO_Write_03F7()
{
	vfdc.ccr = vcpu.al;
}

void vfdcTransRead()
{	/* NOTE: being called by DMA/PIO */
	vfddTransRead();
}
void vfdcTransWrite()
{	/* NOTE: being called by DMA/PIO */
	vfddTransWrite();
}
void vfdcTransInit()
{	/* NOTE: being called internally in vfdc */
	/* read parameters */
	vfdd.cyl       = vfdc.cmd[2];
	vfdd.head      = vfdc.cmd[3];
	vfdd.sector    = 0x01;
	vfdd.nbyte     = GetBPS(vfdc.cmd[5]);
	vfdd.nsector   = vfdc.cmd[6];
	vfdd.gaplen    = vfdc.cmd[7];
	if (!vfdc.cmd[5])
		vfdd.nbyte = vfdc.cmd[8];
	vfdd.count     = 0x00;
	vfddResetCURR;
	/* send trans request */
	if (!vfdc.flagndma && GetENRQ(vfdc.dor)) vdmaSetDRQ(0x02);
	SetMSRExecCmd;
}
void vfdcTransFinal()
{	/* NOTE: being called by DMA/PIO */
	SetST0;
	SetST1;
	SetST2;
	vfdc.ret[0] = vfdc.st0;
	vfdc.ret[1] = vfdc.st1;
	vfdc.ret[2] = vfdc.st2;
	vfdc.ret[3] = vfdd.cyl;
	vfdc.ret[4] = vfdd.head;
	vfdc.ret[5] = vfdd.sector;
	vfdc.ret[6] = GetBPSC(vfdd.nbyte);
	if (GetENRQ(vfdc.dor)) {
		vpicSetIRQ(0x06);     /* TODO: BIOS: INT 0EH Should Call Command 08H */
		vfdc.flagintr = 0x01;
	}
	SetMSRReadyRead;
}
void vfdcRefresh()
{
	if (!vfdd.flagexist) vfdc.dir |= 0x80;
	else                 vfdc.dir &= 0x7f;
}
void vfdcReset()
{
	memset(&vfdc, 0, sizeof(t_fdc));
}
#ifdef VFDC_DEBUG
void IO_Read_F3F0()
{
	/* TODO: print all info */
}
#define mov(n) (vcpu.al=(n))
#define out(n) FUNEXEC(vcpuinsOutPort[(n)])
#endif
void vfdcInit()
{
	vfdcReset();
	vcpuinsInPort[0x03f4] = (t_vaddrcc)IO_Read_03F4;
	vcpuinsInPort[0x03f5] = (t_vaddrcc)IO_Read_03F5;
	vcpuinsInPort[0x03f7] = (t_vaddrcc)IO_Read_03F7;
	vcpuinsOutPort[0x03f2] = (t_vaddrcc)IO_Write_03F2;
	vcpuinsOutPort[0x03f5] = (t_vaddrcc)IO_Write_03F5;
	vcpuinsOutPort[0x03f7] = (t_vaddrcc)IO_Write_03F7;
#ifdef VFDC_DEBUG
	vcpuinsInPort[0x0f3f0] = (t_vaddrcc)IO_Read_F3F0;
	vcpuinsOutPort[0xf3f0] = (t_vaddrcc)IO_Write_F3F0;
	vcpuinsOutPort[0xf3f1] = (t_vaddrcc)IO_Write_F3F1;
	/* TODO: initialize fdc */
#endif
}
void vfdcFinal() {}
/*

*/
