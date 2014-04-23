/* This file is a part of NXVM project. */

#include "memory.h"

#include "vapi.h"
#include "vport.h"
#include "vpic.h"

t_pic vpic1,vpic2;

/* Get flags from ICWs */
#define GetLTIM(vpic) (!!((vpic)->icw1 & 0x08))
#define GetSNGL(vpic) (!!((vpic)->icw1 & 0x02))
#define GetIC4(vpic)  (!!((vpic)->icw1 & 0x01))
#define GetSFNM(vpic) (!!((vpic)->icw4 & 0x10))
#define GetBUF(vpic)  (!!((vpic)->icw4 & 0x08))
#define GetMS(vpic)   (!!((vpic)->icw4 & 0x04))
#define GetAEOI(vpic) (!!((vpic)->icw4 & 0x02))
#define GetuPM(vpic)  (!!((vpic)->icw4 & 0x01))
#define GetR(vpic)    (!!((vpic)->ocw2 & 0x80))
#define GetSL(vpic)   (!!((vpic)->ocw2 & 0x40))
#define GetEOI(vpic)  (!!((vpic)->ocw2 & 0x20))
#define GetESMM(vpic) (!!((vpic)->ocw3 & 0x40))
#define GetSMM(vpic)  (!!((vpic)->ocw3 & 0x20))
#define GetP(vpic)    (!!((vpic)->ocw3 & 0x04))
#define GetRR(vpic)   (!!((vpic)->ocw3 & 0x02))
#define GetRIS(vpic)  (!!((vpic)->ocw3 & 0x01))

/* Get id of highest priority interrupts in different registers */
#define GetIntrTopId(vpic) (GetRegTopId((vpic), ((vpic)->irr & (~(vpic)->imr))))
#define GetIsrTopId(vpic)  (GetRegTopId((vpic), (vpic)->isr))
#define GetIrrTopId(vpic)  (GetRegTopId((vpic), (vpic)->irr))
#define GetImrTopId(vpic)  (GetRegTopId((vpic), (vpic)->imr))

/*
 * GetRegTopId: Internal function
 * Returns id of highest priority interrupt
 * Returns 0x08 if reg is null
 */
static t_nubit8 GetRegTopId(t_pic *vpic, t_nubit8 reg)
{
	t_nubit8 id = 0x00;
	if (reg == 0x00) return 0x08;
	reg = (reg<<(0x08 - (vpic->irx))) | (reg>>(vpic->irx));
	while (!((reg>>id) & 0x01) && (id < 0x08)) id++;
	return (id + vpic->irx) % 0x08;
}
/*
 * GetRegTopId: Internal function
 * Returns flag of higher priority interrupt
 */
static t_bool HasINTR(t_pic *vpic)
{
	t_nubit8 irid;                     /* top requested int id in master pic */
	t_nubit8 isid;                    /* top in service int id in master pic */
	irid = GetIntrTopId(vpic);
	isid = GetIsrTopId(vpic);
	if (irid == 0x08) return 0x00;
	if (isid == 0x08) return 0x01;
	if (irid < vpic->irx) irid += 0x08;
	if (isid < vpic->irx) isid += 0x08;
	if (GetSFNM(vpic)) return irid <= isid;
	else return irid < isid;
}
/*
 * RespondINTR: Internal function
 * Adds INTR to IRR and removes it from ISR
 */
static void RespondINTR(t_pic *vpic, t_nubit8 id)
{
	vpic->isr |= 1<<id;                                  /* put int into ISR */
	vpic->irr &= ~(1<<id);                           /* remove int from  IRR */
	if (GetAEOI(vpic)) {                                    /* Auto EOI Mode */
		vpic->isr &= ~(1<<id);
		if (GetR(vpic)) vpic->irx = (id + 0x01) % 0x08;       /* Rotate Mode */
	}
}

/*
 * IO_Read_00x0
 * PIC provide POLL, IRR, ISR based on OCW3
 * Reference: 16-32.PDF, Page 192
 * Reference: PC.PDF, Page 950
 */
static void IO_Read_00x0(t_pic *vpic)
{
	if (GetP(vpic)) {                                  /* P=1 (Poll Command) */
		vport.iobyte = 0x80 | GetIntrTopId(vpic);
		if (vport.iobyte == 0x88) vport.iobyte = 0x00;
	} else {
		switch (vpic->ocw3&0x03) {
		case 0x02:                                  /* RR=1, RIS=0, Read IRR */
			vport.iobyte = vpic->irr;break;
		case 0x03:                                  /* RR=1, RIS=1, Read ISR */
			vport.iobyte = vpic->isr;break;
		default:                                       /* RR=0, No Operation */
			break;
		}
	}
}
/*
 * IO_Write_00x0
 * PIC get ICW1, OCW2, OCW3
 * Reference: 16-32.PDF, Page 184
 * Reference: PC.PDF, Page 950
 */
