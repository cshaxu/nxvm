/* This file is a part of NXVM project. */

#include "memory.h"

#include "vcpu.h"
#include "vcpuins.h"
#include "vpic.h"
#include "vfdd.h"
#include "vfdc.h"

t_fdc vfdc;

// Commands
#define CMD_READ_TRACK			0x42	// generates IRQ6
#define CMD_SPECIFY				0x03	// * set drive parameters
#define CMD_SENSE_DRIVE_STATUS	0x04
#define CMD_WRITE_DATA			0x05	// * write to the disk
#define CMD_READ_DATA			0x06	// * read from the disk
#define CMD_RECALIBRATE			0x07	// * seek to cylinder 0
#define CMD_SENSE_INTERRUPT		0x08	// * acknowledge IRQ6 get status of last command
#define CMD_WRITE_DELETED_DATA	0x09
#define CMD_READ_ID				0x0a	// generates IRQ6
#define CMD_READ_DELETED_DATA	0x0c
#define CMD_FORMAT_TRACK		0x0d	// *
#define CMD_SEEK				0x0f	// * seek (both(?)) heads to cylinder X
#define CMD_VERSION				0x10	// * used during initialization once
#define CMD_SCAN_EQUAL			0x11
#define CMD_PERPENDICULAR_MODE	0x12	// * used during initialization once maybe
#define CMD_CONFIGURE			0x13	// * set controller parameters
#define CMD_LOCK				0x14	// * protect controller params from a reset
#define CMD_VERIFY				0x16
#define CMD_SCAN_LOW_OR_EQUAL	0x19
#define CMD_SCAN_HIGH_OR_EQUAL	0x1d



// Command Bytes
#define GET_A0(cid)	// Address Line 0
#define GET_C(cid)	// Cylinder Number (0-79)
#define GET_D(cid)	// Data
#define GET_DB(cid)	// Data Bus (D7-D0)
#define GET_DS(cid)	(cid&0x03)		// Drive Select (DS0,DS1)
#define GET_DTL(cid)	// Data Length
#define GET_EOT(cid)	// End Of Track
#define GET_GPL(cid)	// Gap Length
#define GET_H(cid)	// Head Address (0-1)
#define GET_HDS(cid)	(!!(cid&0x04))	// Head Select (0-1)
#define GET_HLT(cid)	(cid>>1)		// Head Load Time
#define GET_HUT(cid)	(cid&0x0f)		// Head Unload Time
#define GET_MFM(cid)	// FM or MFM Mode
#define GET_MT(cid)	// Multi-track
#define GET_N(cid)	// Number
#define GET_NCN(cid)	// New Cylinder Number
#define GET_ND(cid)	(cid&0x01)		// Non-DMA Mode
#define GET_R(cid)	// Record
#define GET_RW(cid)	// Read/Write Signal
#define GET_SC(cid)	// Sector
#define GET_SK(cid)	// Skip
#define GET_SRT(cid)	((cid&0xf0)>>4)	// Step Rate Time
#define GET_STP(cid)	// Step

// Result Bytes
#define BYTE_ERROR		0x80			// Error Code
#define BYTE_PCN(n)		(vfdd.pcn[n])	// Present Cylinder Number
#define BYTE_ST0		(vfdc.st0)		// Status 0
#define BYTE_ST1		(vfdc.st1)		// Status 1
#define BYTE_ST2		(vfdc.st2)		// Status 2
#define BYTE_ST3		(vfdc.st3)		// Status 3

static t_nubit8 bcount;
static t_nubit8 cmd[9],result[7];

void IO_Read_03F4()
{vcpu.al = 0x80;}
void IO_Read_03F5() {}
void IO_Read_03F7() {}
void IO_Write_03F2() {}
void IO_Write_03F5()
{
	cmd[bcount++] = vcpu.al;
	if(GET_DS(cmd[1])) {
		result[0] = BYTE_ERROR;
	} else {
		if((cmd[0] == CMD_SPECIFY) && (bcount == 3)) {
			bcount = 0;
			vfdc.hut = GET_HUT(cmd[1]);
			vfdc.srt = GET_SRT(cmd[1]);
			vfdc.hlt = GET_HLT(cmd[2]);
			vfdc.nd = GET_ND(cmd[2]);
		} else if((cmd[0] == CMD_SENSE_DRIVE_STATUS) && (bcount == 2)) {
			bcount = 0;
			BYTE_ST3 = 0x38 | (GET_HDS(cmd[1])<<2) | GET_DS(cmd[1]);
			result[0] = BYTE_ST3;
		} else if((cmd[0] == CMD_RECALIBRATE) && (bcount == 2)) {
			bcount = 0;
			BYTE_PCN(0) = BYTE_PCN(1) = 0;
			BYTE_ST0 = 0x20 | (GET_HDS(cmd[1])<<2) | GET_DS(cmd[1]);
			vfdc.intr = 1;
			vpicSetIRQ(0x06);	// INT 2EH Should Call Command 08H
		} else if((cmd[0] == CMD_SENSE_INTERRUPT) && (bcount == 1)) {
			bcount = 0;
			if(vfdc.intr) {
				vfdc.intr = 0;
				result[0] = BYTE_ST0;
				result[1] = BYTE_PCN(GET_HDS(BYTE_ST0));
			} else result[0] = BYTE_ERROR;
		} else if((cmd[0] == CMD_SEEK) && (bcount == 3)) {
			bcount = 0;
			BYTE_PCN(GET_HDS(cmd[1])) = cmd[2];
			BYTE_ST0 = 0x20 | (GET_HDS(cmd[1])<<2) | GET_DS(cmd[1]);
			vfdc.intr = 1;
			vpicSetIRQ(0x06);	// INT 2EH Should Call Command 08H
		} else if((cmd[0] == 0x03) && (bcount == 3)) {}
		else if((cmd[0] == 0x03) && (bcount == 3)) {}
		else if((cmd[0] == 0x03) && (bcount == 3)) {}
		else if((cmd[0] == 0x03) && (bcount == 3)) {}
		else if((cmd[0] == 0x03) && (bcount == 3)) {}
		else if((cmd[0] == 0x03) && (bcount == 3)) {}
		else if((cmd[0] == 0x03) && (bcount == 3)) {}
		else if((cmd[0] == 0x03) && (bcount == 3)) {}
		else if((cmd[0] == 0x03) && (bcount == 3)) {}
		else if((cmd[0] == 0x03) && (bcount == 3)) {}
	}
}
void IO_Write_03F7() {}

void FDCInit()
{
	bcount = 0;
	memset(&vfdc,0,sizeof(t_fdc));
	vcpuinsInPort[0x03f4] = (t_vaddrcc)IO_Read_03F4;
	vcpuinsInPort[0x03f5] = (t_vaddrcc)IO_Read_03F5;
	vcpuinsInPort[0x03f7] = (t_vaddrcc)IO_Read_03F7;
	vcpuinsOutPort[0x03f2] = (t_vaddrcc)IO_Write_03F2;
	vcpuinsOutPort[0x03f5] = (t_vaddrcc)IO_Write_03F5;
	vcpuinsOutPort[0x03f7] = (t_vaddrcc)IO_Write_03F7;
}
void FDCTerm() {}