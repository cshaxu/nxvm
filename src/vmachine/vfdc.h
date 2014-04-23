/* This file is a part of NXVM project. */

/* Floppy Driver Controller: Intel 8272A */

#ifndef NXVM_VFDC_H
#define NXVM_VFDC_H

#include "vglobal.h"

typedef t_nubit8 t_fdc_status_cmd;

typedef struct {
	t_nubit8         dor;                         /* digital output register */
	t_nubit8         msr;                            /* main status register */
	t_nubit8         dr;                                    /* data register */
	t_nubit8         dir;                          /* digital input register */
	t_nubit8         ccr;                  /* configuration control register */

	t_nubit4         hut;                                /* head unload time */
	t_nubit4         hlt;                                  /* head load time */
	t_nubit8         srt;                                  /* step rate time */
	t_bool           ndma;                 /* 0 = dma mode; 1 = non-dma mode */
	t_bool           intr;                      /* 0 = no intr; 1 = has intr */

	t_bool           flagro;                         /* write protect status */
	t_nubit8         head;                                   /* present head */
	t_nubit8         pcn;                         /* present cylinder number */

	t_fdc_status_cmd flagcmd;
	t_nubit8         cmd[9];
	t_nubit8         ret[7];
	t_nubit8         st0,st1,st2,st3;                     /* state registers */
} t_fdc;

extern t_fdc vfdc;

void IO_Read_03F4();                            /* read main status register */
void IO_Read_03F5();                                /* read standard results */
void IO_Read_03F7();                          /* read digital input register */
void IO_Write_03F2();                       /* write digital output register */
void IO_Write_03F5();                             /* write standard commands */
void IO_Write_03F7();

void vfdcRefresh();
void vfdcInit();
void vfdcFinal();

#endif
