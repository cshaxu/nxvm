/* This file is a part of NXVM project. */

/* Floppy Driver Controller: Intel 8272A */

#ifndef NXVM_VFDC_H
#define NXVM_VFDC_H

#include "vglobal.h"

/*#define VFDC_DEBUG*/

typedef struct {
	t_nubit8         dor;                         /* digital output register */
	t_nubit8         msr;                            /* main status register */
	t_nubit8         dr;                                    /* data register */
	t_nubit8         dir;                          /* digital input register */
	t_nubit8         ccr;                  /* configuration control register */

	t_nubit4         hut;                                /* head unload time */
	t_nubit4         hlt;                                  /* head load time */
	t_nubit8         srt;                                  /* step rate time */
	t_bool           flagndma;             /* 0 = dma mode; 1 = non-dma mode */
	t_bool           flagintr;                  /* 0 = no intr; 1 = has intr */

	t_nubit8         rwid;                   /* io port command/result rw id */
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

#ifdef VFDC_DEBUG
void    IO_Read_F3F0();                                    /* print all info */
void    IO_Write_F3F0();                                        /* vfdcReset */
#define IO_Write_F3F1 vfdcRefresh                             /* vfdcRefresh */
#endif

#define vfdcPIORead   vfdcTransRead
#define vfdcDMARead   vfdcTransRead
#define vfdcPIOWrite  vfdcTransWrite
#define vfdcDMAWrite  vfdcTransWrite
#define vfdcPIOFinal  vfdcTransFinal
#define vfdcDMAFinal  vfdcTransFinal
void vfdcTransRead();
void vfdcTransWrite();
void vfdcTransInit();
void vfdcTransFinal();
void vfdcRefresh();
void vfdcReset();
void vfdcInit();
void vfdcFinal();

#endif
