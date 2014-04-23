/* This file is a part of NXVM project. */

#include "stdio.h"
#include "memory.h"
#include "time.h"

#include "../vapi.h"
#include "../vmachine.h"

#include "qdcga.h"
#include "qdkeyb.h"
#include "qddisk.h"
#include "qdrtc.h"
#include "qdmisc.h"
#include "qdbios.h"
#include "../../console/asm86.h"

t_faddrcc qdbiosInt[0x100];
static t_nubit16 ics, iip;

void qdbiosSetInt(t_nubit8 intid, t_nubit16 intcs, t_nubit16 intip)
{
	vramVarWord(0x0000, intid * 4 + 0) = intip;
	vramVarWord(0x0000, intid * 4 + 2) = intcs;
}

static void doasm(t_string s)
{
	int id;
	static t_nubit16 jcs[10], jip[10];
	switch (s[0]) {
	case '!':
		id = s[1] - '0';
		jcs[id] = ics;
		jip[id] = iip;
		s[0] = s[1] = ' ';
		break;
	case '#': /* entrance of short jmp */
		id = s[1] - '0';
		vramVarByte(jcs[id], jip[id]+1) = (t_nubit8)(iip - jip[id] + 2);
		s[0] = ' ';
		break;
	}
	iip += assemble(s, 0xf000, (void *)vram.base, ics, iip);
}
void qdbiosAsmInt(t_string stmt)
{
	int i = 0;
	char str[0x1000];
	char *p;
	STRCPY(str,stmt);
	p = str;
	while (i < 0x1000) {
		if (str[i] == ';') {
			str[i] = 0;
			doasm(p);
			p = str + i + 1;
		} else if (!str[i]) {
			doasm(p);
			break;
		} else ++i;
	}
}
void qdbiosMakeInt(t_nubit8 intid, t_string stmt)
{
	qdbiosSetInt(intid, ics, iip);
	qdbiosAsmInt(stmt);
}

void qdbiosExecInt(t_nubit8 intid)
{
	if (qdbiosInt[intid]) ExecFun(qdbiosInt[intid]);
}

#define out(a,b) (vport.iobyte = (b), ExecFun(vport.out[(a)]))
void qdbiosReset()
{
	t_nubit16 i;
	for (i = 0x0000;i < 0x0100;++i) {
		qdbiosInt[i] = (t_faddrcc)NULL;
		qdbiosSetInt((t_nubit8)i, 0xf000, 0x0000);
	}
/* build general int service routine */
	ics = 0xf000;
	iip = 0x0000;
	qdbiosAsmInt("iret");
/* bios data area */
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
/* bios rom info area */
	vramVarWord(0xf000, 0xe6f5) = 0x0008;
	vramVarByte(0xf000, 0xe6f7) = 0xfc;
	vramVarByte(0xf000, 0xe6f8) = 0x00;
	vramVarByte(0xf000, 0xe6f9) = 0x01;
	vramVarByte(0xf000, 0xe6fa) = 0xb4;
	vramVarByte(0xf000, 0xe6fb) = 0x40;
	vramVarByte(0xf000, 0xe6fc) = 0x00;
	vramVarByte(0xf000, 0xe6fd) = 0x00;
	vramVarByte(0xf000, 0xe6fe) = 0x00;
/* first cpu instruction */
	vramVarByte(0xf000, 0xfff0) = 0xea;
	vramVarWord(0xf000, 0xfff1) = 0x7c00;
	vramVarWord(0xf000, 0xfff3) = 0x0000;

/* device initialize */
/* vpic init */
	out(0x20, 0x11);                                      /* ICW1: 0001 0001 */
	out(0x21, 0x08);                                      /* ICW2: 0000 1000 */
	out(0x21, 0x04);                                      /* ICW3: 0000 0100 */
	out(0x21, 0x11);                                      /* ICW4: 0001 0001 */
	out(0xa0, 0x11);                                      /* ICW1: 0001 0001 */
	out(0xa1, 0x70);                                      /* ICW2: 0111 0000 */
	out(0xa1, 0x02);                                      /* ICW3: 0000 0010 */
	out(0xa1, 0x01);                                      /* ICW4: 0000 0001 */
/* vcmos init */
	out(0x70, VCMOS_RTC_REG_B);
	out(0x71, 0x02);
/* vdma init */
	out(0x08, 0x00);
	out(0xd0, 0x00);
	out(0xd6, 0xc0);
/* vfdc init */
	out(0x03f2, 0x00);
	out(0x03f2, 0x0c);
	out(0x03f5, 0x03);                               /* send specify command */
	out(0x03f5, 0xaf);
	out(0x03f5, 0x02);
/* vpit init*/
	out(0x43, 0x36);                 /* al=0011 0110: Mode=3, Counter=0, 16b */
	out(0x40, 0x00);
	out(0x40, 0x00);
	out(0x43, 0x54);                 /* al=0101 0100: Mode=2, Counter=1, LSB */
	out(0x41, 0x12);
	
	/* qdkeyb init */
	qdkeybReset();
	/* qdcga init */
	qdcgaReset();
	/* load cmos data */
	qdrtcReset();
	/* load boot sector */
	qddiskReset();
	/* misc */
	qdmiscReset();
}
