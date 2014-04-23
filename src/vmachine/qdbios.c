#include "memory.h"
#include "time.h"

#include "vapi.h"
#include "vmachine.h"

#include "vcpuins.h"
#include "qdvga.h"
#include "qdkeyb.h"
#include "qdbios.h"

static t_nubit16 sax, sbx, scx, sdx;
#define push_ax  (sax = _ax)
#define push_bx  (sbx = _bx)
#define push_cx  (scx = _cx)
#define push_dx  (sdx = _dx)
#define pop_ax   (_ax = sax)
#define pop_bx   (_bx = sbx)
#define pop_cx   (_cx = scx)
#define pop_dx   (_dx = sdx)
#define add(a,b) ((a) += (b))
#define and(a,b) ((a) &= (b))
#define or(a,b)  ((a) |= (b))
#define cmp(a,b) ((a) == (b))
#define mov(a,b) ((a) = (b))
#define in(a,b)  (ExecFun(vport.in[(b)]), (a) = vcpu.iobyte)
#define out(a,b) (vcpu.iobyte = (b), ExecFun(vport.out[(a)]))
#define shl(a,b) ((a) <<= (b))
#define shr(a,b) ((a) >>= (b))
#define stc      (SetCF)
#define clc      (ClrCF)
#define clp      (ClrPF)
#define cli      (ClrIF)
#define mbp(n)   (vramVarByte(0x0000, (n)))
#define mwp(n)   (vramVarWord(0x0000, (n)))
#define nop
#define inc(n)   ((n)++)
#define dec(n)   ((n)--)
#define _int(n)

