/* This file is a part of NXVM project. */

#include "memory.h"
#include "time.h"

#include "vcpu.h"
#include "vcpuins.h"
#include "vcmos.h"

// 1/1024 Hz INT & ALARM INT are not implemented.

t_cmos vcmos;

/*
 * GetNUM2BCD: Internal function
 * Translates deci interger to BCD code
 */
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
 * GetBCD2NUM: Internal function
 * Translates BCD code to deci integer
 */
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
	t_nubitcc year = tnow->tm_year % 100,century = 19 + tnow->tm_year / 100;
	vcmos.reg[CMOS_RTC_SECOND] = ((t_nubit8)GetNUM2BCD((t_nubitcc)tnow->tm_sec)) & 0x7f;
	vcmos.reg[CMOS_RTC_MINUTE] = ((t_nubit8)GetNUM2BCD((t_nubitcc)tnow->tm_min)) & 0x7f;
	vcmos.reg[CMOS_RTC_HOUR] = ((t_nubit8)GetNUM2BCD((t_nubitcc)tnow->tm_hour)) & 0x3f;
	vcmos.reg[CMOS_RTC_DAY_WEEK] = ((t_nubit8)GetNUM2BCD((t_nubitcc)tnow->tm_wday + 0x01)) & 0x07;
	vcmos.reg[CMOS_RTC_DAY_MONTH] = ((t_nubit8)GetNUM2BCD((t_nubitcc)tnow->tm_mday)) & 0x3f;
	vcmos.reg[CMOS_RTC_MONTH] = ((t_nubit8)GetNUM2BCD((t_nubitcc)tnow->tm_mon + 0x01)) & 0x3f;
	vcmos.reg[CMOS_RTC_YEAR] = ((t_nubit8)GetNUM2BCD(year)) & 0xff;
	vcmos.reg[CMOS_RTC_CENTURY] = ((t_nubit8)GetNUM2BCD(century)) & 0xff;
/*
	vramSetDWord(0x0000,0x046c,
		(t_nubit32)(((((t_float64)((tnow->tm_hour)*60)+tnow->tm_min)*60+tnow->tm_sec)*1000)/VCMOS_TICK)-tickstart);
	if(vramGetDWord(0x0000,0x046c) > 0x001800b0) {
		vramSetDWord(0x0000,0x046c,0x00000000);
		vramSetByte(0x0000,0x0470,vramGetByte(0x0000,0x0470)+0x01);
	}
	vapiPrint("SECOND:%d,%x\n",tnow->tm_sec,vcmosreg[CMOS_RTC_SECOND]);
	vapiPrint("MINUTE:%d,%x\n",tnow->tm_min,vcmosreg[CMOS_RTC_MINUTE]);
	vapiPrint("HOUR:%d,%x\n",tnow->tm_hour,vcmosreg[CMOS_RTC_HOUR]);
	vapiPrint("WDAY:%d,%x\n",tnow->tm_wday,vcmosreg[CMOS_RTC_DAY_WEEK]);
	vapiPrint("MDAY:%d,%x\n",tnow->tm_mday,vcmosreg[CMOS_RTC_DAY_MONTH]);
	vapiPrint("MONTH:%d,%x\n",tnow->tm_mon,vcmosreg[CMOS_RTC_MONTH]);
	vapiPrint("YEAR:%d,%x\n",tnow->tm_year,vcmosreg[CMOS_RTC_YEAR]);
	vapiPrint("CENTURY:%d,%x\n",tnow->tm_year,vcmosreg[CMOS_RTC_CENTURY]);
*/
}

void IO_Write_0070()
{
	vcmos.rid = vcpu.iobyte;                                    /* select reg id */
	if (vcmos.rid & 0x80) vcpu.nmi = 0;             /* if MSB=1, disable NMI */
	else vcpu.nmi = 1;
}
void IO_Write_0071()
{
/*
	t_nubitcc i;
	t_nubit16 checksum = 0;
*/
	vcmos.reg[vcmos.rid] = vcpu.iobyte;
/*
	if((idreg > 0x0f) && (idreg < 0x2e))
		for(i = 0x10;i < 0x2e;++i)
			checksum += vcmosreg[i];
	vcmosreg[CMOS_CHECKSUM_LSB] = checksum&0xff;
	vcmosreg[CMOS_CHECKSUM_MSB] = checksum>>8;
*/
}
void IO_Read_0071()
{
	UpdateTime();
	vcpu.iobyte = vcmos.reg[vcmos.rid];
}

void vcmosInit()
{
	memset(&vcmos, 0x00, sizeof(t_cmos));

#ifdef VCMOS_DEBUG
	/*	Initialization of Registers Here..
		To be implemented!! */
	vcmos.reg[CMOS_RTC_HOUR] = 0x80;
#endif

	vcpuinsInPort[0x0071] = (t_faddrcc)IO_Read_0071;
	vcpuinsOutPort[0x0070] = (t_faddrcc)IO_Write_0070;
	vcpuinsOutPort[0x0071] = (t_faddrcc)IO_Write_0071;
}
void vcmosFinal() {}
