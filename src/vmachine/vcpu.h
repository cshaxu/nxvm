/* This file is a part of NXVM project. */

/* Central Processing Unit: Intel 8086e (80386) */

#ifndef NXVM_VCPU_H
#define NXVM_VCPU_H

#ifdef __cplusplus
/*extern "C" {*/
#endif

#include "vglobal.h"

#ifndef ECPUACT
typedef enum {SREG_CODE, SREG_DATA, SREG_STACK, SREG_LDTR, SREG_TR} t_cpu_sreg_type;
typedef struct {
	t_nubit16 selector;
	/* invisible portion/descriptor part */
	t_cpu_sreg_type sregtype;
	t_nubit32 base;
	t_nubit32 limit;
	t_nubit8  dpl; /* if segment is cs, this is cpl */
	union {
		struct {
			t_bool accessed;
			union {
				struct {
					t_bool defsize; /* 16-bit (0) or 32-bit (1) */
					t_bool conform;
					t_bool readable;
				} exec;
				struct {
					t_bool big;
					t_bool expdown;
					t_bool writable;
				} data;
			};
		} seg;
		struct {
			t_nubit4 type;
		} sys;
	};
} t_cpu_sreg;
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
	t_cpu_sreg cs, ss, ds, es, fs, gs, ldtr, tr;
	t_nubit48 gdtr, idtr;
	t_nubit32 ldtrcr, trcr;
	t_nubit32 cr0, cr1, cr2, cr3;
	t_nubit32 dr0, dr1, dr2, dr3, dr4, dr5, dr6, dr7;
	t_nubit32 tr6, tr7;
	t_bool flagnmi, flaglock, flaghalt;
} t_cpu;

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
#define _ds     (vcpu.ds.selector)
#define _cs     (vcpu.cs.selector)
#define _ss     (vcpu.ss.selector)
#define _ds     (vcpu.ds.selector)
#define _es     (vcpu.es.selector)
#define _fs     (vcpu.fs.selector)
#define _gs     (vcpu.gs.selector)
#define _eflags (vcpu.eflags)
#define _gdtr   (vcpu.gdtr)
#define _idtr   (vcpu.idtr)
#define _ldtr   (vcpu.ldtr)
#define _tr     (vcpu.tr)

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
#define _GetEFLAGS_CF    (GetBit(_eflags, VCPU_EFLAGS_CF))
#define _GetEFLAGS_PF    (GetBit(_eflags, VCPU_EFLAGS_PF))
#define _GetEFLAGS_AF    (GetBit(_eflags, VCPU_EFLAGS_AF))
#define _GetEFLAGS_ZF    (GetBit(_eflags, VCPU_EFLAGS_ZF))
#define _GetEFLAGS_SF    (GetBit(_eflags, VCPU_EFLAGS_SF))
#define _GetEFLAGS_TF    (GetBit(_eflags, VCPU_EFLAGS_TF))
#define _GetEFLAGS_IF    (GetBit(_eflags, VCPU_EFLAGS_IF))
#define _GetEFLAGS_DF    (GetBit(_eflags, VCPU_EFLAGS_DF))
#define _GetEFLAGS_OF    (GetBit(_eflags, VCPU_EFLAGS_OF))
#define _GetEFLAGS_IOPLL (GetBit(_eflags, VCPU_EFLAGS_IOPLL))
#define _GetEFLAGS_IOPLH (GetBit(_eflags, VCPU_EFLAGS_IOPLH))
#define _GetEFLAGS_IOPL  ((_eflags & VCPU_EFLAGS_IOPL) >> 12)
#define _GetEFLAGS_NT    (GetBit(_eflags, VCPU_EFLAGS_NT))
#define _GetEFLAGS_RF    (GetBit(_eflags, VCPU_EFLAGS_RF))
#define _GetEFLAGS_VM    (GetBit(_eflags, VCPU_EFLAGS_VM))
/*
#define _GetEFLAGS_AC    (GetBit(_eflags, VCPU_EFLAGS_AC))
#define _GetEFLAGS_VIF   (GetBit(_eflags, VCPU_EFLAGS_VIF))
#define _GetEFLAGS_VIP   (GetBit(_eflags, VCPU_EFLAGS_VIP))
#define _GetEFLAGS_ID    (GetBit(_eflags, VCPU_EFLAGS_ID))*/
#define _SetEFLAGS_CF    (SetBit(_eflags, VCPU_EFLAGS_CF))
#define _SetEFLAGS_PF    (SetBit(_eflags, VCPU_EFLAGS_PF))
#define _SetEFLAGS_AF    (SetBit(_eflags, VCPU_EFLAGS_AF))
#define _SetEFLAGS_ZF    (SetBit(_eflags, VCPU_EFLAGS_ZF))
#define _SetEFLAGS_SF    (SetBit(_eflags, VCPU_EFLAGS_SF))
#define _SetEFLAGS_TF    (SetBit(_eflags, VCPU_EFLAGS_TF))
#define _SetEFLAGS_IF    (SetBit(_eflags, VCPU_EFLAGS_IF))
#define _SetEFLAGS_DF    (SetBit(_eflags, VCPU_EFLAGS_DF))
#define _SetEFLAGS_OF    (SetBit(_eflags, VCPU_EFLAGS_OF))
#define _SetEFLAGS_IOPLL (SetBit(_eflags, VCPU_EFLAGS_IOPLL))
#define _SetEFLAGS_IOPLH (SetBit(_eflags, VCPU_EFLAGS_IOPLH))
#define _SetEFLAGS_IOPL  (SetBit(_eflags, VCPU_EFLAGS_IOPL)
#define _SetEFLAGS_NT    (SetBit(_eflags, VCPU_EFLAGS_NT))
#define _SetEFLAGS_RF    (SetBit(_eflags, VCPU_EFLAGS_RF))
#define _SetEFLAGS_VM    (SetBit(_eflags, VCPU_EFLAGS_VM))
/*
#define _SetEFLAGS_AC    (SetBit(_eflags, VCPU_EFLAGS_AC))
#define _SetEFLAGS_VIF   (SetBit(_eflags, VCPU_EFLAGS_VIF))
#define _SetEFLAGS_VIP   (SetBit(_eflags, VCPU_EFLAGS_VIP))
#define _SetEFLAGS_ID    (SetBit(_eflags, VCPU_EFLAGS_ID))*/
#define _ClrEFLAGS_CF    (ClrBit(_eflags, VCPU_EFLAGS_CF))
#define _ClrEFLAGS_PF    (ClrBit(_eflags, VCPU_EFLAGS_PF))
#define _ClrEFLAGS_AF    (ClrBit(_eflags, VCPU_EFLAGS_AF))
#define _ClrEFLAGS_ZF    (ClrBit(_eflags, VCPU_EFLAGS_ZF))
#define _ClrEFLAGS_SF    (ClrBit(_eflags, VCPU_EFLAGS_SF))
#define _ClrEFLAGS_TF    (ClrBit(_eflags, VCPU_EFLAGS_TF))
#define _ClrEFLAGS_IF    (ClrBit(_eflags, VCPU_EFLAGS_IF))
#define _ClrEFLAGS_DF    (ClrBit(_eflags, VCPU_EFLAGS_DF))
#define _ClrEFLAGS_OF    (ClrBit(_eflags, VCPU_EFLAGS_OF))
#define _ClrEFLAGS_IOPLL (ClrBit(_eflags, VCPU_EFLAGS_IOPLL))
#define _ClrEFLAGS_IOPLH (ClrBit(_eflags, VCPU_EFLAGS_IOPLH))
#define _ClrEFLAGS_IOPL  (ClrBit(_eflags, VCPU_EFLAGS_IOPL)
#define _ClrEFLAGS_NT    (ClrBit(_eflags, VCPU_EFLAGS_NT))
#define _ClrEFLAGS_RF    (ClrBit(_eflags, VCPU_EFLAGS_RF))
#define _ClrEFLAGS_VM    (ClrBit(_eflags, VCPU_EFLAGS_VM))
/*
#define _ClrEFLAGS_AC    (ClrBit(_eflags, VCPU_EFLAGS_AC))
#define _ClrEFLAGS_VIF   (ClrBit(_eflags, VCPU_EFLAGS_VIF))
#define _ClrEFLAGS_VIP   (ClrBit(_eflags, VCPU_EFLAGS_VIP))
#define _ClrEFLAGS_ID    (ClrBit(_eflags, VCPU_EFLAGS_ID))*/

