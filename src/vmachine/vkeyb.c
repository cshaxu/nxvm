/* This file is a part of NXVM project. */

#include "memory.h"

#include "vkeyb.h"

t_keyb vkeyb;

void vkeybRefresh()
{}
void vkeybInit()
{
	memset(&vkeyb, 0x00, sizeof(t_keyb));
}
void vkeybFinal()
{}