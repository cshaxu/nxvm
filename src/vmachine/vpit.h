/* Copyright 2012-2014 Neko. */

#ifndef NXVM_VPIT_H
#define NXVM_VPIT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vglobal.h"

#define NXVM_DEVICE_PIT "Intel 8254"

typedef enum {
	VPIT_STATUS_RW_READY,
	VPIT_STATUS_RW_LSB,
	VPIT_STATUS_RW_MSB
} t_pit_status_rw;

typedef struct {
	t_nubit8 cw[4]; /* control words[0-2] for counter 0-2,
	                 * and cw[3] is read-back command */

	t_nubit16 init[3];  /* initial counts */
	t_nubit16 count[3]; /* counter[0-2] */
	t_nubit16 latch[3]; /* latch counts */

	t_bool flagready[3]; /* flag of ready */
	t_bool flaglatch[3]; /* flag of latch status */
	t_bool flaggate[3];  /* enable or disable counter */
	t_pit_status_rw flagread[3];  /* flag of low byte read */
	t_pit_status_rw flagwrite[3]; /* flag of low byte write */

	t_faddrcc out[3]; /* action when out signal is valid */
} t_pit;

extern t_pit vpit;

/*
 * Ctrl Word   SC1 | SC0 | RW1   | RW0    | M2   | M1   | M0   | BCD
 * Latch Cmd   SC1 | SC0 | 0     | 0      | x    | x    | x    | x  
 * Read-back   1   | 1   | COUNT | STATUS | CNT2 | CNT1 | CNT0 | 0  
 * Stus Byte   OUT | NC  | RW1   | RW0    | M2   | M1   | M9   | BCD
 */
#define VPIT_GetSC(cw)  (((cw) & 0xc0)>>0x06)
#define VPIT_GetRW(cw)  (((cw) & 0x30)>>0x04)
#define VPIT_GetM(cw)   (((cw) & 0x0e)>>0x01)
#define VPIT_GetBCD(cw) (((cw) & 0x01))

void vpitSetGate(t_nubit8 id, t_bool flaggate);

void vpitRegister();

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
