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
#define CMD_WRITE_DELETED_DATA 0x09
#define CMD_READ_DELETED_DATA  0x0c
#define CMD_FORMAT_TRACK_2     0x0d
#define CMD_SEEK               0x0f  /* ! seek (both(?)) heads to cylinder X */
#define CMD_VERSION            0x10     /* ! used during initialization once */
#define CMD_SCAN_EQUAL         0x11
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
#define CMD_WRITE_DATA         0xc5                   /* ! write to the disk */
#define CMD_READ_DATA          0xe6                  /* ! read from the disk */

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

#define IsCmd(cmdl, count) (vfdc.cmd[0] == (cmdl) && vfdc.flagcmd == (count))
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

static void ExecCmdSpecify()
{
	vfdc.flagcmd  = 0x00;
	vfdc.flagret  = 0x00;
	vfdc.hut      = GetHUT (vfdc.cmd[1]);
	vfdc.srt      = GetSRT (vfdc.cmd[1]);
	vfdc.hlt      = GetHLT (vfdc.cmd[2]);
	vfdc.flagndma = GetNDMA(vfdc.cmd[2]);
}
static void ExecCmdSenseDriveStatus()
{
	vfdc.flagcmd = 0x00;
	vfdc.flagret = 0x00;
	vfdd.head    = GetHDS(vfdc.cmd[1]);
	SetST3;
	vfdc.ret[0]  = vfdc.st3;
}
static void ExecCmdRecalibrate()
{
	vfdc.flagcmd  = 0x00;
	vfdc.flagret  = 0x00;
	vfdd.cyl      = 0x00;
	vfdd.head     = 0x00;
	vfdd.sector   = 0x01;
	SetST0;
	vpicSetIRQ(0x06);         /* TODO: BIOS: INT 0EH Should Call Command 08H */
	vfdc.flagintr = 0x01;
}
static void ExecCmdSenseInterrupt()
{
	vfdc.flagcmd       = 0x00;
	vfdc.flagret       = 0x00;
	if (vfdc.flagintr) {
		vfdc.ret[0]    = vfdc.st0;
		vfdc.ret[1]    = vfdd.cyl;
		vfdc.flagintr  = 0x00;
	} else vfdc.ret[0] = vfdc.st0 = VFDC_RET_ERROR;
}
static void ExecCmdSeek()
{
	vfdc.flagcmd  = 0x00;
	vfdc.flagret  = 0x00;
	vfdd.head     = GetHDS(vfdc.cmd[1]);
	vfdd.cyl      = vfdc.cmd[2];
	SetST0;
	vpicSetIRQ(0x06);
	vfdc.flagintr = 0x01;
}
#define     ExecCmdReadTrack vfdcTransInit
static void ExecCmdReadID()
{
	vfdc.flagcmd  = 0x00;
	vfdc.flagret  = 0x00;
	vfdd.head     = GetHDS(vfdc.cmd[1]);
	vfdd.sector   = 0x01;
	vfdc.dr       = 0x00;                   /* data register: sector id info */
	vfdcTransFinal();
}
static void ExecCmdFormatTrack()
{
	/* NOTE: simplified procedure; dma not used */
	t_nubit8 fillbyte;
	vfdc.flagcmd = 0;
	vfdc.flagret = 0x00;
	/* load parameters*/
	vfdd.head    = GetHDS(vfdc.cmd[1]);
	vfdd.sector  = 0x01;
	vfdd.nbyte   = GetBPS(vfdc.cmd[2]);
	vfdd.nsector = vfdc.cmd[3];
	vfdd.gaplen  = vfdc.cmd[4];
	fillbyte     = vfdc.cmd[5];
	/* execute format track*/
	vfddFormatTrack(fillbyte);
	/* finish transaction */
	SetST0;
	SetST1;
	SetST2;
	vfdc.ret[0] = vfdc.st0;
	vfdc.ret[1] = vfdc.st1;
	vfdc.ret[2] = vfdc.st2;
	vpicSetIRQ(0x06);
	vfdc.flagintr = 0x01;
}
#define     ExecCmdWriteData vfdcTransInit
#define     ExecCmdReadData  vfdcTransInit
/*static void ExecCmdReadData()
{
	t_bool succ;
	vfdc.flagcmd = 0;
	succ = vfddRead(&vfdc.cmd[2],&vfdc.cmd[3],&vfdc.cmd[4],
		vramGetAddress((((t_vaddrcc)vdma1.channel[2].page)<<16)+vdma1.channel[2].baseaddr),
		(vdma1.channel[2].basewc+0x01)/vfdd.nbyte);
	vfdc.pcn = vfdc.cmd[2];
	vfdc.st0 = 0x20 | (vfdc.cmd[3]<<2) | GetDS(vfdc.cmd[1]);
	vfdc.st1 = 0x00 | ((t_nubit8)succ<<2);
	if(vfdc.pcn >= 0x50) vfdc.st2 = 0x10;
	else vfdc.st2 = 0x00;
	vfdc.ret[0] = vfdc.st0;
	vfdc.ret[1] = vfdc.st1;
	vfdc.ret[2] = vfdc.st2;
	vfdc.ret[3] = vfdc.cmd[2];
	vfdc.ret[4] = vfdc.cmd[3];
	vfdc.ret[5] = vfdc.cmd[4];
	vfdc.ret[6] = 0x02;
	vfdc.flagintr = 1;
	vpicSetIRQ(0x06);
}*/

