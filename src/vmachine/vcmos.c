/* This file is a part of NXVM project. */

#include "memory.h"
#include "time.h"

#include "vapi.h"

#include "vport.h"
#include "vcpu.h"
#include "vcmos.h"

t_cmos vcmos;

void IO_Write_0070()
{
	vcmos.rid = vport.iobyte;                                /* select reg id */
	if (vcmos.rid & 0x80) vcpu.flagnmi = 0x00;      /* if MSB=1, disable NMI */
	else vcpu.flagnmi = 0x01;
}
void IO_Write_0071()
{
	t_nubitcc i;
	t_nubit16 checksum = 0x00;
	vcmos.reg[vcmos.rid] = vport.iobyte;
	if((vcmos.rid > 0x0f) && (vcmos.rid < 0x2e))
		for(i = 0x10;i < 0x2e;++i) checksum += vcmos.reg[i];
	vcmos.reg[VCMOS_CHECKSUM_LSB] = (t_nubit8)(checksum & 0xff);
	vcmos.reg[VCMOS_CHECKSUM_MSB] = (t_nubit8)(checksum >> 8);
}
void IO_Read_0071()
{
	vport.iobyte = vcmos.reg[vcmos.rid];
}

void vcmosRefresh()
{
	static time_t tprev = 0;
	time_t tcurr;
	struct tm *ptm;
	t_nubit8 century,year,month,mday,wday,hour,min,sec;

	tcurr = time(NULL);
	if (tcurr == tprev) return;
	else tprev = tcurr;
	ptm = localtime(&tcurr);

	century = (t_nubit8)(19 + ptm->tm_year / 100);
	year    = (t_nubit8)(ptm->tm_year % 100);
	month   = (t_nubit8)(ptm->tm_mon + 0x01);
	mday    = (t_nubit8)(ptm->tm_mday);
	wday    = (t_nubit8)(ptm->tm_wday + 0x01);
	hour    = (t_nubit8)(ptm->tm_hour);
	min     = (t_nubit8)(ptm->tm_min);
	sec     = (t_nubit8)(ptm->tm_sec);

	vcmos.reg[VCMOS_RTC_SECOND]    = Hex2BCD(sec);
	vcmos.reg[VCMOS_RTC_MINUTE]    = Hex2BCD(min);
	vcmos.reg[VCMOS_RTC_HOUR]      = Hex2BCD(hour);
	vcmos.reg[VCMOS_RTC_DAY_WEEK]  = Hex2BCD(wday);
	vcmos.reg[VCMOS_RTC_DAY_MONTH] = Hex2BCD(mday);
	vcmos.reg[VCMOS_RTC_MONTH]     = Hex2BCD(month);
	vcmos.reg[VCMOS_RTC_YEAR]      = Hex2BCD(year);
	vcmos.reg[VCMOS_RTC_CENTURY]   = Hex2BCD(century);
}
void vcmosInit()
{
	memset(&vcmos, 0x00, sizeof(t_cmos));
	vport.in[0x0071] = (t_faddrcc)IO_Read_0071;
	vport.out[0x0070] = (t_faddrcc)IO_Write_0070;
	vport.out[0x0071] = (t_faddrcc)IO_Write_0071;
	vcmosRefresh();
}
void vcmosFinal() {}
