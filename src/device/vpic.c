/* Copyright 2012-2014 Neko. */

/*
 * VPIC implements programmable interrupt controller with
 * two Intel 8259A chips, one master and one slave.
 */

#include "../utils.h"

#include "vbios.h"
#include "vport.h"
#include "vmachine.h"
#include "vpic.h"

t_pic vpic1, vpic2;

/*
 * GetRegTopId: Internal function
 * Returns id of highest priority interrupt
 * Returns 0x08 if reg is null
 */
static t_nubit8 GetRegTopId(t_pic *rpic, t_nubit8 reg) {
	t_nubit8 id = 0;
	if (reg == Zero8) {
		return 0x08;
	}
	reg = (reg << (8 - (rpic->irx))) | (reg>> (rpic->irx));
	while ((id < VPIC_MAX_IRQ_COUNT) && !GetMax1(reg >> id)) {
		id++;
	}
	return (id + rpic->irx) % VPIC_MAX_IRQ_COUNT;
}

/*
 * GetRegTopId: Internal function
 * Returns flag of higher priority interrupt
 */
static t_bool HasINTR(t_pic *rpic) {
	t_nubit8 irid; /* top requested int id in master pic */
	t_nubit8 isid; /* top in service int id in master pic */
	irid = VPIC_GetIntrTopId(rpic);
	isid = VPIC_GetIsrTopId(rpic);
	if (irid == 0x08) {
		/* no interrupt to pick up */
		return False;
	}
	if (isid == 0x08) {
		/* no interrupt in service */
		return True;
	}
	/*
	 * if irid and isid are on the same side of top priority int
	 * request id, do regular comparison; otherwise order them.
	 * for example, if irid < irx, that means irid's priority is lower
	 * than irx, and we need to add VPIC_MAX_IRQ_COUNT to it to ensure
	 * that it's priority is lower than irx in following comparison 
	 * (irid < isid) or (irid <= isid).
	 */
	if (irid < rpic->irx) {
		irid += VPIC_MAX_IRQ_COUNT;
	}
	if (isid < rpic->irx) {
		isid += VPIC_MAX_IRQ_COUNT;
	}
	if (GetBit(rpic->icw4, VPIC_ICW4_SFNM)) {
		return irid <= isid;
	} else {
		return irid < isid;
	}
}

/*
 * RespondINTR: Internal function
 * Adds INTR to IRR and removes it from ISR
 */
static void RespondINTR(t_pic *rpic, t_nubit8 id) {
	SetBit(rpic->isr, VPIC_ISR_IRQ(id)); /* put int into ISR */
	ClrBit(rpic->irr, VPIC_IRR_IRQ(id)); /* remove int from  IRR */
	if (GetBit(rpic->icw4, VPIC_ICW4_AEOI)) {
		/* Auto EOI Mode */
		ClrBit(rpic->isr, VPIC_ISR_IRQ(id));
		if (GetBit(rpic->ocw2, VPIC_OCW2_R)) {
			/* Rotate Mode */
			rpic->irx = (id + 1) % VPIC_MAX_IRQ_COUNT;
		}
	}
}

/*
 * io_read_00x0
 * PIC provide POLL, IRR, ISR based on OCW3
 * Reference: 16-32.PDF, Page 192
 * Reference: PC.PDF, Page 950
 */
static void io_read_00x0(t_pic *rpic) {
	if (GetBit(rpic->ocw3, VPIC_OCW3_P)) {
		/* P=1 (Poll Command) */
		if (VPIC_GetIntrTopId(rpic) == 0x08) {
			/* set all bits to 0 if there's no interrupt in queue */
			vport.iobyte = Zero8;
		} else {
			/* set highest bit to 1 if there's an interrupt in queue */
			vport.iobyte = VPIC_POLL_I | VPIC_GetIntrTopId(rpic);
		}
	} else {
		switch (rpic->ocw3 & (VPIC_OCW3_RR | VPIC_OCW3_RIS)) {
		case 0x02:
			/* RR=1, RIS=0, Read IRR */
			vport.iobyte = rpic->irr;
			break;
		case 0x03:
			/* RR=1, RIS=1, Read ISR */
			vport.iobyte = rpic->isr;
			break;
		default:
			/* RR=0, No Operation */
			break;
		}
	}
}

