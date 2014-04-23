/* This file is a part of NXVM project. */

// Direct Memory Access Controller: Intel 8237A

#ifndef NXVM_VDMA_H
#define NXVM_VDMA_H

#include "vglobal.h"

typedef struct {
	t_nubit16 base_address;
	t_nubit16 base_wordcount;
	t_nubit16 cur_address;
	t_nubit16 cur_wordcount;
	//t_nubit6 mode;
	t_nubit8 page;
} t_dmac_channel;

typedef struct {
	t_dmac_channel channel[4];
	/*t_nubit16 temp_address;
	t_nubit16 temp_wordcount;
	t_nubit8 status;
	t_nubit8 command;
	t_nubit8 temp;
	t_nubit4 mask;
	t_nubit4 request;*/
	t_bool msb;
} t_dmac;

extern t_dmac vdmac1,vdmac2;

void vdmacInit();
void vdmacFinal();

#endif