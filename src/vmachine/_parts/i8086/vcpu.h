/* This file is a part of NXVM project. */

/* Central Processing Unit: Intel 8086 */

#ifndef NXVM_VCPU_H
#define NXVM_VCPU_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "vglobal.h"

typedef struct {
	t_nubit16 selector;
} t_cpu_sreg;
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
	t_nubit16 sp,bp,si,di;
	union {
		t_nubit16 ip;
		t_nubit32 eip;
	};
	union {
		t_nubit16 flags;
		t_nubit32 eflags;
	};
	t_cpu_sreg cs,ds,es,ss;
	t_nubit16 overds, overss;
	t_bool flagnmi;
} t_cpu;

extern t_cpu vcpu;

void vcpuInit();
void vcpuReset();
void vcpuRefresh();
void vcpuFinal();

#define _eax    (vcpu.ax)
#define _ebx    (vcpu.bx)
#define _ecx    (vcpu.cx)
#define _edx    (vcpu.dx)
#define _esp    (vcpu.sp)
#define _ebp    (vcpu.bp)
#define _esi    (vcpu.si)
#define _edi    (vcpu.di)
#define _eip    (vcpu.ip)
#define _eflags (vcpu.flags)
#define _fs     0
#define _gs     0
#define _ds     (vcpu.ds.selector)
#define _cs     (vcpu.cs.selector)
#define _es     (vcpu.es.selector)
#define _ss     (vcpu.ss.selector)
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
#define _ip     (vcpu.ip)
#define _flags  (vcpu.flags)

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
/*
#define VCPU_EFLAGS_AC    0x00040000
#define VCPU_EFLAGS_VIF   0x00080000
#define VCPU_EFLAGS_VIP   0x00100000
#define VCPU_EFLAGS_ID    0x00200000*/
#define _GetEFLAGS_CF    (GetBit(vcpu.eflags, VCPU_EFLAGS_CF))
#define _GetEFLAGS_PF    (GetBit(vcpu.eflags, VCPU_EFLAGS_PF))
#define _GetEFLAGS_AF    (GetBit(vcpu.eflags, VCPU_EFLAGS_AF))
#define _GetEFLAGS_ZF    (GetBit(vcpu.eflags, VCPU_EFLAGS_ZF))
#define _GetEFLAGS_SF    (GetBit(vcpu.eflags, VCPU_EFLAGS_SF))
#define _GetEFLAGS_TF    (GetBit(vcpu.eflags, VCPU_EFLAGS_TF))
#define _GetEFLAGS_IF    (GetBit(vcpu.eflags, VCPU_EFLAGS_IF))
#define _GetEFLAGS_DF    (GetBit(vcpu.eflags, VCPU_EFLAGS_DF))
#define _GetEFLAGS_OF    (GetBit(vcpu.eflags, VCPU_EFLAGS_OF))
#define _GetEFLAGS_IOPLL (GetBit(vcpu.eflags, VCPU_EFLAGS_IOPLL))
#define _GetEFLAGS_IOPLH (GetBit(vcpu.eflags, VCPU_EFLAGS_IOPLH))
#define _GetEFLAGS_IOPL  ((vcpu.eflags & VCPU_EFLAGS_IOPL) >> 12)
#define _GetEFLAGS_NT    (GetBit(vcpu.eflags, VCPU_EFLAGS_NT))
#define _GetEFLAGS_RF    (GetBit(vcpu.eflags, VCPU_EFLAGS_RF))
#define _GetEFLAGS_VM    (GetBit(vcpu.eflags, VCPU_EFLAGS_VM))
/*
#define _GetEFLAGS_AC    (GetBit(vcpu.eflags, VCPU_EFLAGS_AC))
#define _GetEFLAGS_VIF   (GetBit(vcpu.eflags, VCPU_EFLAGS_VIF))
#define _GetEFLAGS_VIP   (GetBit(vcpu.eflags, VCPU_EFLAGS_VIP))
#define _GetEFLAGS_ID    (GetBit(vcpu.eflags, VCPU_EFLAGS_ID))*/
#define _SetEFLAGS_CF    (SetBit(vcpu.eflags, VCPU_EFLAGS_CF))
#define _SetEFLAGS_PF    (SetBit(vcpu.eflags, VCPU_EFLAGS_PF))
#define _SetEFLAGS_AF    (SetBit(vcpu.eflags, VCPU_EFLAGS_AF))
#define _SetEFLAGS_ZF    (SetBit(vcpu.eflags, VCPU_EFLAGS_ZF))
#define _SetEFLAGS_SF    (SetBit(vcpu.eflags, VCPU_EFLAGS_SF))
#define _SetEFLAGS_TF    (SetBit(vcpu.eflags, VCPU_EFLAGS_TF))
#define _SetEFLAGS_IF    (SetBit(vcpu.eflags, VCPU_EFLAGS_IF))
#define _SetEFLAGS_DF    (SetBit(vcpu.eflags, VCPU_EFLAGS_DF))
#define _SetEFLAGS_OF    (SetBit(vcpu.eflags, VCPU_EFLAGS_OF))
#define _SetEFLAGS_IOPLL (SetBit(vcpu.eflags, VCPU_EFLAGS_IOPLL))
#define _SetEFLAGS_IOPLH (SetBit(vcpu.eflags, VCPU_EFLAGS_IOPLH))
#define _SetEFLAGS_IOPL  (SetBit(vcpu.eflags, VCPU_EFLAGS_IOPL)
#define _SetEFLAGS_NT    (SetBit(vcpu.eflags, VCPU_EFLAGS_NT))
#define _SetEFLAGS_RF    (SetBit(vcpu.eflags, VCPU_EFLAGS_RF))
#define _SetEFLAGS_VM    (SetBit(vcpu.eflags, VCPU_EFLAGS_VM))
/*
#define _SetEFLAGS_AC    (SetBit(vcpu.eflags, VCPU_EFLAGS_AC))
#define _SetEFLAGS_VIF   (SetBit(vcpu.eflags, VCPU_EFLAGS_VIF))
#define _SetEFLAGS_VIP   (SetBit(vcpu.eflags, VCPU_EFLAGS_VIP))
#define _SetEFLAGS_ID    (SetBit(vcpu.eflags, VCPU_EFLAGS_ID))*/
#define _ClrEFLAGS_CF    (ClrBit(vcpu.eflags, VCPU_EFLAGS_CF))
#define _ClrEFLAGS_PF    (ClrBit(vcpu.eflags, VCPU_EFLAGS_PF))
#define _ClrEFLAGS_AF    (ClrBit(vcpu.eflags, VCPU_EFLAGS_AF))
#define _ClrEFLAGS_ZF    (ClrBit(vcpu.eflags, VCPU_EFLAGS_ZF))
#define _ClrEFLAGS_SF    (ClrBit(vcpu.eflags, VCPU_EFLAGS_SF))
#define _ClrEFLAGS_TF    (ClrBit(vcpu.eflags, VCPU_EFLAGS_TF))
#define _ClrEFLAGS_IF    (ClrBit(vcpu.eflags, VCPU_EFLAGS_IF))
#define _ClrEFLAGS_DF    (ClrBit(vcpu.eflags, VCPU_EFLAGS_DF))
#define _ClrEFLAGS_OF    (ClrBit(vcpu.eflags, VCPU_EFLAGS_OF))
#define _ClrEFLAGS_IOPLL (ClrBit(vcpu.eflags, VCPU_EFLAGS_IOPLL))
#define _ClrEFLAGS_IOPLH (ClrBit(vcpu.eflags, VCPU_EFLAGS_IOPLH))
#define _ClrEFLAGS_IOPL  (ClrBit(vcpu.eflags, VCPU_EFLAGS_IOPL)
#define _ClrEFLAGS_NT    (ClrBit(vcpu.eflags, VCPU_EFLAGS_NT))
#define _ClrEFLAGS_RF    (ClrBit(vcpu.eflags, VCPU_EFLAGS_RF))
#define _ClrEFLAGS_VM    (ClrBit(vcpu.eflags, VCPU_EFLAGS_VM))
/*
#define _ClrEFLAGS_AC    (ClrBit(vcpu.eflags, VCPU_EFLAGS_AC))
#define _ClrEFLAGS_VIF   (ClrBit(vcpu.eflags, VCPU_EFLAGS_VIF))
#define _ClrEFLAGS_VIP   (ClrBit(vcpu.eflags, VCPU_EFLAGS_VIP))
#define _ClrEFLAGS_ID    (ClrBit(vcpu.eflags, VCPU_EFLAGS_ID))*/
 
#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
