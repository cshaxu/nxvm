/* This file is a part of NXVM project. */

#include "memory.h"

#include "vcpuins.h"
#include "vcpu.h"
#include "vpic.h"
#include "vkbc.h"

#define GetBit(a,b) (!!((a) &   (b)))
#define SetBit(a,b) (   (a) |=  (b))
#define ClrBit(a,b) (   (a) &= ~(b))

#define VKBC_FLAG_CONTROL_OUTBUF_FULL           0x01
#define VKBC_FLAG_STATUS_INBUF_FULL             0x02
#define VKBC_FLAG_STATUS_SYSTEM_FLAG            0x04
#define VKBC_FLAG_STATUS_COMMAND                0x08
#define VKBC_FLAG_STATUS_INHIBIT_SWITCH         0x10
#define VKBC_FLAG_STATUS_TRANSMIT_TIME_OUT      0x20
#define VKBC_FLAG_STATUS_RECEIVE_TIME_OUT       0x40
#define VKBC_FLAG_STATUS_PARITY_ERROR           0x80

#define VKBC_FLAG_CONTROL_OUTBUF_FULL_INTERRUPT 0x01
#define VKBC_FLAG_CONTROL_RESERVED_BIT_1        0x02
#define VKBC_FLAG_CONTROL_SYSTEM_FLAG           0x04
#define VKBC_FLAG_CONTROL_INHIBIT_OVERRIDE      0x08
#define VKBC_FLAG_CONTROL_DISABLE_KEYBOARD      0x10
#define VKBC_FLAG_CONTROL_PC_MODE               0x20
#define VKBC_FLAG_CONTROL_PC_COMPATIBILITY_MODE 0x40
#define VKBC_FLAG_CONTROL_RESERVED_BIT_7        0x80

#define VKBC_FLAG_OUTPORT_RESET                 0x01
#define VKBC_FLAG_OUTPORT_A20_LINE              0x02
#define VKBC_FLAG_OUTPORT_UNDEFINED_BIT_2       0x04
#define VKBC_FLAG_OUTPORT_UNDEFINED_BIT_3       0x08
#define VKBC_FLAG_OUTPORT_OUTBUF_FULL           0x10
#define VKBC_FLAG_OUTPORT_INBUF_FULL            0x20
#define VKBC_FLAG_OUTPORT_CLOCK_LINE            0x40
#define VKBC_FLAG_OUTPORT_DATA_LINE             0x80

#define GetIBF (GetBit(vkbc.status, VKBC_FLAG_STATUS_INBUF_FULL))
#define SetIBF (SetBit(vkbc.status, VKBC_FLAG_STATUS_INBUF_FULL))
#define ClrIBF (ClrBit(vkbc.status, VKBC_FLAG_STATUS_INBUF_FULL))
#define GetOBF (GetBit(vkbc.status, VKBC_FLAG_CONTROL_OUTBUF_FULL))
#define SetOBF (SetBit(vkbc.status, VKBC_FLAG_CONTROL_OUTBUF_FULL))
#define ClrOBF (ClrBit(vkbc.status, VKBC_FLAG_CONTROL_OUTBUF_FULL))
#define GetCMD (GetBit(vkbc.status, VKBC_FLAG_STATUS_COMMAND))
#define SetCMD (SetBit(vkbc.status, VKBC_FLAG_STATUS_COMMAND))
#define ClrCMD (ClrBit(vkbc.status, VKBC_FLAG_STATUS_COMMAND))

static void SetInBuf(t_nubit8 byte)
{
	vkbc.inbuf = byte;
	SetIBF;
}
static void SetOutBuf(t_nubit8 byte)
{
	vkbc.outbuf = byte;
	SetOBF;
	if (GetBit (vkbc.control, VKBC_FLAG_CONTROL_OUTBUF_FULL_INTERRUPT))
		vpicSetIRQ(0x01);
}
static t_nubit8 GetInBuf()
{
	ClrIBF;
	return vkbc.inbuf;
}
static t_nubit8 GetOutBuf()
{
	ClrOBF;
	return vkbc.outbuf;
}
static void SetFlagA20(t_bool flag)
{
	vkbc.flaga20 = flag;
}
static void SetFlagReset(t_bool flag)
{
	vkbc.flagreset = flag;
	/*  */
}

#define GetInhibitStatus (                                                    \
	GetBit(vkbc.control, VKBC_FLAG_CONTROL_INHIBIT_OVERRIDE) ? 0 :            \
	GetBit(vkbc.control, VKBC_FLAG_CONTROL_DISABLE_KEYBOARD))

