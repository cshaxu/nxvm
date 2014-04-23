/* This file is a part of NXVM project. */

/* KeyBoard Internal Controller: Intel 8048 */

#ifndef NXVM_VKEYB_H
#define NXVM_VKEYB_H

#include "vglobal.h"

typedef struct {
	t_nubit8 scancode;
	t_nubit8 repeat;
	t_nubit8 delay;
	t_nubit8 inbuf;
	t_bool flagcmd;

	t_bool flagledcaps;
	t_bool flaglednum;
	t_bool flagledscroll;
} t_keyb;

extern t_keyb vkeyb;

void vkeybGetInput(t_nubit8 byte);
void vkeybRefresh();
void vkeybInit();
void vkeybFinal();

#endif