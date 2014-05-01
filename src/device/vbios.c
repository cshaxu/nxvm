/* Copyright 2012-2014 Neko. */

/* VBIOS loads bios data, interrupt routines and post routines for all devices. */

#include "../utils.h"

#include "vram.h"
#include "vhdd.h"

#include "vmachine.h"
#include "vbios.h"

t_bios vbios;

static t_bool flagBoot;
static t_nubit16 ics, iip;

void deviceConnectBiosSetBoot(t_bool flagHdd) {flagBoot = flagHdd;}
t_bool deviceConnectBiosGetBoot() {return flagBoot;}

static t_nubit32 assemble(const t_strptr stmt, t_nubit16 seg, t_nubit16 off) {
	t_nubit32 len;
	t_nubit8 code[0x10000];
	len = utilsAasm32x(stmt, code, 0);
	if (!len) {
		PRINTF("vbios: invalid x86 assembly instruction.\n");
	}
	MEMCPY((void *) vramGetRealAddr(seg, off), (void *) code, len);
	return len;
}

static void biosLoadData() {
	MEMSET((void *) vramGetRealAddr(0x0040, 0x0000), 0x00, 0x100);
	vramRealByte(0x0040, 0x0000) = 0xf8;
	vramRealByte(0x0040, 0x0001) = 0x03;
	vramRealByte(0x0040, 0x0008) = 0x78;
	vramRealByte(0x0040, 0x0008) = 0x03;
	vramRealByte(0x0040, 0x000e) = 0xc0;
	vramRealByte(0x0040, 0x000f) = 0x9f;
	vramRealByte(0x0040, 0x0010) = 0x21;
	vramRealByte(0x0040, 0x0013) = 0x7f;
	vramRealByte(0x0040, 0x0014) = 0x02;
	vramRealByte(0x0040, 0x0017) = 0x20;
	vramRealByte(0x0040, 0x001a) = 0x1e;
	vramRealByte(0x0040, 0x001b) = 0x04;
	vramRealByte(0x0040, 0x001c) = 0x1e;
	vramRealByte(0x0040, 0x001d) = 0x04;
	vramRealByte(0x0040, 0x001e) = 0x1c;
	vramRealByte(0x0040, 0x001f) = 0x0d;
	vramRealByte(0x0040, 0x0020) = 0x1c;
	vramRealByte(0x0040, 0x0021) = 0x0d;
	vramRealByte(0x0040, 0x0022) = 0x22;
	vramRealByte(0x0040, 0x0023) = 0x67;
	vramRealByte(0x0040, 0x003e) = 0x01;
	vramRealByte(0x0040, 0x0040) = 0x25;
	vramRealByte(0x0040, 0x0041) = 0x09;
	vramRealByte(0x0040, 0x0045) = 0x01;
	vramRealByte(0x0040, 0x0047) = 0x01;
	vramRealByte(0x0040, 0x0048) = 0x02;
	vramRealByte(0x0040, 0x0049) = 0x03;
	vramRealByte(0x0040, 0x004a) = 0x50;
	vramRealByte(0x0040, 0x004d) = 0x10;
	vramRealByte(0x0040, 0x0051) = 0x05;
	vramRealByte(0x0040, 0x0060) = 0x0e;
	vramRealByte(0x0040, 0x0061) = 0x0d;
	vramRealByte(0x0040, 0x0063) = 0xd4;
	vramRealByte(0x0040, 0x0064) = 0x03;
	vramRealByte(0x0040, 0x0065) = 0x29;
	vramRealByte(0x0040, 0x0066) = 0x30;
	vramRealByte(0x0040, 0x0074) = 0x01;
	vramRealByte(0x0040, 0x0075) = vhdd.flagexist ? 0x01 : 0x00; /* number of hard disks */
	vramRealByte(0x0040, 0x0076) = 0xc0;
	vramRealByte(0x0040, 0x0078) = 0x14;
	vramRealByte(0x0040, 0x007c) = 0x0a;
	vramRealByte(0x0040, 0x0080) = 0x1e;
	vramRealByte(0x0040, 0x0082) = 0x3e;
	vramRealByte(0x0040, 0x0084) = 0x18;
	vramRealByte(0x0040, 0x0085) = 0x10;
	vramRealByte(0x0040, 0x0087) = 0x60;
	vramRealByte(0x0040, 0x0088) = 0x09;
	vramRealByte(0x0040, 0x0089) = 0x11;
	vramRealByte(0x0040, 0x008a) = 0x0b;
	vramRealByte(0x0040, 0x008f) = 0x77;
	vramRealByte(0x0040, 0x0090) = 0x17;
	vramRealByte(0x0040, 0x0096) = 0x10;
	vramRealByte(0x0040, 0x0097) = 0x02;
	vramRealByte(0x0040, 0x00a8) = 0x3a;
	vramRealByte(0x0040, 0x00a9) = 0x5d;
	vramRealByte(0x0040, 0x00ab) = 0xc0;
	vramRealByte(0x0040, 0x0100) = flagBoot ? 0x80 : 0x00; /* boot disk */
}