static void INT_13_00_FDD_ResetDrive()
{
	if (cmp(_dl, 0x00)) {
		mov(_ah, 0x00);
		clc;
	} else {
		/* only one drive */
		mov(_ah, 0x0c);
		stc;
	}
	mov(mbp(0x0441), _ah); /* set status*/
}
static void INT_13_01_FDD_GetDiskStatus()
{
	mov(_ah, mbp(0x0441)); /* get status */
}
static void INT_13_02_FDD_ReadSector()
{
/*	t_nubit8 drive  = _dl;
	t_nubit8 head   = _dh;
	t_nubit8 cyl    = _ch;
	t_nubit8 sector = _cl;
	t_nubit8 nsec   = _al;*/
	if (_dl || _dh > 1 || _cl > 18 || _ch > 79) {
		/* sector not found */
		mov(_ah, 0x04);
		stc;
	} else {
//t_vaddrcc ifddGetAddress(t_nubit8 cyl, t_nubit8 head, t_nubit8 sector)
//{return (vfdd.base + ((cyl * 2 + head) * 18 + (sector-1)) * 512);}
//		memcpy((void *)vramGetAddr(_es,_bx),
//			(void *)ifddGetAddress(cyl,head,sector), nsec * 512);
		/* set dma */
		push_bx;
		push_cx;
		push_dx;
		push_ax;
		out(0x000b, 0x86); /* set dma mode: block, inc, write, chn2 */
		mov(_cx, _bx);
		mov(_dx, _es);
		shr(_cx, 0x04);
		add(_dx, _cx);
		mov(_ax, _dx);
		shl(_ax, 0x04);
		mov(_cx, _bx);
		and(_cx, 0x000f);
		or(_ax, _cx);      /* calc base addr */
		shr(_dx, 0x0c);    /* calc page register */
		out(0x0004, _al);  /* set addr low byte */
		out(0x0004, _ah);  /* set addr high byte */
		out(0x0081, _dl);  /* set page register */
		pop_ax;
		pop_dx;
		pop_cx;
		pop_bx;
		mov(_ah, 0x00);    /* clear ah for wc */
		shl(_ax, 0x09);    /* calc wc from al */
		dec(_ax);
		out(0x0005, _al);  /* set wc low byte */
		out(0x0005, _ah);  /* set wc high byte */
		out(0x000a, 0x02); /* unmask dma1.chn2 */
		out(0x00d4, 0x00); /* unmask dma2.chn0 */
		/* set fdc */
		out(0x03f5, 0x0f); /* send seek command */
		shl(_dh, 0x02);    /* calc hds byte */
		or (_dl, _dh);     /* calc hds byte */
		shr(_dh, 0x02);    /* calc hds byte */
		out(0x03f5, _dl);  /* set seek hds byte */
		out(0x03f5, _ch);  /* set seek cyl */
/* Note: here vfdc set IRQ6 and vcpu calls INT E */
		do {
			vcpuinsExecInt();
			nop;
			in(_al, 0x03f4); /* get msr */
			and(_al, 0xc0);  /* get ready write status */
		} while(!cmp(_al, 0x80));
		out(0x03f5, 0xe6); /* send read command */
		out(0x03f5, _dl);  /* set stdi hds */
		out(0x03f5, _ch);  /* set stdi cyl */
		out(0x03f5, _dh);  /* set stdi head */
		out(0x03f5, _cl);  /* set stdi start sector id */
		out(0x03f5, 0x02); /* set stdi sector size code (512) */
		out(0x03f5, 0x12); /* set stdi end sector id */
		out(0x03f5, 0x1b); /* set stdi gap length */
		out(0x03f5, 0xff); /* set stdi customized sect size (0xff) */
		/* now everything is ready; DRQ also generated. */
		do {
			nop;
			vdmaRefresh();
			in(_al, 0x03f4); /* get msr */
			and(_al, 0xc0);  /* get ready read status */
		} while(!cmp(_al, 0xc0));
		in(_al, 0x03f5); /* get stdo st0 */
		in(_al, 0x03f5); /* get stdo st1 */
		in(_al, 0x03f5); /* get stdo st2 */
		in(_al, 0x03f5); /* get stdo cyl */
		in(_al, 0x03f5); /* get stdo head */
		in(_al, 0x03f5); /* get stdo sector */
		in(_al, 0x03f5); /* get stdo sector size code */
		mov(_ah, 0x00);
		clc;
	}
	mov(mbp(0x0441), _ah); /* set status*/
}
static void INT_13_03_FDD_WriteSector()
{
/*	t_nubit8 drive  = _dl;
	t_nubit8 head   = _dh;
	t_nubit8 cyl    = _ch;
	t_nubit8 sector = _cl;*/
	if (_dl || _dh > 1 || _cl > 18 || _ch > 79) {
		/* sector not found */
		mov(_ah, 0x04);
		stc;
	} else {
//t_vaddrcc ifddGetAddress(t_nubit8 cyl, t_nubit8 head, t_nubit8 sector)
//{return (vfdd.base + ((cyl * 2 + head) * 18 + (sector-1)) * 512);}
//		memcpy((void *)ifddGetAddress(cyl,head,sector),
//			(void *)vramGetAddr(_es,_bx), _al * 512);
		/* set dma */
		push_bx;
		push_cx;
		push_dx;
		push_ax;
		out(0x000b, 0x8a); /* set dma mode: block, inc, read, chn2 */
		mov(_cx, _bx);
		mov(_dx, _es);
		shr(_cx, 0x04);
		add(_dx, _cx);
		mov(_ax, _dx);
		shl(_ax, 0x04);
		mov(_cx, _bx);
		and(_cx, 0x000f);
		or(_ax, _cx);      /* calc base addr */
		shr(_dx, 0x0c);    /* calc page register */
		out(0x0004, _al);  /* set addr low byte */
		out(0x0004, _ah);  /* set addr high byte */
		out(0x0081, _dl);  /* set page register */
		pop_ax;
		pop_dx;
		pop_cx;
		pop_bx;
		mov(_ah, 0x00);    /* clear ah for wc */
		shl(_ax, 0x09);    /* calc wc from al */
		dec(_ax);
		out(0x0005, _al);  /* set wc low byte */
		out(0x0005, _ah);  /* set wc high byte */
		out(0x000a, 0x02); /* unmask dma1.chn2 */
		out(0x00d4, 0x00); /* unmask dma2.chn0 */
		/* set fdc */
		out(0x03f5, 0x0f); /* send seek command */
		shl(_dh, 0x02);    /* calc hds byte */
		or (_dl, _dh);      /* calc hds byte */
		shr(_dh, 0x02);    /* calc hds byte */
		out(0x03f5, _dl);  /* set seek hds byte */
		out(0x03f5, _ch);  /* set seek cyl */
/* Note: here vfdc set IRQ6 and vcpu calls INT E */
		do {
			vcpuinsExecInt();
			nop;
			in(_al, 0x03f4); /* get msr */
			and(_al, 0xc0);  /* get ready write status */
		} while(!cmp(_al, 0x80));
		out(0x03f5, 0xc5); /* send write command */
		out(0x03f5, _dl);  /* set stdi hds */
		out(0x03f5, _ch);  /* set stdi cyl */
		out(0x03f5, _dh);  /* set stdi head */
		out(0x03f5, _cl);  /* set stdi start sector id */
		out(0x03f5, 0x02); /* set stdi sector size code (512) */
		out(0x03f5, 0x12); /* set stdi end sector id */
		out(0x03f5, 0x1b); /* set stdi gap length */
		out(0x03f5, 0xff); /* set stdi customized sect size (0xff) */
		/* now everything is ready; DRQ also generated. */
		do {
			vdmaRefresh();
			nop;
			in(_al, 0x03f4); /* get msr */
			and(_al, 0xc0);  /* get ready read status */
		} while(!cmp(_al, 0xc0));
		in(_al, 0x03f5); /* get stdo st0 */
		in(_al, 0x03f5); /* get stdo st1 */
		in(_al, 0x03f5); /* get stdo st2 */
		in(_al, 0x03f5); /* get stdo cyl */
		in(_al, 0x03f5); /* get stdo head */
		in(_al, 0x03f5); /* get stdo sector */
		in(_al, 0x03f5); /* get stdo sector size code */
		mov(_ah, 0x00);
		clc;
	}
	mov(mbp(0x0441), _ah); /* set status*/
}
static void INT_13_08_FDD_GetParameter()
{
	switch (_dl) {
	case 0x00:
	case 0x01:
		mov(_ah, 0x00);
		mov(_bl, 0x04);
		mov(_cx, 0x4f12);
		mov(_dx, 0x0102);
		mov(_di, mwp(0x1e * 4 + 0));
		mov(_es, mwp(0x1e * 4 + 2));
		clc;
		break;
	case 0x02:
	case 0x03:
		mov(_ah, 0x01);
		stc;
		clp;
		cli;
		break;
	}
	mov(mbp(0x0441), _ah); /* set status*/
}
static void INT_13_15_FDD_GetDriveType()
{
	switch (_dl) {
	case 0x00:
	case 0x01:
		mov(_ah, 0x01);
		clc;
		break;
	case 0x02:
	case 0x03:
		mov(_ah, 0x00);
		stc;
		break;
	default:
		break;
	}
	mov(mbp(0x0441), _ah); /* set status*/
}

