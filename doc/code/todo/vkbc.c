/* This file is a part of NXVM project. */

#include "memory.h"

#include "../system/vapi.h"
#include "vcpuins.h"
#include "vcpu.h"
#include "vpic.h"
#include "vkeyb.h"
#include "vkbc.h"

/* NOTE: references
 * http://bbs.chinaunix.net/thread-3609756-1-1.html
 * http://www.cnblogs.com/huqingyu/archive/2005/02/17/105376.html
 * http://blog.csdn.net/luo_brian/article/details/8460334
 */

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

static void SetInBuf(t_nubit8 byte) {
    vkbc.inbuf = byte;
    SetIBF;
}
static void SetOutBuf(t_nubit8 byte) {
    vkbc.outbuf = byte;
    SetOBF;
    if (GetBit (vkbc.control, VKBC_FLAG_CONTROL_OUTBUF_FULL_INTERRUPT))
        vpicSetIRQ(0x01);
}
static t_nubit8 GetInBuf() {
    ClrIBF;
    return vkbc.inbuf;
}
static t_nubit8 GetOutBuf() {
    t_nubit8 out = vkbc.outbuf;
    if (vkeybBufSize()) {
        vkbc.outbuf = vkeybBufPop();
        SetOBF;
    } else ClrOBF;
    return out;
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

void IO_Read_0060() {
    if (!GetOBF) return;
    vcpu.iobyte = GetOutBuf();
}
void IO_Read_0064() {
    vcpu.iobyte = vkbc.status;
}
void IO_Write_0060() {
    t_nubit8 cmdbyte = 0x00;
    if (GetIBF) return;
    if (GetCMD) cmdbyte = vkbc.inbuf;
    SetInBuf(vcpu.iobyte);
    if (GetCMD) {
        ClrCMD;
        switch (cmdbyte) {
        case 0x60:
            vkbc.control   = GetInBuf();
            break;
        case 0x61:
            vkbc.ram[0x01] = GetInBuf();
            break;
        case 0x62:
            vkbc.ram[0x02] = GetInBuf();
            break;
        case 0x63:
            vkbc.ram[0x03] = GetInBuf();
            break;
        case 0x64:
            vkbc.ram[0x04] = GetInBuf();
            break;
        case 0x65:
            vkbc.ram[0x05] = GetInBuf();
            break;
        case 0x66:
            vkbc.ram[0x06] = GetInBuf();
            break;
        case 0x67:
            vkbc.ram[0x07] = GetInBuf();
            break;
        case 0x68:
            vkbc.ram[0x08] = GetInBuf();
            break;
        case 0x69:
            vkbc.ram[0x09] = GetInBuf();
            break;
        case 0x6a:
            vkbc.ram[0x0a] = GetInBuf();
            break;
        case 0x6b:
            vkbc.ram[0x0b] = GetInBuf();
            break;
        case 0x6c:
            vkbc.ram[0x0c] = GetInBuf();
            break;
        case 0x6d:
            vkbc.ram[0x0d] = GetInBuf();
            break;
        case 0x6e:
            vkbc.ram[0x0e] = GetInBuf();
            break;
        case 0x6f:
            vkbc.ram[0x0f] = GetInBuf();
            break;
        case 0x70:
            vkbc.ram[0x10] = GetInBuf();
            break;
        case 0x71:
            vkbc.ram[0x11] = GetInBuf();
            break;
        case 0x72:
            vkbc.ram[0x12] = GetInBuf();
            break;
        case 0x73:
            vkbc.ram[0x13] = GetInBuf();
            break;
        case 0x74:
            vkbc.ram[0x14] = GetInBuf();
            break;
        case 0x75:
            vkbc.ram[0x15] = GetInBuf();
            break;
        case 0x76:
            vkbc.ram[0x16] = GetInBuf();
            break;
        case 0x77:
            vkbc.ram[0x17] = GetInBuf();
            break;
        case 0x78:
            vkbc.ram[0x18] = GetInBuf();
            break;
        case 0x79:
            vkbc.ram[0x19] = GetInBuf();
            break;
        case 0x7a:
            vkbc.ram[0x1a] = GetInBuf();
            break;
        case 0x7b:
            vkbc.ram[0x1b] = GetInBuf();
            break;
        case 0x7c:
            vkbc.ram[0x1c] = GetInBuf();
            break;
        case 0x7d:
            vkbc.ram[0x1d] = GetInBuf();
            break;
        case 0x7e:
            vkbc.ram[0x1e] = GetInBuf();
            break;
        case 0x7f:
            vkbc.ram[0x1f] = GetInBuf();
            break;
        case 0xd1:
            vkbc.inbuf     = GetInBuf();
            vkbc.flagdata  = !!(vkbc.inbuf & VKBC_FLAG_OUTPORT_DATA_LINE);
            vkbc.flagclock = !!(vkbc.inbuf & VKBC_FLAG_OUTPORT_CLOCK_LINE);
            if (!!(vkbc.inbuf & VKBC_FLAG_OUTPORT_OUTBUF_FULL))
                SetOutBuf(vkbc.outbuf);
            vkbc.flaga20   = !!(vkbc.inbuf & VKBC_FLAG_OUTPORT_A20_LINE);
            vkbc.flagreset = !!(vkbc.inbuf & VKBC_FLAG_OUTPORT_RESET);
            break;
        case 0xd2:
            SetOutBuf(GetInBuf());
            break;
        default:                         /* invalid parameter, could be data */
            break;
        }
    }
    if (GetIBF) vkeybSetInput(GetInBuf());
}
void IO_Write_0064() {
    if (GetIBF) return;
    SetInBuf(vcpu.iobyte);
    SetCMD;
    switch (GetInBuf()) {
    case 0x20:                    /* read keyboard controller's command byte */
        SetOutBuf(vkbc.control);
        break;
    case 0x21:
        SetOutBuf(vkbc.ram[0x01]);
        break;
    case 0x22:
        SetOutBuf(vkbc.ram[0x02]);
        break;
    case 0x23:
        SetOutBuf(vkbc.ram[0x03]);
        break;
    case 0x24:
        SetOutBuf(vkbc.ram[0x04]);
        break;
    case 0x25:
        SetOutBuf(vkbc.ram[0x05]);
        break;
    case 0x26:
        SetOutBuf(vkbc.ram[0x06]);
        break;
    case 0x27:
        SetOutBuf(vkbc.ram[0x07]);
        break;
    case 0x28:
        SetOutBuf(vkbc.ram[0x08]);
        break;
    case 0x29:
        SetOutBuf(vkbc.ram[0x09]);
        break;
    case 0x2a:
        SetOutBuf(vkbc.ram[0x0a]);
        break;
    case 0x2b:
        SetOutBuf(vkbc.ram[0x0b]);
        break;
    case 0x2c:
        SetOutBuf(vkbc.ram[0x0c]);
        break;
    case 0x2d:
        SetOutBuf(vkbc.ram[0x0d]);
        break;
    case 0x2e:
        SetOutBuf(vkbc.ram[0x0e]);
        break;
    case 0x2f:
        SetOutBuf(vkbc.ram[0x0f]);
        break;
    case 0x30:
        SetOutBuf(vkbc.ram[0x10]);
        break;
    case 0x31:
        SetOutBuf(vkbc.ram[0x11]);
        break;
    case 0x32:
        SetOutBuf(vkbc.ram[0x12]);
        break;
    case 0x33:
        SetOutBuf(vkbc.ram[0x13]);
        break;
    case 0x34:
        SetOutBuf(vkbc.ram[0x14]);
        break;
    case 0x35:
        SetOutBuf(vkbc.ram[0x15]);
        break;
    case 0x36:
        SetOutBuf(vkbc.ram[0x16]);
        break;
    case 0x37:
        SetOutBuf(vkbc.ram[0x17]);
        break;
    case 0x38:
        SetOutBuf(vkbc.ram[0x18]);
        break;
    case 0x39:
        SetOutBuf(vkbc.ram[0x19]);
        break;
    case 0x3a:
        SetOutBuf(vkbc.ram[0x1a]);
        break;
    case 0x3b:
        SetOutBuf(vkbc.ram[0x1b]);
        break;
    case 0x3c:
        SetOutBuf(vkbc.ram[0x1c]);
        break;
    case 0x3d:
        SetOutBuf(vkbc.ram[0x1d]);
        break;
    case 0x3e:
        SetOutBuf(vkbc.ram[0x1e]);
        break;
    case 0x3f:
        SetOutBuf(vkbc.ram[0x1f]);
        break;
    case 0x60:                   /* write keyboard controller's command byte */
        break;
    case 0x61:
        break;
    case 0x62:
        break;
    case 0x63:
        break;
    case 0x64:
        break;
    case 0x65:
        break;
    case 0x66:
        break;
    case 0x67:
        break;
    case 0x68:
        break;
    case 0x69:
        break;
    case 0x6a:
        break;
    case 0x6b:
        break;
    case 0x6c:
        break;
    case 0x6d:
        break;
    case 0x6e:
        break;
    case 0x6f:
        break;
    case 0x70:
        break;
    case 0x71:
        break;
    case 0x72:
        break;
    case 0x73:
        break;
    case 0x74:
        break;
    case 0x75:
        break;
    case 0x76:
        break;
    case 0x77:
        break;
    case 0x78:
        break;
    case 0x79:
        break;
    case 0x7a:
        break;
    case 0x7b:
        break;
    case 0x7c:
        break;
    case 0x7d:
        break;
    case 0x7e:
        break;
    case 0x7f:
        break;
    case 0xa4:                                 /* test if password installed */
        SetOutBuf((vkbc.flagpswd) ? 0xfa : 0xf1);
        break;
    case 0xa5:
        break;                /* NOTE: modify password; not supported */
    case 0xa6:
        break;                /* NOTE: enable password; not supported */
    case 0xa7:                                                /* block mouse */
        SetBit(vkbc.control, VKBC_FLAG_CONTROL_PC_MODE);
        break;
    case 0xa8:                                               /* enable mouse */
        ClrBit(vkbc.control, VKBC_FLAG_CONTROL_PC_MODE);
        break;
    case 0xa9:
        SetOutBuf(0x01);
        break;    /* NOTE: test mouse port; disabled */
    case 0xaa:
        SetOutBuf(0x55);
        break;          /* self test; keyboard is ok */
    case 0xab:
        SetOutBuf(0x00);
        break;           /* interface test; no error */
    case 0xac:
        break;                /* NOTE: diagnostic dump; not supported */
    case 0xad:                                   /* disable keyboard feature */
        SetBit(vkbc.control, VKBC_FLAG_CONTROL_DISABLE_KEYBOARD);
        break;
    case 0xae:                                  /* enable keyboard interface */
        ClrBit(vkbc.control, VKBC_FLAG_CONTROL_DISABLE_KEYBOARD);
        break;
    case 0xc0:
        break;             /* NOTE: read input port p1; not supported */
    case 0xc1:
        break;
    /* NOTE: poll low 4 bits of input port and place in sr 4-7; not supported */
    case 0xc2:
        break;
    /* NOTE: poll high 4 bits of input port and place in sr 4-7; not supported */
    case 0xd0:
        SetOBF;
        SetOutBuf(GetOutPort);
        break;/* ! read output port p2 */
    case 0xd1:
        break;                                /* write output port p2 */
    case 0xd2:
        break;                              /* write to output buffer */
    case 0xd3:
        break;   /* NOTE: write to mouse output buffer; not supported */
    case 0xd4:
        break;            /* NOTE: write data to mouse; not supported */
    case 0xdd:
        vkbc.flaga20 = 0x00;
        break;               /* disable a20 line */
    case 0xde:
        vkbc.flaga20 = 0x01;
        break;                /* enable a20 line */
    case 0xe0:                                            /* read test input */
        SetOutBuf((vkbc.flagdata << 1) | (vkbc.flagclock << 0));
        break;
    case 0xf0:
        break;           /* NOTE: pulse output port p2; not supported */
    case 0xf1:
        break;           /* NOTE: pulse output port p2; not supported */
    case 0xf2:
        break;           /* NOTE: pulse output port p2; not supported */
    case 0xf3:
        break;           /* NOTE: pulse output port p2; not supported */
    case 0xf4:
        break;           /* NOTE: pulse output port p2; not supported */
    case 0xf5:
        break;           /* NOTE: pulse output port p2; not supported */
    case 0xf6:
        break;           /* NOTE: pulse output port p2; not supported */
    case 0xf7:
        break;           /* NOTE: pulse output port p2; not supported */
    case 0xf8:
        break;           /* NOTE: pulse output port p2; not supported */
    case 0xf9:
        break;           /* NOTE: pulse output port p2; not supported */
    case 0xfa:
        break;           /* NOTE: pulse output port p2; not supported */
    case 0xfb:
        break;           /* NOTE: pulse output port p2; not supported */
    case 0xfc:
        break;           /* NOTE: pulse output port p2; not supported */
    case 0xfd:
        break;           /* NOTE: pulse output port p2; not supported */
    case 0xfe:
        vkbc.flagreset = 0x01;
        break;             /* restarts the cpu */
    case 0xff:
        break;                                          /* do nothing */
    default:
        break;                                     /* invalid command */
    }
}

void vkbcSetResponse(t_nubit8 byte) {
    SetOutBuf(byte);
}
void vkbcRefresh() {
}
#ifdef VKBC_DEBUG
void IO_Read_FF60() {
    vapiPrint("KBC Info\n========\n");
    vapiPrint("status = %x, control = %x, inbuf = %x, outbuf = %x\n",
              vkbc.status, vkbc.control, vkbc.inbuf, vkbc.outbuf);
    vapiPrint("kbd.bufptr = %x, kbd.bufsize = %x\n",
              vkeyb.bufptr, vkeyb.bufsize);
}
#define mov(n) (vcpu.iobyte=(n))
#define out(n) FUNEXEC(vcpuinsOutPort[(n)])
#endif
void vkbcInit() {
    memset(&vkbc, 0x00, sizeof(t_kbc));
    SetBit(vkbc.status, VKBC_FLAG_STATUS_SYSTEM_FLAG);
    vcpuinsInPort[0x0060]  = (t_faddrcc)IO_Read_0060;
    vcpuinsInPort[0x0064]  = (t_faddrcc)IO_Read_0064;
    vcpuinsOutPort[0x0060] = (t_faddrcc)IO_Write_0060;
    vcpuinsOutPort[0x0064] = (t_faddrcc)IO_Write_0064;
#ifdef VKBC_DEBUG
    vcpuinsInPort[0xff60]  = (t_faddrcc)IO_Read_FF60;
    mov(0x60);
    out(0x64);
    mov(0x01);
    out(0x60);
#endif
}
void vkbcFinal() {
}