static void IO_Write_00x0(t_pic *vpic)
{
	t_nubitcc id;
	if (vport.iobyte & 0x10) {                                     /* ICW1 (D4=1) */
		vpic->icw1 = vport.iobyte;
		vpic->flaginit = ICW2;
		if (GetIC4(vpic)) ;                                   /* D0=1, IC4=1 */
		else vpic->icw4 = 0x00;                               /* D0=0, IC4=0 */
		if (GetSNGL(vpic)) ;                         /* D1=1, SNGL=1, ICW3=0 */
		else ;                                       /* D1=0, SNGL=0, ICW3=1 */
		if (GetLTIM(vpic)) ;           /* D3=1, LTIM=1, Level Triggered Mode */
		else ;                         /* D3=0, LTIM=0, Edge  Triggered Mode */
	} else {                                                  /* OCWs (D4=0) */
		if (vport.iobyte & 0x08) {                                 /* OCW3 (D3=1) */
			if (vport.iobyte & 0x40) {        /* ESMM=1: Enable Special Mask Mode */
				vpic->ocw3 = vport.iobyte;
				if (GetSMM(vpic)) ;          /* SMM=1: Set Special Mask Mode */
				else ;                     /* SMM=0: Clear Sepcial Mask Mode */
			} else                                       /* ESMM=0: Keep SMM */
				vpic->ocw3 = (vpic->ocw3 & 0x20) | (vport.iobyte & 0xdf);
		} else {                                              /* OCW2 (D3=0) */
			switch (vport.iobyte & 0xe0) {
			                        /* D7=R, D6=SL, D5=EOI(End Of Interrupt) */
			case 0x80:      /* 100: Set (Rotate Priorities in Auto EOI Mode) */
				if (GetAEOI(vpic)) vpic->ocw2 = vport.iobyte;
				break;
			case 0x00:    /* 000: Clear (Rotate Priorities in Auto EOI Mode) */
				if (GetAEOI(vpic)) vpic->ocw2 = vport.iobyte;
				/* Bug in easyVM (0x00 ?= 0x20) */
				break;
			case 0x20:                      /* 001: Non-specific EOI Command */
				/* Set bit of highest priority interrupt in ISR to 0,
				 IR0 > IR1 > IR2(IR8 > ... > IR15) > IR3 > ... > IR7 */
				vpic->ocw2 = vport.iobyte;
				if (vpic->isr) {
					id = GetIsrTopId(vpic);
					//vapiPrint("EOI %d\n", id);
					vpic->isr &= ~(1<<id);
				}
				break;
			case 0x60:                          /* 011: Specific EOI Command */
				vpic->ocw2 = vport.iobyte;
				if (vpic->isr) {
					id = vpic->ocw2 & 0x07;                  /* Get L2,L1,L0 */
					vpic->isr &= ~(1<<id);
				}
				/* Bug in easyVM: "isr &= (1<<i)" */
				break;
			case 0xa0:         /* 101: Rotate Priorities on Non-specific EOI */
				vpic->ocw2 = vport.iobyte;
				if (vpic->isr) {
					id = GetIsrTopId(vpic);
					vpic->isr &= ~(1<<id);
					vpic->irx = (id + 0x01) % 0x08;
				}
				break;
			case 0xe0:       /* 111: Rotate Priority on Specific EOI Command */
				vpic->ocw2 = vport.iobyte;
				if (vpic->isr) {
					id = GetIsrTopId(vpic);
					vpic->isr &= ~(1<<id);
					vpic->irx = ((vpic->ocw2 & 0x07) + 0x01) % 0x08;
				}
				break;
			case 0xc0: /* 110: Set Priority (does not reset current ISR bit) */
				vpic->ocw2 = vport.iobyte;
				vpic->irx = ((vpic->ocw2 & 0x07) + 0x01) % 0x08;
				break;
			case 0x40:                                  /* 010: No Operation */
				break;
			default:break;}
		}
	}
}
/*
 * IO_Read_00x1
 * PIC provide IMR
 * Reference: 16-32.PDF, Page 184
 */
static void IO_Read_00x1(t_pic *vpic)
{
	vport.iobyte = vpic->imr;
}
/*
 * IO_Write_00x1
 * PIC get ICW2, ICW3, ICW4, OCW1 after ICW1
 */
