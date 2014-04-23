/* This file is a part of NXVM project. */

/* Central Processing Unit: Intel 8086e (80386) */

#ifndef NXVM_VCPU_H
#define NXVM_VCPU_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "vglobal.h"

#define VCPU_EFLAGS_CF 0x00000001
#define VCPU_EFLAGS_PF 0x00000004
#define VCPU_EFLAGS_AF 0x00000010
#define VCPU_EFLAGS_ZF 0x00000040
#define VCPU_EFLAGS_SF 0x00000080
#define VCPU_EFLAGS_TF 0x00000100
#define VCPU_EFLAGS_IF 0x00000200
#define VCPU_EFLAGS_DF 0x00000400
#define VCPU_EFLAGS_OF 0x00000800
#define VCPU_EFLAGS_IOPLL 0x00001000
#define VCPU_EFLAGS_IOPLH 0x00002000
#define VCPU_EFLAGS_IOPL  0x00003000
#define VCPU_EFLAGS_NT    0x00004000
#define VCPU_EFLAGS_RF    0x00010000
#define VCPU_EFLAGS_VM    0x00020000
#define VCPU_EFLAGS_AC    0x00040000
#define VCPU_EFLAGS_VIF   0x00080000
#define VCPU_EFLAGS_VIP   0x00100000
#define VCPU_EFLAGS_ID    0x00200000

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
		t_nubit16 sp;
		t_nubit32 esp;
	};
	union {
		t_nubit16 bp;
		t_nubit32 ebp;
	};
	union {
		t_nubit16 si;
		t_nubit32 esi;
	};
	union {
		t_nubit16 di;
		t_nubit32 edi;
	};
	union {
		t_nubit16 ip;
		t_nubit32 eip;
	};
	union {
		t_nubit16 flags;
		t_nubit32 eflags;
	};
	t_nubit16 cs, ss, ds, es, fs, gs;
	t_nubit32 cscr, sscr, dscr, escr, fscr, gscr; /* cache for segments */
	t_nubit16 overds, overss;
	t_nubit32 overdscr, oversscr;
	t_nubit48 gdtr, idtr;
	t_nubit16 ldtr, tr;
	t_nubit32 ldtrcr, trcr;
	t_nubit32 cr0, cr1, cr2, cr3;
	t_nubit32 dr0, dr1, dr2, dr3, dr4, dr5, dr6, dr7;
	t_nubit32 tr6, tr7;
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

#define _eax    (vcpu.eax)
#define _ebx    (vcpu.ebx)
#define _ecx    (vcpu.ecx)
#define _edx    (vcpu.edx)
#define _esp    (vcpu.esp)
#define _ebp    (vcpu.ebp)
#define _esi    (vcpu.esi)
#define _edi    (vcpu.edi)
#define _eip    (vcpu.eip)
#define _ax     (vcpu.ax)
#define _bx     (vcpu.bx)
#define _cx     (vcpu.cx)
#define _dx     (vcpu.dx)
#define _ah     (vcpu.ah)
#define _bh     (vcpu.bh)
#define _ch     (vcpu.ch)
#define _dh     (vcpu.dh)
#define _al     (vcpu.al)
#define _bl     (vcpu.bl)
#define _cl     (vcpu.cl)
#define _dl     (vcpu.dl)
#define _sp     (vcpu.sp)
#define _bp     (vcpu.bp)
#define _si     (vcpu.si)
#define _di     (vcpu.di)
#define _ds     (vcpu.ds)
#define _cs     (vcpu.cs)
#define _ss     (vcpu.ss)
#define _ds     (vcpu.ds)
#define _es     (vcpu.es)
#define _fs     (vcpu.fs)
#define _gs     (vcpu.gs)
#define _eflags (vcpu.eflags)

