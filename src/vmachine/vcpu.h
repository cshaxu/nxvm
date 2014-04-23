/* This file is a part of NXVM project. */

/* Central Processing Unit: Intel 8086 */

#ifndef NXVM_VCPU_H
#define NXVM_VCPU_H

#include "vglobal.h"

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
	t_bool flagnmi;
	t_bool flagterm;
} t_cpu;

extern t_cpu vcpu;

void vcpuRefresh();
void vcpuInit();
void vcpuFinal();

#endif