/*
 * io_write_00x0
 * PIC get ICW1, OCW2, OCW3
 * Reference: 16-32.PDF, Page 184
 * Reference: PC.PDF, Page 950
 */
static void io_write_00x0(t_pic *rpic) {
	t_nubit8 id;
	if (GetBit(vport.iobyte, VPIC_ICW1_I)) {
		/* ICW1 (D4=1) */
		rpic->icw1 = vport.iobyte;
		rpic->status = ICW2;
		if (GetBit(rpic->icw1, VPIC_ICW1_IC4)) {
			/* D0=1, IC4=1 */
		} else {
			/* D0=0, IC4=0 */
			rpic->icw4 = Zero8;
		}
		if (GetBit(rpic->icw1, VPIC_ICW1_SNGL)) {
			/* D1=1, SNGL=1, ICW3=0 */
		} else {
			/* D1=0, SNGL=0, ICW3=1 */
		}
		if (GetBit(rpic->icw1, VPIC_ICW1_LTIM)) {
			/* D3=1, LTIM=1, Level Triggered Mode */
		} else {
			/* D3=0, LTIM=0, Edge  Triggered Mode */
		}
	} else {
		/* OCWs (D4=0) */
		if (GetBit(vport.iobyte, VPIC_OCW3_I)) {
			/* OCW3 (D3=1) */
			if (GetBit(vport.iobyte, VPIC_OCW3_ESMM)) {
				/* ESMM=1: Enable Special Mask Mode */
				rpic->ocw3 = vport.iobyte;
				if (GetBit(rpic->ocw3, VPIC_OCW3_SMM)) {
					/* SMM=1: Set Special Mask Mode */
				} else {
					/* SMM=0: Clear Sepcial Mask Mode */
				}
			} else {
				/* ESMM=0: Keep SMM */
				rpic->ocw3 = (rpic->ocw3 & VPIC_OCW3_SMM) | (vport.iobyte & ~VPIC_OCW3_SMM);
			}
		} else {
			/* OCW2 (D3=0) */
			switch (vport.iobyte & (VPIC_OCW2_EOI | VPIC_OCW2_SL | VPIC_OCW2_R)) {
				/* D7=R, D6=SL, D5=EOI(End Of Interrupt) */
			case 0x80:
				/* 100: Set (Rotate Priorities in Auto EOI Mode) */
				if (GetBit(rpic->icw4, VPIC_ICW4_AEOI)) {
					rpic->ocw2 = vport.iobyte;
				}
				break;
			case 0x00:
				/* 000: Clear (Rotate Priorities in Auto EOI Mode) */
				if (GetBit(rpic->icw4, VPIC_ICW4_AEOI)) {
					rpic->ocw2 = vport.iobyte;
				}
				/* Bug in easyVM (0x00 ?= 0x20) */
				break;
			case 0x20:
				/* 001: Non-specific EOI Command */
				/* Set bit of highest priority interrupt in ISR to 0,
				 IR0 > IR1 > IR2(IR8 > ... > IR15) > IR3 > ... > IR7 */
				rpic->ocw2 = vport.iobyte;
				if (rpic->isr) {
					id = VPIC_GetIsrTopId(rpic);
					ClrBit(rpic->isr, VPIC_ISR_IRQ(id));
				}
				break;
			case 0x60:
				/* 011: Specific EOI Command */
				rpic->ocw2 = vport.iobyte;
				if (rpic->isr) {
					/* Get L2,L1,L0 */
					id = rpic->ocw2 & VPIC_OCW2_L;
					ClrBit(rpic->isr, VPIC_ISR_IRQ(id));
				}
				/* Bug in easyVM: "isr &= (1 << i)" */
				break;
			case 0xa0:
				/* 101: Rotate Priorities on Non-specific EOI */
				rpic->ocw2 = vport.iobyte;
				if (rpic->isr) {
					id = VPIC_GetIsrTopId(rpic);
					ClrBit(rpic->isr, VPIC_ISR_IRQ(id));
					rpic->irx = (id + 1) % VPIC_MAX_IRQ_COUNT;
				}
				break;
			case 0xe0:
				/* 111: Rotate Priority on Specific EOI Command */
				rpic->ocw2 = vport.iobyte;
				if (rpic->isr) {
					id = VPIC_GetIsrTopId(rpic);
					ClrBit(rpic->isr, VPIC_ISR_IRQ(id));
					rpic->irx = ((rpic->ocw2 & VPIC_OCW2_L) + 1) % VPIC_MAX_IRQ_COUNT;
				}
				break;
			case 0xc0:
				/* 110: Set Priority (does not reset current ISR bit) */
				rpic->ocw2 = vport.iobyte;
				rpic->irx = (VPIC_GetOCW2_L(rpic->ocw2) + 1) % VPIC_MAX_IRQ_COUNT;
				break;
			case 0x40:
				/* 010: No Operation */
				break;
			default:
				break;
			}
		}
	}
}

