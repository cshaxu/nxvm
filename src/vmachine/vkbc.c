/* This file is a part of NXVM project. */

#include "memory.h"

#include "vport.h"
#include "vkbc.h"

void IO_Read_0064() {vport.iobyte = 0x10;}

void vkbcInit()
{
	vport.in[0x0064] = (t_faddrcc)IO_Read_0064;
}
void vkbcReset() {}
void vkbcRefresh() {}
void vkbcFinal() {}