#define _GetCF _GetEFLAGS_CF
#define _SetCF _SetEFLAGS_CF
#define _ClrCF _ClrEFLAGS_CF
#define _GetAF _GetEFLAGS_AF
#define _SetAF _SetEFLAGS_AF
#define _ClrAF _ClrEFLAGS_AF

#define VCPU_CR0_PE     0x00000001
#define VCPU_CR0_TS     0x00000008
#define VCPU_CR0_PG     0x80000000
#define _GetCR0_PE  (GetBit(vcpu.cr0, VCPU_CR0_PE))
#define _GetCR0_PG  (GetBit(vcpu.cr0, VCPU_CR0_PG))
#define _SetCR0_TS  (SetBit(vcpu.cr0, VCPU_CR0_TS))

#define _MakePageFaultErrorCode(p,wr,us) ((p) | ((wr) << 1) | ((us) << 2))

#define VCPU_LINEAR_DIR    0xffc00000
#define VCPU_LINEAR_PAGE   0x003ff000
#define VCPU_LINEAR_OFFSET 0x00000fff
#define _GetLinear_DIR(linear)    (((linear) & VCPU_LINEAR_DIR)    >> 22)
#define _GetLinear_PAGE(linear)   (((linear) & VCPU_LINEAR_PAGE)   >> 12)
#define _GetLinear_OFFSET(linear) (((linear) & VCPU_LINEAR_OFFSET) >>  0)