/*
 * io_read_00x1
 * PIC provide IMR
 * Reference: 16-32.PDF, Page 184
 */
static void io_read_00x1(t_pic *rpic) {vport.iobyte = rpic->imr;}

/*
 * io_write_00x1
 * PIC get ICW2, ICW3, ICW4, OCW1 after ICW1
 */
static void io_write_00x1(t_pic *rpic) {
	switch (rpic->status) {
	case ICW2:
		rpic->icw2 = vport.iobyte & VPIC_ICW2_VALID;
		if (!GetBit(rpic->icw1, VPIC_ICW1_SNGL)) {
			/* ICW1.SNGL=0, ICW3=1 */
			rpic->status = ICW3;
		} else if (GetBit(rpic->icw1, VPIC_ICW1_IC4)) {
			/* ICW1.SNGL=1, IC4=1 */
			rpic->status = ICW4;
		} else {
			/* ICW1.SNGL=1, IC4=0 */
			rpic->status = OCW1;
		}
		break;
	case ICW3:
		rpic->icw3 = vport.iobyte;
		if (GetBit(rpic->icw1, VPIC_ICW1_IC4)) {
			/* ICW1.IC4=1 */
			rpic->status = ICW4;
		} else {
			rpic->status = OCW1;
		}
		break;
	case ICW4:
		rpic->icw4 = vport.iobyte & VPIC_ICW4_VALID;
		if (GetBit(rpic->icw4, VPIC_ICW4_uPM)) {
			/* uPM=1, 16-bit 80x86 */
		} else {
			/* uPM=0, 8-bit 8080/8085 */
		}
		if (GetBit(rpic->icw4, VPIC_ICW4_AEOI)) {
			/* AEOI=1, Automatic End of Interrupt */
		} else {
			/* AEOI=0, Non-automatic End of Interrupt */
		}
		if (GetBit(rpic->icw4, VPIC_ICW4_BUF)) {
			/* BUF=1, Buffer */
			if (GetBit(rpic->icw4, VPIC_ICW4_MS)) {
				/* M/S=1, Master 8259A */
			} else {
				/* M/S=0, Slave 8259A */
			}
		} else {
			/* BUF=0, Non-buffer */
		}
		if (GetBit(rpic->icw4, VPIC_ICW4_SFNM)) {
			/* SFNM=1, Special Fully Nested Mode */
		} else {
			/* SFNM=0, Non-special Fully Nested Mode */
		}
		rpic->status = OCW1;
		break;
	case OCW1:
		rpic->ocw1 = vport.iobyte;
		if (GetBit(rpic->ocw3, VPIC_OCW3_SMM)) {
			rpic->isr &= ~(rpic->imr);
		}
		break;
	default:
		break;
	}
}

