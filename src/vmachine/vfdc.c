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

#define VFDC_ERROR 0x80                                        /* Error Code */

/* Commands */
#define CMD_SPECIFY            0x03                  /* set drive parameters */
#define CMD_SENSE_DRIVE_STATUS 0x04
#define CMD_RECALIBRATE        0x07                 /* !* seek to cylinder 0 */
#define CMD_SENSE_INTERRUPT    0x08
                            /* ! acknowledge IRQ6 get status of last command */
#define CMD_WRITE_DELETED_DATA 0x09
#define CMD_READ_ID            0x0a                        /* generates irq6 */
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

#define GET_MFM(cbyte)                                     /* FM or MFM Mode */
#define GET_MT(cbyte)                                         /* Multi-track */
#define GET_N(cbyte)                                               /* Number */
#define GET_NCN(cbyte)                                /* New Cylinder Number */
#define GET_R(cbyte)                                               /* Record */
#define GET_RW(cbyte)                                   /* Read/Write Signal */
#define GET_SC(cbyte)                                              /* Sector */
#define GET_SK(cbyte)                                                /* Skip */
#define GET_STP(cbyte)                                               /* Step */

#define GetDS(cbyte)  ((cbyte) & 0x03)             /* Drive Select (DS0,DS1) */
#define GetHUT(cbyte) ((cbyte) & 0x0f)                   /* Head Unload Time */
#define GetSRT(cbyte) ((cbyte) >> 4)                       /* Step Rate Time */
#define GetHLT(cbyte) ((cbyte) >> 1)                       /* Head Load Time */
#define GetND(cbyte)  ((cbyte) & 0x01)                       /* Non-DMA Mode */
#define GetHDS(cbyte) (!!((cbyte) & 0x04))              /* Head Select (0-1) */

#define IsCmd(cmdl, count) (vfdc.cmd[0] == (cmdl) && vfdc.flagcmd == (count))
static void ExecCmdSpecify()
{
	vfdc.flagcmd = 0;
	vfdc.hut = GetHUT(vfdc.cmd[1]);
	vfdc.srt = GetSRT(vfdc.cmd[1]);
	vfdc.hlt = GetHLT(vfdc.cmd[2]);
	vfdc.nd  = GetND (vfdc.cmd[2]);
}
static void ExecCmdSenseDriveStatus()
{
	vfdc.flagcmd = 0;
	/* TODO: verify 0x38 is feasible */
	vfdc.st3 = 0x38 | (GetHDS(vfdc.cmd[1]) <<2 ) | GetDS(vfdc.cmd[1]);
	vfdc.ret[0] = vfdc.st3;
}
static void ExecCmdRecalibrate()
{
	vfdc.flagcmd = 0;
	vfdc.pcn = 0x00;
	vfdc.st0 = 0x20 | GetDS(vfdc.cmd[1]);
	vfdc.intr = 1;
	vpicSetIRQ(0x06);         /* TODO: BIOS: INT 0EH Should Call Command 08H */
}
static void ExecCmdSenseInterrupt()
{
	vfdc.flagcmd = 0;
	if(vfdc.intr) {
		vfdc.intr = 0;
		vfdc.ret[0] = vfdc.st0;
		vfdc.ret[1] = vfdc.pcn;
	} else vfdc.ret[0] = VFDC_ERROR;
}
static void ExecCmdSeek()
{
	vfdc.flagcmd = 0;
	vfdc.st0 = 0x20 | (GetHDS(vfdc.cmd[1]) << 2) | GetDS(vfdc.cmd[1]);
	vfdc.pcn = vfdc.cmd[2];
	vfdc.intr = 1;
	vpicSetIRQ(0x06);
}
static void ExecCmdReadData()
{
	t_bool succ;
	vfdc.flagcmd = 0;
	succ = vfddRead(&vfdc.cmd[2],&vfdc.cmd[3],&vfdc.cmd[4],
		vramGetAddress((((t_vaddrcc)vdma1.channel[2].page)<<16)+vdma1.channel[2].baseaddr),
		(vdma1.channel[2].basewc+0x01)/VFDD_BYTES);
	vfdc.pcn = vfdc.cmd[2];
	vfdc.st0 = 0x20 | (vfdc.cmd[3]<<2) | GET_DS(vfdc.cmd[1]);
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
	vfdc.intr = 1;
	vpicSetIRQ(0x06);
}
static void ExecCmdWriteData()
{
	t_bool succ;
	vfdc.flagcmd = 0;
	succ = vfddWrite(&vfdc.cmd[2],&vfdc.cmd[3],&vfdc.cmd[4],
		vramGetAddress((((t_vaddrcc)vdma1.channel[2].page)<<16)+vdma1.channel[2].baseaddr),
		(vdma1.channel[2].basewc+0x01)/VFDD_BYTES);
	vfdc.pcn = vfdc.cmd[2];
	vfdc.st0 = 0x20 | (vfdc.cmd[3]<<2) | GET_DS(vfdc.cmd[1]);
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
	vfdc.intr = 1;
	vpicSetIRQ(0x06);
}
static void ExecCmdReadTrack()
{
	t_bool succ;
	vfdc.flagcmd = 0;
	succ = vfddRead(&vfdc.cmd[2],&vfdc.cmd[3],&vfdc.cmd[4],
		vramGetAddress((((t_vaddrcc)vdma1.channel[2].page)<<16)+vdma1.channel[2].baseaddr),
		VFDD_SECTORS);
	vfdc.pcn = vfdc.cmd[2];
	vfdc.st0 = 0x20 | (vfdc.cmd[3]<<2) | GET_DS(vfdc.cmd[1]);
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
	vfdc.intr = 1;
	vpicSetIRQ(0x06);
}
static void ExecCmdFormatTrack()
{
	vfdc.flagcmd = 0;
	vfddFormat(vfdc.cmd[5]);
	vfdc.pcn = 0x00;
	vfdc.st0 = 0x00;
	vfdc.st1 = 0x00;
	vfdc.st2 = 0x00;
	vfdc.ret[0] = vfdc.st0;
	vfdc.ret[1] = vfdc.st1;
	vfdc.ret[2] = vfdc.st2;
	vfdc.ret[3] = 0x00;
	vfdc.ret[4] = 0x00;
	vfdc.ret[5] = 0x00;
	vfdc.ret[6] = 0x00;
	vfdc.intr = 1;
	vpicSetIRQ(0x06);
}