#define VCPU_PGENTRY_BASE  0xfffff000
#define VCPU_PGENTRY_D     0x00000040
#define VCPU_PGENTRY_A     0x00000020
#define VCPU_PGENTRY_US    0x00000004
#define VCPU_PGENTRY_RW    0x00000002
#define VCPU_PGENTRY_P     0x00000001
#define _GetPageEntry_P(pgentry)    (GetBit((pgentry), VCPU_PGENTRY_P))
#define _GetPageEntry_RW(pgentry)   (GetBit((pgentry), VCPU_PGENTRY_RW))
#define _GetPageEntry_US(pgentry)   (GetBit((pgentry), VCPU_PGENTRY_US))
#define _GetPageEntry_A(pgentry)    (GetBit((pgentry), VCPU_PGENTRY_A))
#define _GetPageEntry_D(pgentry)    (GetBit((pgentry), VCPU_PGENTRY_D))
#define _GetPageEntry_BASE(pgentry) ((pgentry) & VCPU_PGENTRY_BASE)
#define _SetPageEntry_A(pgentry)    (SetBit((pgentry), VCPU_PGENTRY_A))
#define _SetPageEntry_D(pgentry)    (SetBit((pgentry), VCPU_PGENTRY_D))

#define VCPU_SELECTOR_RPL 0x0003 /* requestor's privilege level */
#define VCPU_SELECTOR_TI  0x0004 /* table indicator */
#define VCPU_SELECTOR_IDX 0xfff8 /* index */
#define _GetSelector_RPL(selector)    (((selector) & VCPU_SELECTOR_RPL) >> 0)
#define _GetSelector_TI(selector)     (GetBit((selector), VCPU_SELECTOR_TI))
#define _GetSelector_INDEX(selector)  (((selector) & VCPU_SELECTOR_IDX) >> 3)
#define _GetSelector_OFFSET(selector) (((selector) & VCPU_SELECTOR_IDX) >> 0)
#define _IsSelectorNull(selector)     (!_GetSelector_TI(selector) && !_GetSelector_INDEX(selector))

/* DESCRIPTOR DEFITION I: General Part */
#define VCPU_DESC_TYPE 0x00000f0000000000 /* descriptor type */
#define VCPU_DESC_S    0x0000100000000000 /* system segment (0) or user segment (1) */
#define VCPU_DESC_DPL  0x0000600000000000 /* descriptor previlege level */
#define VCPU_DESC_P    0x0000800000000000 /* descriptor presence */

 /* descriptor type */
