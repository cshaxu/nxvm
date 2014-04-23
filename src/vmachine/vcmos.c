/* This file is a part of NXVM project. */

#include "memory.h"
#include "time.h"

#include "vcpu.h"
#include "vcpuins.h"
#include "vcmos.h"

// 1/1024 Hz INT & ALARM INT are not implemented.

#define CMOS_RTC_SECOND			0x00
#define CMOS_RTC_SECOND_ALARM	0x01
#define CMOS_RTC_MINUTE			0x02
#define CMOS_RTC_MINUTE_ALARM	0x03
#define CMOS_RTC_HOUR			0x04
#define CMOS_RTC_HOUR_ALARM		0x05
#define CMOS_RTC_DAY_WEEK		0x06
#define CMOS_RTC_DAY_MONTH		0x07
#define CMOS_RTC_MONTH			0x08
#define CMOS_RTC_YEAR			0x09
#define CMOS_RTC_REG_A			0x0a
#define CMOS_RTC_REG_B			0x0b
#define CMOS_RTC_REG_C			0x0c
#define CMOS_RTC_REG_D			0x0d
#define CMOS_STATUS_DIAG		0x0e
#define CMOS_STATUS_SHUTDOWN	0x0f
#define CMOS_TYPE_DISK_FLOPPY	0x10
#define CMOS_TYPE_DISK_FIXED	0x12
#define CMOS_EQUIPMENT			0x14
#define CMOS_BASEMEM_LSB		0x15
#define CMOS_BASEMEM_MSB		0x16
#define CMOS_EXTMEM_LSB			0x17
#define CMOS_EXTMEM_MSB			0x18
#define CMOS_DRIVE_C			0x19
#define CMOS_DRIVE_D			0x1a
#define CMOS_CHECKSUM_MSB		0x2e
#define CMOS_CHECKSUM_LSB		0x2f
#define CMOS_EXTMEM1MB_LSB		0x30
#define CMOS_EXTMEM1MB_MSB		0x31
#define CMOS_RTC_CENTURY		0x32
#define CMOS_FLAGS_INFO			0x33

t_nubit8 vcmosreg[0x40];

static t_nubit8 index;

static t_nubitcc GetNUM2BCD(t_nubitcc num)
{
	t_nubitcc i = 0,bcd = 0;
	while(num > 0) {
		bcd |= ((num%10)<<(i*4));
		i++;
		num /= 10;
	}
	return bcd;
}
/*
static t_nubitcc GetBCD2NUM(t_nubitcc bcd)
{
	t_nubitcc i = 1,num = 0;
	while(bcd > 0) {
		num += (bcd&0x0f)*i;
		bcd >>= 4;
		i *= 10;
	}
	return num;
}*/

static void UpdateTime()
{	
	time_t tseconds = time(NULL);
	struct tm *tnow = localtime(&tseconds);
	t_nubitcc year = tnow->tm_year%100,century = 19+tnow->tm_year/100;
	vcmosreg[CMOS_RTC_SECOND] = ((t_nubit8)GetNUM2BCD((t_nubitcc)tnow->tm_sec))&0x7f;
	vcmosreg[CMOS_RTC_MINUTE] = ((t_nubit8)GetNUM2BCD((t_nubitcc)tnow->tm_min))&0x7f;
	vcmosreg[CMOS_RTC_HOUR] = ((t_nubit8)GetNUM2BCD((t_nubitcc)tnow->tm_hour))&0x3f;
	vcmosreg[CMOS_RTC_DAY_WEEK] = ((t_nubit8)GetNUM2BCD((t_nubitcc)tnow->tm_wday+1))&0x07;
	vcmosreg[CMOS_RTC_DAY_MONTH] = ((t_nubit8)GetNUM2BCD((t_nubitcc)tnow->tm_mday))&0x3f;
	vcmosreg[CMOS_RTC_MONTH] = ((t_nubit8)GetNUM2BCD((t_nubitcc)tnow->tm_mon+1))&0x3f;
	vcmosreg[CMOS_RTC_YEAR] = ((t_nubit8)GetNUM2BCD(year))&0xff;
	vcmosreg[CMOS_RTC_CENTURY] = ((t_nubit8)GetNUM2BCD(century))&0xff;
	//vramSetDWord(0x0000,0x046c,
	//	(t_nubit32)(((((t_double)((tnow->tm_hour)*60)+tnow->tm_min)*60+tnow->tm_sec)*1000)/VCMOS_TICK)-tickstart);
	//if(vramGetDWord(0x0000,0x046c) > 0x001800b0) {
	//	vramSetDWord(0x0000,0x046c,0x00000000);
	//	vramSetByte(0x0000,0x0470,vramGetByte(0x0000,0x0470)+0x01);
	//}
	//vapiPrint("SECOND:%d,%x\n",tnow->tm_sec,vcmosreg[CMOS_RTC_SECOND]);
	//vapiPrint("MINUTE:%d,%x\n",tnow->tm_min,vcmosreg[CMOS_RTC_MINUTE]);
	//vapiPrint("HOUR:%d,%x\n",tnow->tm_hour,vcmosreg[CMOS_RTC_HOUR]);
	//vapiPrint("WDAY:%d,%x\n",tnow->tm_wday,vcmosreg[CMOS_RTC_DAY_WEEK]);
	//vapiPrint("MDAY:%d,%x\n",tnow->tm_mday,vcmosreg[CMOS_RTC_DAY_MONTH]);
	//vapiPrint("MONTH:%d,%x\n",tnow->tm_mon,vcmosreg[CMOS_RTC_MONTH]);
	//vapiPrint("YEAR:%d,%x\n",tnow->tm_year,vcmosreg[CMOS_RTC_YEAR]);
	//vapiPrint("CENTURY:%d,%x\n",tnow->tm_year,vcmosreg[CMOS_RTC_CENTURY]);
}

void IO_Write_0070()
{
	index = vcpu.al;
}
void IO_Write_0071()
{
	t_nubitcc i;
	t_nubit16 checksum = 0;
	vcmosreg[index] = vcpu.al;
	if((index > 0x0f) && (index < 0x2e))
		for(i = 0x10;i < 0x2e;++i)
			checksum += vcmosreg[i];
	vcmosreg[CMOS_CHECKSUM_LSB] = checksum&0xff;
	vcmosreg[CMOS_CHECKSUM_MSB] = checksum>>8;
}
void IO_Read_0071()
{
	UpdateTime();
	vcpu.al = vcmosreg[index];
}

void CMOSInit()
{
	index = 0x00;
	memset(vcmosreg,0x00,0x40);

	/*	Initialization of Registers Here..
		To be implemented!! */
	vcmosreg[CMOS_RTC_HOUR] = 0x80;

	vcpuinsInPort[0x0071] = (t_faddrcc)IO_Read_0071;
	vcpuinsOutPort[0x0070] = (t_faddrcc)IO_Write_0070;
	vcpuinsOutPort[0x0071] = (t_faddrcc)IO_Write_0071;
}
void CMOSTerm() {}