void IO_Read_03F4()
{
	/* TODO: remember to modify msr when executing commands */
	vcpu.al = vfdc.msr;
}
void IO_Read_03F5()
{
	vcpu.al = vfdc.ret[vfdc.flagret++];
	if(vfdc.ret[0] == VFDC_RET_ERROR) {
		vfdc.flagret = 0;
		return;
	}
	if(IsRet(CMD_SENSE_DRIVE_STATUS,1))   {vfdc.flagret = 0;}
	else if(IsRet(CMD_SENSE_INTERRUPT,2)) {vfdc.flagret = 0;}
	else if(IsRet(CMD_READ_DATA,7))       {vfdc.flagret = 0;}
	else if(IsRet(CMD_WRITE_DATA,7))      {vfdc.flagret = 0;}
	else if(IsRet(CMD_READ_TRACK,7))      {vfdc.flagret = 0;}
	else if(IsRet(CMD_FORMAT_TRACK,7))    {vfdc.flagret = 0;}
}
void IO_Read_03F7()
{
	/* TODO: remember to modify dir when executing commands */
	vcpu.al = vfdc.dir;
}

void IO_Write_03F2()
{
	vfdc.dor = vcpu.al;
}
void IO_Write_03F5()
{

	vfdc.cmd[vfdc.flagcmd++] = vcpu.al;
	if(GetDS(vfdc.cmd[1])) {
		vfdc.ret[0] = VFDC_RET_ERROR;
		return;
	}
	if(IsCmd(CMD_SPECIFY, 3))                 ExecCmdSpecify();
	else if(IsCmd(CMD_SENSE_DRIVE_STATUS, 2)) ExecCmdSenseDriveStatus();
	else if(IsCmd(CMD_RECALIBRATE,2))         ExecCmdRecalibrate();
	else if(IsCmd(CMD_SENSE_INTERRUPT,1))     ExecCmdSenseInterrupt();
	else if(IsCmd(CMD_SEEK,3))                ExecCmdSeek();
	else if(IsCmd(CMD_READ_TRACK,9))          ExecCmdReadTrack();
	else if(IsCmd(CMD_READ_ID,2))             ExecCmdReadID();
	else if(IsCmd(CMD_FORMAT_TRACK,9))        ExecCmdFormatTrack();
	else if(IsCmd(CMD_WRITE_DATA,9))          ExecCmdWriteData();
	else if(IsCmd(CMD_READ_DATA,9))           ExecCmdReadData();
}
void IO_Write_03F7()
{
	vfdc.ccr = vcpu.al;
}

void vfdcTransInit()
{	/* NOTE: being called internally in vfdc */
	vfdc.flagcmd   = 0x00;
	vfdc.flagret   = 0x00;
	/* read parameters */
	vfdd.cyl       = vfdc.cmd[2];
	vfdd.head      = vfdc.cmd[3];
	vfdd.sector    = 0x01;
	vfdd.nbyte     = GetBPS(vfdc.cmd[5]);
	vfdd.nsector   = vfdc.cmd[6];
	vfdd.gaplen    = vfdc.cmd[7];
	if (!vfdc.cmd[5])
		vfdd.nbyte = vfdc.cmd[8];
	/* send trans request */
	vfdd.count    = 0x00;
	vfdd.curr      = vfddSetCURR;
	if (!vfdc.flagndma) vdmaSetDRQ(0x02);
	vfdc.flagis    = 0x01;
}
void vfdcTransFinal()
{	/* NOTE: being called by DMA/PIO */
	vfdc.flagis = 0x00;
	SetST0;
	SetST1;
	SetST2;
	vfdc.ret[0] = vfdc.st0;
	vfdc.ret[1] = vfdc.st1;
	vfdc.ret[2] = vfdc.st2;
	vfdc.ret[3] = vfdd.cyl;
	vfdc.ret[4] = vfdd.head;
	vfdc.ret[5] = vfdd.sector;                      /* TODO: correct the EOT */
	vfdc.ret[6] = GetBPSC(vfdd.nbyte);
	vpicSetIRQ(0x06);
	vfdc.flagintr = 0x01;
}
void vfdcRefresh()
{
	/* TODO: test/write msr here */
	if(!vfdd.base) vfdc.dir = 0x80;
}
void vfdcInit()
{
	memset(&vfdc, 0, sizeof(t_fdc));
	vcpuinsInPort[0x03f4] = (t_vaddrcc)IO_Read_03F4;
	vcpuinsInPort[0x03f5] = (t_vaddrcc)IO_Read_03F5;
	vcpuinsInPort[0x03f7] = (t_vaddrcc)IO_Read_03F7;
	vcpuinsOutPort[0x03f2] = (t_vaddrcc)IO_Write_03F2;
	vcpuinsOutPort[0x03f5] = (t_vaddrcc)IO_Write_03F5;
	vcpuinsOutPort[0x03f7] = (t_vaddrcc)IO_Write_03F7;
}
void vfdcFinal() {}