#define _GetDesc_Type(descriptor) (((descriptor) & VCPU_DESC_TYPE) >> 40)
/* system segment (0) or user segment (1) */
#define _GetDesc_S(descriptor)    (GetBit((descriptor), VCPU_DESC_S))
/* descriptor previlege level */
#define _GetDesc_DPL(descriptor)  (((descriptor) & VCPU_DESC_DPL) >> 45)
/* descriptor presence */
#define _GetDesc_P(descriptor)    (GetBit((descriptor), VCPU_DESC_P))

#define _IsDescUser(descriptor)    (_GetDesc_S(descriptor))
#define _IsDescSys(desciprtor)     (!_GetDesc_S(descriptor))
#define _IsDescPresent(descriptor) (_GetDesc_P(descriptor))

/* DESCRIPTOR DEFINITION II: System Gate Part */
#define VCPU_DESC_SYS_TYPE_0 0x0000010000000000
#define VCPU_DESC_SYS_TYPE_1 0x0000020000000000
#define VCPU_DESC_SYS_TYPE_2 0x0000040000000000
#define VCPU_DESC_SYS_TYPE_3 0x0000080000000000
#define VCPU_DESC_GATE_TYPE_E VCPU_DESC_SYS_TYPE_3 /* 32-bit gate */
#define VCPU_DESC_TSS_TYPE_B  VCPU_DESC_SYS_TYPE_1  /* busy tss */
#define VCPU_DESC_TSS_TYPE_E  VCPU_DESC_GATE_TYPE_E /* 32-bit tss */
#define VCPU_DESC_SYS_TYPE_TSS_16_AVL  0x01
#define VCPU_DESC_SYS_TYPE_LDT         0x02
#define VCPU_DESC_SYS_TYPE_TSS_16_BUSY 0x03
#define VCPU_DESC_SYS_TYPE_CALLGATE_16 0x04
#define VCPU_DESC_SYS_TYPE_TASKGATE    0x05
#define VCPU_DESC_SYS_TYPE_INTGATE_16  0x06
#define VCPU_DESC_SYS_TYPE_TRAPGATE_16 0x07
#define VCPU_DESC_SYS_TYPE_TSS_32_AVL  0x09
#define VCPU_DESC_SYS_TYPE_TSS_32_BUSY 0x0b
#define VCPU_DESC_SYS_TYPE_CALLGATE_32 0x0c
#define VCPU_DESC_SYS_TYPE_INTGATE_32  0x0e
#define VCPU_DESC_SYS_TYPE_TRAPGATE_32 0x0f

#define _IsDescLDT(descriptor)        (_IsDescSys(descriptor) && (_GetDesc_Type(descriptor) == VCPU_DESC_SYS_TYPE_LDT))
#define _IsDescTaskGate(descriptor)   (_IsDescSys(descriptor) && (_GetDesc_Type(descriptor) == VCPU_DESC_SYS_TYPE_TASKGATE))
#define _IsDescTSS(descriptor)        (_IsDescSys(descriptor) && ((_GetDesc_Type(descriptor) & 0x05) == 0x01))
#define _IsDescTSSBusy(descriptor)    (_IsDescSys(descriptor) && ((_GetDesc_Type(descriptor) & 0x07) == 0x03))
#define _IsDescTSS32(descriptor)      (_IsDescSys(descriptor) && ((_GetDesc_Type(descriptor) & 0x0d) == 0x09))
#define _IsDescTSS16Avl(descriptor)   (_IsDescSys(descriptor) && (_GetDesc_Type(descriptor) == VCPU_DESC_SYS_TYPE_TSS_16_AVL))
#define _IsDescTSS16Busy(descriptor)  (_IsDescSys(descriptor) && (_GetDesc_Type(descriptor) == VCPU_DESC_SYS_TYPE_TSS_16_BUSY))
#define _IsDescTSS32Avl(descriptor)   (_IsDescSys(descriptor) && (_GetDesc_Type(descriptor) == VCPU_DESC_SYS_TYPE_TSS_32_AVL))
#define _IsDescTSS32Busy(descriptor)  (_IsDescSys(descriptor) && (_GetDesc_Type(descriptor) == VCPU_DESC_SYS_TYPE_TSS_32_BUSY))
#define _IsDescCallGate(descriptor)   (_IsDescSys(descriptor) && ((_GetDesc_Type(descriptor) & 0x07) == 0x04))
#define _IsDescCallGate16(descriptor) (_IsDescSys(descriptor) && (_GetDesc_Type(descriptor) == VCPU_DESC_SYS_TYPE_CALLGATE_16))
#define _IsDescCallGate32(descriptor) (_IsDescSys(descriptor) && (_GetDesc_Type(descriptor) == VCPU_DESC_SYS_TYPE_CALLGATE_32))
#define _IsDescIntGate(descriptor)    (_IsDescSys(descriptor) && ((_GetDesc_Type(descriptor) & 0x07) == 0x06))
#define _IsDescIntGate16(descriptor)  (_IsDescSys(descriptor) && (_GetDesc_Type(descriptor) == VCPU_DESC_SYS_TYPE_INTGATE_16))
#define _IsDescIntGate32(descriptor)  (_IsDescSys(descriptor) && (_GetDesc_Type(descriptor) == VCPU_DESC_SYS_TYPE_INTGATE_32))
#define _IsDescTrapGate(descriptor)   (_IsDescSys(descriptor) && ((_GetDesc_Type(descriptor) & 0x07) == 0x07))
#define _IsDescTrapGate16(descriptor) (_IsDescSys(descriptor) && (_GetDesc_Type(descriptor) == VCPU_DESC_SYS_TYPE_TRAPGATE_16))
#define _IsDescTrapGate32(descriptor) (_IsDescSys(descriptor) && (_GetDesc_Type(descriptor) == VCPU_DESC_SYS_TYPE_TRAPGATE_32))

