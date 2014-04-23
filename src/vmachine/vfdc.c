/* This file is a part of NXVM project. */

#include "memory.h"

#include "vapi.h"
#include "vport.h"
#include "vcpu.h"
#include "vpic.h"
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
static t_nubit8 GetBPSC(t_nubit16 cbyte)                  /* sector size code */
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
                           (0x00                    << 5) |                   \
                           ((vfdd.cyl >= VFDD_NCYL) << 4) |                   \
                           (0x00                    << 3) |                   \
                           (vfdd.head               << 2) |                   \
                           (GetDS(vfdc.cmd[1])      << 0))
#define SetST1 (vfdc.st1 = ((vfdd.sector >= (vfdd.nsector + 1)) << 7) |       \
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
	vfdd.cyl    = 0x00;
	vfdd.head   = 0x00;
	vfdd.sector = 0x01;
	vfddResetCURR;
	SetST0;
	vfdc.st0   |= 0x20;
	if (GetENRQ(vfdc.dor)) {
		vpicSetIRQ(0x06);
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
	vfdd.head   = GetHDS(vfdc.cmd[1]);
	vfdd.cyl    = vfdc.cmd[2];
	vfdd.sector = 0x01;
	vfddResetCURR;
	SetST0;
	vfdc.st0   |= 0x20;
	if (GetENRQ(vfdc.dor)) {
		vpicSetIRQ(0x06);
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
	vfdc.ret[3] = 0x00;
	vfdc.ret[4] = 0x00;
	vfdc.ret[5] = 0x00;
	vfdc.ret[6] = 0x00;
	if (GetENRQ(vfdc.dor)) {
		vpicSetIRQ(0x06);
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
	vfdc.ret[5] = vfdd.sector;                      /* NOTE: eot not changed */
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
	vcpu.iobyte = vfdc.msr;
}
void IO_Read_03F5()
{
	if (!GetMSRReadyRead) return;
	else SetMSRProcRead;
	vcpu.iobyte = vfdc.ret[vfdc.rwid++];
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
		if (vfdc.rwid >= 7) SetMSRReadyWrite;break;
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
	vcpu.iobyte = vfdc.dir;
}

void IO_Write_03F2()
{
	if (!(vfdc.dor & 0x04) && (vcpu.iobyte & 0x04)) SetMSRReadyWrite; 
	vfdc.dor = vcpu.iobyte;
	if (!(vfdc.dor & 0x04)) vfdcReset();
}
void IO_Write_03F5()
{
	if (!GetMSRReadyWrite) return;
	else SetMSRProcWrite;
	vfdc.cmd[vfdc.rwid++] = vcpu.iobyte;
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
	vfdc.ccr = vcpu.iobyte;
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
	vfdd.sector    = vfdc.cmd[4];
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
		vpicSetIRQ(0x06);
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
	t_nubit8 ccr = vfdc.ccr;
	memset(&vfdc, 0, sizeof(t_fdc));
	vfdc.ccr = ccr;
}
#ifdef VFDC_DEBUG
void IO_Read_F3F0()
{
	t_nubitcc i;
	vapiPrint("FDC INFO\n========\n");
	vapiPrint("msr = %x, dir = %x, dor = %x, ccr = %x, dr = %x\n",
		vfdc.msr,vfdc.dir,vfdc.dor,vfdc.ccr,vfdc.dr);
	vapiPrint("hut = %x, hlt = %x, srt = %x, Non-DMA = %x, INTR = %x\n",
		vfdc.hut,vfdc.hlt,vfdc.srt,vfdc.flagndma,vfdc.flagintr);
	vapiPrint("rwid = %x, st0 = %x, st1 = %x, st2 = %x, st3 = %x\n",
		vfdc.rwid,vfdc.st0,vfdc.st1,vfdc.st2,vfdc.st3);
	for(i = 0;i < 9;++i) vapiPrint("cmd[%d] = %x, ",i,vfdc.cmd[i]);
	vapiPrint("\n");
	for(i = 0;i < 7;++i) vapiPrint("ret[%d] = %x, ",i,vfdc.ret[i]);
	vapiPrint("\n");
	vapiPrint("FDD INFO\n========\n");
	vapiPrint("cyl = %x, head = %x, sector = %x\n",
		vfdd.cyl,vfdd.head,vfdd.sector);
	vapiPrint("nsector = %x, nbyte = %x, gaplen = %x\n",
		vfdd.nsector,vfdd.nbyte,vfdd.gaplen);
	vapiPrint("ReadOnly = %x, Exist = %x\n",
		vfdd.flagro,vfdd.flagexist);
	vapiPrint("base = %x, curr = %x, count = %x\n",
		vfdd.base,vfdd.curr,vfdd.count);
}
void IO_Write_F3F0()
{
	vfdcReset();
	vfdc.dor = 0x0c;
	SetMSRReadyWrite;
}

#include "vcpu.h"
#define mov(n) (vcpu.iobyte = (n))
#define out(n) ExecFun(vport.out[(n)])
#endif
void vfdcInit()
{
	memset(&vfdc, 0, sizeof(t_fdc));
	vfdc.ccr = 0x02;
	vport.in[0x03f4] = (t_faddrcc)IO_Read_03F4;
	vport.in[0x03f5] = (t_faddrcc)IO_Read_03F5;
	vport.in[0x03f7] = (t_faddrcc)IO_Read_03F7;
	vport.out[0x03f2] = (t_faddrcc)IO_Write_03F2;
	vport.out[0x03f5] = (t_faddrcc)IO_Write_03F5;
	vport.out[0x03f7] = (t_faddrcc)IO_Write_03F7;
#ifdef VFDC_DEBUG
	vport.in[0x0f3f0] = (t_faddrcc)IO_Read_F3F0;
	vport.out[0xf3f0] = (t_faddrcc)IO_Write_F3F0;
	vport.out[0xf3f1] = (t_faddrcc)IO_Write_F3F1;
	/* initialize fdc */
	mov(0x00);
	out(0x03f2);
	mov(0x0c);
	out(0x03f2);
	mov(CMD_SPECIFY);
	out(0x03f5);
	mov(0xaf);
	out(0x03f5);
	mov(0x02);
	out(0x03f5);
#endif
}
void vfdcFinal() {}
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
