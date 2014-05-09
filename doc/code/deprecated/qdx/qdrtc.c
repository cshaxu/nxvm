#include "stdlib.h"
#include "vcpuins.h"

#include "vapi.h"

#include "vport.h"
#include "vcpu.h"
#include "vram.h"
#include "qdrtc.h"

#define QDRTC_TICK  54.9254
#define QDRTC_VBIOS_ADDR_RTC_DAILY_COUNTER 0x046c
#define QDRTC_VBIOS_ADDR_RTC_ROLLOVER      0x0470

#define Hex2BCD(x)  ((((x) / 10) << 4) | ((x) % 10))
#define BCD2Hex(x)  ((x) & 0x0f + (((x) & 0xf0) >> 4) * 10)

t_rtc qdrtc;

void IO_Write_0070() {
    qdrtc.iobyte = vcpu.iobyte & 0x7f;
}
void IO_Read_0071() {
    switch (qdrtc.iobyte) {
    case 0x0b:
        vcpu.iobyte = 0x02;
        break;
    default:
        vcpu.iobyte = 0x00;
        break;
    }
}

void qdrtcGetTimeTickCount() {
    _cx = vramVarWord(0x0000, QDRTC_VBIOS_ADDR_RTC_DAILY_COUNTER + 2);
    _dx = vramVarWord(0x0000, QDRTC_VBIOS_ADDR_RTC_DAILY_COUNTER + 0);
    if (vramVarByte(0x0000, QDRTC_VBIOS_ADDR_RTC_ROLLOVER) == 0x00)
        _al = 0x00;
    else
        _al = 0x01;
    vramVarByte(0x0000, QDRTC_VBIOS_ADDR_RTC_ROLLOVER) = 0x00;
}
void qdrtcSetTimeTickCount() {
    vramVarWord(0x0000, QDRTC_VBIOS_ADDR_RTC_DAILY_COUNTER + 2) = _cx;
    vramVarWord(0x0000, QDRTC_VBIOS_ADDR_RTC_DAILY_COUNTER + 0) = _dx;
    vramVarByte(0x0000, QDRTC_VBIOS_ADDR_RTC_ROLLOVER) = 0x00;
}
void qdrtcGetCmosTime() {
    t_float64 total = (vramVarDWord(0x0000, QDRTC_VBIOS_ADDR_RTC_DAILY_COUNTER) *
                       QDRTC_TICK) / 1000;
    t_nubit8 hour = (t_nubit8)(total / 3600);
    t_nubit8 min  = (t_nubit8)((total - hour * 3600) / 60);
    t_nubit8 sec  = (t_nubit8)(total - hour * 3600 - min * 60);
    _ch = Hex2BCD(hour);
    _cl = Hex2BCD(min);
    _dh = Hex2BCD(sec);
    _dl = 0x00;
    ClrCF;
}
void qdrtcSetCmosTime() {
    t_nubit8 hour = BCD2Hex(_ch);
    t_nubit8 min  = BCD2Hex(_cl);
    t_nubit8 sec  = BCD2Hex(_dh);
    vramVarDWord(0x0000, QDRTC_VBIOS_ADDR_RTC_DAILY_COUNTER) =
        (t_nubit32)(((hour * 3600 + min * 60 + sec) * 1000) / QDRTC_TICK);
    ClrCF;
}
void qdrtcGetCmosDate() {
    struct tm *t = gmtime(&qdrtc.start);
    if (t->tm_year >= 100) {      /* tm_year starts from 1900 */
        _ch = 0x20;                                 /* century: 20 (BCD) */
        _cl = Hex2BCD(t->tm_year - 100);
    } else {
        _ch = 0x19;
        _cl = Hex2BCD(t->tm_year - 100);
    }
    _dh = Hex2BCD(t->tm_mon + 1);
    _dl = Hex2BCD(t->tm_mday);
    ClrCF;
}
void qdrtcSetCmosDate() {
    struct tm *t = gmtime(&qdrtc.start);
    t->tm_year = BCD2Hex(_cl);
    if (_ch == 0x20) t->tm_year += 100;
}
void qdrtcSetAlarmClock() {
    /* return a fail to cpu */
    SetCF;
}

void vapiCallBackRtcUpdateTime() {
    vramVarDWord(0x0000, QDRTC_VBIOS_ADDR_RTC_DAILY_COUNTER) += 1;
    if (vramVarDWord(0x0000, QDRTC_VBIOS_ADDR_RTC_DAILY_COUNTER) >= 0x1800b2) {
        vramVarDWord(0x0000, QDRTC_VBIOS_ADDR_RTC_DAILY_COUNTER) = 0x00000000;
        vramVarByte(0x0000, QDRTC_VBIOS_ADDR_RTC_ROLLOVER) = 0x01;
    }
}

void qdrtcInit() {
    /* this is actually a part of bios initialization */
    struct tm *t;
    t_nubitcc hour;
    t_nubitcc min;
    t_nubitcc sec;

    vport.in[0x0071]  = (t_faddrcc)IO_Read_0071;
    vport.out[0x0070] = (t_faddrcc)IO_Write_0070;

    qdrtc.start = time(NULL) + 3600 * (-7);
    t = gmtime(&qdrtc.start);
    hour = t->tm_hour;
    min  = t->tm_min;
    sec  = t->tm_sec;
    vramVarDWord(0x0000, QDRTC_VBIOS_ADDR_RTC_DAILY_COUNTER) =
        (t_nubit32)(((hour * 3600 + min * 60 + sec) * 1000) / QDRTC_TICK);
    vramVarByte(0x0000, QDRTC_VBIOS_ADDR_RTC_ROLLOVER) = 0x00;
}
void qdrtcFinal() {
}