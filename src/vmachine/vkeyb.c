/* This file is a part of NXVM project. */

#include "memory.h"

#include "vkbc.h"
#include "vkeyb.h"

t_keyb vkeyb;

void vkeybGetInput(t_nubit8 byte)
{
	t_nubit8 cmdbyte = 0x00;
	if (vkeyb.flagcmd) {
		cmdbyte = vkeyb.inbuf;
		vkeyb.inbuf = byte;
		vkeyb.flagcmd = 0x00;
		switch (cmdbyte) {
		case 0xed:
			vkeyb.flagledscroll = vkeyb.inbuf & 0x01;
			vkeyb.flaglednum    = vkeyb.inbuf & 0x02;
			vkeyb.flagledcaps   = vkeyb.inbuf & 0x04;
			break;
		case 0xf0:
			vkeyb.scancode = vkeyb.inbuf;
			break;
		case 0xf3:
			vkeyb.repeat = ((vkeyb.inbuf & 0x60) >> 5);
			vkeyb.delay  = ( vkeyb.inbuf & 0x1f      );
		default: break;
		}
	} else {
		vkeyb.inbuf = byte;
		switch (vkeyb.inbuf) {
		case 0xed: vkeyb.flagcmd = 0x01; break;            /* write led info */
		case 0xee: vkbcSetOutBuf(0xee); break;                       /* echo */
		case 0xf0: vkeyb.flagcmd = 0x01; break;      /* select scan code set */
		case 0xf2: break;           /* NOTE: read keyboard id; not supported */
		case 0xf3: vkeyb.flagcmd = 0x01; break; /* set repeat rate and delay */
		default: /* TODO: process normal input */
			break;
		}
	}
}
void vkeybRefresh()
{}
void vkeybInit()
{
	memset(&vkeyb, 0x00, sizeof(t_keyb));
}
void vkeybFinal()
{}