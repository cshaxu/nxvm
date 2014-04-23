/* This file is a part of NXVM project. */

#ifndef NXVM_VCPU_H
#define NXVM_VCPU_H

#include "vglobal.h"

#define CF 0x0001
#define PF 0x0004
#define AF 0x0010
#define ZF 0x0040
#define SF 0x0080
#define TF 0x0100
#define IF 0x0200
#define DF 0x0400
#define OF 0x0800

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
	t_nubit16 sp,bp,si,di,ip,flags;
	t_nubit16 cs,ds,es,ss;
} t_cpu;

extern t_cpu vcpu;
extern t_bool cpuTermFlag;

void vcpuInsExec();

void CPUInit();
void CPUTerm();

#endif