static void IO_Write_00x1(t_pic *vpic)
{
	switch (vpic->flaginit) {
	case ICW2:
		vpic->icw2 = vport.iobyte & (~0x07);
		if (!((vpic->icw1) & 0x02))             /* ICW1.D1=0, SNGL=0, ICW3=1 */
			vpic->flaginit = ICW3;
		else if (vpic->icw1 & 0x01)                      /* ICW1.D0=1, IC4=1 */
			vpic->flaginit = ICW4;
		else
			vpic->flaginit = OCW1;
		break;
	case ICW3:
		vpic->icw3 = vport.iobyte;
		if ((vpic->icw1) & 0x01) vpic->flaginit = ICW4;      /* ICW1.D0=1, IC4=1 */
		else vpic->flaginit = OCW1;
		break;
	case ICW4:
		vpic->icw4 = vport.iobyte & 0x1f;
		if ((vpic->icw4) & 0x01) ;                     /*uPM=1, 16-bit 80x86 */
		else ;                                     /* uPM=0, 8-bit 8080/8085 */
		if ((vpic->icw4) & 0x02) ;     /* AEOI=1, Automatic End of Interrupt */
		else ;                     /* AEOI=0, Non-automatic End of Interrupt */
		if ((vpic->icw4) & 0x08) {                          /* BUF=1, Buffer */
			if ((vpic->icw4) & 0x04) ;                /* M/S=1, Master 8259A */
			else ;                                     /* M/S=0, Slave 8259A */
		} else ;                                        /* BUF=0, Non-buffer */
		if ((vpic->icw4) & 0x10) ;      /* SFNM=1, Special Fully Nested Mode */
		else ;                      /* SFNM=0, Non-special Fully Nested Mode */
		vpic->flaginit = OCW1;
		break;
	case OCW1:
		vpic->ocw1 = vport.iobyte;
		if (GetSMM(vpic)) vpic->isr &= ~(vpic->imr);
		break;
	default:
		break;
	}
}

void IO_Read_0020() {IO_Read_00x0(&vpic1);}
void IO_Read_0021() {IO_Read_00x1(&vpic1);}
void IO_Read_00A0() {IO_Read_00x0(&vpic2);}
void IO_Read_00A1() {IO_Read_00x1(&vpic2);}
void IO_Write_0020() {IO_Write_00x0(&vpic1);}
void IO_Write_0021() {IO_Write_00x1(&vpic1);}
void IO_Write_00A0() {IO_Write_00x0(&vpic2);}
void IO_Write_00A1() {IO_Write_00x1(&vpic2);}

/* Test I/O Ports */
#ifdef VPIC_DEBUG
void IO_Read_FF20()
{
	vport.iobyte = 0xff;

	vapiPrint("INFO PIC 1\n==========\n");
	vapiPrint("Init Status = %d, IRX = %x\n",
		vpic1.flaginit, vpic1.irx);
	vapiPrint("IRR = %x, ISR = %x, IMR = %x, intr = %x\n",
		vpic1.irr, vpic1.isr, vpic1.imr, vpic1.irr & (~vpic1.imr));
	vapiPrint("ICW1 = %x, LTIM = %d, SNGL = %d, IC4 = %d\n",
		vpic1.icw1, GetLTIM(&vpic1), GetSNGL(&vpic1), GetIC4(&vpic1));
	vapiPrint("ICW2 = %x\n",vpic1.icw2);
	vapiPrint("ICW3 = %x\n", vpic1.icw3);
	vapiPrint("ICW4 = %x, SFNM = %d, BUF = %d, M/S = %d, AEOI = %d, uPM = %d\n",
		vpic1.icw4, GetSFNM(&vpic1), GetBUF(&vpic1), GetMS(&vpic1),
		GetAEOI(&vpic1), GetuPM(&vpic1));
	vapiPrint("OCW1 = %x\n", vpic1.ocw1);
	vapiPrint("OCW2 = %x, R = %d, SL = %d, EOI = %d, L = %d\n",
		vpic1.ocw2, GetR(&vpic1), GetSL(&vpic1), GetEOI(&vpic1),
		vpic1.ocw2 & 0x07);
	vapiPrint("OCW3 = %x, ESMM = %d, SMM = %d, P = %d, RR = %d, RIS = %d\n",
		vpic1.ocw3, GetESMM(&vpic1), GetSMM(&vpic1),
		GetP(&vpic1), GetRR(&vpic1), GetRIS(&vpic1));

	vapiPrint("INFO PIC 2\n==========\n");
	vapiPrint("Init Status = %d, IRX = %x\n",
		vpic2.flaginit, vpic2.irx);
	vapiPrint("IRR = %x, ISR = %x, IMR = %x, intr = %x\n",
		vpic2.irr, vpic2.isr, vpic2.imr, vpic2.irr & (~vpic2.imr));
	vapiPrint("ICW1 = %x, LTIM = %d, SNGL = %d, IC4 = %d\n",
		vpic2.icw1, GetLTIM(&vpic2), GetSNGL(&vpic2), GetIC4(&vpic2));
	vapiPrint("ICW2 = %x\n",vpic2.icw2);
	vapiPrint("ICW3 = %x\n", vpic2.icw3);
	vapiPrint("ICW4 = %x, SFNM = %d, BUF = %d, M/S = %d, AEOI = %d, uPM = %d\n",
		vpic2.icw4, GetSFNM(&vpic2), GetBUF(&vpic2), GetMS(&vpic2),
		GetAEOI(&vpic2), GetuPM(&vpic2));
	vapiPrint("OCW1 = %x\n", vpic2.ocw1);
	vapiPrint("OCW2 = %x, R = %d, SL = %d, EOI = %d, L = %d\n",
		vpic2.ocw2, GetR(&vpic2), GetSL(&vpic2), GetEOI(&vpic2),
		vpic2.ocw2 & 0x07);
	vapiPrint("OCW3 = %x, ESMM = %d, SMM = %d, P = %d, RR = %d, RIS = %d\n",
		vpic2.ocw3, GetESMM(&vpic2), GetSMM(&vpic2),
		GetP(&vpic2), GetRR(&vpic2), GetRIS(&vpic2));
}
void IO_Read_FF21() {vport.iobyte = (t_nubit8)vpicScanINTR();}
void IO_Read_FF22() {vport.iobyte = vpicGetINTR();}
void IO_Write_FF20() {vpicSetIRQ(vport.iobyte);}
#endif