/* PIC1 provide POLL, IRR, ISR based on OCW3 */
static void io_read_0020() {io_read_00x0(&vpic1);}
/* PIC1 provide IMR */
static void io_read_0021() {io_read_00x1(&vpic1);}
/* PIC2 provide POLL, IRR, ISR based on OCW3 */
static void io_read_00A0() {io_read_00x0(&vpic2);}
/* PIC2 provide IMR */
static void io_read_00A1() {io_read_00x1(&vpic2);}
/* PIC1 get ICW1, OCW2, OCW3 */
static void io_write_0020() {io_write_00x0(&vpic1);}
/* PIC1 get ICW2, ICW3, ICW4, OCW1 after ICW1 */
static void io_write_0021() {io_write_00x1(&vpic1);}
/* PIC2 get ICW1, OCW2, OCW3 */
static void io_write_00A0() {io_write_00x0(&vpic2);}
/* PIC2 get ICW2, ICW3, ICW4, OCW1 after ICW1 */
static void io_write_00A1() {io_write_00x1(&vpic2);}

/*
 * vpicSetIRQ
 * Puts int request into IRR
 * Called by int request sender of devices, e.g. vpitIntTick
 */
void vpicSetIRQ(t_nubit8 irqid) {
	switch(irqid) {
	case 0x00: case 0x01: /* case 0x02: */ case 0x03:
	case 0x04: case 0x05: case 0x06: case 0x07:
		SetBit(vpic1.irr, VPIC_IRR_IRQ(irqid));
		break;
	case 0x08: case 0x09: case 0x0a: case 0x0b:
	case 0x0c: case 0x0d: case 0x0e: case 0x0f:
		SetBit(vpic1.irr, VPIC_IRR_IRQ(0x02));
		SetBit(vpic2.irr, VPIC_IRR_IRQ(irqid - 0x08));
		break;
	default: break;
	}
}

/*
 * vpicScanINTR
 * Returns true if system has a valid INTR
 * Called by CPU
 */
t_bool vpicScanINTR() {
	t_bool flagintr;
	flagintr = HasINTR(&vpic1);
	if (flagintr && (VPIC_GetIntrTopId(&vpic1) == 2)) {
		/* check slave pic */
		flagintr = HasINTR(&vpic2);
	}
	return flagintr;
}

/* Peeks highest priority interrupt without responding to IRQ */
t_nubit8 vpicPeekINTR() {
	t_nubit8 irid1; /* top requested int id in master pic */
	t_nubit8 irid2; /* top requested int id in slave pic */
	irid1 = VPIC_GetIntrTopId(&vpic1);
	if (irid1 == 2) {
		/* if IR2 has int request, then test slave pic */
		irid2 = VPIC_GetIntrTopId(&vpic2);
		/* find the final int id based on slave ICW2 */
		return (irid2 | vpic2.icw2);
	} else {
		/* find the final int id based on master ICW2 */
		return (irid1 | vpic1.icw2);
	}
}

/*
 * vpicGetINTR
 * Returns the id of int request with highest priority
 * Called by CPU, who is responding to this interrupt
 */
t_nubit8 vpicGetINTR() {
	t_nubit8 irid1; /* top requested int id in master pic */
	t_nubit8 irid2; /* top requested int id in slave pic */
	irid1 = VPIC_GetIntrTopId(&vpic1);
	RespondINTR(&vpic1, irid1);
	if (irid1 == 2) {
		/* if IR2 has int request, then test slave pic */
		irid2 = VPIC_GetIntrTopId(&vpic2);
		RespondINTR(&vpic2, irid2);
		/* find the final int id based on slave ICW2 */
		return (irid2 | vpic2.icw2);
	} else {
		/* find the final int id based on master ICW2 */
		return (irid1 | vpic1.icw2);
	}
}

