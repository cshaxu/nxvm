/* This file is a part of NXVM project. */

#include "memory.h"

#include "vcpuins.h"
#include "vcpu.h"
#include "vkbc.h"

#define GetBit(a,b) (!!((a) &   (b)))
#define SetBit(a,b) (   (a) |=  (b))
#define ClrBit(a,b) (   (a) &= ~(b))

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

#define GetIBF (GetBit(vkbc.status, VKBC_FLAG_STATUS_INPUT_BUFFER_FULL))
#define SetIBF (SetBit(vkbc.status, VKBC_FLAG_STATUS_INPUT_BUFFER_FULL))
#define ClrIBF (ClrBit(vkbc.status, VKBC_FLAG_STATUS_INPUT_BUFFER_FULL))
#define GetOBF (GetBit(vkbc.status, VKBC_FLAG_STATUS_OUTPUT_BUFFER_FULL))
#define SetOBF (SetBit(vkbc.status, VKBC_FLAG_STATUS_OUTPUT_BUFFER_FULL))
#define ClrOBF (ClrBit(vkbc.status, VKBC_FLAG_STATUS_OUTPUT_BUFFER_FULL))
#define GetCMD (GetBit(vkbc.status, VKBC_FLAG_STATUS_COMMAND))
#define SetCMD (SetBit(vkbc.status, VKBC_FLAG_STATUS_COMMAND))
#define ClrCMD (ClrBit(vkbc.status, VKBC_FLAG_STATUS_COMMAND))

#define SetIP (vkbc.inport =                                                  \
	((!GetInhibitStatus) << 7) |                                              \
	(0x01                << 6) |                 /* display is mda, not cga */\
	(0x01                << 5) |                    /* jumper not installed */\
	(0x00                << 4) |                   /* disable 2nd 256kb ram */\
	(0x00                << 3) |                                              \
	(0x00                << 2) |                                              \
	(0x00                << 1) |                                              \
	(0x00                << 0))

#define GetInhibitStatus (\
	GetBit(vkbc.control, VKBC_FLAG_COMMAND_INHIBIT_OVERRIDE) ? 0 : \
	GetBit(vkbc.control, VKBC_FLAG_COMMAND_DISABLE_KEYBOARD))

t_kbc vkbc;

