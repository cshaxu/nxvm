/* This file is a part of NXVM project. */

/* KeyBoard Controller: Intel 8042 */

#ifndef NXVM_VKBC_H
#define NXVM_VKBC_H

#include "vglobal.h"

#define VKBC_DEBUG

typedef struct {
	t_nubit8  status;                                     /* status register */
	t_nubit8  outbuf;                                       /* output buffer */
	t_nubit8  inbuf;                                         /* input buffer */
	t_nubit8  outport;                                        /* output port */
	t_nubit8  inport;                                          /* input port */
	t_nubit8  ram[0x20];

	t_bool    flagpswd;
	t_nubit8  pswd;
} t_kbc;

#define control ram[0x00]                                /* control register */

extern t_kbc vkbc;

/*
 * SR  PE | RT | TT | IS | CD | SF | IBF | OBF 
 * OB  scan code / return / 
 * IB  command / parameter
 * CR  0  | CM | DA | DK | IO | SF | 0   | OBFI 
 * IP  ~KI | MDA | ~MJI | 256K | UNDEFINED 0-3
 * OP 
 */

void IO_Read_0060();
void IO_Read_0064();
void IO_Write_0060();
void IO_Write_0064();

void vkbcRefresh();
void vkbcInit();
void vkbcFinal();

#endif