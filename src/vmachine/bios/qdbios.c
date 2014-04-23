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
#include "../debug/aasm.h"

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

#define VBIOS_POST_ROUTINE "  \
\
; init vpic1                \n\
mov al, 11 ; icw1 0001 0001 \n\
out 20, al                  \n\
mov al, 08 ; icw2 0000 1000 \n\
out 21, al                  \n\
mov al, 04 ; icw3 0000 0100 \n\
out 21, al                  \n\
mov al, 11 ; icw4 0001 0001 \n\
out 21, al                  \n\
\
; init vpic2                \n\
mov al, 11 ; icw1 0001 0001 \n\
out a0, al                  \n\
mov al, 70 ; icw2 0111 0000 \n\
out a1, al                  \n\
mov al, 02 ; icw3 0000 0010 \n\
out a1, al                  \n\
mov al, 01 ; icw4 0000 0001 \n\
out a1, al                  \n\
\
; init vcmos                \n\
mov al, 0b ; select reg b   \n\
out 70, al                  \n\
mov al, 01 ; 24 hour mode   \n\
out 71, al                  \n\
\
; init vdma                 \n\
mov al, 00                  \n\
out 08, al ;                \n\
out d0, al ;                \n\
mov al, c0                  \n\
out d6, al ;                \n\
\
; init vfdc                 \n\
mov al, 00                  \n\
mov dx, 03f2                \n\
out dx, al                  \n\
mov al, 0c                  \n\
mov dx, 03f2                \n\
out dx, al                  \n\
mov al, 03                  \n\
mov dx, 03f5                \n\
out dx, al ; cmd specify    \n\
mov al, af                  \n\
mov dx, 03f5                \n\
out dx, al                  \n\
mov al, 02                  \n\
mov dx, 03f5                \n\
out dx, al                  \n\
\
; init vpit                                       \n\
mov al, 36 ; 0011 0110 mode = 3, counter = 0, 16b \n\
out 43, al                                        \n\
mov al, 00                                        \n\
out 40, al ; initial count (0x10000)              \n\
out 40, al                                        \n\
mov al, 54 ; 0101 0100 mode = 2, counter = 1, LSB \n\
out 43, al                                        \n\
mov al, 12                                        \n\
out 41, al ; initial count (0x12)                 \n\
\
; start operating system    \n\
xor ax, ax                  \n\
xor dx, dx                  \n\
jmp 0000:7c00               \n"

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
	/* qdkeyb init */
	qdkeybReset();
	/* qdcga init */
	qdcgaReset();
	/* load boot sector */
	qddiskReset();
	/* misc */
	qdmiscReset();
	/* load cmos data */
	qdrtcReset();
}
static void vbiosLoadPost()
{
	char stmt[0x1000];
	SPRINTF(stmt, "jmp %04x:%04x", ics, iip);
	aasm(stmt, 0xf000, 0xfff0);
	iip += (t_nubit16)aasm(VBIOS_POST_ROUTINE, ics, iip);
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