/* DESCRIPTOR DEFINITION II: Segment Part */
#define VCPU_DESC_SEG_LIMIT_0  0x000000000000ffff
#define VCPU_DESC_SEG_LIMIT_1  0x000f000000000000
#define VCPU_DESC_SEG_BASE_0   0x000000ffffff0000
#define VCPU_DESC_SEG_BASE_1   0xff00000000000000
#define VCPU_DESC_SEG_G        0x0080000000000000 /* granularity */
#define VCPU_DESC_USER_AVL     0x0010000000000000 /* available for programmers */
#define VCPU_DESC_USER_TYPE_A  0x0000010000000000 /* descriptor type: accessed (code/data) */
#define VCPU_DESC_DATA_TYPE_W  0x0000020000000000 /* descriptor type: writable    (data) */
#define VCPU_DESC_DATA_TYPE_E  0x0000040000000000 /* descriptor type: expand-down (data) */
#define VCPU_DESC_CODE_TYPE_R  0x0000020000000000 /* descriptor type: readable    (code) */
#define VCPU_DESC_CODE_TYPE_C  0x0000040000000000 /* descriptor type: conforming  (code) */
#define VCPU_DESC_USER_TYPE_CD 0x0000080000000000 /* descriptor type: executable */
#define VCPU_DESC_DATA_B       0x0040000000000000 /* big segment (data) */
#define VCPU_DESC_CODE_D       0x0040000000000000 /* default size (code) */

/* segment limit */
#define _GetDescSeg_Limit(descriptor) \
	((((descriptor) & VCPU_DESC_SEG_LIMIT_0) >> 0) | (((descriptor) & VCPU_DESC_SEG_LIMIT_1) >> 32))
/* segment base */
#define _GetDescSeg_Base(descriptor) \
	((((descriptor) & VCPU_DESC_SEG_BASE_0) >> 16) | (((descriptor) & VCPU_DESC_SEG_BASE_1) >> 32))
/* segment granularity */
#define _GetDescSeg_G(descriptor)        (GetBit((descriptor), VCPU_DESC_SEG_G))

