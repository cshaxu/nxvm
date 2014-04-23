/* This file is a part of NXVM project. */

#include "stdio.h"
#include "memory.h"
#include "time.h"

#include "../vapi.h"
#include "../vmachine.h"
#include "../debug/aasm.h"

#include "qdcga.h"
#include "qdkeyb.h"
#include "qddisk.h"
#include "qdmisc.h"
#include "qdbios.h"

#include "qdrtc.h"
#include "post.h"

t_faddrcc qdbiosInt[0x100];
static t_nubit16 ics, iip;

void qdbiosMakeInt(t_nubit8 intid, t_string stmt)
{
	t_nubit16 len;
	vramVarWord(0x0000, intid * 4 + 0) = iip;
	vramVarWord(0x0000, intid * 4 + 2) = ics;
	len = (t_nubit16)aasm(stmt, ics, iip);
	if (len) iip += len;
	else {
		vapiPrint("Critical internal error: invalid asm instruction.\n");
	}
}
void qdbiosExecInt(t_nubit8 intid)
{
	if (qdbiosInt[intid]) ExecFun(qdbiosInt[intid]);
}

static void vbiosLoadData()
{
	memset((void *)vramGetAddr(0x0040,0x0000), 0x00, 0x100);
	vramVarByte(0x0040, 0x0000) = 0xf8;
	vramVarByte(0x0040, 0x0001) = 0x03;
	vramVarByte(0x0040, 0x0008) = 0x78;
	vramVarByte(0x0040, 0x0008) = 0x03;
	vramVarByte(0x0040, 0x000e) = 0xc0;
	vramVarByte(0x0040, 0x000f) = 0x9f;
	vramVarByte(0x0040, 0x0010) = 0x21;
	vramVarByte(0x0040, 0x0013) = 0x7f;
	vramVarByte(0x0040, 0x0014) = 0x02;
	vramVarByte(0x0040, 0x0017) = 0x20;
	vramVarByte(0x0040, 0x001a) = 0x1e;
	vramVarByte(0x0040, 0x001b) = 0x04;
	vramVarByte(0x0040, 0x001c) = 0x1e;
	vramVarByte(0x0040, 0x001d) = 0x04;
	vramVarByte(0x0040, 0x001e) = 0x1c;
	vramVarByte(0x0040, 0x001f) = 0x0d;
	vramVarByte(0x0040, 0x0020) = 0x1c;
	vramVarByte(0x0040, 0x0021) = 0x0d;
	vramVarByte(0x0040, 0x0022) = 0x22;
	vramVarByte(0x0040, 0x0023) = 0x67;
	vramVarByte(0x0040, 0x003e) = 0x01;
	vramVarByte(0x0040, 0x0040) = 0x25;
	vramVarByte(0x0040, 0x0041) = 0x09;
	vramVarByte(0x0040, 0x0045) = 0x01;
	vramVarByte(0x0040, 0x0047) = 0x01;
	vramVarByte(0x0040, 0x0048) = 0x02;
	vramVarByte(0x0040, 0x0049) = 0x03;
	vramVarByte(0x0040, 0x004a) = 0x50;
	vramVarByte(0x0040, 0x004d) = 0x10;
	vramVarByte(0x0040, 0x0051) = 0x05;
	vramVarByte(0x0040, 0x0060) = 0x0e;
	vramVarByte(0x0040, 0x0061) = 0x0d;
	vramVarByte(0x0040, 0x0063) = 0xd4;
	vramVarByte(0x0040, 0x0064) = 0x03;
	vramVarByte(0x0040, 0x0065) = 0x29;
	vramVarByte(0x0040, 0x0066) = 0x30;
	vramVarByte(0x0040, 0x0074) = 0x01;
	vramVarByte(0x0040, 0x0075) = vhdd.flagexist ? 0x01 : 0x00; /* number of hard disks */
	vramVarByte(0x0040, 0x0076) = 0xc0;
	vramVarByte(0x0040, 0x0078) = 0x14;
	vramVarByte(0x0040, 0x007c) = 0x0a;
	vramVarByte(0x0040, 0x0080) = 0x1e;
	vramVarByte(0x0040, 0x0082) = 0x3e;
	vramVarByte(0x0040, 0x0084) = 0x18;
	vramVarByte(0x0040, 0x0085) = 0x10;
	vramVarByte(0x0040, 0x0087) = 0x60;
	vramVarByte(0x0040, 0x0088) = 0x09;
	vramVarByte(0x0040, 0x0089) = 0x11;
	vramVarByte(0x0040, 0x008a) = 0x0b;
	vramVarByte(0x0040, 0x008f) = 0x77;
	vramVarByte(0x0040, 0x0090) = 0x17;
	vramVarByte(0x0040, 0x0096) = 0x10;
	vramVarByte(0x0040, 0x0097) = 0x02;
	vramVarByte(0x0040, 0x00a8) = 0x3a;
	vramVarByte(0x0040, 0x00a9) = 0x5d;
	vramVarByte(0x0040, 0x00ab) = 0xc0;
	vramVarByte(0x0040, 0x0100) = vmachine.flagboot ? 0x80 : 0x00; /* boot disk */
}
static void vbiosLoadRomInfo()
{
	vramVarWord(0xf000, 0xe6f5) = 0x0008;
	vramVarByte(0xf000, 0xe6f7) = 0xfc;
	vramVarByte(0xf000, 0xe6f8) = 0x00;
	vramVarByte(0xf000, 0xe6f9) = 0x01;
	vramVarByte(0xf000, 0xe6fa) = 0xb4;
	vramVarByte(0xf000, 0xe6fb) = 0x40;
	vramVarByte(0xf000, 0xe6fc) = 0x00;
	vramVarByte(0xf000, 0xe6fd) = 0x00;
	vramVarByte(0xf000, 0xe6fe) = 0x00;
}
static void vbiosLoadInt()
{
	t_nubit16 i;
	for (i = 0x0000;i < 0x0100;++i) {
		qdbiosInt[i] = (t_faddrcc)NULL;
		vramVarWord(0x0000, i * 4 + 0) = iip;
		vramVarWord(0x0000, i * 4 + 2) = ics;
	}
	iip += (t_nubit16)aasm("iret", ics, iip);
	/* load rtc int */
	qdbiosMakeInt(0x08, HARD_RTC_INT_08);
	qdbiosMakeInt(0x1a, SOFT_RTC_INT_1A);
	/* load bios service int */
	qdbiosMakeInt(0x11, SOFT_MISC_INT_11);
	qdbiosMakeInt(0x12, SOFT_MISC_INT_12);
	qdbiosMakeInt(0x15, SOFT_MISC_INT_15);
	/* load disk int */
	qdbiosMakeInt(0x0e, HARD_FDD_INT_0E);
	qdbiosMakeInt(0x13, SOFT_DISK_INT_13);
	qdbiosMakeInt(0x40, SOFT_DISK_INT_40);
	/* qdkeyb init */
	qdkeybReset();
	/* qdcga init */
	qdcgaReset();
	/* qddisk init: remaining int for hdd */
	qddiskReset();
}
static void vbiosLoadPost()
{
	char stmt[0x1000];
	SPRINTF(stmt, "jmp %04x:%04x", ics, iip);
	aasm(stmt, 0xf000, 0xfff0);
	iip += (t_nubit16)aasm(VBIOS_POST_VPIC,  ics, iip);
	iip += (t_nubit16)aasm(VBIOS_POST_VDMA,  ics, iip);
	iip += (t_nubit16)aasm(VBIOS_POST_VFDC,  ics, iip);
	iip += (t_nubit16)aasm(VBIOS_POST_VPIT,  ics, iip);
	iip += (t_nubit16)aasm(VBIOS_POST_VCMOS, ics, iip);
	iip += (t_nubit16)aasm(VBIOS_POST_QDRTC, ics, iip);
	iip += (t_nubit16)aasm(VBIOS_POST_BOOT,  ics, iip);
	/* hard disk param table */
	vramVarWord(0x0000, 0x0104) = 0xe431;
	vramVarWord(0x0000, 0x0106) = 0xf000;
	vramVarWord(0xf000, 0xe431 +  0) = vhdd.ncyl;
	vramVarByte(0xf000, 0xe431 +  2) = (t_nubit8)vhdd.nhead;
	vramVarByte(0xf000, 0xe431 +  3) = 0xa0;
	vramVarByte(0xf000, 0xe431 +  4) = (t_nubit8)vhdd.nsector;
	vramVarWord(0xf000, 0xe431 +  5) = 0xffff;
	vramVarByte(0xf000, 0xe431 +  7) = 0x00;
	vramVarByte(0xf000, 0xe431 +  8) = 0x08;
	vramVarWord(0xf000, 0xe431 +  9) = vhdd.ncyl;
	vramVarByte(0xf000, 0xe431 + 11) = (t_nubit8)vhdd.nhead;
	vramVarWord(0xf000, 0xe431 + 12) = 0x0000;
	vramVarByte(0xf000, 0xe431 + 14) = (t_nubit8)vhdd.nsector;
	vramVarByte(0xf000, 0xe431 + 15) = 0x00;
}

void qdbiosReset()
{
	ics = 0xf000;
	iip = 0x0000;
/* bios data area */
	vbiosLoadData();
/* bios rom info area */
	vbiosLoadRomInfo();
/* bios interrupt services */
	vbiosLoadInt();
/* bios init/post program */
	vbiosLoadPost();
}
