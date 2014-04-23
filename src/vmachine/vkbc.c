/* This file is a part of NXVM project. */

#include "memory.h"

#include "vkbc.h"

t_kbc vkbc;

void vkbcRefresh()
{}
void vkbcInit()
{
	memset(&vkbc, 0x00, sizeof(t_kbc));
}
void vkbcFinal()
{}