#define GetInPort (                                                           \
	((!GetInhibitStatus) << 7) |                                              \
	(vkbc.flagmda        << 6) |                 /* display is mda, not cga */\
	(vkbc.flagjumper     << 5) |                    /* jumper not installed */\
	(vkbc.flagram256     << 4) |                   /* disable 2nd 256kb ram */\
	(0x00                << 3) |                                              \
	(0x00                << 2) |                                              \
	(0x00                << 1) |                                              \
	(0x00                << 0))
#define GetOutPort (                                                          \
	vkbc.flagdata        << 7 |                                               \
	vkbc.flagclock       << 6 |                                               \
	GetIBF               << 5 |                                               \
	GetOBF               << 4 |                                               \
	0x00                 << 3 |                                               \
	0x00                 << 2 |                                               \
	vkbc.flaga20         << 1 |                                               \
	vkbc.flagreset       << 0)

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
	if (GetIBF) return;
	if (GetCMD) {
		switch (vkbc.inbuf) {
		case 0x60: SetInBuf(vcpu.iobyte); vkbc.control   = GetInBuf(); return;
		case 0x61: SetInBuf(vcpu.iobyte); vkbc.ram[0x01] = GetInBuf(); return;
		case 0x62: SetInBuf(vcpu.iobyte); vkbc.ram[0x02] = GetInBuf(); return;
		case 0x63: SetInBuf(vcpu.iobyte); vkbc.ram[0x03] = GetInBuf(); return;
		case 0x64: SetInBuf(vcpu.iobyte); vkbc.ram[0x04] = GetInBuf(); return;
		case 0x65: SetInBuf(vcpu.iobyte); vkbc.ram[0x05] = GetInBuf(); return;
		case 0x66: SetInBuf(vcpu.iobyte); vkbc.ram[0x06] = GetInBuf(); return;
		case 0x67: SetInBuf(vcpu.iobyte); vkbc.ram[0x07] = GetInBuf(); return;
		case 0x68: SetInBuf(vcpu.iobyte); vkbc.ram[0x08] = GetInBuf(); return;
		case 0x69: SetInBuf(vcpu.iobyte); vkbc.ram[0x09] = GetInBuf(); return;
		case 0x6a: SetInBuf(vcpu.iobyte); vkbc.ram[0x0a] = GetInBuf(); return;
		case 0x6b: SetInBuf(vcpu.iobyte); vkbc.ram[0x0b] = GetInBuf(); return;
		case 0x6c: SetInBuf(vcpu.iobyte); vkbc.ram[0x0c] = GetInBuf(); return;
		case 0x6d: SetInBuf(vcpu.iobyte); vkbc.ram[0x0d] = GetInBuf(); return;
		case 0x6e: SetInBuf(vcpu.iobyte); vkbc.ram[0x0e] = GetInBuf(); return;
		case 0x6f: SetInBuf(vcpu.iobyte); vkbc.ram[0x0f] = GetInBuf(); return;
		case 0x70: SetInBuf(vcpu.iobyte); vkbc.ram[0x10] = GetInBuf(); return;
		case 0x71: SetInBuf(vcpu.iobyte); vkbc.ram[0x11] = GetInBuf(); return;
		case 0x72: SetInBuf(vcpu.iobyte); vkbc.ram[0x12] = GetInBuf(); return;
		case 0x73: SetInBuf(vcpu.iobyte); vkbc.ram[0x13] = GetInBuf(); return;
		case 0x74: SetInBuf(vcpu.iobyte); vkbc.ram[0x14] = GetInBuf(); return;
		case 0x75: SetInBuf(vcpu.iobyte); vkbc.ram[0x15] = GetInBuf(); return;
		case 0x76: SetInBuf(vcpu.iobyte); vkbc.ram[0x16] = GetInBuf(); return;
		case 0x77: SetInBuf(vcpu.iobyte); vkbc.ram[0x17] = GetInBuf(); return;
		case 0x78: SetInBuf(vcpu.iobyte); vkbc.ram[0x18] = GetInBuf(); return;
		case 0x79: SetInBuf(vcpu.iobyte); vkbc.ram[0x19] = GetInBuf(); return;
		case 0x7a: SetInBuf(vcpu.iobyte); vkbc.ram[0x1a] = GetInBuf(); return;
		case 0x7b: SetInBuf(vcpu.iobyte); vkbc.ram[0x1b] = GetInBuf(); return;
		case 0x7c: SetInBuf(vcpu.iobyte); vkbc.ram[0x1c] = GetInBuf(); return;
		case 0x7d: SetInBuf(vcpu.iobyte); vkbc.ram[0x1d] = GetInBuf(); return;
		case 0x7e: SetInBuf(vcpu.iobyte); vkbc.ram[0x1e] = GetInBuf(); return;
		case 0x7f: SetInBuf(vcpu.iobyte); vkbc.ram[0x1f] = GetInBuf(); return;
		case 0xa5: SetInBuf(vcpu.iobyte);                  GetInBuf();
			                            /* NOTE: set password; not supported */
			return;
		case 0xa6: SetInBuf(vcpu.iobyte);                  GetInBuf();
			                         /* NOTE: enable password; not supported */
			return;
		case 0xd1: SetInBuf(vcpu.iobyte);                  GetInBuf();
			vkbc.flagdata  = !!(vcpu.iobyte & VKBC_FLAG_OUTPORT_DATA_LINE);
			vkbc.flagclock = !!(vcpu.iobyte & VKBC_FLAG_OUTPORT_CLOCK_LINE);
			      /* TODO: need to do something when data/clock line changes */
			if (!!(vcpu.iobyte & VKBC_FLAG_OUTPORT_OUTBUF_FULL))
				SetOutBuf(vkbc.outbuf);
			vkbc.flaga20   = !!(vcpu.iobyte & VKBC_FLAG_OUTPORT_A20_LINE);
			vkbc.flagreset = !!(vcpu.iobyte & VKBC_FLAG_OUTPORT_RESET);
			return;
		case 0xd2: SetInBuf(vcpu.iobyte);
			SetOutBuf(GetInBuf());                   /* TODO: need to verify */
			return;
		case 0xd3: SetInBuf(vcpu.iobyte);                  GetInBuf();
			               /* NOTE: write mouse output buffer; not supported */
			return;
		case 0xd4: SetInBuf(vcpu.iobyte);                  GetInBuf();
			                     /* NOTE: write data to mouse; not supported */
			return;
		default: break;
		}
	}
	SetInBuf(vcpu.iobyte);
	ClrCMD;
	switch (GetInBuf()) {
	default: break;
	}
}
void IO_Write_0064()
{
	SetInBuf(vcpu.iobyte);
	SetCMD;
	switch (GetInBuf()) {                           /* TODO: need to verify  */
	case 0x20:                    /* read keyboard controller's command byte */
		       SetOutBuf(vkbc.control);   break;
	case 0x21: SetOutBuf(vkbc.ram[0x01]); break;
	case 0x22: SetOutBuf(vkbc.ram[0x02]); break;
	case 0x23: SetOutBuf(vkbc.ram[0x03]); break;
	case 0x24: SetOutBuf(vkbc.ram[0x04]); break;
	case 0x25: SetOutBuf(vkbc.ram[0x05]); break;
	case 0x26: SetOutBuf(vkbc.ram[0x06]); break;
	case 0x27: SetOutBuf(vkbc.ram[0x07]); break;
	case 0x28: SetOutBuf(vkbc.ram[0x08]); break;
	case 0x29: SetOutBuf(vkbc.ram[0x09]); break;
	case 0x2a: SetOutBuf(vkbc.ram[0x0a]); break;
	case 0x2b: SetOutBuf(vkbc.ram[0x0b]); break;
	case 0x2c: SetOutBuf(vkbc.ram[0x0c]); break;
	case 0x2d: SetOutBuf(vkbc.ram[0x0d]); break;
	case 0x2e: SetOutBuf(vkbc.ram[0x0e]); break;
	case 0x2f: SetOutBuf(vkbc.ram[0x0f]); break;
	case 0x30: SetOutBuf(vkbc.ram[0x10]); break;
	case 0x31: SetOutBuf(vkbc.ram[0x11]); break;
	case 0x32: SetOutBuf(vkbc.ram[0x12]); break;
	case 0x33: SetOutBuf(vkbc.ram[0x13]); break;
	case 0x34: SetOutBuf(vkbc.ram[0x14]); break;
	case 0x35: SetOutBuf(vkbc.ram[0x15]); break;
	case 0x36: SetOutBuf(vkbc.ram[0x16]); break;
	case 0x37: SetOutBuf(vkbc.ram[0x17]); break;
	case 0x38: SetOutBuf(vkbc.ram[0x18]); break;
	case 0x39: SetOutBuf(vkbc.ram[0x19]); break;
	case 0x3a: SetOutBuf(vkbc.ram[0x1a]); break;
	case 0x3b: SetOutBuf(vkbc.ram[0x1b]); break;
	case 0x3c: SetOutBuf(vkbc.ram[0x1c]); break;
	case 0x3d: SetOutBuf(vkbc.ram[0x1d]); break;
	case 0x3e: SetOutBuf(vkbc.ram[0x1e]); break;
	case 0x3f: SetOutBuf(vkbc.ram[0x1f]); break;
	case 0x60:                   /* write keyboard controller's command byte */
		       break;
	case 0x61: break;
	case 0x62: break;
	case 0x63: break;
	case 0x64: break;
	case 0x65: break;
	case 0x66: break;
	case 0x67: break;
	case 0x68: break;
	case 0x69: break;
	case 0x6a: break;
	case 0x6b: break;
	case 0x6c: break;
	case 0x6d: break;
	case 0x6e: break;
	case 0x6f: break;
	case 0x70: break;
	case 0x71: break;
	case 0x72: break;
	case 0x73: break;
	case 0x74: break;
	case 0x75: break;
	case 0x76: break;
	case 0x77: break;
	case 0x78: break;
	case 0x79: break;
	case 0x7a: break;
	case 0x7b: break;
	case 0x7c: break;
	case 0x7d: break;
	case 0x7e: break;
	case 0x7f: break;
	case 0xa4:                                 /* test if password installed */
		SetOutBuf((vkbc.flagpswd) ? 0xfa : 0xf1); break;
	case 0xa5: break;                /* NOTE: modify password; not supported */
	case 0xa6: break;                /* NOTE: enable password; not supported */
	case 0xa7:                                                /* block mouse */
		SetBit(vkbc.control, VKBC_FLAG_CONTROL_PC_MODE); break;
	case 0xa8:                                               /* enable mouse */
		ClrBit(vkbc.control, VKBC_FLAG_CONTROL_PC_MODE); break;
	case 0xa9: SetOutBuf(0x01); break;          /* test mouse port; disabled */
	case 0xaa: SetOutBuf(0x55); break;          /* self test; keyboard is ok */
	case 0xab: SetOutBuf(0x00); break;           /* interface test; no error */
	case 0xac: break;                /* NOTE: diagnostic dump; not supported */
	case 0xad:                                   /* disable keyboard feature */
		SetBit(vkbc.control, VKBC_FLAG_CONTROL_DISABLE_KEYBOARD); break;
	case 0xae:                                  /* enable keyboard interface */
		ClrBit(vkbc.control, VKBC_FLAG_CONTROL_DISABLE_KEYBOARD); break;
	case 0xc0: break;             /* NOTE: read input port p1; not supported */
	case 0xc1: break;
   /* NOTE: poll low 4 bits of input port and place in sr 4-7; not supported */
	case 0xc2: break;
  /* NOTE: poll high 4 bits of input port and place in sr 4-7; not supported */
	case 0xd0: SetOBF; SetOutBuf(GetOutPort); break;/* ! read output port p2 */
	case 0xd1: break;                                /* write output port p2 */
	case 0xd2: break;                              /* write to output buffer */
	case 0xd3: break;                        /* write to mouse output buffer */
	case 0xd4: break;                                 /* write data to mouse */
	case 0xdd: vkbc.flaga20 = 0x00; break;               /* disable a20 line */
	case 0xde: vkbc.flaga20 = 0x01; break;                /* enable a20 line */
	case 0xe0:                                            /* read test input */
		SetOutBuf((vkbc.flagdata << 1) | (vkbc.flagclock << 0)); break;
	case 0xf0: break;           /* NOTE: pulse output port p2; not supported */
	case 0xf1: break;           /* NOTE: pulse output port p2; not supported */
	case 0xf2: break;           /* NOTE: pulse output port p2; not supported */
	case 0xf3: break;           /* NOTE: pulse output port p2; not supported */
	case 0xf4: break;           /* NOTE: pulse output port p2; not supported */
	case 0xf5: break;           /* NOTE: pulse output port p2; not supported */
	case 0xf6: break;           /* NOTE: pulse output port p2; not supported */
	case 0xf7: break;           /* NOTE: pulse output port p2; not supported */
	case 0xf8: break;           /* NOTE: pulse output port p2; not supported */
	case 0xf9: break;           /* NOTE: pulse output port p2; not supported */
	case 0xfa: break;           /* NOTE: pulse output port p2; not supported */
	case 0xfb: break;           /* NOTE: pulse output port p2; not supported */
	case 0xfc: break;           /* NOTE: pulse output port p2; not supported */
	case 0xfd: break;           /* NOTE: pulse output port p2; not supported */
	case 0xfe: vkbc.flagreset = 0x01; break;       /* TODO: restarts the cpu */
	case 0xff: break;                                          /* do nothing */
	default:   break;                                     /* invalid command */
	}
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