/* TODO: REF http://bbs.chinaunix.net/thread-3609756-1-1.html */
/* TODO: REF http://www.cnblogs.com/huqingyu/archive/2005/02/17/105376.html */


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
	if (!GetIBF && GetCMD) {
		switch (vkbc.inbuf) {
		case 0x60: vkbc.control   = vcpu.iobyte; break;
		case 0x61: vkbc.ram[0x01] = vcpu.iobyte; break;
		case 0x62: vkbc.ram[0x02] = vcpu.iobyte; break;
		case 0x63: vkbc.ram[0x03] = vcpu.iobyte; break;
		case 0x64: vkbc.ram[0x04] = vcpu.iobyte; break;
		case 0x65: vkbc.ram[0x05] = vcpu.iobyte; break;
		case 0x66: vkbc.ram[0x06] = vcpu.iobyte; break;
		case 0x67: vkbc.ram[0x07] = vcpu.iobyte; break;
		case 0x68: vkbc.ram[0x08] = vcpu.iobyte; break;
		case 0x69: vkbc.ram[0x09] = vcpu.iobyte; break;
		case 0x6a: vkbc.ram[0x0a] = vcpu.iobyte; break;
		case 0x6b: vkbc.ram[0x0b] = vcpu.iobyte; break;
		case 0x6c: vkbc.ram[0x0c] = vcpu.iobyte; break;
		case 0x6d: vkbc.ram[0x0d] = vcpu.iobyte; break;
		case 0x6e: vkbc.ram[0x0e] = vcpu.iobyte; break;
		case 0x6f: vkbc.ram[0x0f] = vcpu.iobyte; break;
		case 0x70: vkbc.ram[0x10] = vcpu.iobyte; break;
		case 0x71: vkbc.ram[0x11] = vcpu.iobyte; break;
		case 0x72: vkbc.ram[0x12] = vcpu.iobyte; break;
		case 0x73: vkbc.ram[0x13] = vcpu.iobyte; break;
		case 0x74: vkbc.ram[0x14] = vcpu.iobyte; break;
		case 0x75: vkbc.ram[0x15] = vcpu.iobyte; break;
		case 0x76: vkbc.ram[0x16] = vcpu.iobyte; break;
		case 0x77: vkbc.ram[0x17] = vcpu.iobyte; break;
		case 0x78: vkbc.ram[0x18] = vcpu.iobyte; break;
		case 0x79: vkbc.ram[0x19] = vcpu.iobyte; break;
		case 0x7a: vkbc.ram[0x1a] = vcpu.iobyte; break;
		case 0x7b: vkbc.ram[0x1b] = vcpu.iobyte; break;
		case 0x7c: vkbc.ram[0x1c] = vcpu.iobyte; break;
		case 0x7d: vkbc.ram[0x1d] = vcpu.iobyte; break;
		case 0x7e: vkbc.ram[0x1e] = vcpu.iobyte; break;
		case 0x7f: vkbc.ram[0x1f] = vcpu.iobyte; break;
		case 0xa5:                      /* NOTE: set password; not supported */
			break;
		case 0xa6:                   /* NOTE: enable password; not supported */
			break;
		default: break;
		}
		ClrCMD;
		vkbc.inbuf = vcpu.iobyte;
		ClrIBF;
	} else {
		vkbc.inbuf = vcpu.iobyte;
		SetIBF;
		ClrCMD;
	}
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
	case 0xa4: ClrIBF;                         /* test if password installed */
		vkbc.outbuf = (vkbc.flagpswd) ? 0xfa : 0xf1;
		SetOBF; break;
	case 0xa5: ClrIBF; break;        /* NOTE: modify password; not supported */
	case 0xa6: ClrIBF; break;        /* NOTE: enable password; not supported */
	case 0xa7: ClrIBF;                                        /* block mouse */
		SetBit(vkbc.control, VKBC_FLAG_COMMAND_PC_MODE); break;
	case 0xa8: ClrIBF;                                       /* enable mouse */
		ClrBit(vkbc.control, VKBC_FLAG_COMMAND_PC_MODE); break;
	case 0xa9: ClrIBF;                                    /* test mouse port */
		vkbc.outbuf = 0x01;                             /* mouse not enabled */
		SetOBF; break;
	case 0xaa: ClrIBF;                                           /* self test*/
		vkbc.outbuf = 0x55;                                /* keyboard is ok */
		SetOBF; break;
	case 0xab: ClrIBF;                                     /* interface test */
		vkbc.outbuf = 0x00;                             /* no error detected */
		SetOBF; break;
	case 0xac: ClrIBF; break;        /* NOTE: diagnostic dump; not supported */
	case 0xad: ClrIBF;                           /* disable keyboard feature */
		SetBit(vkbc.control, VKBC_FLAG_COMMAND_DISABLE_KEYBOARD); break;
	case 0xae: ClrIBF;                          /* enable keyboard interface */
		ClrBit(vkbc.control, VKBC_FLAG_COMMAND_DISABLE_KEYBOARD); break;
	case 0xc0: ClrIBF;                                 /* read input port p1 */
		/* update vkbc.inport here */
		SetIP;
		vkbc.outbuf = vkbc.inport;
		SetOBF; break;
	case 0xc1:ClrIBF;   /* poll low 4 bits of input port and place in sr 4-7 */
		SetIP;
		vkbc.status = (vkbc.status & 0x0f) | ((vkbc.inport & 0x0f) << 4);
		break;
	case 0xc2: ClrIBF; /* poll high 4 bits of input port and place in sr 4-7 */
		SetIP;
		vkbc.status = (vkbc.status & 0x0f) | (vkbc.inport & 0xf0);
		break;
	case 0xd0:                                        /* read output port p2 */
		ClrIBF;
		SetOBF;
		vkbc.outbuf = (0x00   << 7) |           /* TODO: need command detail */
		              (0x00   << 6) |           /* TODO: need command detail */
		              (GetIBF << 5) |
		              (GetOBF << 4) |
		              (0x00   << 3) |
		              (0x00   << 2) |
		              (0x00   << 1) |                    /* TODO: a20 status */
		              (0x00   << 1);
		break;
	case 0xd1: ClrIBF; break;                        /* write output port p2 */
	case 0xd2: ClrIBF; break;                      /* write to output buffer */
	case 0xd3: ClrIBF; break;                /* write to mouse output buffer */
	case 0xd4: ClrIBF; break;                   /* TODO: need command detail */
	case 0xdd:
		ClrIBF;
		/* TODO: block a20 line */
		break;
	case 0xde:
		ClrIBF;
		/* TODO: enable a20 line */
		break;
	case 0xe0:
		ClrIBF;
		/* TODO: need command detail
		vkbc.outbuf = (vkbc.data  << 1) |
		              (vkbc.clock << 0);
		*/
		SetOBF;
		break;
	case 0xf0:
	case 0xf1:
	case 0xf2:
	case 0xf3:
	case 0xf4:
	case 0xf5:
	case 0xf6:
	case 0xf7:
	case 0xf8:
	case 0xf9:
	case 0xfa:
	case 0xfb:
	case 0xfc:
	case 0xfd:
		ClrIBF;
		/* TODO: l4b of this command controls 4 outbits of p2 */
		/* TODO: need command detail */
		/* NOTE: not used */
		break;
	case 0xfe:
		ClrIBF;
		/* restarts the cpu */
		break;
	default:
		ClrIBF;
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
	vcpuinsInPort[0x0060]  = (t_vaddrcc)IO_Read_0060;
	vcpuinsInPort[0x0064]  = (t_vaddrcc)IO_Read_0064;
	vcpuinsOutPort[0x0060] = (t_vaddrcc)IO_Write_0060;
	vcpuinsOutPort[0x0064] = (t_vaddrcc)IO_Write_0064;
}
void vkbcFinal()
{}
