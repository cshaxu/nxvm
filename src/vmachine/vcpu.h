/* This file is a part of NXVM project. */

/* Central Processing Unit: Intel 8086e */

#ifndef NXVM_VCPU_H
#define NXVM_VCPU_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "vglobal.h"

#define VCPU_EFLAGS_CF 0x0001
#define VCPU_EFLAGS_PF 0x0004
#define VCPU_EFLAGS_AF 0x0010
#define VCPU_EFLAGS_ZF 0x0040
#define VCPU_EFLAGS_SF 0x0080
#define VCPU_EFLAGS_TF 0x0100
#define VCPU_EFLAGS_IF 0x0200
#define VCPU_EFLAGS_DF 0x0400
#define VCPU_EFLAGS_OF 0x0800

#ifndef ECPUACT
typedef struct {
	union {
		union {
			struct {t_nubit8 al,ah;};
			t_nubit16 ax;
		};
		t_nubit32 eax;
	};
	union {
		union {
			struct {t_nubit8 bl,bh;};
			t_nubit16 bx;
		};
		t_nubit32 ebx;
	};
	union {
		union {
			struct {t_nubit8 cl,ch;};
			t_nubit16 cx;
		};
		t_nubit32 ecx;
	};
	union {
		union {
			struct {t_nubit8 dl,dh;};
			t_nubit16 dx;
		};
		t_nubit32 edx;
	};
	union {
		struct {t_nubit16 sp, hwsp;};
		t_nubit32 esp;
	};
	union {
		struct {t_nubit16 bp, hwbp;};
		t_nubit32 ebp;
	};
	union {
		struct {t_nubit16 si, hwsi;};
		t_nubit32 esi;
	};
	union {
		struct {t_nubit16 di, hwdi;};
		t_nubit32 edi;
	};
	t_nubit32 eip, eflags;
	t_nubit16 cs,ds,ss,es,fs,gs;
	t_nubit16 overds, overss;
	t_bool flagnmi;
} t_cpu;

extern t_cpu vcpu;
#else
#include "ecpu/ecpu.h"
typedef t_ecpu t_cpu;
#define vcpu ecpu
#endif

void vcpuInit();
void vcpuReset();
void vcpuRefresh();
void vcpuFinal();

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
#define _eip   (vcpu.eip)
#define _ds    (vcpu.ds)
#define _cs    (vcpu.cs)
#define _es    (vcpu.es)
#define _ss    (vcpu.ss)
#define _eflags (vcpu.eflags)
#define _of    (GetBit(_eflags, VCPU_EFLAGS_OF))
#define _sf    (GetBit(_eflags, VCPU_EFLAGS_SF))
#define _zf    (GetBit(_eflags, VCPU_EFLAGS_ZF))
#define _cf    (GetBit(_eflags, VCPU_EFLAGS_CF))
#define _af    (GetBit(_eflags, VCPU_EFLAGS_AF))
#define _pf    (GetBit(_eflags, VCPU_EFLAGS_PF))
#define _df    (GetBit(_eflags, VCPU_EFLAGS_DF))
#define _tf    (GetBit(_eflags, VCPU_EFLAGS_TF))
#define _if    (GetBit(_eflags, VCPU_EFLAGS_IF))
#define SetOF  (SetBit(_eflags, VCPU_EFLAGS_OF))
#define SetSF  (SetBit(_eflags, VCPU_EFLAGS_SF))
#define SetZF  (SetBit(_eflags, VCPU_EFLAGS_ZF))
#define SetCF  (SetBit(_eflags, VCPU_EFLAGS_CF))
#define SetAF  (SetBit(_eflags, VCPU_EFLAGS_AF))
#define SetPF  (SetBit(_eflags, VCPU_EFLAGS_PF))
#define SetDF  (SetBit(_eflags, VCPU_EFLAGS_DF))
#define SetTF  (SetBit(_eflags, VCPU_EFLAGS_TF))
#define SetIF  (SetBit(_eflags, VCPU_EFLAGS_IF))
#define ClrOF  (ClrBit(_eflags, VCPU_EFLAGS_OF))
#define ClrSF  (ClrBit(_eflags, VCPU_EFLAGS_SF))
#define ClrZF  (ClrBit(_eflags, VCPU_EFLAGS_ZF))
#define ClrCF  (ClrBit(_eflags, VCPU_EFLAGS_CF))
#define ClrAF  (ClrBit(_eflags, VCPU_EFLAGS_AF))
#define ClrPF  (ClrBit(_eflags, VCPU_EFLAGS_PF))
#define ClrDF  (ClrBit(_eflags, VCPU_EFLAGS_DF))
#define ClrTF  (ClrBit(_eflags, VCPU_EFLAGS_TF))
#define ClrIF  (ClrBit(_eflags, VCPU_EFLAGS_IF))

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
