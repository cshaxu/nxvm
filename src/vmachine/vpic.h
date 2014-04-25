/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VPIC_H
#define NXVM_VPIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vglobal.h"

#define NXVM_DEVICE_PIC "Intel 8259A"

typedef enum {ICW1, ICW2, ICW3, ICW4, OCW1} t_pic_init_status;

#define ocw1 imr
typedef struct {
	t_nubit8 irr;  /* Interrupt Request Register */
	t_nubit8 imr;  /* Interrupt Mask Register */
	t_nubit8 isr;  /* In Service Register */
	t_nubit8 icw1, icw2, icw3, icw4, ocw2, ocw3; /* command words */
	t_pic_init_status status; /* initialization status */
	t_nubit8 irx; /* current highest ir id */
} t_pic;

extern t_pic vpic1, vpic2;

/*
 * ICW1: x  | x    | x   | 1    | LTIM | x   | SNGL | IC4
 * ICW2: T7 | T6   | T5  | T4   | T3   | x   | x    | x
 * ICW3: S7 | S6   | S5  | S4   | S3   | S2  | S1   | S0   (master)
 * ICW3: x  | x    | x   | x    | x    | ID2 | ID1  | ID0  (slave)
 * ICW4: 0  | 0    | 0   | SFNM | BUF  | M/S | AEOI | uPM
 * OCW1: M7 | M6   | M5  | M4   | M3   | M2  | M1   | M0
 * OCW2: R  | SL   | EOI | 0    | 0    | L2  | L1   | L0
 * OCW3: 0  | ESMM | SMM | 0    | 1    | P   | RR   | RIS
 * POLL: I  | x    | x   | x    | x    | W2  | W1   | W0
 */

/* Get flags from ICWs */
#define VPIC_GetLTIM(vpic) (!!((vpic)->icw1 & 0x08))
#define VPIC_GetSNGL(vpic) (!!((vpic)->icw1 & 0x02))
#define VPIC_GetIC4(vpic)  (!!((vpic)->icw1 & 0x01))
#define VPIC_GetSFNM(vpic) (!!((vpic)->icw4 & 0x10))
#define VPIC_GetBUF(vpic)  (!!((vpic)->icw4 & 0x08))
#define VPIC_GetMS(vpic)   (!!((vpic)->icw4 & 0x04))
#define VPIC_GetAEOI(vpic) (!!((vpic)->icw4 & 0x02))
#define VPIC_GetuPM(vpic)  (!!((vpic)->icw4 & 0x01))
#define VPIC_GetR(vpic)    (!!((vpic)->ocw2 & 0x80))
#define VPIC_GetSL(vpic)   (!!((vpic)->ocw2 & 0x40))
#define VPIC_GetEOI(vpic)  (!!((vpic)->ocw2 & 0x20))
#define VPIC_GetESMM(vpic) (!!((vpic)->ocw3 & 0x40))
#define VPIC_GetSMM(vpic)  (!!((vpic)->ocw3 & 0x20))
#define VPIC_GetP(vpic)    (!!((vpic)->ocw3 & 0x04))
#define VPIC_GetRR(vpic)   (!!((vpic)->ocw3 & 0x02))
#define VPIC_GetRIS(vpic)  (!!((vpic)->ocw3 & 0x01))

/* Get id of highest priority interrupts in different registers */
#define VPIC_GetIntrTopId(vpic) (GetRegTopId((vpic), ((vpic)->irr & (~(vpic)->imr))))
#define VPIC_GetIsrTopId(vpic)  (GetRegTopId((vpic), (vpic)->isr))
#define VPIC_GetIrrTopId(vpic)  (GetRegTopId((vpic), (vpic)->irr))
#define VPIC_GetImrTopId(vpic)  (GetRegTopId((vpic), (vpic)->imr))

void vpicSetIRQ(t_nubit8 irqid);
t_bool vpicScanINTR();
t_nubit8 vpicPeekINTR();
t_nubit8 vpicGetINTR();

void vpicRegister();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
