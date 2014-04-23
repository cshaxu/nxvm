/* This file is a part of NXVM project. */

#include "memory.h"
#include "vport.h"

t_port vport;

void IO_Read_VOID() {}
void IO_Write_VOID() {}

void vportInit()
{
	t_nubitcc i;
	memset(&vport, 0x00, sizeof(t_port));
	for(i = 0;i < 0x10000;++i) {
		vport.in[i]  = (t_faddrcc)IO_Read_VOID;
		vport.out[i] = (t_faddrcc)IO_Write_VOID;
	}
}
void vportRefresh() {}
void vportFinal() {}