void IO_Read_03F4()
{
	/* TODO: remember to modify msr when executing commands */
	vcpu.al = vfdc.msr;
}
void IO_Read_03F5()
{
	vcpu.al = vfdc.ret[vfdc.flagcmd++];
	if(vfdc.ret[0] == VFDC_ERROR) {
		vfdc.flagcmd = 0;
		return;
	}
	if(IsCmd(CMD_SENSE_DRIVE_STATUS,1))   {vfdc.flagcmd = 0;}
	else if(IsCmd(CMD_SENSE_INTERRUPT,2)) {vfdc.flagcmd = 0;}
	else if(IsCmd(CMD_READ_DATA,7))       {vfdc.flagcmd = 0;}
	else if(IsCmd(CMD_WRITE_DATA,7))      {vfdc.flagcmd = 0;}
	else if(IsCmd(CMD_READ_TRACK,7))      {vfdc.flagcmd = 0;}
	else if(IsCmd(CMD_FORMAT_TRACK,7))    {vfdc.flagcmd = 0;}
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
		vfdc.ret[0] = VFDC_ERROR;
		return;
	}
	if(IsCmd(CMD_SPECIFY, 3))                 ExecCmdSpecify();
	else if(IsCmd(CMD_SENSE_DRIVE_STATUS, 2)) ExecCmdSenseDriveStatus();
	else if(IsCmd(CMD_RECALIBRATE,2))         ExecCmdRecalibrate();
	else if(IsCmd(CMD_SENSE_INTERRUPT,1))     ExecCmdSenseInterrupt();
	else if(IsCmd(CMD_SEEK,3))                ExecCmdSeek();
	else if(IsCmd(CMD_READ_DATA,9))           ExecCmdReadData();
	else if(IsCmd(CMD_WRITE_DATA,9))          ExecCmdWriteData();
	else if(IsCmd(CMD_READ_TRACK,9))          ExecCmdReadTrack();
	else if(IsCmd(CMD_FORMAT_TRACK,9))        ExecCmdFormatTrack();
}
void IO_Write_03F7()
{
	vfdc.ccr = vcpu.al;
}

void vfdcRefresh()
{
	/* TODO: test/write msr here */
	if(!vfdd.ptrbase) vfdc.dir = 0x80;
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
