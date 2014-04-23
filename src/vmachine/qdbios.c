#include "memory.h"

#include "../system/vapi.h"
#include "vcpu.h"
#include "vram.h"

#include "qdvga.h"
#include "qdfdd.h"
#include "qdkeyb.h"
#include "qdrtc.h"
#include "qdbios.h"

/* vga */
void INT_10()
{
	t_nubit16 tmpCX = vcpu.cx;
	switch (vcpu.ah) {
	case 0x00:
		qdvgaSetDisplayMode();
		break;
	case 0x01:
		qdvgaSetCursorShape();
		break;
	case 0x02:
		qdvgaSetCursorPos();
		break;
	case 0x03:
		qdvgaGetCursorPos();
		break;
	case 0x04:
		break;
	case 0x05:
		qdvgaSetDisplayPage();
		break;
	case 0x06:
		qdvgaScrollUp();
		break;
	case 0x07:
		qdvgaScrollDown();
		break;
	case 0x08:
		qdvgaGetCharProp();
		break;
	case 0x09:
		qdvgaDisplayCharProp();
		break;
	case 0x0a:
		qdvgaDisplayChar();
		break;
	case 0x0b:
		//qdvgaSetPalette();
		break;
	case 0x0c:
		//qdvgaDisplayPixel();
		break;
	case 0x0d:
		//qdvgaGetPixel();
		break;
	case 0x0e:
		vcpu.cx = 0x01;
		qdvgaDisplayCharProp();
		vcpu.cx = tmpCX;
		break;
	case 0x0f:
		qdvgaGetAdapterStatus();
		break;
	case 0x10:
		break;
	case 0x11:
		qdvgaGenerateChar();
		break;
	case 0x12:
		if (vcpu.bl == 0x10)
			qdvgaGetAdapterInfo();
		break;
	case 0x13:
		qdvgaDisplayStr();
		break;
	case 0x1a:
		if (vcpu.al == 0x00) {
			vcpu.al = 0x1a;
			vcpu.bh = 0x00;
			vcpu.bl = 0x08;
		}
		break;
	default:
		break;
	}
	vapiDisplayPaint();
}
/* device test*/
void INT_11()
{vcpu.ax = 0x0061;}
/* memory test*/
void INT_12()
{vcpu.ax = 0x027f;}
/* fdd */
void INT_13()
{
	switch (vcpu.ah) {
	case 0x00:
		qdfddResetDrive();
		break;
	case 0x01:
		qdfddGetDiskStatus();
		break;
	case 0x02:
		qdfddReadSector();
		break;
	case 0x03:
		qdfddWriteSector();
		break;
	case 0x08:
		qdfddGetParameter();
		break;
	case 0x15:
		qdfddGetDriveType();
		break;
	default:
		break;
	}
}
/* com ports */
void INT_14()
{/* do nothing */}
/* bios: device detect */
void INT_15()
{
	int MemorySize = 1;
	switch (vcpu.ah)
	{
	case 0xc0:
		vcpu.es = 0xf000;
		vcpu.bx = 0xe6f5;
		vcpu.ah = 0x00;
		ClrBit(vcpu.flags, VCPU_FLAG_CF);
		break;
	case 0x24:
		if (vcpu.al == 0x03) {
			/*ClrBit(vramWord(vcpu.ss, vcpu.sp + 4), VCPU_FLAG_CF);*/
			// remember: all flags set in INT handler should be modified
			ClrBit(vcpu.flags, VCPU_FLAG_CF);
			vcpu.ah=0;
			vcpu.bx=3;
		}
		break;
	case 0x88:
		ClrBit(vcpu.flags, VCPU_FLAG_CF);
		/*ClrBit(vramWord(vcpu.ss, vcpu.sp + 4), VCPU_FLAG_CF);*/
		if (MemorySize > 16)		
			vcpu.ax = 0x3c00;					
		else		
			vcpu.ax = MemorySize * 1024 - 256;
		break;
	case 0xd8:
		SetBit(vramWord(vcpu.ss, vcpu.sp + 4), VCPU_FLAG_CF);
		vcpu.ah=0x86;
		break;
	}
}
/* keyb */
void INT_16()
{
	switch (vcpu.ah) {
	case 0x00:
	case 0x10:
		qdkeybReadInput();
		break;
	case 0x01:
	case 0x11:
		qdkeybGetStatus();
		break;
	case 0x02:
		qdkeybGetShiftStatus();
		break;
	default:
		break;
	}
}
/* lpt ports */
void INT_17()
{/* do nothing */}
/* rtc */
void INT_1A()
{
	switch(vcpu.ah) {
	case 0x00:
		qdrtcGetTimeTickCount();
		break;
	case 0x01:
		qdrtcSetTimeTickCount();
		break;
	case 0x2:
		qdrtcGetCmosTime();
		break;
	case 0x3:
		qdrtcSetCmosTime();
		break;
	case 0x4:
		qdrtcGetCmosDate();
		break;
	case 0x5:
		qdrtcSetCmosDate();
		break;
	case 0x6:
		qdrtcSetAlarmClock();
		break;
	default:
		break;
	}
}