static void biosLoadRomInfo() {
	vramRealWord(0xf000, 0xe6f5) = 0x0008;
	vramRealByte(0xf000, 0xe6f7) = 0xfc;
	vramRealByte(0xf000, 0xe6f8) = 0x00;
	vramRealByte(0xf000, 0xe6f9) = 0x01;
	vramRealByte(0xf000, 0xe6fa) = 0xb4;
	vramRealByte(0xf000, 0xe6fb) = 0x40;
	vramRealByte(0xf000, 0xe6fc) = 0x00;
	vramRealByte(0xf000, 0xe6fd) = 0x00;
	vramRealByte(0xf000, 0xe6fe) = 0x00;
}

static void biosLoadInt() {
	t_nubit16 i;
	iip += (t_nubit16) assemble("iret", VBIOS_AREA_START_SEG, VBIOS_AREA_START_OFF);
	for (i = 0x0000;i < 0x0100;++i) {
		if (vbios.intTable[i]) {
			vramRealWord(0x0000, i * 4 + 0) = iip;
			vramRealWord(0x0000, i * 4 + 2) = ics;
			iip += (t_nubit16) assemble(vbios.intTable[i], ics, iip);
		} else {
			vramRealWord(0x0000, i * 4 + 0) = VBIOS_AREA_START_OFF;
			vramRealWord(0x0000, i * 4 + 2) = VBIOS_AREA_START_SEG;
		}
	}
}

static void biosLoadPost() {
	t_nubitcc i;
	t_string stmt;
	SPRINTF(stmt, "jmp %04x:%04x", ics, iip);
	assemble(stmt, VBIOS_AREA_POST_SEG, VBIOS_AREA_POST_OFF);
	for (i = 0;i < vbios.numPosts;++i) {
		iip += (t_nubit16) assemble(vbios.postTable[i], ics, iip);
	}
    iip += (t_nubit16) assemble(VBIOS_POST_BOOT, ics, iip);
}

static void biosLoadAdditional() {
	/* hard disk param table */
	vramRealWord(0x0000, 0x0104) = 0xe431;
	vramRealWord(0x0000, 0x0106) = 0xf000;
	vramRealWord(0xf000, 0xe431 +  0) = vhdd.ncyl;
	vramRealByte(0xf000, 0xe431 +  2) = (t_nubit8)vhdd.nhead;
	vramRealByte(0xf000, 0xe431 +  3) = 0xa0;
	vramRealByte(0xf000, 0xe431 +  4) = (t_nubit8)vhdd.nsector;
	vramRealWord(0xf000, 0xe431 +  5) = 0xffff;
	vramRealByte(0xf000, 0xe431 +  7) = 0x00;
	vramRealByte(0xf000, 0xe431 +  8) = 0x08;
	vramRealWord(0xf000, 0xe431 +  9) = vhdd.ncyl;
	vramRealByte(0xf000, 0xe431 + 11) = (t_nubit8)vhdd.nhead;
	vramRealWord(0xf000, 0xe431 + 12) = 0x0000;
	vramRealByte(0xf000, 0xe431 + 14) = (t_nubit8)vhdd.nsector;
	vramRealByte(0xf000, 0xe431 + 15) = 0x00;
}

void vbiosAddPost(t_strptr stmt) {vbios.postTable[vbios.numPosts++] = stmt;}

void vbiosAddInt(t_strptr stmt, t_nubit8 intid) {vbios.intTable[intid] = stmt;}

static void init() {
	MEMSET(&vbios, 0x00, sizeof(t_bios));
	vbiosAddInt(VBIOS_INT_SOFT_MISC_11, 0x11);
	vbiosAddInt(VBIOS_INT_SOFT_MISC_12, 0x12);
	vbiosAddInt(VBIOS_INT_SOFT_MISC_15, 0x15);
}

/* Loads bios to ram */
static void reset() {
	/* bios area starts at f000:0000 */
	ics = VBIOS_AREA_START_SEG;
	iip = VBIOS_AREA_START_OFF;
	biosLoadData();       /* bios data area */
	biosLoadRomInfo();    /* bios rom info area */
	biosLoadInt();        /* bios interrupt services */
	biosLoadPost();       /* bios init/post routines */
	biosLoadAdditional(); /* additional bios data */
}

static void refresh() {}

static void final() {}

void vbiosRegister() {vmachineAddMe;}

void devicePrintBios() {
	PRINTF("Boot Disk: %s\n", deviceConnectBiosGetBoot() ? "Hard Drive" : "Floppy");
}
