/* This file is a part of NXVM project. */

#include "memory.h"

#include "vcpu.h"
#include "vcpuins.h"
#include "vfdd.h"
#include "vfdc.h"

t_fdc vfdc;

#define READ_TRACK			0x02	// generates IRQ6
#define SPECIFY				0x03	// * set drive parameters
#define SENSE_DRIVE_STATUS	0x04
#define WRITE_DATA			0x05	// * write to the disk
#define READ_DATA			0x06	// * read from the disk
#define RECALIBRATE			0x07	// * seek to cylinder 0
#define SENSE_INTERRUPT		0x08	// * ack IRQ6 get status of last command
#define WRITE_DELETED_DATA	0x09
#define READ_ID				0x0a	// generates IRQ6
#define READ_DELETED_DATA	0x0c
#define FORMAT_TRACK		0x0d	// *
#define SEEK				0x0f	// * seek both heads to cylinder X
#define VERSION				0x10	// * used during initialization once
#define SCAN_EQUAL			0x11
#define PERPENDICULAR_MODE	0x12	// * used during initialization once maybe
#define CONFIGURE			0x13	// * set controller parameters
#define LOCK				0x14	// * protect controller params from a reset
#define VERIFY				0x16
#define SCAN_LOW_OR_EQUAL	0x19
#define SCAN_HIGH_OR_EQUAL	0x1d

static t_nubit8 bcount;
static t_nubit8 cmd[9],status[7];

void IO_Read_03F4()
{vcpu.al = 0x80;}
void IO_Read_03F5() {}
void IO_Read_03F7() {}
void IO_Write_03F2() {}
void IO_Write_03F5()
{
	cmd[bcount++] = vcpu.al;
	if(bcount > 1) {
		if((cmd[0] == SPECIFY) && (bcount == 3)) {
			bcount = 0;
			vfdc.hut = cmd[1]&0x0f;
			vfdc.srt = (cmd[1]&0xf0)>>4;
			vfdc.hlt = cmd[2]>>1;
			vfdc.nondma = cmd[2]&0x01;
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