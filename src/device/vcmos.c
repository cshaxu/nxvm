/* Copyright 2012-2014 Neko. */

/* VCMOS implements CMOS and Real Time Clock DS1302. */

#include "../utils.h"

#include "vcpu.h"

#include "vbios.h"
#include "vport.h"
#include "vcmos.h"

t_cmos vcmos;

static void io_write_0070() {
    vcmos.data.regId = vport.data.ioByte; /* select reg id */
    if (GetMSB8(vcmos.data.regId)) {
        /* if MSB=1, disable NMI */
        vcpu.data.flagMaskNMI = True;
    } else {
        vcpu.data.flagMaskNMI = False;
    }
}

static void io_write_0071() {
    t_nubit8 i;
    t_nubit16 checksum = Zero16;
    vcmos.connect.reg[vcmos.data.regId] = vport.data.ioByte;
    if ((vcmos.data.regId >= VCMOS_TYPE_DISK_FLOPPY) && (vcmos.data.regId < VCMOS_CHECKSUM_MSB)) {
        for (i = VCMOS_TYPE_DISK_FLOPPY; i < VCMOS_CHECKSUM_MSB; ++i) {
            checksum += vcmos.connect.reg[i];
        }
    }
    vcmos.connect.reg[VCMOS_CHECKSUM_LSB] = GetMax8(checksum);
    vcmos.connect.reg[VCMOS_CHECKSUM_MSB] = GetMax8(checksum >> 8);
}

static void io_read_0071() {
    vport.data.ioByte = vcmos.connect.reg[vcmos.data.regId];
}

void vcmosInit() {
    MEMSET((void *)(&vcmos), Zero8, sizeof(t_cmos));
    vportAddRead(0x0071, (t_faddrcc) io_read_0071);
    vportAddWrite(0x0070, (t_faddrcc) io_write_0070);
    vportAddWrite(0x0071, (t_faddrcc) io_write_0071);
    vbiosAddPost(VCMOS_POST);
    vbiosAddInt(VCMOS_INT_HARD_RTC_08, 0x08);
    vbiosAddInt(VCMOS_INT_SOFT_RTC_1A, 0x1a);
    vcmosReset();
    vcmosRefresh();
}

void vcmosReset() {
    MEMSET((void *)(&vcmos.data), Zero8, sizeof(t_cmos_data));
}

void vcmosRefresh() {
    static time_t tPrev = 0;
    time_t tCurr;
    struct tm *ptm;
    t_nubit8 century, year, month, mday, wday, hour, min, sec;

    tCurr = time(NULL);
    if (tCurr == tPrev) {
        return;
    } else {
        tPrev = tCurr;
    }
    ptm = LOCALTIME(&tCurr);

    century = GetMax8(19 + ptm->tm_year / 100);
    year    = GetMax8(ptm->tm_year % 100);
    month   = GetMax8(ptm->tm_mon + 1);
    mday    = GetMax8(ptm->tm_mday);
    wday    = GetMax8(ptm->tm_wday + 1);
    hour    = GetMax8(ptm->tm_hour);
    min     = GetMax8(ptm->tm_min);
    sec     = GetMax8(ptm->tm_sec);

    vcmos.connect.reg[VCMOS_RTC_SECOND]    = Hex2BCD(sec);
    vcmos.connect.reg[VCMOS_RTC_MINUTE]    = Hex2BCD(min);
    vcmos.connect.reg[VCMOS_RTC_HOUR]      = Hex2BCD(hour);
    vcmos.connect.reg[VCMOS_RTC_DAY_WEEK]  = Hex2BCD(wday);
    vcmos.connect.reg[VCMOS_RTC_DAY_MONTH] = Hex2BCD(mday);
    vcmos.connect.reg[VCMOS_RTC_MONTH]     = Hex2BCD(month);
    vcmos.connect.reg[VCMOS_RTC_YEAR]      = Hex2BCD(year);
    vcmos.connect.reg[VCMOS_RTC_CENTURY]   = Hex2BCD(century);
}

void vcmosFinal() {}