#define _GetDescUser_Avl(descriptor)     (GetBit((descriptor), VCPU_DESC_USER_AVL))
#define _GetDescUser_Type_A(descriptor)  (GetBit((descriptor), VCPU_DESC_USER_TYPE_A))
#define _SetDescUser_Type_A(descriptor)  (SetBit((descriptor), VCPU_DESC_USER_TYPE_A))
#define _GetDescData_Type_W(descriptor)  (GetBit((descriptor), VCPU_DESC_DATA_TYPE_W))
#define _GetDescData_Type_E(descriptor)  (GetBit((descriptor), VCPU_DESC_DATA_TYPE_E))
#define _GetDescCode_Type_R(descriptor)  (GetBit((descriptor), VCPU_DESC_CODE_TYPE_R))
#define _GetDescCode_Type_C(descriptor)  (GetBit((descriptor), VCPU_DESC_CODE_TYPE_C))
#define _GetDescUser_Type_CD(descriptor) (GetBit((descriptor), VCPU_DESC_USER_TYPE_CD))
#define _GetDescData_B(descriptor)       (GetBit((descriptor), VCPU_DESC_DATA_B))
#define _GetDescCode_D(descriptor)       (GetBit((descriptor), VCPU_DESC_CODE_D))

#define _IsDescSegGranularLarge(descriptor)  (_GetDescSeg_G(descriptor))
#define _IsDescUserAccessed(descriptor)      (_IsDescUser(descriptor) && _GetDescUser_Type_A(descriptor))
#define _IsDescUserExecutable(descriptor)    (_IsDescUser(descriptor) && _GetDescUser_Type_CD(descriptor))
#define _IsDescCode(descriptor)              (_IsDescUser(descriptor) && _IsDescUserExecutable(descriptor))
#define _IsDescData(descriptor)              (_IsDescUser(descriptor) && !_IsDescUserExecutable(descriptor))
#define _IsDescDataWritable(descriptor)      (_IsDescData(descriptor) && _GetDescData_Type_W(descriptor))
#define _IsDescDataExpDown(descriptor)       (_IsDescData(descriptor) && _GetDescData_Type_E(descriptor))
#define _IsDescCodeReadable(descriptor)      (_IsDescCode(descriptor) && _GetDescCode_Type_R(descriptor))
#define _IsDescCodeConform(descriptor)       (_IsDescCode(descriptor) && _GetDescCode_Type_C(descriptor))
#define _IsDescDataBig(descriptor)           (_IsDescData(descriptor) && _GetDescData_B(descriptor))
#define _IsDescCode32(descriptor)            (_IsDescCode(descriptor) && _GetDescCode_D(descriptor))

#define _MakeDescSeg(base,limit,type,s,dpl,p,avl,db,g) \
	(((t_nubit64)((base)  & 0xff000000) << 32) | \
	 ((t_nubit64)((g)     & 0x00000001) << 55) | \
	 ((t_nubit64)((db)    & 0x00000001) << 54) | \
	 ((t_nubit64)((avl)   & 0x00000001) << 52) | \
	 ((t_nubit64)((limit) & 0x000f0000) << 32) | \
	 ((t_nubit64)((p)     & 0x00000001) << 47) | \
	 ((t_nubit64)((dpl)   & 0x00000003) << 45) | \
	 ((t_nubit64)((s)     & 0x00000001) << 44) | \
	 ((t_nubit64)((type)  & 0x0000000f) << 40) | \
	 ((t_nubit64)((base)  & 0x00ffffff) << 16) | \
	 ((t_nubit64)((limit) & 0x0000ffff) << 0 ))

/* DESCRIPTOR DEFINITION III: System Part */
#define VCPU_DESC_GATE_SELECTOR 0x00000000ffff0000
#define VCPU_DESC_GATE_OFFSET_0 0x000000000000ffff
#define VCPU_DESC_GATE_OFFSET_1 0xffff000000000000 /* offset of call/int/trap gate */
#define VCPU_DESC_CALL_COUNT    0x0000001f00000000 /* parameter count of call gate */

#define _GetDescGate_Selector(descriptor) (((descriptor) & VCPU_DESC_GATE_SELECTOR) >> 16)
#define _GetDescGate_Offset(descriptor) \
	(((descriptor) & VCPU_DESC_GATE_OFFSET_0) | (((descriptor) & VCPU_DESC_GATE_OFFSET_1) >> 32))
#define _GetDescCall_Count(descriptor)    (((descriptor) & VCPU_DESC_CALL_COUNT) >> 32)

