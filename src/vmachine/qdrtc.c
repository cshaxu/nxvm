#include "stdlib.h"
#include "time.h"

#include "vram.h"
#include "vcpu.h"
#include "qdrtc.h"

#define QDRTC_TICK  54.9254
#define QDRTC_VBIOS_ADDR_RTC_DAILY_COUNTER 0x046c
#define QDRTC_VBIOS_ADDR_RTC_ROLLOVER      0x0470

#define Hex2BCD(x)  ((((x) / 10) << 4) | ((x) % 10))
#define BCD2Hex(x)  ((x) & 0x0f + (((x) & 0xf0) >> 4) * 10)

static time_t qdrtc_time_start;
void qdrtcGetTimeTickCount()
{
	vcpu.cx = vramWord(0x0000, QDRTC_VBIOS_ADDR_RTC_DAILY_COUNTER + 2);
	vcpu.dx = vramWord(0x0000, QDRTC_VBIOS_ADDR_RTC_DAILY_COUNTER + 0);
	if (vramByte(0x0000, QDRTC_VBIOS_ADDR_RTC_ROLLOVER) == 0x00)
		vcpu.al = 0x00;
	else
		vcpu.al = 0x01;
	vramByte(0x0000, QDRTC_VBIOS_ADDR_RTC_ROLLOVER) = 0x00;
}
void qdrtcSetTimeTickCount()
{
	vramWord(0x0000, QDRTC_VBIOS_ADDR_RTC_DAILY_COUNTER + 2) = vcpu.cx;
	vramWord(0x0000, QDRTC_VBIOS_ADDR_RTC_DAILY_COUNTER + 0) = vcpu.dx;
	vramByte(0x0000, QDRTC_VBIOS_ADDR_RTC_ROLLOVER) = 0x00;
}
void qdrtcGetCmosTime()
{
	t_float64 total = (vramDWord(0x0000, QDRTC_VBIOS_ADDR_RTC_DAILY_COUNTER) *
		               QDRTC_TICK) / 1000;
	t_nubit8 hour = (t_nubit8)(total / 3600);
	t_nubit8 min  = (t_nubit8)((total - hour * 3600) / 60);
	t_nubit8 sec  = (t_nubit8)(total - hour * 3600 - min * 60); 
	vcpu.ch = Hex2BCD(hour);
	vcpu.cl = Hex2BCD(min);
	vcpu.dh = Hex2BCD(sec);
	vcpu.dl = 0x00;
	ClrBit(vcpu.flags, VCPU_FLAG_CF);
}
void qdrtcSetCmosTime()
{
	t_nubit8 hour = BCD2Hex(vcpu.ch);
	t_nubit8 min  = BCD2Hex(vcpu.cl);
	t_nubit8 sec  = BCD2Hex(vcpu.dh);
	vramDWord(0x0000, QDRTC_VBIOS_ADDR_RTC_DAILY_COUNTER) = 
		(t_nubit32)(((hour * 3600 + min * 60 + sec) * 1000) / QDRTC_TICK);
	ClrBit(vcpu.flags, VCPU_FLAG_CF);
}
void qdrtcGetCmosDate()
{
	struct tm *t = gmtime(&qdrtc_time_start);
	if(t->tm_year >= 100) {       /* tm_year starts from 1900 */
		vcpu.ch = 0x20;                                 /* century: 20 (BCD) */
		vcpu.cl = Hex2BCD(t->tm_year - 100);
	} else {
		vcpu.ch = 0x19;
		vcpu.cl = Hex2BCD(t->tm_year - 100);
	}
	vcpu.dh = Hex2BCD(t->tm_mon + 1);
	vcpu.dl = Hex2BCD(t->tm_mday);
	ClrBit(vcpu.flags, VCPU_FLAG_CF);
}
void qdrtcSetCmosDate()
{
	struct tm *t = gmtime(&qdrtc_time_start);
	t->tm_year = BCD2Hex(vcpu.cl);
	if(vcpu.ch == 0x20) t->tm_year += 100;	
}
void qdrtcSetAlarmClock()
{
	/* return a fail to cpu */
	SetBit(vcpu.flags, VCPU_FLAG_CF);
}

void qdrtcInit()
{
	/* this is actually a part of bios initialization */
	struct tm *t;
	t_nubit8 hour;
	t_nubit8 min;
	t_nubit8 sec;
	qdrtc_time_start = time(NULL) + 3600 * (-8);
	t = gmtime(&qdrtc_time_start);
	hour = t->tm_hour;
	min  = t->tm_min;
	sec  = t->tm_sec;
	vramDWord(0x0000, QDRTC_VBIOS_ADDR_RTC_DAILY_COUNTER) = 
		(t_nubit32)(((hour * 3600 + min * 60 + sec) * 1000) / QDRTC_TICK);
	vramByte(0x0000, QDRTC_VBIOS_ADDR_RTC_ROLLOVER) = 0x00;
}
void qdrtcFinal()
{}