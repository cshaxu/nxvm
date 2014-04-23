/* This file is a part of NXVM project. */

/* KeyBoard Internal Controller: Intel 8048 */

#ifndef NXVM_VKEYB_H
#define NXVM_VKEYB_H

#include "vglobal.h"

#define VKEYB_SIZE_BUFFER 0x10

typedef struct {
	t_nubit8 bufptr;
	t_nubit8 bufsize;
	t_nubit8 buffer[VKEYB_SIZE_BUFFER];

	t_nubit8 inbuf;
	t_nubit8 scancode;                                     /* NOTE: not used */
	t_nubit8 repeat;                                       /* NOTE: not used */
	t_nubit8 delay;                                        /* NOTE: not used */
	t_bool   flagcmd;
	t_bool   flagenable;                                   /* NOTE: not used */
	t_bool   flagledcaps;                                  /* NOTE: not used */
	t_bool   flaglednum;                                   /* NOTE: not used */
	t_bool   flagledscroll;                                /* NOTE: not used */
} t_keyb;

extern t_keyb vkeyb;

t_nubit8 vkeybBufSize();
t_nubit8 vkeybBufPop();
void vkeybBufPush(t_nubit8 byte, t_bool done);

void vkeybSetInput(t_nubit8 byte);
void vkeybDefault();
void vkeybReset();
void vkeybRefresh();
void vkeybInit();
void vkeybFinal();

#endif
