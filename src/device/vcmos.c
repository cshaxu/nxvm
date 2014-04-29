/* Copyright 2012-2014 Neko. */

/* VCMOS implements CMOS and Real Time Clock DS1302. */

#include "../utils.h"

#include "vcpu.h"

#include "vbios.h"
#include "vport.h"
#include "vmachine.h"
#include "vcmos.h"

t_cmos vcmos;

static void io_write_0070() {
	vcmos.rid = vport.iobyte; /* select reg id */
	if (GetMSB8(vcmos.rid)) {
		/* if MSB=1, disable NMI */
		vcpu.flagmasknmi = 1;
	} else {
		vcpu.flagmasknmi = 0;
	}
}

static void io_write_0071() {
	t_nubit8 i;
	t_nubit16 checksum = 0x00;
	vcmos.reg[vcmos.rid] = vport.iobyte;
	if ((vcmos.rid > 0x0f) && (vcmos.rid < 0x2e)) {
		for (i = 0x10;i < 0x2e;++i) {
			checksum += vcmos.reg[i];
		}
	}
	vcmos.reg[VCMOS_CHECKSUM_LSB] = (t_nubit8)(checksum & 0xff);
	vcmos.reg[VCMOS_CHECKSUM_MSB] = (t_nubit8)(checksum >> 8);
}

static void io_read_0071() {
	vport.iobyte = vcmos.reg[vcmos.rid];
}

static void refresh();

static void init() {
	memset(&vcmos, 0x00, sizeof(t_cmos));
	vport.in[0x0071] = (t_faddrcc) io_read_0071;
	vport.out[0x0070] = (t_faddrcc) io_write_0070;
	vport.out[0x0071] = (t_faddrcc) io_write_0071;
    vbiosAddPost(VCMOS_POST);
	vbiosAddInt(VCMOS_INT_HARD_RTC_08, 0x08);
	vbiosAddInt(VCMOS_INT_SOFT_RTC_1A, 0x1a);
	refresh();
}

static void reset() {}

static void refresh() {
	static time_t tprev = 0;
	time_t tcurr;
	struct tm *ptm;
	t_nubit8 century, year, month, mday, wday, hour, min, sec;

	tcurr = time(NULL);
	if (tcurr == tprev) {
		return;
	} else {
		tprev = tcurr;
	}
	ptm = LOCALTIME(&tcurr);

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

static void final() {}

void vcmosRegister() {
	vmachine.deviceTable[VMACHINE_DEVICE_INIT][vmachine.numDevices] = (t_faddrcc) init;
	vmachine.deviceTable[VMACHINE_DEVICE_RESET][vmachine.numDevices] = (t_faddrcc) reset;
	vmachine.deviceTable[VMACHINE_DEVICE_REFRESH][vmachine.numDevices] = (t_faddrcc) refresh;
	vmachine.deviceTable[VMACHINE_DEVICE_FINAL][vmachine.numDevices] = (t_faddrcc) final;
	vmachine.numDevices++;
}
