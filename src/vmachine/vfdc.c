/* This file is a part of NXVM project. */

#include "memory.h"

#include "vcpu.h"
#include "vcpuins.h"
#include "vpic.h"
#include "vram.h"
#include "vdmac.h"
#include "vfdd.h"
#include "vfdc.h"

t_fdc vfdc;
#define VFDC_ERROR	0x80		// Error Code

// Commands
#define CMD_READ_TRACK			0x42	// generates IRQ6
#define CMD_FORMAT_TRACK		0x4d	// generates IRQ6
#define CMD_SPECIFY				0x03	// * set drive parameters
#define CMD_SENSE_DRIVE_STATUS	0x04
#define CMD_WRITE_DATA			0xc5	// * write to the disk
#define CMD_READ_DATA			0xe6	// * read from the disk
#define CMD_RECALIBRATE			0x07	// * seek to cylinder 0
#define CMD_SENSE_INTERRUPT		0x08	// * acknowledge IRQ6 get status of last command
//#define CMD_WRITE_DELETED_DATA	0x09
//#define CMD_READ_ID				0x0a	// generates IRQ6
//#define CMD_READ_DELETED_DATA	0x0c
//#define CMD_FORMAT_TRACK		0x0d	// *
#define CMD_SEEK				0x0f	// * seek (both(?)) heads to cylinder X
//#define CMD_VERSION				0x10	// * used during initialization once
//#define CMD_SCAN_EQUAL			0x11
//#define CMD_PERPENDICULAR_MODE	0x12	// * used during initialization once maybe
//#define CMD_CONFIGURE			0x13	// * set controller parameters
//#define CMD_LOCK				0x14	// * protect controller params from a reset
//#define CMD_VERIFY				0x16
//#define CMD_SCAN_LOW_OR_EQUAL	0x19
//#define CMD_SCAN_HIGH_OR_EQUAL	0x1d

// Command Bytes
//#define GET_A0(cid)	// Address Line 0
//#define GET_C(cid)	// Cylinder Number (0-79)
//#define GET_D(cid)	// Data
//#define GET_DB(cid)	// Data Bus (D7-D0)
#define GET_DS(cid)	(cid&0x03)		// Drive Select (DS0,DS1)
//#define GET_DTL(cid)	// Data Length
//#define GET_EOT(cid)	// End Of Track
//#define GET_GPL(cid)	// Gap Length
//#define GET_H(cid)	// Head Address (0-1)
#define GET_HDS(cid)	(!!(cid&0x04))	// Head Select (0-1)
//#define GET_HLT(cid)	(cid>>1)		// Head Load Time
//#define GET_HUT(cid)	(cid&0x0f)		// Head Unload Time
//#define GET_MFM(cid)	// FM or MFM Mode
//#define GET_MT(cid)	// Multi-track
//#define GET_N(cid)	// Number
//#define GET_NCN(cid)	// New Cylinder Number
//#define GET_ND(cid)	(cid&0x01)		// Non-DMA Mode
//#define GET_R(cid)	// Record
//#define GET_RW(cid)	// Read/Write Signal
//#define GET_SC(cid)	// Sector
//#define GET_SK(cid)	// Skip
//#define GET_SRT(cid)	((cid&0xf0)>>4)	// Step Rate Time
//#define GET_STP(cid)	// Step

static t_nubit8 bcount;
static t_nubit8 cmd[9],result[7];