/*
#define VCPU_TSSDESC_TYPE_B 0x0000020000000000
#define _GetTssDesc_TYPE_B(descriptor) (GetBit((descriptor), VCPU_TSSDESC_TYPE_B))
#define _SetTssDesc_TYPE_B(descriptor) (SetBit((descriptor), VCPU_TSSDESC_TYPE_B))
#define _ClrTssDesc_TYPE_B(descriptor) (ClrBit((descriptor), VCPU_TSSDESC_TYPE_B))
#define _GetTssDesc_TYPE    _GetDesc_TYPE
#define _GetTssDesc_TYPE_32 _GetDesc_Type_32
#define _GetTssDesc_BASE    _GetDescSeg_Base
#define _GetTssDesc_LIMIT   _GetDescSeg_Limit
#define _GetTssDesc_S       _GetDesc_S
#define _GetTssDesc_DPL     _GetDesc_DPL
#define _GetTssDesc_P       _GetDesc_P
#define _GetTssDesc_G       _GetDescSeg_G

#define _GetLdtDesc_TYPE    _GetDesc_Type
#define _GetLdtDesc_BASE    _GetDescSeg_Base
#define _GetLdtDesc_LIMIT   _GetDescSeg_Limit
#define _GetLdtDesc_S       _GetDesc_S
#define _GetLdtDesc_DPL     _GetDesc_DPL
#define _GetLdtDesc_P       _GetDesc_P
#define _GetLdtDesc_G       _GetDescSeg_G

#define VCPU_GDTR_LIMIT 0x00000000ffff
#define VCPU_GDTR_BASE  0xffffffff0000
#define _GetGDTR_LIMIT ((vcpu.gdtr & VCPU_GDTR_LIMIT) >>  0)
#define _GetGDTR_BASE  ((vcpu.gdtr & VCPU_GDTR_BASE)  >> 16)

#define _GetLDTR_LIMIT (_GetDescSeg_Limit(vcpu.ldtr.descriptor))*/

#define VCPU_IDTR_LIMIT 0x00000000ffff
#define VCPU_IDTR_BASE  0xffffffff0000
#define _GetIDTR_LIMIT (vcpu.idtr & VCPU_IDTR_LIMIT)
#define _GetIDTR_BASE  ((vcpu.idtr & VCPU_IDTR_BASE) >> 16)
#define _LoadIDTR16(base,limit)  (vcpu.idtr = (GetMax24(base) << 16) | (t_nubit16)(limit))
#define _LoadIDTR32(base,limit)  (vcpu.idtr = (GetMax32(base) << 16) | (t_nubit16)(limit))

#define VCPU_CR0_PE     0x00000001
#define VCPU_CR0_TS     0x00000008
#define VCPU_CR0_PG     0x80000000
#define _GetCR0_PE  (GetBit(vcpu.cr0, VCPU_CR0_PE))
#define _GetCR0_PG  (GetBit(vcpu.cr0, VCPU_CR0_PG))
#define _GetCR0_TS  (GetBit(vcpu.cr0, VCPU_CR0_TS))
#define _SetCR0_TS  (SetBit(vcpu.cr0, VCPU_CR0_TS))
#define _ClrCR0_TS  (ClrBit(vcpu.cr0, VCPU_CR0_TS))

#define VCPU_CR3_BASE   0xfffff000
#define _GetCR3_Base    (vcpu.cr3 & VCPU_CR3_BASE)

#define _LoadGDTR16(base,limit)  (vcpu.gdtr = (GetMax24(base) << 16) | (t_nubit16)(limit))
#define _LoadGDTR32(base,limit)  (vcpu.gdtr = (GetMax32(base) << 16) | (t_nubit16)(limit))
#define _LoadTR
#define _LoadCR0
#define _LoadCR1
#define _LoadCR2
#define _LoadCR3
#define _LoadDR0
#define _LoadDR1
#define _LoadDR2
#define _LoadDR3
#define _LoadDR4
#define _LoadDR5
#define _LoadDR6
#define _LoadDR7
#define _LoadTR6
#define _LoadTR7

#define _IsProtected (_GetCR0_PE && !_GetEFLAGS_VM)
#define _GetCPL  (_GetCR0_PE ? (_GetEFLAGS_VM ? 3 : vcpu.cs.dpl) : 0)

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

#ifdef __cplusplus
/*}_EOCD_*/
#endif

#endif
