/* This file is a part of NXVM project. */

/* Programmable Interrupt Controller: Intel 8259A (Master+Slave) */

#ifndef NXVM_VPIC_H
#define NXVM_VPIC_H

#include "vglobal.h"

/*#define VPIC_DEBUG*/

typedef enum {ICW1,ICW2,ICW3,ICW4,OCW1} t_pic_status_init;

#define ocw1 imr
typedef struct {
	t_nubit8          irr;                     /* Interrupt Request Register */
	t_nubit8          imr;                        /* Interrupt Mask Register */
	t_nubit8          isr;                            /* In Service Register */
	t_nubit8          icw1,icw2,icw3,icw4,ocw2,ocw3;        /* command words */
	t_pic_status_init flaginit;                     /* initialization status */
	t_nubit8          irx;                          /* current highest ir id */
} t_pic;

extern t_pic vpic1,vpic2;

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

/*
 * Wrappers for IO_*_00x0
 * Reference: vpic.c
 */
void IO_Read_0020();            /* PIC1 provide POLL, IRR, ISR based on OCW3 */
void IO_Read_0021();                                     /* PIC1 provide IMR */
void IO_Read_00A0();            /* PIC2 provide POLL, IRR, ISR based on OCW3 */
void IO_Read_00A1();                                     /* PIC2 provide IMR */
void IO_Write_0020();                           /* PIC1 get ICW1, OCW2, OCW3 */
void IO_Write_0021();          /* PIC1 get ICW2, ICW3, ICW4, OCW1 after ICW1 */
void IO_Write_00A0();                           /* PIC2 get ICW1, OCW2, OCW3 */
void IO_Write_00A1();          /* PIC2 get ICW2, ICW3, ICW4, OCW1 after ICW1 */
/* Test I/O Ports */
#ifdef VPIC_DEBUG
void IO_Read_FF20();                /* Print all variables for pic1 and pic2 */
void IO_Read_FF21();                                    /* Test vpicScanINTR */
void IO_Read_FF22();                                     /* Test vpicGetINTR */
void IO_Write_FF20();                                     /* Test vpicSetIRQ */
#endif

/*
 * vpicSetIRQ
 * Puts int request into IRR
 * Called by int request sender of devices, e.g. vpitIntTick
 */
void vpicSetIRQ(t_nubit8 irqid);
/*
 * vpicScanINTR
 * Returns true if system has a valid INTR
 * Called by CPU
 */
t_bool vpicScanINTR();
/*
 * vpicGetINTR
 * Returns the id of int request with highest priority
 * Called by CPU, who is responding to this interrupt
 */
t_nubit8 vpicGetINTR();
/*
 * vpicInit: Public interface for vmachine
 * Initializes PIC
 * Sets zero for pic structs
 * Registers I/O ports for master and slave pics
 */
void vpicInit();
/*
 * vpicRefresh: Public interface for vmachine
 * Refreshes PIC
 * If slave pic has irq, set master pic IRQ 2
 */
void vpicRefresh();
/*
 * vpicFinal: Public interface for vmachine
 * Terminates PIC
 * Frees any allocated memory
 */
void vpicFinal();

#endif