#define QDBIOS_RTC_TICK  54.9254
#define QDBIOS_ADDR_RTC_DAILY_COUNTER 0x046c
#define QDBIOS_ADDR_RTC_ROLLOVER      0x0470

static void INT_1A_00_BIOS_GetTimeTickCount()
{
	mov(_cx, mwp(QDBIOS_ADDR_RTC_DAILY_COUNTER + 2));
	mov(_dx, mwp(QDBIOS_ADDR_RTC_DAILY_COUNTER + 0));
	if (cmp(mbp(QDBIOS_ADDR_RTC_ROLLOVER),0x00))
		mov(_al, 0x00);
	else
		mov(_al, 0x01);
	mov(mbp(QDBIOS_ADDR_RTC_ROLLOVER), 0x00);
}
static void INT_1A_01_BIOS_SetTimeTickCount()
{
	mov(mwp(QDBIOS_ADDR_RTC_DAILY_COUNTER + 2), _cx);
	mov(mwp(QDBIOS_ADDR_RTC_DAILY_COUNTER + 0), _dx);
	mov(mbp(QDBIOS_ADDR_RTC_ROLLOVER), 0x00);
}
static void INT_1A_02_CMOS_GetCmosTime()
{
/*	t_float64 total = (vramVarDWord(0x0000, QDBIOS_ADDR_RTC_DAILY_COUNTER) *
		               QDRTC_TICK) / 1000;
	t_nubit8 hour = (t_nubit8)(total / 3600);
	t_nubit8 min  = (t_nubit8)((total - hour * 3600) / 60);
	t_nubit8 sec  = (t_nubit8)(total - hour * 3600 - min * 60);
	mov(_ch, Hex2BCD(hour));
	mov(_cl, Hex2BCD(min));
	mov(_dh, Hex2BCD(sec));*/
	mov(_ch, vcmos.reg[VCMOS_RTC_HOUR]);
	mov(_cl, vcmos.reg[VCMOS_RTC_MINUTE]);
	mov(_dh, vcmos.reg[VCMOS_RTC_SECOND]);
	push_ax;
	out(0x0070, VCMOS_RTC_REG_B);
	in(_al, 0x0071);
	and(_al, 0x01);
	mov(_dl, _al);
	pop_ax;
	clc;
}
static void INT_1A_03_CMOS_SetCmosTime()
{
	push_ax;
	out(0x0070, VCMOS_RTC_HOUR);
	out(0x0071, _ch);
	out(0x0070, VCMOS_RTC_MINUTE);
	out(0x0071, _cl);
	out(0x0070, VCMOS_RTC_SECOND);
	out(0x0071, _dh);
	if (cmp(_dl, 0x00)) {
		out(0x0070, VCMOS_RTC_REG_B);
		in(_al, 0x0071);
		and(_al, 0xfe);
		out(0x0070, VCMOS_RTC_REG_B);
		out(0x0071, _al);
	} else {
		out(0x0070, VCMOS_RTC_REG_B);
		in(_al, 0x0071);
		or(_al, 0x01);
		out(0x0070, VCMOS_RTC_REG_B);
		out(0x0071, _al);
	}
	pop_ax;
/*	t_nubit8 hour = BCD2Hex(_ch);
	t_nubit8 min  = BCD2Hex(_cl);
	t_nubit8 sec  = BCD2Hex(_dh);
	vramVarDWord(0x0000, QDBIOS_ADDR_RTC_DAILY_COUNTER) = 
		(t_nubit32)(((hour * 3600 + min * 60 + sec) * 1000) / QDRTC_TICK);*/
	clc;
}
static void INT_1A_04_CMOS_GetCmosDate()
{
	//struct tm *t = gmtime(&qdbios_rtc_start);
	//if(t->tm_year >= 100) {       /* tm_year starts from 1900 */
	//	mov(_ch, 0x20);                                 /* century: 20 (BCD) */
	//	mov(_cl, Hex2BCD(t->tm_year - 100));
	//} else {
	//	mov(_ch, 0x19);
	//	mov(_cl, Hex2BCD(t->tm_year - 100));
	//}
	//mov(_dh, Hex2BCD(t->tm_mon + 1));
	//mov(_dl, Hex2BCD(t->tm_mday));
	push_ax;
	out(0x0070, VCMOS_RTC_CENTURY);
	in(_al, 0x0071);
	mov(_ch, _al);
	out(0x0070, VCMOS_RTC_YEAR);
	in(_al, 0x0071);
	mov(_cl, _al);
	out(0x0070, VCMOS_RTC_MONTH);
	in(_al, 0x0071);
	mov(_dh, _al);
	out(0x0070, VCMOS_RTC_DAY_MONTH);
	in(_al, 0x0071);
	mov(_dl, _al);
	pop_ax;
	clc;
}
static void INT_1A_05_CMOS_SetCmosDate()
{
/*
	struct tm *t = gmtime(&qdbios_rtc_start);
	t->tm_year = BCD2Hex(_cl);
	if(cmp(_ch, 0x20)) t->tm_year += 100;*/
	push_ax;
	out(0x0070, VCMOS_RTC_CENTURY);
	mov(_al, _ch);
	out(0x0071, _al);
	out(0x0070, VCMOS_RTC_YEAR);
	mov(_al, _cl);
	out(0x0071, _al);
	out(0x0070, VCMOS_RTC_MONTH);
	mov(_al, _dh);
	out(0x0071, _al);
	out(0x0070, VCMOS_RTC_DAY_MONTH);
	mov(_al, _dl);
	out(0x0071, _al);
	pop_ax;
	clc;
}
static void INT_1A_06_CMOS_SetAlarmClock()
{
	/* return a fail to cpu */
	stc;
}

