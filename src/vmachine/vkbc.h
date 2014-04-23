/* This file is a part of NXVM project. */

/* KeyBoard Controller: Intel 8042 */

#ifndef NXVM_VKBC_H
#define NXVM_VKBC_H

#include "vglobal.h"

typedef struct {
	t_nubit8 x;
} t_kbc;

extern t_kbc vkbc;

void vkbcRefresh();
void vkbcInit();
void vkbcFinal();

#endif