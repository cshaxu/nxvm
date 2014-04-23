/* This file is a part of NXVM project. */

/* KeyBoard Internal Controller: Intel 8048 */

#ifndef NXVM_VKEYB_H
#define NXVM_VKEYB_H

#include "vglobal.h"

typedef struct {
	t_nubit8 x;
} t_keyb;

extern t_keyb vkeyb;

void vkeybRefresh();
void vkeybInit();
void vkeybFinal();

#endif