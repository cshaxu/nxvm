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
	t_nubit8 irx; /* id of current top potential ir */
} t_pic;

extern t_pic vpic1, vpic2;

#define VPIC_MAX_IRQ_COUNT 8

/*
 * IRR: IRQ7 | IRQ6 | IRQ5 | IRQ4 | IRQ3 | IRQ2 | IRQ1 | IRQ0
 * IMR: IRQ7 | IRQ6 | IRQ5 | IRQ4 | IRQ3 | IRQ2 | IRQ1 | IRQ0
 * ISR: IRQ7 | IRQ6 | IRQ5 | IRQ4 | IRQ3 | IRQ2 | IRQ1 | IRQ0
 */

/* interrupt request register bits */
#define VPIC_IRR_IRQ(id) (1 << (id))

/* interrupt mask register bits */
#define VPIC_IMR_IRQ(id) (1 << (id))

/* in-service request register bits */
#define VPIC_ISR_IRQ(id) (1 << (id))

/*
 * ICW1: 0  | 0    | 0   | I    | LTIM | x   | SNGL | IC4
 * ICW2: T7 | T6   | T5  | T4   | T3   | x   | x    | x
 * ICW3: S7 | S6   | S5  | S4   | S3   | S2  | S1   | S0   (master)
 * ICW3: x  | x    | x   | x    | x    | ID2 | ID1  | ID0  (slave)
 * ICW4: 0  | 0    | 0   | SFNM | BUF  | M/S | AEOI | uPM
 * OCW1: M7 | M6   | M5  | M4   | M3   | M2  | M1   | M0
 * OCW2: R  | SL   | EOI | 0    | 0    | L2  | L1   | L0
 * OCW3: 0  | ESMM | SMM | 0    | I    | P   | RR   | RIS
 * POLL: I  | x    | x   | x    | x    | W2  | W1   | W0
 */

/* ICW1 bits */
#define VPIC_ICW1_IC4  0x01 /* ICW4 is needed */
#define VPIC_ICW1_SNGL 0x02 /* single(1) 8259 or cascading(0) 8259's */
#define VPIC_ICW1_LTIM 0x08 /* level(1) or edge(0) triggered mode */
#define VPIC_ICW1_I    0x10 /* must be 1 for icw1 */

/* ICW2 bits */
#define VPIC_ICW2_VALID 0xf8 /* A7-A3 of x86 interrupt vector */

/* ICW3 master bits */
#define VPIC_ICW3_S(id) (1 << (id)) /* int req id has slave (1) or not(0) */

/* ICW4 bits */
#define VPIC_ICW4_uPM   0x01 /* uPM */
#define VPIC_ICW4_AEOI  0x02 /* auto end of interrupt */
#define VPIC_ICW4_MS    0x04 /* master(1) or slave(0) */
#define VPIC_ICW4_BUF   0x08 /* buffered(1) or not(0) */
#define VPIC_ICW4_SFNM  0x10 /* special fully nested mode (1) or sequential (0) */
#define VPIC_ICW4_VALID 0x1f

/* OCW1 bits */
#define VPIC_OCW1_IMR(id) (1 << (id)) /* irq id is masked */

/* OCW2 bits */
#define VPIC_OCW2_L   0x07 /* interrupt request level to act upon */
#define VPIC_OCW2_EOI 0x20 /* eoi type */
#define VPIC_OCW2_SL  0x40 /* specific eoi command */
#define VPIC_OCW2_R   0x80 /* rotate priority */
#define VPIC_GetOCW2_L(cocw2) ((cocw2) & VPIC_OCW2_L)

/* OCW3 bits */
#define VPIC_OCW3_RIS  0x01 /* read irr(1) or isr(0) on next read */
#define VPIC_OCW3_RR   0x02 /* act(1) on value of bit 0 or no(0) action if bit 0 set */
#define VPIC_OCW3_P    0x04 /* poll command issued(1) or not(0) */
#define VPIC_OCW3_I    0x08 /* must be 1 for ocw3 */
#define VPIC_OCW3_SMM  0x20 /* set(1) or reset(0) special mask */
#define VPIC_OCW3_ESMM 0x40 /* act(1) on value of bit 5 or no(0) action if bit 5 set */

/* POLL bits */
#define VPIC_POLL_I 0x80 /* must be 1 for poll command */

/* Get id of highest priority interrupts in different registers */
#define VPIC_GetIntrTopId(rpic) (GetRegTopId((rpic), ((rpic)->irr & (~((rpic)->imr)))))
#define VPIC_GetIsrTopId(rpic)  (GetRegTopId((rpic), (rpic)->isr))
#define VPIC_GetIrrTopId(rpic)  (GetRegTopId((rpic), (rpic)->irr))
#define VPIC_GetImrTopId(rpic)  (GetRegTopId((rpic), (rpic)->imr))

void vpicSetIRQ(t_nubit8 irqId);
t_bool vpicScanINTR();
t_nubit8 vpicPeekINTR();
t_nubit8 vpicGetINTR();

void vpicRegister();

#define VPIC_POST "           \
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
out a1, al                  \n"

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
