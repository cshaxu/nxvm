/* This file is a part of NXVM project. */

/* Direct Memory Access Controller: Intel 8237A */

#ifndef NXVM_VDMA_H
#define NXVM_VDMA_H

#include "vglobal.h"

#define VDMA_DEBUG

typedef struct {
	t_nubit16 baseaddr;                                      /* base address */
	t_nubit16 basewc;                                     /* base word count */
	t_nubit16 curraddr;                                   /* current address */
	t_nubit16 currwc;                                  /* current word count */
	//t_nubit6 mode;
	t_nubit8 page;
} t_dmac;

typedef struct {
	t_dmac dmac[4];
	/*t_nubit16 temp_address;
	t_nubit16 temp_wordcount;
	t_nubit8 status;
	t_nubit8 command;
	t_nubit8 temp;
	t_nubit4 mask;
	t_nubit4 request;*/
	t_bool msb;
} t_dma;

extern t_dma vdma1,vdma2;

void vdmaInit();
void vdmaFinal();

#endif