void vpicSetIRQ(t_nubit8 irqid)
{
	switch(irqid) {
	case 0x00:	vpic1.irr |= 0x01;break;
	case 0x01:	vpic1.irr |= 0x02;break;
/*	case 0x02:	vpic1.irr |= 0x04;break; */
	case 0x03:	vpic1.irr |= 0x08;break;
	case 0x04:	vpic1.irr |= 0x10;break;
	case 0x05:	vpic1.irr |= 0x20;break;
	case 0x06:	vpic1.irr |= 0x40;break;
	case 0x07:	vpic1.irr |= 0x80;break;
	case 0x08:	vpic2.irr |= 0x01;vpic1.irr |= 0x04;break;
	case 0x09:	vpic2.irr |= 0x02;vpic1.irr |= 0x04;break;
	case 0x0a:	vpic2.irr |= 0x04;vpic1.irr |= 0x04;break;
	case 0x0b:	vpic2.irr |= 0x08;vpic1.irr |= 0x04;break;
	case 0x0c:	vpic2.irr |= 0x10;vpic1.irr |= 0x04;break;
	case 0x0d:	vpic2.irr |= 0x20;vpic1.irr |= 0x04;break;
	case 0x0e:	vpic2.irr |= 0x40;vpic1.irr |= 0x04;break;
	case 0x0f:	vpic2.irr |= 0x80;vpic1.irr |= 0x04;break;
	default:break;}
}
t_bool   vpicScanINTR()
{
	t_bool intr1;
	intr1 = HasINTR(&vpic1);
	if (intr1 && (GetIntrTopId(&vpic1) == 0x02) )
			return HasINTR(&vpic2);
	return intr1;
}
t_nubit8 vpicPeekINTR()
{
	t_nubit8 irid1;                    /* top requested int id in master pic */
	t_nubit8 irid2;                     /* top requested int id in slave pic */
	irid1 = GetIntrTopId(&vpic1);
	if (irid1 == 0x02) {      /* if IR2 has int request, then test slave pic */
		irid2 = GetIntrTopId(&vpic2);
		                        /* find the final int id based on slave ICW2 */
		return (irid2 | vpic2.icw2);
	} else                     /* find the final int id based on master ICW2 */
		return (irid1 | vpic1.icw2);
}
t_nubit8 vpicGetINTR()
{
	t_nubit8 irid1;                    /* top requested int id in master pic */
	t_nubit8 irid2;                     /* top requested int id in slave pic */
	irid1 = GetIntrTopId(&vpic1);
	RespondINTR(&vpic1, irid1);
	if (irid1 == 0x02) {      /* if IR2 has int request, then test slave pic */
		irid2 = GetIntrTopId(&vpic2);
		RespondINTR(&vpic2, irid2);
		                        /* find the final int id based on slave ICW2 */
		return (irid2 | vpic2.icw2);
	} else                     /* find the final int id based on master ICW2 */
		return (irid1 | vpic1.icw2);
}
void vpicInit()
{
	vport.in[0x0020] = (t_faddrcc)IO_Read_0020;
	vport.in[0x0021] = (t_faddrcc)IO_Read_0021;
	vport.in[0x00a0] = (t_faddrcc)IO_Read_00A0;
	vport.in[0x00a1] = (t_faddrcc)IO_Read_00A1;
	vport.out[0x0020] = (t_faddrcc)IO_Write_0020;
	vport.out[0x0021] = (t_faddrcc)IO_Write_0021;
	vport.out[0x00a0] = (t_faddrcc)IO_Write_00A0;
	vport.out[0x00a1] = (t_faddrcc)IO_Write_00A1;
#ifdef VPIC_DEBUG
	vport.in[0xff20] = (t_faddrcc)IO_Read_FF20;
	vport.in[0xff21] = (t_faddrcc)IO_Read_FF21;
	vport.in[0xff22] = (t_faddrcc)IO_Read_FF22;
	vport.out[0xff20] = (t_faddrcc)IO_Write_FF20;
#endif
}
void vpicReset()
{
	memset(&vpic1, 0x00, sizeof(t_pic));
	memset(&vpic2, 0x00, sizeof(t_pic));
	vpic1.flaginit = vpic2.flaginit = ICW1;
	vpic1.ocw3 = vpic2.ocw3 = 0x02;
}
void vpicRefresh()
{
	if (vpic2.irr & (~vpic2.imr))      /* see if slave pic has requested int */
		vpic1.irr |= 0x04;        /* pass the request into IR2 of master pic */
	else
		vpic1.irr &= 0xfb;                     /* remove IR2 from master pic */
}
void vpicFinal() {}