void IO_Read_03F4()
{vcpu.al = vfdc.msr;}
void IO_Read_03F5()
{
	vcpu.al = result[bcount++];
	if(result[0] == VFDC_ERROR) bcount = 0;
	else {
		if((cmd[0] == CMD_SENSE_DRIVE_STATUS) && (bcount == 1)) {bcount = 0;}
		else if((cmd[0] == CMD_SENSE_INTERRUPT) && (bcount == 2)) {bcount = 0;}
		else if((cmd[0] == CMD_READ_DATA) && (bcount == 7)) {bcount = 0;}
		else if((cmd[0] == CMD_WRITE_DATA) && (bcount == 7)) {bcount = 0;}
		else if((cmd[0] == CMD_READ_TRACK) && (bcount == 7)) {bcount = 0;}
		else if((cmd[0] == CMD_FORMAT_TRACK) && (bcount == 7)) {bcount = 0;}
	}
}
void IO_Read_03F7()
{vcpu.al = vfdc.dir;}
void IO_Write_03F2()
{vfdc.dor = vcpu.al;}
void IO_Write_03F5()
{
	t_bool succ;
	cmd[bcount++] = vcpu.al;
	if(GET_DS(cmd[1])) {
		result[0] = VFDC_ERROR;
	} else {
		if((cmd[0] == CMD_SPECIFY) && (bcount == 3)) {
			bcount = 0;
			/*vfdc.hut = GET_HUT(cmd[1]);
			vfdc.srt = GET_SRT(cmd[1]);
			vfdc.hlt = GET_HLT(cmd[2]);
			vfdc.nd = GET_ND(cmd[2]);*/
		} else if((cmd[0] == CMD_SENSE_DRIVE_STATUS) && (bcount == 2)) {
			bcount = 0;
			vfdc.st3 = 0x38 | (GET_HDS(cmd[1])<<2) | GET_DS(cmd[1]);
			result[0] = vfdc.st3;
		} else if((cmd[0] == CMD_RECALIBRATE) && (bcount == 2)) {
			bcount = 0;
			vfdc.pcn = 0;
			vfdc.st0 = 0x20 | (GET_HDS(cmd[1])<<2) | GET_DS(cmd[1]);
			vfdc.intr = 1;
			vpicSetIRQ(0x06);	// INT 0EH Should Call Command 08H
		} else if((cmd[0] == CMD_SENSE_INTERRUPT) && (bcount == 1)) {
			bcount = 0;
			if(vfdc.intr) {
				vfdc.intr = 0;
				result[0] = vfdc.st0;
				result[1] = vfdc.pcn;
			} else result[0] = VFDC_ERROR;
		} else if((cmd[0] == CMD_SEEK) && (bcount == 3)) {
			bcount = 0;
			vfdc.pcn = cmd[2];
			vfdc.st0 = 0x20 | (GET_HDS(cmd[1])<<2) | GET_DS(cmd[1]);
			vfdc.intr = 1;
			vpicSetIRQ(0x06);	// INT 0EH Should Call Command 08H
		} else if((cmd[0] == CMD_READ_DATA) && (bcount == 9)) {
			bcount = 0;
			succ = vfddRead(&cmd[2],&cmd[3],&cmd[4],
				vramGetAddress((((t_vaddrcc)vdmac1.channel[2].page)<<16)+vdmac1.channel[2].base_address),
				(vdmac1.channel[2].base_wordcount+0x01)/VFDD_BYTES);
			vfdc.pcn = cmd[2];
			vfdc.st0 = 0x20 | (cmd[3]<<2) | GET_DS(cmd[1]);
			vfdc.st1 = 0x00 | ((t_nubit8)succ<<2);
			if(vfdc.pcn >= 0x50) vfdc.st2 = 0x10;
			else vfdc.st2 = 0x00;
			result[0] = vfdc.st0;
			result[1] = vfdc.st1;
			result[2] = vfdc.st2;
			result[3] = cmd[2];
			result[4] = cmd[3];
			result[5] = cmd[4];
			result[6] = 0x02;
			vfdc.intr = 1;
			vpicSetIRQ(0x06);
		} else if((cmd[0] == CMD_WRITE_DATA) && (bcount == 9)) {
			bcount = 0;
			succ = vfddWrite(&cmd[2],&cmd[3],&cmd[4],
				vramGetAddress((((t_vaddrcc)vdmac1.channel[2].page)<<16)+vdmac1.channel[2].base_address),
				(vdmac1.channel[2].base_wordcount+0x01)/VFDD_BYTES);
			vfdc.pcn = cmd[2];
			vfdc.st0 = 0x20 | (cmd[3]<<2) | GET_DS(cmd[1]);
			vfdc.st1 = 0x00 | ((t_nubit8)succ<<2);
			if(vfdc.pcn >= 0x50) vfdc.st2 = 0x10;
			else vfdc.st2 = 0x00;
			result[0] = vfdc.st0;
			result[1] = vfdc.st1;
			result[2] = vfdc.st2;
			result[3] = cmd[2];
			result[4] = cmd[3];
			result[5] = cmd[4];
			result[6] = 0x02;
			vfdc.intr = 1;
			vpicSetIRQ(0x06);
		} else if((cmd[0] == CMD_READ_TRACK) && (bcount == 9)) {
			bcount = 0;
			succ = vfddRead(&cmd[2],&cmd[3],&cmd[4],
				vramGetAddress((((t_vaddrcc)vdmac1.channel[2].page)<<16)+vdmac1.channel[2].base_address),
				VFDD_SECTORS);
			vfdc.pcn = cmd[2];
			vfdc.st0 = 0x20 | (cmd[3]<<2) | GET_DS(cmd[1]);
			vfdc.st1 = 0x00 | ((t_nubit8)succ<<2);
			if(vfdc.pcn >= 0x50) vfdc.st2 = 0x10;
			else vfdc.st2 = 0x00;
			result[0] = vfdc.st0;
			result[1] = vfdc.st1;
			result[2] = vfdc.st2;
			result[3] = cmd[2];
			result[4] = cmd[3];
			result[5] = cmd[4];
			result[6] = 0x02;
			vfdc.intr = 1;
			vpicSetIRQ(0x06);
		} else if((cmd[0] == CMD_FORMAT_TRACK) && (bcount == 9)) {
			bcount = 0;
			vfddFormat(cmd[5]);
			vfdc.pcn = 0x00;
			vfdc.st0 = 0x00;
			vfdc.st1 = 0x00;
			vfdc.st2 = 0x00;
			result[0] = vfdc.st0;
			result[1] = vfdc.st1;
			result[2] = vfdc.st2;
			result[3] = 0x00;
			result[4] = 0x00;
			result[5] = 0x00;
			result[6] = 0x00;
			vfdc.intr = 1;
			vpicSetIRQ(0x06);
		}
	}
}
void IO_Write_03F7()
{vfdc.ccr = vcpu.al;}

void vfdcInit()
{
	bcount = 0;
	memset(&vfdc,0,sizeof(t_fdc));
	vfdc.msr = 0x80;
	if(!vfdd.ptrbase) vfdc.dir = 0x80;
	vcpuinsInPort[0x03f4] = (t_vaddrcc)IO_Read_03F4;
	vcpuinsInPort[0x03f5] = (t_vaddrcc)IO_Read_03F5;
	vcpuinsInPort[0x03f7] = (t_vaddrcc)IO_Read_03F7;
	vcpuinsOutPort[0x03f2] = (t_vaddrcc)IO_Write_03F2;
	vcpuinsOutPort[0x03f5] = (t_vaddrcc)IO_Write_03F5;
	vcpuinsOutPort[0x03f7] = (t_vaddrcc)IO_Write_03F7;
}
void vfdcFinal() {}
