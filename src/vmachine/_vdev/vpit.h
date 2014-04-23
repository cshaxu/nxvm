/* This file is a part of NXVM project. */

/* Programmable Interval Timer: Intel 8254 */

#ifndef NXVM_VPIT_H
#define NXVM_VPIT_H

#include "stdio.h"

#include "vglobal.h"

#define VPIT_DEBUG

typedef enum {
	VPIT_STATUS_RW_READY,
	VPIT_STATUS_RW_LSB,
	VPIT_STATUS_RW_MSB
} t_pit_status_rw;

typedef struct {
	t_nubit8         cw[4];              /* control words[0-2] for counter 0-2,
	                                          and cw[3] is read-back command */

	t_nubit16        init[3];                              /* initial counts */
	t_nubit16        count[3];                               /* counter[0-2] */
	t_nubit16        latch[3];                               /* latch counts */

	t_bool           flagready[3];                          /* flag of ready */
	t_bool           flaglatch[3];                   /* flag of latch status */
	t_pit_status_rw  flagread[3];                   /* flag of low byte read */
	t_pit_status_rw  flagwrite[3];                 /* flag of low byte write */
	t_bool           flaggate[3];               /* enable or disable counter */

	t_faddrcc        out[3];              /* action when out signal is valid */
} t_pit;

extern t_pit vpit;

/*
 * Ctrl Word   SC1 | SC0 | RW1   | RW0    | M2   | M1   | M0   | BCD
 * Latch Cmd   SC1 | SC0 | 0     | 0      | x    | x    | x    | x  
 * Read-back   1   | 1   | COUNT | STATUS | CNT2 | CNT1 | CNT0 | 0  
 * Stus Byte   OUT | NC  | RW1   | RW0    | M2   | M1   | M9   | BCD
 */

void IO_Read_0040();                                       /* read counter 0 */
void IO_Read_0041();                                       /* read counter 1 */
void IO_Read_0042();                                       /* read counter 2 */
void IO_Write_0040();                                     /* write counter 0 */
void IO_Write_0041();                                     /* write counter 1 */
void IO_Write_0042();                                     /* write counter 2 */
void IO_Write_0043();                                  /* write control word */
/* Test I/O Ports */
#ifdef VPIT_DEBUG
void IO_Read_FF40();                                       /* prnit all info */
void IO_Write_FF40();                                            /* set gate */
void IO_Read_FF41();                                              /* refresh */
#endif

void vpitIntSystemTimer();               /* generate system timer interrupts */
#define vpitRefDRAM NULL                              /* dummy: refresh dram */

void vpitSetGate(t_nubit8 id, t_bool gate);  /* set gate value and load init */
void vpitRefresh();                                    /* act as a CLK pulse */
void vpitInit();
void vpitFinal();

#endif