/* rtc update */
void INT_08()
{
	if (cmp(vmachine.flaginit,0x00)) return;
	push_ax;
	mov(_ax, 0x0001);
	add(mwp(QDBIOS_ADDR_RTC_DAILY_COUNTER+0),_ax);
	if (cmp(mwp(QDBIOS_ADDR_RTC_DAILY_COUNTER+0),0x0000))
		add(mwp(QDBIOS_ADDR_RTC_DAILY_COUNTER+2),_ax);
	if (cmp(mwp(QDBIOS_ADDR_RTC_DAILY_COUNTER+0),0x00b2)) {
		if (cmp(mwp(QDBIOS_ADDR_RTC_DAILY_COUNTER+2),0x0018)) {
			mov(mwp(QDBIOS_ADDR_RTC_DAILY_COUNTER+0), 0x0000);
			mov(mwp(QDBIOS_ADDR_RTC_DAILY_COUNTER+2), 0x0000);
			mov(mbp(QDBIOS_ADDR_RTC_ROLLOVER),0x00);
		}
	}
	pop_ax;
	_int(0x1c);
	push_ax;
	mov(_al, 0x20);
	out(0x20, _al); /* send eoi command to vpic */
	pop_ax;
/*	vramVarDWord(0x0000, QDBIOS_ADDR_RTC_DAILY_COUNTER) += 1;
	if (vramVarDWord(0x0000, QDBIOS_ADDR_RTC_DAILY_COUNTER) >= 0x1800b2) {
		vramVarDWord(0x0000, QDBIOS_ADDR_RTC_DAILY_COUNTER) = 0x00000000;
		vramVarByte(0x0000, QDBIOS_ADDR_RTC_ROLLOVER) = 0x01;
	}*/
}
/* fdd respond to int */
void INT_0E()
{
	push_ax;
	push_dx;
	mov(_dx, 0x03f5);
	mov(_al, 0x08);
 	out(_dx, _al); /* send senseint command */ 
	in (_al, _dx); /* read senseint ret st0: 0x20 */
	in (_al, _dx); /* read senseint ret cyl: 0x00 */
	/* FDC 3F5 4A to check id; not needed for vm */
	mov(_al, 0x20);
	out(0x20, _al); /* send eoi command to vpic */
	pop_dx;
	pop_ax;
}
/* vga */
void INT_10()
{
	t_nubit16 tmpCX = _cx;
	switch (_ah) {
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
		_cx = 0x01;
		qdvgaDisplayCharProp();
		_cx = tmpCX;
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
		if (_bl == 0x10)
			qdvgaGetAdapterInfo();
		break;
	case 0x13:
		qdvgaDisplayStr();
		break;
	case 0x1a:
		if (_al == 0x00) {
			_al = 0x1a;
			_bh = 0x00;
			_bl = 0x08;
		}
		break;
	default:
		break;
	}
}
/* device test*/
void INT_11() {mov(_ax, 0x0061);}
/* memory test*/
void INT_12() {mov(_ax, 0x027f);}
/* fdd */
void INT_13()
{
	switch (_ah) {
	case 0x00: INT_13_00_FDD_ResetDrive();   break;
	case 0x01: INT_13_01_FDD_GetDiskStatus();break;
	case 0x02: INT_13_02_FDD_ReadSector();   break;
	case 0x03: INT_13_03_FDD_WriteSector();  break;
	case 0x08: INT_13_08_FDD_GetParameter(); break;
	case 0x15: INT_13_15_FDD_GetDriveType(); break;
	default:   break;
	}
}
/* com ports */
void INT_14()
{/* do nothing */}
/* bios: device detect */
void INT_15()
{
	int MemorySize = 1;
	switch (_ah)
	{
	case 0xc0:
		_es = 0xf000;
		_bx = 0xe6f5;
		_ah = 0x00;
		// remember: all flags set in INT handler should be modified
		ClrCF;
		/*ClrBit(vramVarWord(_ss, _sp + 4), VCPU_FLAG_CF);*/
		break;
	case 0x24:
		if (_al == 0x03) {
			ClrCF;
			/*ClrBit(vramVarWord(_ss, _sp + 4), VCPU_FLAG_CF);*/
			_ah=0x00;
			_bx=0x0003;
		}
		break;
	case 0x88:
		ClrCF;
		/*ClrBit(vramVarWord(_ss, _sp + 4), VCPU_FLAG_CF);*/
		if (MemorySize > 16)		
			_ax = 0x3c00;					
		else		
			_ax = MemorySize * 1024 - 256;
		break;
	case 0xd8:
		SetCF;
		/* SetBit(vramVarWord(_ss, _sp + 4), VCPU_FLAG_CF);*/
		_ah=0x86;
		break;
	}
}
/* keyb */
void INT_16()
{
	switch (_ah) {
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
	switch(_ah) {
	case 0x00: INT_1A_00_BIOS_GetTimeTickCount(); break;
	case 0x01: INT_1A_01_BIOS_SetTimeTickCount(); break;
	case 0x02: INT_1A_02_CMOS_GetCmosTime();      break;
	case 0x03: INT_1A_03_CMOS_SetCmosTime();      break;
	case 0x04: INT_1A_04_CMOS_GetCmosDate();      break;
	case 0x05: INT_1A_05_CMOS_SetCmosDate();      break;
	case 0x06: INT_1A_06_CMOS_SetAlarmClock();    break;
	default:                                      break;
	}
}

t_bool qdbiosExecInt(t_nubit8 intid)
{
	switch (intid) {
	case 0x08: /* HARDWARE rtc update */
		INT_08();return 0x01;
	case 0x0e: /* HARDWARE fdd sense int */
		INT_0E();return 0x01;
	case 0x10: /* SOFTWARE vga operate */
		INT_10();return 0x01;
	case 0x13: /* SOFTWARE fdd operate */
		INT_13();return 0x01;
	case 0x15: /* SOFTWARE bios operate */
		INT_15();return 0x01;
	case 0x16: /* SOFTWARE keyb opterate */
		INT_16();return 0x01;
	case 0x1a: /* SOFTWARE rtc operate */
		INT_1A();return 0x01;
	case 0x11: /* SOFTWARE bios operate */
		INT_11();return 0x01;
	case 0x12: /* SOFTWARE bios operate */
		INT_12();return 0x01;
	case 0x14: /* SOFTWARE bios operate */
		INT_14();return 0x01;
	case 0x17: /* SOFTWARE bios operate */
		INT_17();return 0x01;
	default:     return 0x00;
	}
}

void qdbiosRefresh() {}
void qdbiosInit()
{
	t_nubit8 hour, min, sec;
	t_nubit16 i;
/* build interrupt vector table */
	for (i = 0x0000;i < 0x0100;++i) {
		vramVarByte(0x0000, i*4 + 0x00) = 0x00;
		vramVarByte(0x0000, i*4 + 0x01) = 0x00;
		vramVarByte(0x0000, i*4 + 0x02) = 0x00;
		vramVarByte(0x0000, i*4 + 0x03) = 0xf0;
	}
/* build general int service routine */
	vramVarByte(0xf000, 0x0000) = 0xcf;
/* special: INT 09, send eoi command to vpic */
	vramVarByte(0xf000, 0x0001) = 0xb0;	/* mov al,20 */
	vramVarByte(0xf000, 0x0002) = 0x20;
	vramVarByte(0xf000, 0x0003) = 0xe6; /* out 20,al */
	vramVarByte(0xf000, 0x0004) = 0x20;
	vramVarByte(0xf000, 0x0005) = 0xcf; /* iret */
	vramVarByte(0x0000, 0x24 + 0x00) = 0x01;
	vramVarByte(0x0000, 0x24 + 0x01) = 0x00;
	vramVarByte(0x0000, 0x24 + 0x02) = 0x00;
	vramVarByte(0x0000, 0x24 + 0x03) = 0xf0;
/* special: INT 16, call warpper "out bb,al" */
	vramVarByte(0xf000, 0x0006) = 0xe6; /* out bb,al */
	vramVarByte(0xf000, 0x0007) = 0xbb;
	vramVarByte(0xf000, 0x0008) = 0xcf; /* iret */
	vramVarByte(0x0000, 0x58 + 0x00) = 0x06;
	vramVarByte(0x0000, 0x58 + 0x01) = 0x00;
	vramVarByte(0x0000, 0x58 + 0x02) = 0x00;
	vramVarByte(0x0000, 0x58 + 0x03) = 0xf0;
/* bios data area */
	memset((void *)vramGetAddr(0x0040,0x0000), 0x00, 0x100);
	vramVarByte(0x0040, 0x0000) = 0xf8;
	vramVarByte(0x0040, 0x0001) = 0x03;
	vramVarByte(0x0040, 0x0008) = 0x78;
	vramVarByte(0x0040, 0x0008) = 0x03;
	vramVarByte(0x0040, 0x000e) = 0xc0;
	vramVarByte(0x0040, 0x000f) = 0x9f;
	vramVarByte(0x0040, 0x0010) = 0x61;
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
/*
	RTC Tick Count Storage
	vramVarByte(0x0040, 0x006c) = 0xf7;
	vramVarByte(0x0040, 0x006d) = 0xd6;
	vramVarByte(0x0040, 0x006e) = 0x0f;
*/
	vramVarByte(0x0040, 0x0074) = 0x01;
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

/* device initialize */
	qdkeybInit();
	qdvgaInit();

/* load real time */
	hour = BCD2Hex(vcmos.reg[VCMOS_RTC_HOUR]);
	min  = BCD2Hex(vcmos.reg[VCMOS_RTC_MINUTE]);
	sec  = BCD2Hex(vcmos.reg[VCMOS_RTC_SECOND]);
	vramVarDWord(0x0000, QDBIOS_ADDR_RTC_DAILY_COUNTER) = 
		(t_nubit32)(((hour * 3600 + min * 60 + sec) * 1000) / QDBIOS_RTC_TICK);
	vramVarByte(0x0000, QDBIOS_ADDR_RTC_ROLLOVER) = 0x00;

/* load boot sector */
	vapiFloppyInsert("d:/msdos.img");
	memcpy((void *)vramGetAddr(0x0000,0x7c00), (void *)vfdd.base, 0x200);
	vramVarByte(0xf000, 0xfff0) = 0xea;
	vramVarWord(0xf000, 0xfff1) = 0x7c00;
	vramVarWord(0xf000, 0xfff3) = 0x0000;
}
void qdbiosFinal()
{
	qdvgaFinal();
	qdkeybFinal();
}