/*
Test Case for regular IBM PC use
Initialize (ICW1, ICW2, ICW3, ICW4 50%)
o20 11
o21 08
o21 04
o21 11
oa0 11
oa1 70
oa1 02
oa1 01
	PrintInfo
iff20
	Mask IRQ 5 and IRQ c by OCW1
o21 20
oa1 10
	SetIRQs
off20 1
off20 5
off20 a
off20 c
off20 d
	ScanINTR
		iff21
		result should be 01
	GetINTR
		iff22
		result should be 09
	EOI 0x20, PrintInfo, look at IRR, ISR(0), IMR
		o20 20
	ScanINTR
		iff21
		result should be 01
	GetINTR
		iff22
		result should be 72 now ISR1 is 4, ISR2 is 4
		SetIRQ
			off20 0
		ScanINTR
			iff21
			result should be 01
		GetINTR
			iff22
			result should be 08
		EOI
			o20 20, now ISR1 should be 4, ISR2 should be 4
		SetIRQ
			off20 8
		ScanINTR
			iff21
			result should be 01
		GetINTR
			iff22
			result should be 01, ISR2 should be 5
		EOI
			oa0 20, now ISR1 should be 4, ISR2 should be 4
		SetIRQ
			off20 4
		ScanINTR
			iff21
			result should be 00
	EOI
		oa0 20
		o20 20
	ScanINTR, PrintInfo, look at IRR, ISR, IMR
	GetINTR, PrintInfo, look at IRR, ISR, IMR
	EOI 0x20, PrintInfo, look at IRR, ISR, IMR (think about pic2)

	Test case for port commands
	Initialize
o20 11
o21 08
o21 04
o21 11
oa0 11
oa1 70
oa1 02
oa1 01
	SetIRQs
off20 1
off20 5
off20 a
off20 c
off20 d
	Test ESMM (OCW3 50%)
		o20 29	ocw3 = 0010 1001, see if D5 changes
		o20 4a	ocw3 = 0100 1010, see if D5 changes
		o20 6c	ocw3 = 0110 1100, see if D5 changes
		o20 49	ocw3 = 0100 1001, see if D5 changes
	Test SMM (OCW1)
		o20 49, disable SMM
		iff21
		iff22, get ISR 1
		o21 33
		iff20, print info
		o20 6c, enable SMM
		o21 33
		iff20, print info
	Test P/RR/RIS (OCW3 50%)
		o20 4c	ocw3 = 0100 1100, enable poll
		i20		see poll
		o20 4a	ocw3 = 0100 1010, disable poll, enable IRR
		i20		see irr
		o20 4b	ocw3 = 0100 1011, disable poll, enable ISR
		i20		see isr
	Test AEOI (ICW4 50%)
	not tested yet
	Test OCW2
	not tested yet
*/
