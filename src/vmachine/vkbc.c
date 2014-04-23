/* This file is a part of NXVM project. */

#include "memory.h"

#include "vcpuins.h"
#include "vcpu.h"
#include "vkbc.h"

#define GetBit(a,b) ((a) &   (b))
#define SetBit(a,b) ((a) |=  (b))
#define ClrBit(a,b) ((a) &= ~(b))

#define VKBC_FLAG_STATUS_OUTPUT_BUFFER_FULL            0x01
#define VKBC_FLAG_STATUS_INPUT_BUFFER_FULL             0x02
#define VKBC_FLAG_STATUS_SYSTEM_FLAG                   0x04
#define VKBC_FLAG_STATUS_COMMAND                       0x08
#define VKBC_FLAG_STATUS_INHIBIT_SWITCH                0x10
#define VKBC_FLAG_STATUS_TRANSMIT_TIME_OUT             0x20
#define VKBC_FLAG_STATUS_RECEIVE_TIME_OUT              0x40
#define VKBC_FLAG_STATUS_PARITY_ERROR                  0x80

#define VKBC_FLAG_COMMAND_OUTPUT_BUFFER_FULL_INTERRUPT 0x01
#define VKBC_FLAG_COMMAND_RESERVED_BIT_1               0x02
#define VKBC_FLAG_COMMAND_SYSTEM_FLAG                  0x04
#define VKBC_FLAG_COMMAND_INHIBIT_OVERRIDE             0x08
#define VKBC_FLAG_COMMAND_DISABLE_KEYBOARD             0x10
#define VKBC_FLAG_COMMAND_PC_MODE                      0x20
#define VKBC_FLAG_COMMAND_PC_COMPATIBILITY_MODE        0x40
#define VKBC_FLAG_COMMAND_RESERVED_BIT_7               0x80

#define SetIBF (SetBit(vkbc.status, VKBC_FLAG_STATUS_INPUT_BUFFER_FULL))
#define ClrIBF (ClrBit(vkbc.status, VKBC_FLAG_STATUS_INPUT_BUFFER_FULL))
#define SetOBF (SetBit(vkbc.status, VKBC_FLAG_STATUS_OUTPUT_BUFFER_FULL))
#define ClrOBF (ClrBit(vkbc.status, VKBC_FLAG_STATUS_OUTPUT_BUFFER_FULL))
#define SetCMD (SetBit(vkbc.status, VKBC_FLAG_STATUS_COMMAND))
#define ClrCMD (ClrBit(vkbc.status, VKBC_FLAG_STATUS_COMMAND))

t_kbc vkbc;

/* TODO:
 * 
 *
 *
 *
 */