#define _GetCF    (GetBit(_eflags, VCPU_EFLAGS_CF))
#define _GetPF    (GetBit(_eflags, VCPU_EFLAGS_PF))
#define _GetAF    (GetBit(_eflags, VCPU_EFLAGS_AF))
#define _GetZF    (GetBit(_eflags, VCPU_EFLAGS_ZF))
#define _GetSF    (GetBit(_eflags, VCPU_EFLAGS_SF))
#define _GetTF    (GetBit(_eflags, VCPU_EFLAGS_TF))
#define _GetIF    (GetBit(_eflags, VCPU_EFLAGS_IF))
#define _GetDF    (GetBit(_eflags, VCPU_EFLAGS_DF))
#define _GetOF    (GetBit(_eflags, VCPU_EFLAGS_OF))
#define _GetIOPLL (GetBit(_eflags, VCPU_EFLAGS_IOPLL))
#define _GetIOPLH (GetBit(_eflags, VCPU_EFLAGS_IOPLH))
#define _GetIOPL  ((_eflags & VCPU_EFLAGS_IOPL) >> 12)
#define _GetNT    (GetBit(_eflags, VCPU_EFLAGS_NT))
#define _GetRF    (GetBit(_eflags, VCPU_EFLAGS_RF))
#define _GetVM    (GetBit(_eflags, VCPU_EFLAGS_VM))
/*#define _GetAC    (GetBit(_eflags, VCPU_EFLAGS_AC))
#define _GetVIF   (GetBit(_eflags, VCPU_EFLAGS_VIF))
#define _GetVIP   (GetBit(_eflags, VCPU_EFLAGS_VIP))
#define _GetID    (GetBit(_eflags, VCPU_EFLAGS_ID))*/

#define _SetCF    (SetBit(_eflags, VCPU_EFLAGS_CF))
#define _SetPF    (SetBit(_eflags, VCPU_EFLAGS_PF))
#define _SetAF    (SetBit(_eflags, VCPU_EFLAGS_AF))
#define _SetZF    (SetBit(_eflags, VCPU_EFLAGS_ZF))
#define _SetSF    (SetBit(_eflags, VCPU_EFLAGS_SF))
#define _SetTF    (SetBit(_eflags, VCPU_EFLAGS_TF))
#define _SetIF    (SetBit(_eflags, VCPU_EFLAGS_IF))
#define _SetDF    (SetBit(_eflags, VCPU_EFLAGS_DF))
#define _SetOF    (SetBit(_eflags, VCPU_EFLAGS_OF))
#define _SetIOPLL (SetBit(_eflags, VCPU_EFLAGS_IOPLL))
#define _SetIOPLH (SetBit(_eflags, VCPU_EFLAGS_IOPLH))
#define _SetIOPL  (SetBit(_eflags, VCPU_EFLAGS_IOPL)
#define _SetNT    (SetBit(_eflags, VCPU_EFLAGS_NT))
#define _SetRF    (SetBit(_eflags, VCPU_EFLAGS_RF))
#define _SetVM    (SetBit(_eflags, VCPU_EFLAGS_VM))
/*#define _SetAC    (SetBit(_eflags, VCPU_EFLAGS_AC))
#define _SetVIF   (SetBit(_eflags, VCPU_EFLAGS_VIF))
#define _SetVIP   (SetBit(_eflags, VCPU_EFLAGS_VIP))
#define _SetID    (SetBit(_eflags, VCPU_EFLAGS_ID))*/

#define _ClrCF    (ClrBit(_eflags, VCPU_EFLAGS_CF))
#define _ClrPF    (ClrBit(_eflags, VCPU_EFLAGS_PF))
#define _ClrAF    (ClrBit(_eflags, VCPU_EFLAGS_AF))
#define _ClrZF    (ClrBit(_eflags, VCPU_EFLAGS_ZF))
#define _ClrSF    (ClrBit(_eflags, VCPU_EFLAGS_SF))
#define _ClrTF    (ClrBit(_eflags, VCPU_EFLAGS_TF))
#define _ClrIF    (ClrBit(_eflags, VCPU_EFLAGS_IF))
#define _ClrDF    (ClrBit(_eflags, VCPU_EFLAGS_DF))
#define _ClrOF    (ClrBit(_eflags, VCPU_EFLAGS_OF))
#define _ClrIOPLL (ClrBit(_eflags, VCPU_EFLAGS_IOPLL))
#define _ClrIOPLH (ClrBit(_eflags, VCPU_EFLAGS_IOPLH))
#define _ClrIOPL  (ClrBit(_eflags, VCPU_EFLAGS_IOPL)
#define _ClrNT    (ClrBit(_eflags, VCPU_EFLAGS_NT))
#define _ClrRF    (ClrBit(_eflags, VCPU_EFLAGS_RF))
/*#define _ClrVM    (ClrBit(_eflags, VCPU_EFLAGS_VM))
#define _ClrAC    (ClrBit(_eflags, VCPU_EFLAGS_AC))
#define _ClrVIF   (ClrBit(_eflags, VCPU_EFLAGS_VIF))
#define _ClrVIP   (ClrBit(_eflags, VCPU_EFLAGS_VIP))
#define _ClrID    (ClrBit(_eflags, VCPU_EFLAGS_ID))*/

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
