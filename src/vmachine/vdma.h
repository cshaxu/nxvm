/* This file is a part of NXVM project. */

/* Direct Memory Access Controller: Intel 8237A */

#ifndef NXVM_VDMA_H
#define NXVM_VDMA_H

#include "vglobal.h"

#define VDMA_DEBUG

typedef t_nubit8 t_page;

typedef struct {
	t_nubit16 baseaddr;                                      /* base address */
	t_nubit16 basewc;                                     /* base word count */
	t_nubit16 curraddr;                                   /* current address */
	t_nubit16 currwc;                                  /* current word count */
	t_nubit6  mode;                                         /* mode register */
	t_page    page;                                         /* page register */
} t_dmac;

typedef struct {
	t_dmac    dmac[4];
	t_nubit8  command;
	t_nubit8  status;
	t_nubit4  mask;
	t_nubit4  request;
	t_nubit8  temp;
	t_nubit16 tempaddr;
	t_nubit16 tempwc;
	t_bool    flagmsb;
} t_dma;

extern t_dma vdma1,vdma2;


void vdmaSetDREQ(t_nubit8 dreqid); /* not implemented */
void vdmaGetDACK(); /* not implemented */
void vdmaSetEOP(); /* not implemented */

void vdmaReset();
void vdmaRefresh(); /* TODO: act as CLK input */
void vdmaInit();
void vdmaFinal();

#endif