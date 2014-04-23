/* This file is a part of NXVM project. */

#include "memory.h"

#include "vkbc.h"
#include "vkeyb.h"

t_keyb vkeyb;

/* NOTE: reference http://www.computer-engineering.org/ps2keyboard/ */

t_nubit8 vkeybBufSize()
{
	return vkeyb.bufsize;
}
t_nubit8 vkeybBufPop()
{
	t_nubit8 ret = vkeyb.buffer[vkeyb.bufptr];
	if (vkeyb.bufsize) {
		vkeyb.bufptr++;
		if (vkeyb.bufptr == VKEYB_SIZE_BUFFER) vkeyb.bufptr = 0x00;
		vkeyb.bufsize--;
	}
	return ret;
}
void vkeybBufPush(t_nubit8 byte, t_bool done)
{
	if (vkeyb.bufsize == VKEYB_SIZE_BUFFER) {
		/* send error message */
		if (vkeyb.scancode == 0x01) vkbcSetResponse(0xff);
		else vkbcSetResponse(0x00);
	} else {
		vkeyb.buffer[(vkeyb.bufptr + vkeyb.bufsize) % VKEYB_SIZE_BUFFER] =
			byte;
		vkeyb.bufsize++;
	}
	if (done) vkbcSetResponse(vkeybBufPop());
}
void vkeybDefault()
{
	memset(&vkeyb, 0x00, sizeof(t_keyb));
	vkeyb.repeat   = 0x0b;
	vkeyb.delay    = 0x01;
	vkeyb.scancode = 0x02;
}
void vkeybSetInput(t_nubit8 byte)
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
		case 0xed: vkbcSetResponse(0xfa);                  /* write led info */
			vkeyb.flagcmd = 0x01;
			break;
		case 0xee: vkbcSetResponse(0xee);                            /* echo */
			break;
		case 0xf0: vkbcSetResponse(0xfa);            /* select scan code set */
			vkeyb.flagcmd = 0x01;
			break;
		case 0xf2: break;           /* NOTE: read keyboard id; not supported */
		case 0xf3: vkbcSetResponse(0xfa);       /* set repeat rate and delay */
			vkeyb.flagcmd = 0x01;
			break;
		case 0xf4: vkbcSetResponse(0xfa);   /* enable keyboard, clear buffer */
			memset(vkeyb.buffer, 0x00, VKEYB_SIZE_BUFFER);
			vkeyb.flagenable = 0x01;
			break;
		case 0xf5: vkbcSetResponse(0xfa);                /* default, disable */
			vkeybDefault();
			vkeyb.flagenable = 0x00;
			break;
		case 0xf6: vkbcSetResponse(0xfa);                     /* set default */
			vkeybDefault();
			vkeyb.flagenable = 0x01;
			break;
		case 0xf7: break;    /* NOTE: set repeat for all keys; not supported */
		case 0xf8: break;        /* NOTE: set mak/brk for all; not supported */
		case 0xf9: break;      /* NOTE: set make for all keys; not supported */
		case 0xfa: break;    /* NOTE: set rep/mak/brk for all; not supported */
		case 0xfb: break;       /* NOTE: set repeat for a key; not supported */
		case 0xfc: break;      /* NOTE: set mak/brk for a key; not supported */
		case 0xfd: break;      /* NOTE: set mak/brk for a key; not supported */
		case 0xfe: break;                     /* NOTE: resend; not supported */
		case 0xff: vkbcSetResponse(0xfa);                           /* reset */
			vkeybReset();
			break;
		default:   vkbcSetResponse(0xfe);                 /* invalid command */
			break;
		}
	}
}
void vkeybReset()
{
	/* exec self assessment */
	vkeybDefault();
	vkeyb.flagledscroll = 0x01;
	vkeyb.flaglednum    = 0x01;
	vkeyb.flagledcaps   = 0x01;
	/* finalize */
	vkbcSetResponse(0xaa);
	vkeyb.flagledscroll = 0x00;
	vkeyb.flaglednum    = 0x00;
	vkeyb.flagledcaps   = 0x00;
}
void vkeybRefresh()
{}
void vkeybInit()
{
	vkeybReset();
}
void vkeybFinal()
{}