void IO_Read_0060()
{
	vcpu.iobyte = vkbc.outbuf;
	ClrOBF;
}
void IO_Read_0064()
{
	vcpu.iobyte = vkbc.status;
}
void IO_Write_0060()
{
	vkbc.inbuf = vcpu.iobyte;
	SetIBF;
	ClrCMD;
}
void IO_Write_0064()
{
	vkbc.inbuf = vcpu.iobyte;
	SetIBF;
	SetCMD;
	switch (vkbc.inbuf) {
	case 0x20:                    /* read keyboard controller's command byte */
		       ClrIBF; vkbc.outbuf = vkbc.control;   SetOBF; break;
	case 0x21: ClrIBF; vkbc.outbuf = vkbc.ram[0x01]; SetOBF; break;
	case 0x22: ClrIBF; vkbc.outbuf = vkbc.ram[0x02]; SetOBF; break;
	case 0x23: ClrIBF; vkbc.outbuf = vkbc.ram[0x03]; SetOBF; break;
	case 0x24: ClrIBF; vkbc.outbuf = vkbc.ram[0x04]; SetOBF; break;
	case 0x25: ClrIBF; vkbc.outbuf = vkbc.ram[0x05]; SetOBF; break;
	case 0x26: ClrIBF; vkbc.outbuf = vkbc.ram[0x06]; SetOBF; break;
	case 0x27: ClrIBF; vkbc.outbuf = vkbc.ram[0x07]; SetOBF; break;
	case 0x28: ClrIBF; vkbc.outbuf = vkbc.ram[0x08]; SetOBF; break;
	case 0x29: ClrIBF; vkbc.outbuf = vkbc.ram[0x09]; SetOBF; break;
	case 0x2a: ClrIBF; vkbc.outbuf = vkbc.ram[0x0a]; SetOBF; break;
	case 0x2b: ClrIBF; vkbc.outbuf = vkbc.ram[0x0b]; SetOBF; break;
	case 0x2c: ClrIBF; vkbc.outbuf = vkbc.ram[0x0c]; SetOBF; break;
	case 0x2d: ClrIBF; vkbc.outbuf = vkbc.ram[0x0d]; SetOBF; break;
	case 0x2e: ClrIBF; vkbc.outbuf = vkbc.ram[0x0e]; SetOBF; break;
	case 0x2f: ClrIBF; vkbc.outbuf = vkbc.ram[0x0f]; SetOBF; break;
	case 0x30: ClrIBF; vkbc.outbuf = vkbc.ram[0x10]; SetOBF; break;
	case 0x31: ClrIBF; vkbc.outbuf = vkbc.ram[0x11]; SetOBF; break;
	case 0x32: ClrIBF; vkbc.outbuf = vkbc.ram[0x12]; SetOBF; break;
	case 0x33: ClrIBF; vkbc.outbuf = vkbc.ram[0x13]; SetOBF; break;
	case 0x34: ClrIBF; vkbc.outbuf = vkbc.ram[0x14]; SetOBF; break;
	case 0x35: ClrIBF; vkbc.outbuf = vkbc.ram[0x15]; SetOBF; break;
	case 0x36: ClrIBF; vkbc.outbuf = vkbc.ram[0x16]; SetOBF; break;
	case 0x37: ClrIBF; vkbc.outbuf = vkbc.ram[0x17]; SetOBF; break;
	case 0x38: ClrIBF; vkbc.outbuf = vkbc.ram[0x18]; SetOBF; break;
	case 0x39: ClrIBF; vkbc.outbuf = vkbc.ram[0x19]; SetOBF; break;
	case 0x3a: ClrIBF; vkbc.outbuf = vkbc.ram[0x1a]; SetOBF; break;
	case 0x3b: ClrIBF; vkbc.outbuf = vkbc.ram[0x1b]; SetOBF; break;
	case 0x3c: ClrIBF; vkbc.outbuf = vkbc.ram[0x1c]; SetOBF; break;
	case 0x3d: ClrIBF; vkbc.outbuf = vkbc.ram[0x1d]; SetOBF; break;
	case 0x3e: ClrIBF; vkbc.outbuf = vkbc.ram[0x1e]; SetOBF; break;
	case 0x3f: ClrIBF; vkbc.outbuf = vkbc.ram[0x1f]; SetOBF; break;
	case 0x60:                   /* write keyboard controller's command byte */
		       ClrIBF; break;
	case 0x61: ClrIBF; break;
	case 0x62: ClrIBF; break;
	case 0x63: ClrIBF; break;
	case 0x64: ClrIBF; break;
	case 0x65: ClrIBF; break;
	case 0x66: ClrIBF; break;
	case 0x67: ClrIBF; break;
	case 0x68: ClrIBF; break;
	case 0x69: ClrIBF; break;
	case 0x6a: ClrIBF; break;
	case 0x6b: ClrIBF; break;
	case 0x6c: ClrIBF; break;
	case 0x6d: ClrIBF; break;
	case 0x6e: ClrIBF; break;
	case 0x6f: ClrIBF; break;
	case 0x70: ClrIBF; break;
	case 0x71: ClrIBF; break;
	case 0x72: ClrIBF; break;
	case 0x73: ClrIBF; break;
	case 0x74: ClrIBF; break;
	case 0x75: ClrIBF; break;
	case 0x76: ClrIBF; break;
	case 0x77: ClrIBF; break;
	case 0x78: ClrIBF; break;
	case 0x79: ClrIBF; break;
	case 0x7a: ClrIBF; break;
	case 0x7b: ClrIBF; break;
	case 0x7c: ClrIBF; break;
	case 0x7d: ClrIBF; break;
	case 0x7e: ClrIBF; break;
	case 0x7f: ClrIBF; break;
	case 0xa4:                                 /* test if password installed */
		ClrIBF;
		vkbc.outbuf = (vkbc.flagpswd) ? 0xfa : 0xf1;
		SetOBF; break;
	case 0xa5: ClrIBF; break;            /* modify pswd and flagpswd in 0x60 */
	case 0xa6:
		ClrIBF;
		if (vkbc.flagpswd) ;  /* TODO: if match, turn on; otherwise turn off */
		else ;                                     /* TODO: turn on keyboard */
		break;
	case 0xa7:                                                /* block mouse */
		ClrIBF;
		SetBit(vkbc.control, VKBC_FLAG_COMMAND_PC_MODE);
		/* NOTE: vmouse.flagclock = 0x00 */
		break;
	case 0xa8:                                               /* enable mouse */
		ClrIBF;
		ClrBit(vkbc.control, VKBC_FLAG_COMMAND_PC_MODE);
		/* NOTE: vmouse.flagclock = 0x01 */
		break;
	case 0xa9:                                            /* test mouse port */
		ClrIBF;
		vkbc.outbuf = 0x01;                           /* mouse not supported */
		SetOBF; break;
	case 0xaa:                                                   /* self test*/
		ClrIBF;
		vkbc.outbuf = 0x55;                                      /* no error */
		SetOBF; break;
		break;
	case 0xab:                                             /* interface test */
		ClrIBF;
		vkbc.outbuf = 0x00;                                      /* no error */
		SetOBF; break;
		break;
	case 0xac:                                            /* diagnostic dump */
		ClrIBF;
		/* TODO: need command detail */
		break;
	case 0xad:                                   /* disable keyboard feature */
		ClrIBF;
		SetBit(vkbc.control, VKBC_FLAG_COMMAND_DISABLE_KEYBOARD);
		/* TODO: vkeyb.flagclock = 0x00; vkeyb.clock = 0x00; */
		break;
	case 0xae:                                  /* enable keyboard interface */
		ClrIBF;
		ClrBit(vkbc.control, VKBC_FLAG_COMMAND_DISABLE_KEYBOARD);
		/* TODO: vkeyb.flagclock = 0x01; */
		break;
	case 0xc0: /* read input port */
		if (0) ;
		break;
	case 0xd0:
		break;
	default:
		break;
	}
	vkbc.control = vcpu.iobyte;
}

void vkbcRefresh()
{}
void vkbcInit()
{
	memset(&vkbc, 0x00, sizeof(t_kbc));
	SetBit(vkbc.status, VKBC_FLAG_STATUS_SYSTEM_FLAG);
	vcpuinsInPort[0x0060] = (t_vaddrcc)IO_Read_0060;
	vcpuinsInPort[0x0064] = (t_vaddrcc)IO_Read_0064;
	vcpuinsOutPort[0x0060] = (t_vaddrcc)IO_Write_0060;
	vcpuinsOutPort[0x0064] = (t_vaddrcc)IO_Write_0064;
}
void vkbcFinal()
{}