t_bool qdbiosExecInt(t_nubit8 intid)
{
	switch (intid) {
	case 0x10: /* vga */
		INT_10();return 0x01;
	case 0x13: /* fdd */
		INT_13();return 0x01;
	case 0x15: /* bios */
		INT_15();return 0x01;
	case 0x16: /* keyb */
		INT_16();return 0x01;
	case 0x1a: /* rtc */
		INT_1A();return 0x01;
	case 0x11: /* bios */
		INT_11();return 0x01;
	case 0x12: /* bios */
		INT_12();return 0x01;
	case 0x14: /* bios */
		INT_14();return 0x01;
	case 0x17: /* bios */
		INT_17();return 0x01;
	default:     return 0x00;
	}
}
void qdbiosInit()
{
	t_nubit16 i;
/* build interrupt vector table */
	for (i = 0x0000;i < 0x0100;++i) {
		vramSetByte(0x0000, i*4 + 0x00, 0x00);
		vramSetByte(0x0000, i*4 + 0x01, 0x00);
		vramSetByte(0x0000, i*4 + 0x02, 0x00);
		vramSetByte(0x0000, i*4 + 0x03, 0xf0);
	}
/* build general int service routine */
	vramSetByte(0xf000, 0x0000, 0xcf);
/* special: out bb,al for INT 16 */
	vramSetByte(0xf000, 0x0001, 0xe6);  
	vramSetByte(0xf000, 0x0002, 0xbb);
	vramSetByte(0xf000, 0x0003, 0xcf);
	vramSetByte(0x0000, 0x58 + 0x00, 0x01);
	vramSetByte(0x0000, 0x58 + 0x01, 0x00);
	vramSetByte(0x0000, 0x58 + 0x02, 0x00);
	vramSetByte(0x0000, 0x58 + 0x03, 0xf0);
/* bios data area */
	for(i = 0x0000;i < 0x0100;++i) vramSetByte(0x0040, i, 0x00);
	vramSetByte(0x0040, 0x0000, 0xf8);
	vramSetByte(0x0040, 0x0001, 0x03);
	vramSetByte(0x0040, 0x0008, 0x78);
	vramSetByte(0x0040, 0x0008, 0x03);
	vramSetByte(0x0040, 0x000e, 0xc0);
	vramSetByte(0x0040, 0x000f, 0x9f);
	vramSetByte(0x0040, 0x0010, 0x61);
	vramSetByte(0x0040, 0x0013, 0x7f);
	vramSetByte(0x0040, 0x0014, 0x02);
	vramSetByte(0x0040, 0x0017, 0x20);
	vramSetByte(0x0040, 0x001a, 0x1e);
	vramSetByte(0x0040, 0x001b, 0x04);
	vramSetByte(0x0040, 0x001c, 0x1e);
	vramSetByte(0x0040, 0x001d, 0x04);
	vramSetByte(0x0040, 0x001e, 0x1c);
	vramSetByte(0x0040, 0x001f, 0x0d);
	vramSetByte(0x0040, 0x0020, 0x1c);
	vramSetByte(0x0040, 0x0021, 0x0d);
	vramSetByte(0x0040, 0x0022, 0x22);
	vramSetByte(0x0040, 0x0023, 0x67);
	vramSetByte(0x0040, 0x003e, 0x01);
	vramSetByte(0x0040, 0x0040, 0x25);
	vramSetByte(0x0040, 0x0041, 0x09);
	vramSetByte(0x0040, 0x0045, 0x01);
	vramSetByte(0x0040, 0x0047, 0x01);
	vramSetByte(0x0040, 0x0048, 0x02);
	vramSetByte(0x0040, 0x0049, 0x03);
	vramSetByte(0x0040, 0x004a, 0x50);
	vramSetByte(0x0040, 0x004d, 0x10);
	vramSetByte(0x0040, 0x0051, 0x05);
	vramSetByte(0x0040, 0x0060, 0x0e);
	vramSetByte(0x0040, 0x0061, 0x0d);
	vramSetByte(0x0040, 0x0063, 0xd4);
	vramSetByte(0x0040, 0x0064, 0x03);
	vramSetByte(0x0040, 0x0065, 0x29);
	vramSetByte(0x0040, 0x0066, 0x30);
/*
	RTC Tick Count Storage
	vramSetByte(0x0040, 0x006c, 0xf7);
	vramSetByte(0x0040, 0x006d, 0xd6);
	vramSetByte(0x0040, 0x006e, 0x0f);
*/
	vramSetByte(0x0040, 0x0074, 0x01);
	vramSetByte(0x0040, 0x0076, 0xc0);
	vramSetByte(0x0040, 0x0078, 0x14);
	vramSetByte(0x0040, 0x007c, 0x0a);
	vramSetByte(0x0040, 0x0080, 0x1e);
	vramSetByte(0x0040, 0x0082, 0x3e);
	vramSetByte(0x0040, 0x0084, 0x18);
	vramSetByte(0x0040, 0x0085, 0x10);
	vramSetByte(0x0040, 0x0087, 0x60);
	vramSetByte(0x0040, 0x0088, 0x09);
	vramSetByte(0x0040, 0x0089, 0x11);
	vramSetByte(0x0040, 0x008a, 0x0b);
	vramSetByte(0x0040, 0x008f, 0x77);
	vramSetByte(0x0040, 0x0090, 0x17);
	vramSetByte(0x0040, 0x0096, 0x10);
	vramSetByte(0x0040, 0x0097, 0x02);
	vramSetByte(0x0040, 0x00a8, 0x3a);
	vramSetByte(0x0040, 0x00a9, 0x5d);
	vramSetByte(0x0040, 0x00ab, 0xc0);
/* bios rom info area */
	vramSetWord(0xf000, 0xe6f5, 0x0008);
	vramSetByte(0xf000, 0xe6f7, 0xfc);
	vramSetByte(0xf000, 0xe6f8, 0x00);
	vramSetByte(0xf000, 0xe6f9, 0x01);
	vramSetByte(0xf000, 0xe6fa, 0xb4);
	vramSetByte(0xf000, 0xe6fb, 0x40);
	vramSetByte(0xf000, 0xe6fc, 0x00);
	vramSetByte(0xf000, 0xe6fd, 0x00);
	vramSetByte(0xf000, 0xe6fe, 0x00);

/* device initialize */
	qdfddInit();
	qdrtcInit();
	qdkeybInit();
	qdvgaInit();

/* load boot sector */
	vapiFloppyInsert("d:/msdos.img");
	memcpy((void *)vramGetAddress(0x7c00), (void *)qdfdd.base, 0x200);
	vramSetByte(0xf000, 0xfff0, 0xea);
	vramSetWord(0xf000, 0xfff1, 0x7c00);
	vramSetWord(0xf000, 0xfff3, 0x0000);
}
void qdbiosFinal()
{
	qdvgaFinal();
	qdkeybFinal();
	qdrtcFinal();
	qdfddFinal();
}