static void init() {
	vport.in[0x0020] = (t_faddrcc) io_read_0020;
	vport.in[0x0021] = (t_faddrcc) io_read_0021;
	vport.in[0x00a0] = (t_faddrcc) io_read_00A0;
	vport.in[0x00a1] = (t_faddrcc) io_read_00A1;
	vport.out[0x0020] = (t_faddrcc) io_write_0020;
	vport.out[0x0021] = (t_faddrcc) io_write_0021;
	vport.out[0x00a0] = (t_faddrcc) io_write_00A0;
	vport.out[0x00a1] = (t_faddrcc) io_write_00A1;
    vbiosAddPost(VPIC_POST);
}

static void reset() {
	MEMSET(&vpic1, Zero8, sizeof(t_pic));
	MEMSET(&vpic2, Zero8, sizeof(t_pic));
	vpic1.status = vpic2.status = ICW1;
	vpic1.ocw3 = vpic2.ocw3 = VPIC_OCW3_RR;
}

static void refresh() {
	if (vpic2.irr & (~vpic2.imr)) {
		/* if slave pic has requested int, then
		 * pass the request into IR2 of master pic */
		SetBit(vpic1.irr, VPIC_IRR_IRQ(2));
	} else {
		/* remove IR2 from master pic */
		ClrBit(vpic1.irr, VPIC_IRR_IRQ(2));
	}
}

static void final() {}

void vpicRegister() {vmachineAddMe;}

static void printPic(t_pic *rpic) {
	PRINTF("Init Status = %d, IRX = %x\n",
		rpic->status, rpic->irx);
	PRINTF("IRR = %x, ISR = %x, IMR = %x, intr = %x\n",
		rpic->irr, rpic->isr, rpic->imr, rpic->irr & (~rpic->imr));
	PRINTF("ICW1 = %x, LTIM = %d, SNGL = %d, IC4 = %d\n",
		rpic->icw1, GetBit(rpic->icw1, VPIC_ICW1_LTIM), GetBit(rpic->icw1, VPIC_ICW1_SNGL),
		GetBit(rpic->icw1, VPIC_ICW1_IC4));
	PRINTF("ICW2 = %x\n",rpic->icw2);
	PRINTF("ICW3 = %x\n", rpic->icw3);
	PRINTF("ICW4 = %x, SFNM = %d, BUF = %d, M/S = %d, AEOI = %d, uPM = %d\n",
		rpic->icw4, GetBit(rpic->icw4, VPIC_ICW4_SFNM), GetBit(rpic->icw4, VPIC_ICW4_BUF),
		GetBit(rpic->icw4, VPIC_ICW4_MS), GetBit(rpic->icw4, VPIC_ICW4_AEOI),
		GetBit(rpic->icw4, VPIC_ICW4_uPM));
	PRINTF("OCW1 = %x\n", rpic->ocw1);
	PRINTF("OCW2 = %x, R = %d, SL = %d, EOI = %d, L = %d\n",
		rpic->ocw2, GetBit(rpic->ocw2, VPIC_OCW2_R), GetBit(rpic->ocw2, VPIC_OCW2_SL),
		GetBit(rpic->ocw2, VPIC_OCW2_EOI), rpic->ocw2 & VPIC_OCW2_L);
	PRINTF("OCW3 = %x, ESMM = %d, SMM = %d, P = %d, RR = %d, RIS = %d\n",
		rpic->ocw3, GetBit(rpic->ocw3, VPIC_OCW3_ESMM), GetBit(rpic->ocw3, VPIC_OCW3_SMM),
		GetBit(rpic->ocw3, VPIC_OCW3_P), GetBit(rpic->ocw3, VPIC_OCW3_RR),
		GetBit(rpic->ocw3, VPIC_OCW3_RIS));
}

/* Print PIC status */
void devicePrintPic() {
	PRINTF("INFO PIC 1\n==========\n");
	printPic(&vpic1);
	PRINTF("INFO PIC 2\n==========\n");
	printPic(&vpic2);
}

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
