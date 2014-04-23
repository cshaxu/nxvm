/* This file is a part of NXVM project. */

// Programmable Interval Timer: 8254

#ifndef NXVM_VPIT_H
#define NXVM_VPIT_H

#include "time.h"
#include "vglobal.h"

#define VPIT_DEBUG
#define VPIT_TICK 54.925493

typedef struct {
	t_nubit8  cw[4];                     /* control words[0-2] for counter 0-2,
	                                          and cw[3] is read-back command */
	t_nubit16 init[3];                                     /* initial counts */
	t_nubit16 latch[3];                                      /* latch counts */
	t_nubit16 counter[3];                                    /* counter[0-2] */
	clock_t   clock1[3],clock2[3];                                  /* timer */
} t_pit;

extern t_pit vpit;

/*
 * Ctrl Word   SC1 | SC0 | RW1   | RW0    | M2   | M1   | M0   | BCD
 * Latch Cmd   SC1 | SC0 | 0     | 0      | x    | x    | x    | x  
 * Read-back   1   | 1   | COUNT | STATUS | CNT2 | CNT1 | CNT0 | 0  
 * Stus Byte   OUT | NC  | RW1   | RW0    | M2   | M1   | M9   | BCD
 */

/*
 * vpitIntTick
 * Generates interrupt requests from counter 0
 */
void vpitIntTick();

void IO_Read_0040();                                       /* read counter 0 */
void IO_Read_0041();                                       /* read counter 1 */
void IO_Read_0042();                                       /* read counter 2 */
void IO_Write_0040();                                     /* write counter 0 */
void IO_Write_0041();                                     /* write counter 1 */
void IO_Write_0042();                                     /* write counter 2 */
void IO_Write_0043();                                  /* write control word */

void vpitRefresh();
void vpitInit();
void vpitFinal();

#endif