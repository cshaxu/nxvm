/* This file is a part of NXVM project. */

#include "stdio.h"

#include "../vmachine.h"

#include "qdbios.h"
#include "qddisk.h"

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
#define in(a,b)  (ExecFun(vport.in[(b)]), (a) = vport.iobyte)
#define out(a,b) (vport.iobyte = (b), ExecFun(vport.out[(a)]))
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

#define QDBIOS_VAR_VCMOS 0
#define QDBIOS_VAR_QDRTC 1
#define QDBIOS_RTC QDBIOS_VAR_VCMOS

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
//	mov(_al, 0x20);
//	out(0x20, _al); /* send eoi command to vpic */
	pop_ax;
/*	vramVarDWord(0x0000, QDBIOS_ADDR_RTC_DAILY_COUNTER) += 1;
	if (vramVarDWord(0x0000, QDBIOS_ADDR_RTC_DAILY_COUNTER) >= 0x1800b2) {
		vramVarDWord(0x0000, QDBIOS_ADDR_RTC_DAILY_COUNTER) = 0x00000000;
		vramVarByte(0x0000, QDBIOS_ADDR_RTC_ROLLOVER) = 0x01;
	}*/
}

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

void qdrtcReset()
{
	t_nubit8 hour,min,sec;
	asmint(0x08) {
		qdbiosMakeInt(
			0x08, "               \
			cli                 \n\
			push ds             \n\
			push ax             \n\
			pushf               \n\
			mov ax, 40          \n\
			mov ds, ax          \n\
			add word [006c], 1  \n\
			adc word [006e], 0  \n\
			cmp word [006c], b2 \n\
			jnz $(label1)       \n\
			cmp word [006e], 18 \n\
			jnz $(label1)       \n\
			mov word [006c], 0  \n\
			mov word [006e], 0  \n\
			mov word [0070], 1  \n\
			$(label1):          \n\
			popf                \n\
			pop ax              \n\
			pop ds              \n\
			int 1c              \n\
			push ax             \n\
			mov al, 20          \n\
			out 20, al          \n\
			pop ax              \n\
			sti                 \n\
			iret                \n");
	} else {
		qdbiosInt[0x08] = (t_faddrcc)INT_08; /* hard rtc */
		qdbiosMakeInt(0x08, "qdx 08\niret");
	}
	qdbiosInt[0x1a] = (t_faddrcc)INT_1A; /* soft rtc */
	qdbiosMakeInt(0x1a, "qdx 1a\niret");
	/* load cmos data */
	hour = BCD2Hex(vcmos.reg[VCMOS_RTC_HOUR]);
	min  = BCD2Hex(vcmos.reg[VCMOS_RTC_MINUTE]);
	sec  = BCD2Hex(vcmos.reg[VCMOS_RTC_SECOND]);
	vramVarDWord(0x0000, QDBIOS_ADDR_RTC_DAILY_COUNTER) = 
		(t_nubit32)(((hour * 3600 + min * 60 + sec) * 1000) / QDBIOS_RTC_TICK);
	vramVarByte(0x0000, QDBIOS_ADDR_RTC_ROLLOVER) = 0x00;
}
