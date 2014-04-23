/* This file is a part of NXVM project. */

/* Central Processing Unit: Intel 8086 */

#ifndef NXVM_VCPU_H
#define NXVM_VCPU_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vglobal.h"

#define VCPU_DEBUG

#define VCPU_FLAG_CF 0x0001
#define VCPU_FLAG_PF 0x0004
#define VCPU_FLAG_AF 0x0010
#define VCPU_FLAG_ZF 0x0040
#define VCPU_FLAG_SF 0x0080
#define VCPU_FLAG_TF 0x0100
#define VCPU_FLAG_IF 0x0200
#define VCPU_FLAG_DF 0x0400
#define VCPU_FLAG_OF 0x0800

typedef struct {
	union {
		struct {t_nubit8 al,ah;};
		t_nubit16 ax;
	};
	union {
		struct {t_nubit8 bl,bh;};
		t_nubit16 bx;
	};
	union {
		struct {t_nubit8 cl,ch;};
		t_nubit16 cx;
	};
	union {
		struct {t_nubit8 dl,dh;};
		t_nubit16 dx;
	};
	t_nubit8  iobyte;                               /* for i/o communication */
	t_nubit16 sp,bp,si,di,ip,flags;
	t_nubit16 cs,ds,es,ss;
	t_nubit16 overds, overss;
	t_bool flagnmi;
	t_bool flagterm;
} t_cpu;

#define _ax    (vcpu.ax)
#define _bx    (vcpu.bx)
#define _cx    (vcpu.cx)
#define _dx    (vcpu.dx)
#define _ah    (vcpu.ah)
#define _bh    (vcpu.bh)
#define _ch    (vcpu.ch)
#define _dh    (vcpu.dh)
#define _al    (vcpu.al)
#define _bl    (vcpu.bl)
#define _cl    (vcpu.cl)
#define _dl    (vcpu.dl)
#define _sp    (vcpu.sp)
#define _bp    (vcpu.bp)
#define _si    (vcpu.si)
#define _di    (vcpu.di)
#define _ip    (vcpu.ip)
#define _ds    (vcpu.ds)
#define _cs    (vcpu.cs)
#define _es    (vcpu.es)
#define _ss    (vcpu.ss)
#define _flags (vcpu.flags)
#define _of    (GetBit(vcpu.flags, VCPU_FLAG_OF))
#define _sf    (GetBit(vcpu.flags, VCPU_FLAG_SF))
#define _zf    (GetBit(vcpu.flags, VCPU_FLAG_ZF))
#define _cf    (GetBit(vcpu.flags, VCPU_FLAG_CF))
#define _af    (GetBit(vcpu.flags, VCPU_FLAG_AF))
#define _pf    (GetBit(vcpu.flags, VCPU_FLAG_PF))
#define _df    (GetBit(vcpu.flags, VCPU_FLAG_DF))
#define _tf    (GetBit(vcpu.flags, VCPU_FLAG_TF))
#define _if    (GetBit(vcpu.flags, VCPU_FLAG_IF))

extern t_cpu vcpu;

void vcpuRefresh();
void vcpuInit();
void vcpuFinal();

#ifdef __cplusplus
}
#endif

#endif
