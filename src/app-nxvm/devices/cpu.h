/* Copyright 2012-2014 Neko. */

#ifndef CORE_MACHINE_CPU_H
#define CORE_MACHINE_CPU_H


#include "app-nxvm/devices/cpu_interface.h"
#include "app-nxvm/devices/fpu_interface.h"
#include "lib/types/types_interface.h"
#include "app-nxvm/devices/device_support.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum core_machine_cpu_segment {
    CORE_MACHINE_CPU_SEGMENT_ES,
    CORE_MACHINE_CPU_SEGMENT_CS,
    CORE_MACHINE_CPU_SEGMENT_SS,
    CORE_MACHINE_CPU_SEGMENT_DS,
    CORE_MACHINE_CPU_SEGMENT_FS,
    CORE_MACHINE_CPU_SEGMENT_GS
} core_machine_cpu_segment;

typedef enum core_machine_cpu_watchpoint {
    CORE_MACHINE_CPU_WATCH_READ,
    CORE_MACHINE_CPU_WATCH_WRITE,
    CORE_MACHINE_CPU_WATCH_EXECUTE
} core_machine_cpu_watchpoint;

typedef struct core_machine_cpu_execution_context
    core_machine_cpu_execution_context;

void core_machine_cpu_execution_request_stop(
    core_machine_cpu_execution_context *context);
lib_u8 core_machine_cpu_execution_consume_stop_request(
    core_machine_cpu_execution_context *context);
void core_machine_cpu_execution_request_debug_pause(
    core_machine_cpu_execution_context *context);
lib_u8 core_machine_cpu_execution_consume_debug_pause_request(
    core_machine_cpu_execution_context *context);
void core_machine_cpu_execution_request_reset(
    core_machine_cpu_execution_context *context);
lib_u8 core_machine_cpu_execution_consume_reset_request(
    core_machine_cpu_execution_context *context);
void core_machine_cpu_execution_request_shutdown(
    core_machine_cpu_execution_context *context);
lib_u8 core_machine_cpu_execution_consume_shutdown_request(
    core_machine_cpu_execution_context *context);
void core_machine_cpu_state_initialize(
    core_machine_cpu_execution_context *context);
void core_machine_cpu_state_reset(core_machine_cpu_execution_context *context);
void core_machine_cpu_execution_context_bind_profiles(
    core_machine_cpu_execution_context *context,
    core_machine_cpu_profile cpu_profile,
    core_machine_fpu_profile fpu_profile,
    lib_u8 cpu_80386_cr_mov_ignores_mod);
#include "lib/types/types_interface.h"


#define CORE_MACHINE_DEVICE_CPU CORE_MACHINE_CPU_DEVICE_NAME

typedef enum {
    SREG_DATA,
    SREG_STACK,
    SREG_CODE,
    SREG_LDTR,
    SREG_TR,
    SREG_GDTR,
    SREG_IDTR
} t_cpu_data_sreg_type;

typedef struct {
    lib_u8 flagValid;
    lib_u16 selector;
    /* invisible portion/descriptor part */
    t_cpu_data_sreg_type sregtype;
    lib_u32 base;
    lib_u32 limit;
    lib_u8  dpl; /* if segment is cs, this is cpl */
    union {
        struct {
            lib_u8 executable;
            lib_u8 accessed;
            union {
                struct {
                    lib_u8 defsize; /* 16-bit (0) or 32-bit (1) */
                    lib_u8 conform;
                    lib_u8 readable;
                } exec;
                struct {
                    lib_u8 big;
                    lib_u8 expdown;
                    lib_u8 writable;
                } data;
            };
        } seg;
        struct {
            lib_u8 type;
        } sys;
    };
} t_cpu_data_sreg;

typedef struct {
    /* general registers */
    union {
        union {
            struct {
                lib_u8 al,ah;
            };
            lib_u16 ax;
        };
        lib_u32 eax;
    };
    union {
        union {
            struct {
                lib_u8 bl,bh;
            };
            lib_u16 bx;
        };
        lib_u32 ebx;
    };
    union {
        union {
            struct {
                lib_u8 cl,ch;
            };
            lib_u16 cx;
        };
        lib_u32 ecx;
    };
    union {
        union {
            struct {
                lib_u8 dl,dh;
            };
            lib_u16 dx;
        };
        lib_u32 edx;
    };
    union {
        lib_u16 sp;
        lib_u32 esp;
    };
    union {
        lib_u16 bp;
        lib_u32 ebp;
    };
    union {
        lib_u16 si;
        lib_u32 esi;
    };
    union {
        lib_u16 di;
        lib_u32 edi;
    };
    union {
        lib_u16 ip;
        lib_u32 eip;
    };
    union {
        lib_u16 flags;
        lib_u32 eflags;
    };
    /* segment registers */
    t_cpu_data_sreg es, cs, ss, ds, fs, gs;
    t_cpu_data_sreg ldtr, tr, gdtr, idtr;
    /* control registers */
    lib_u32 cr0, cr1, cr2, cr3, cr4, cr5, cr6, cr7;
    lib_u32 dr0, dr1, dr2, dr3, dr4, dr5, dr6, dr7;
    lib_u32 tr0, tr1, tr2, tr3, tr4, tr5, tr6, tr7;
    /* control flags */
    lib_u8 flagMaskNMI, flagNMI, flagHalt;
} t_cpu_data;

typedef struct {
    t_cpu_data data;
} t_cpu;

#define VCPU_PAGESIZE 0x1000

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
#define VCPU_EFLAGS_RESERVED 0xfffc802a
/*
#define VCPU_EFLAGS_AC    0x00040000
#define VCPU_EFLAGS_VIF   0x00080000
#define VCPU_EFLAGS_VIP   0x00100000
#define VCPU_EFLAGS_ID    0x00200000
#define VCPU_EFLAGS_RESERVED 0xffc0802a
*/
#define _GetEFLAGS_CF    (CORE_MACHINE_BIT_IS_SET(cpu_state.data.eflags, VCPU_EFLAGS_CF))
#define _GetEFLAGS_PF    (CORE_MACHINE_BIT_IS_SET(cpu_state.data.eflags, VCPU_EFLAGS_PF))
#define _GetEFLAGS_AF    (CORE_MACHINE_BIT_IS_SET(cpu_state.data.eflags, VCPU_EFLAGS_AF))
#define _GetEFLAGS_ZF    (CORE_MACHINE_BIT_IS_SET(cpu_state.data.eflags, VCPU_EFLAGS_ZF))
#define _GetEFLAGS_SF    (CORE_MACHINE_BIT_IS_SET(cpu_state.data.eflags, VCPU_EFLAGS_SF))
#define _GetEFLAGS_TF    (CORE_MACHINE_BIT_IS_SET(cpu_state.data.eflags, VCPU_EFLAGS_TF))
#define _GetEFLAGS_IF    (CORE_MACHINE_BIT_IS_SET(cpu_state.data.eflags, VCPU_EFLAGS_IF))
#define _GetEFLAGS_DF    (CORE_MACHINE_BIT_IS_SET(cpu_state.data.eflags, VCPU_EFLAGS_DF))
#define _GetEFLAGS_OF    (CORE_MACHINE_BIT_IS_SET(cpu_state.data.eflags, VCPU_EFLAGS_OF))
#define _GetEFLAGS_IOPLL (CORE_MACHINE_BIT_IS_SET(cpu_state.data.eflags, VCPU_EFLAGS_IOPLL))
#define _GetEFLAGS_IOPLH (CORE_MACHINE_BIT_IS_SET(cpu_state.data.eflags, VCPU_EFLAGS_IOPLH))
#define _GetEFLAGS_IOPL  ((cpu_state.data.eflags & VCPU_EFLAGS_IOPL) >> 12)
#define _GetEFLAGS_NT    (CORE_MACHINE_BIT_IS_SET(cpu_state.data.eflags, VCPU_EFLAGS_NT))
#define _GetEFLAGS_RF    (CORE_MACHINE_BIT_IS_SET(cpu_state.data.eflags, VCPU_EFLAGS_RF))
#define _GetEFLAGS_VM    (CORE_MACHINE_BIT_IS_SET(cpu_state.data.eflags, VCPU_EFLAGS_VM))
/*
#define _GetEFLAGS_AC    (CORE_MACHINE_BIT_IS_SET(cpu_state.data.eflags, VCPU_EFLAGS_AC))
#define _GetEFLAGS_VIF   (CORE_MACHINE_BIT_IS_SET(cpu_state.data.eflags, VCPU_EFLAGS_VIF))
#define _GetEFLAGS_VIP   (CORE_MACHINE_BIT_IS_SET(cpu_state.data.eflags, VCPU_EFLAGS_VIP))
#define _GetEFLAGS_ID    (CORE_MACHINE_BIT_IS_SET(cpu_state.data.eflags, VCPU_EFLAGS_ID))*/
#define _SetEFLAGS_CF    (CORE_MACHINE_BIT_SET(cpu_state.data.eflags, VCPU_EFLAGS_CF))
#define _SetEFLAGS_PF    (CORE_MACHINE_BIT_SET(cpu_state.data.eflags, VCPU_EFLAGS_PF))
#define _SetEFLAGS_AF    (CORE_MACHINE_BIT_SET(cpu_state.data.eflags, VCPU_EFLAGS_AF))
#define _SetEFLAGS_ZF    (CORE_MACHINE_BIT_SET(cpu_state.data.eflags, VCPU_EFLAGS_ZF))
#define _SetEFLAGS_SF    (CORE_MACHINE_BIT_SET(cpu_state.data.eflags, VCPU_EFLAGS_SF))
#define _SetEFLAGS_TF    (CORE_MACHINE_BIT_SET(cpu_state.data.eflags, VCPU_EFLAGS_TF))
#define _SetEFLAGS_IF    (CORE_MACHINE_BIT_SET(cpu_state.data.eflags, VCPU_EFLAGS_IF))
#define _SetEFLAGS_DF    (CORE_MACHINE_BIT_SET(cpu_state.data.eflags, VCPU_EFLAGS_DF))
#define _SetEFLAGS_OF    (CORE_MACHINE_BIT_SET(cpu_state.data.eflags, VCPU_EFLAGS_OF))
#define _SetEFLAGS_IOPLL (CORE_MACHINE_BIT_SET(cpu_state.data.eflags, VCPU_EFLAGS_IOPLL))
#define _SetEFLAGS_IOPLH (CORE_MACHINE_BIT_SET(cpu_state.data.eflags, VCPU_EFLAGS_IOPLH))
#define _SetEFLAGS_IOPL  (CORE_MACHINE_BIT_SET(cpu_state.data.eflags, VCPU_EFLAGS_IOPL)
#define _SetEFLAGS_NT    (CORE_MACHINE_BIT_SET(cpu_state.data.eflags, VCPU_EFLAGS_NT))
#define _SetEFLAGS_RF    (CORE_MACHINE_BIT_SET(cpu_state.data.eflags, VCPU_EFLAGS_RF))
#define _SetEFLAGS_VM    (CORE_MACHINE_BIT_SET(cpu_state.data.eflags, VCPU_EFLAGS_VM))
/*
#define _SetEFLAGS_AC    (CORE_MACHINE_BIT_SET(cpu_state.data.eflags, VCPU_EFLAGS_AC))
#define _SetEFLAGS_VIF   (CORE_MACHINE_BIT_SET(cpu_state.data.eflags, VCPU_EFLAGS_VIF))
#define _SetEFLAGS_VIP   (CORE_MACHINE_BIT_SET(cpu_state.data.eflags, VCPU_EFLAGS_VIP))
#define _SetEFLAGS_ID    (CORE_MACHINE_BIT_SET(cpu_state.data.eflags, VCPU_EFLAGS_ID))*/
#define _ClrEFLAGS_CF    (CORE_MACHINE_BIT_CLEAR(cpu_state.data.eflags, VCPU_EFLAGS_CF))
#define _ClrEFLAGS_PF    (CORE_MACHINE_BIT_CLEAR(cpu_state.data.eflags, VCPU_EFLAGS_PF))
#define _ClrEFLAGS_AF    (CORE_MACHINE_BIT_CLEAR(cpu_state.data.eflags, VCPU_EFLAGS_AF))
#define _ClrEFLAGS_ZF    (CORE_MACHINE_BIT_CLEAR(cpu_state.data.eflags, VCPU_EFLAGS_ZF))
#define _ClrEFLAGS_SF    (CORE_MACHINE_BIT_CLEAR(cpu_state.data.eflags, VCPU_EFLAGS_SF))
#define _ClrEFLAGS_TF    (CORE_MACHINE_BIT_CLEAR(cpu_state.data.eflags, VCPU_EFLAGS_TF))
#define _ClrEFLAGS_IF    (CORE_MACHINE_BIT_CLEAR(cpu_state.data.eflags, VCPU_EFLAGS_IF))
#define _ClrEFLAGS_DF    (CORE_MACHINE_BIT_CLEAR(cpu_state.data.eflags, VCPU_EFLAGS_DF))
#define _ClrEFLAGS_OF    (CORE_MACHINE_BIT_CLEAR(cpu_state.data.eflags, VCPU_EFLAGS_OF))
#define _ClrEFLAGS_IOPLL (CORE_MACHINE_BIT_CLEAR(cpu_state.data.eflags, VCPU_EFLAGS_IOPLL))
#define _ClrEFLAGS_IOPLH (CORE_MACHINE_BIT_CLEAR(cpu_state.data.eflags, VCPU_EFLAGS_IOPLH))
#define _ClrEFLAGS_IOPL  (CORE_MACHINE_BIT_CLEAR(cpu_state.data.eflags, VCPU_EFLAGS_IOPL)
#define _ClrEFLAGS_NT    (CORE_MACHINE_BIT_CLEAR(cpu_state.data.eflags, VCPU_EFLAGS_NT))
#define _ClrEFLAGS_RF    (CORE_MACHINE_BIT_CLEAR(cpu_state.data.eflags, VCPU_EFLAGS_RF))
#define _ClrEFLAGS_VM    (CORE_MACHINE_BIT_CLEAR(cpu_state.data.eflags, VCPU_EFLAGS_VM))
/*
#define _ClrEFLAGS_AC    (CORE_MACHINE_BIT_CLEAR(cpu_state.data.eflags, VCPU_EFLAGS_AC))
#define _ClrEFLAGS_VIF   (CORE_MACHINE_BIT_CLEAR(cpu_state.data.eflags, VCPU_EFLAGS_VIF))
#define _ClrEFLAGS_VIP   (CORE_MACHINE_BIT_CLEAR(cpu_state.data.eflags, VCPU_EFLAGS_VIP))
#define _ClrEFLAGS_ID    (CORE_MACHINE_BIT_CLEAR(cpu_state.data.eflags, VCPU_EFLAGS_ID))*/

#define VCPU_ModRM_MOD 0xc0
#define VCPU_ModRM_REG 0x38
#define VCPU_ModRM_RM  0x07
#define VCPU_SIB_SS    0xc0
#define VCPU_SIB_Index 0x38
#define VCPU_SIB_Base  0x07
#define _GetModRM_MOD(cmodrm) (((cmodrm) & VCPU_ModRM_MOD) >> 6)
#define _GetModRM_REG(cmodrm) (((cmodrm) & VCPU_ModRM_REG) >> 3)
#define _GetModRM_RM(cmodrm)  (((cmodrm) & VCPU_ModRM_RM)  >> 0)
#define _GetSIB_SS(csib)      (((csib) & VCPU_SIB_SS)      >> 6)
#define _GetSIB_Index(csib)   (((csib) & VCPU_SIB_Index)   >> 3)
#define _GetSIB_Base(csib)    (((csib) & VCPU_SIB_Base)    >> 0)

#define VCPU_CR0_PE 0x00000001
#define VCPU_CR0_TS 0x00000008
#define VCPU_CR0_PG 0x80000000
#define _GetCR0_PE (CORE_MACHINE_BIT_IS_SET(cpu_state.data.cr0, VCPU_CR0_PE))
#define _GetCR0_PG (CORE_MACHINE_BIT_IS_SET(cpu_state.data.cr0, VCPU_CR0_PG))
#define _SetCR0_TS (CORE_MACHINE_BIT_SET(cpu_state.data.cr0, VCPU_CR0_TS))

#define _MakePageFaultErrorCode(p, wr, us) ((p) | ((wr) << 1) | ((us) << 2))

#define VCPU_LINEAR_DIR    0xffc00000
#define VCPU_LINEAR_PAGE   0x003ff000
#define VCPU_LINEAR_OFFSET 0x00000fff
#define _GetLinear_Dir(linear)    (((linear) & VCPU_LINEAR_DIR)    >> 22)
#define _GetLinear_Page(linear)   (((linear) & VCPU_LINEAR_PAGE)   >> 12)
#define _GetLinear_Offset(linear) (((linear) & VCPU_LINEAR_OFFSET) >>  0)

#define VCPU_PGENTRY_BASE  0xfffff000 /* base address of page entry */
#define VCPU_PGENTRY_D     0x00000040 /* dirty */
#define VCPU_PGENTRY_A     0x00000020 /* accessed */
#define VCPU_PGENTRY_US    0x00000004 /* user/supervisor */
#define VCPU_PGENTRY_RW    0x00000002 /* writable */
#define VCPU_PGENTRY_P     0x00000001 /* present */
#define _GetPageEntry_P(pge)      (CORE_MACHINE_BIT_IS_SET((pge), VCPU_PGENTRY_P))
#define _GetPageEntry_RW(pge)     (CORE_MACHINE_BIT_IS_SET((pge), VCPU_PGENTRY_RW))
#define _GetPageEntry_US(pge)     (CORE_MACHINE_BIT_IS_SET((pge), VCPU_PGENTRY_US))
#define _GetPageEntry_A(pge)      (CORE_MACHINE_BIT_IS_SET((pge), VCPU_PGENTRY_A))
#define _GetPageEntry_D(pge)      (CORE_MACHINE_BIT_IS_SET((pge), VCPU_PGENTRY_D))
#define _GetPageEntry_Base(pge)   ((pge) & VCPU_PGENTRY_BASE)
#define _SetPageEntry_A(pge)      (CORE_MACHINE_BIT_SET((pge), VCPU_PGENTRY_A))
#define _SetPageEntry_D(pge)      (CORE_MACHINE_BIT_SET((pge), VCPU_PGENTRY_D))
#define _IsPageEntryPresent(pge)  _GetPageEntry_P(pge)
#define _IsPageEntryWritable(pge) _GetPageEntry_RW(pge)
#define _GetPageSize              VCPU_PAGESIZE

#define VCPU_SELECTOR_RPL 0x0003 /* requestor's privilege level */
#define VCPU_SELECTOR_TI  0x0004 /* table indicator */
#define VCPU_SELECTOR_IDX 0xfff8 /* index */
#define _GetSelector_RPL(selector)    (((selector) & VCPU_SELECTOR_RPL) >> 0)
#define _GetSelector_TI(selector)     (CORE_MACHINE_BIT_IS_SET((selector), VCPU_SELECTOR_TI))
#define _GetSelector_Index(selector)  (((selector) & VCPU_SELECTOR_IDX) >> 3)
#define _GetSelector_Offset(selector) (((selector) & VCPU_SELECTOR_IDX) >> 0)
#define _IsSelectorNull(selector)     (!_GetSelector_TI(selector) && !_GetSelector_Index(selector))

/* DESCRIPTOR DEFITION I: General Part */
#define VCPU_DESC_TYPE 0x00000f0000000000 /* descriptor type */
#define VCPU_DESC_S    0x0000100000000000 /* system segment (0) or user segment (1) */
#define VCPU_DESC_DPL  0x0000600000000000 /* descriptor previlege level */
#define VCPU_DESC_P    0x0000800000000000 /* descriptor presence */

/* descriptor type */
#define _GetDesc_Type(descriptor) (((descriptor) & VCPU_DESC_TYPE) >> 40)
/* system segment (0) or user segment (1) */
#define _GetDesc_S(descriptor)    (CORE_MACHINE_BIT_IS_SET((descriptor), VCPU_DESC_S))
/* descriptor previlege level */
#define _GetDesc_DPL(descriptor)  (((descriptor) & VCPU_DESC_DPL) >> 45)
/* descriptor presence */
#define _GetDesc_P(descriptor)    (CORE_MACHINE_BIT_IS_SET((descriptor), VCPU_DESC_P))

#define _IsDescUser(descriptor)    (_GetDesc_S(descriptor))
#define _IsDescSys(descriptor)     (!_GetDesc_S(descriptor))
#define _IsDescPresent(descriptor) (_GetDesc_P(descriptor))

/* DESCRIPTOR DEFINITION II: System Gate Part */
#define VCPU_DESC_SYS_TYPE_0 0x0000010000000000
#define VCPU_DESC_SYS_TYPE_1 0x0000020000000000
#define VCPU_DESC_SYS_TYPE_2 0x0000040000000000
#define VCPU_DESC_SYS_TYPE_3 0x0000080000000000

#define VCPU_DESC_SYS_TYPE_E  VCPU_DESC_SYS_TYPE_3 /* 32-bit sys desc */
#define VCPU_DESC_GATE_TYPE_E VCPU_DESC_SYS_TYPE_E /* 32-bit gate */
#define VCPU_DESC_TSS_TYPE_B  VCPU_DESC_SYS_TYPE_1  /* busy tss */
#define VCPU_DESC_TSS_TYPE_E  VCPU_DESC_SYS_TYPE_E /* 32-bit tss */
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

#define _GetDescSys_Type_E(descriptor)   (CORE_MACHINE_BIT_IS_SET((descriptor), VCPU_DESC_TSS_TYPE_E))
#define _GetDescTSS_Type_B(descriptor)   (CORE_MACHINE_BIT_IS_SET((descriptor), VCPU_DESC_TSS_TYPE_B))
#define _SetDescTSS_Type_B(descriptor)   (CORE_MACHINE_BIT_SET((descriptor), VCPU_DESC_TSS_TYPE_B))
#define _ClrDescTSS_Type_B(descriptor)   (CORE_MACHINE_BIT_CLEAR((descriptor), VCPU_DESC_TSS_TYPE_B))

#define _IsDescSys32(descriptor)      (_IsDescSys(descriptor) && _GetDescSys_Type_E(descriptor))
#define _IsDescLDT(descriptor)        (_IsDescSys(descriptor) && (_GetDesc_Type(descriptor) == VCPU_DESC_SYS_TYPE_LDT))
#define _IsDescTaskGate(descriptor)   (_IsDescSys(descriptor) && (_GetDesc_Type(descriptor) == VCPU_DESC_SYS_TYPE_TASKGATE))
#define _IsDescTSS(descriptor)        (_IsDescSys(descriptor) && ((_GetDesc_Type(descriptor) & 0x05) == 0x01))
#define _IsDescTSSAvl(descriptor)     (_IsDescSys(descriptor) && ((_GetDesc_Type(descriptor) & 0x07) == 0x01))
#define _IsDescTSSBusy(descriptor)    (_IsDescSys(descriptor) && ((_GetDesc_Type(descriptor) & 0x07) == 0x03))
#define _SetDescTSSBusy(descriptor)   (_SetDescTSS_Type_B(descriptor))
#define _ClrDescTSSBusy(descriptor)   (_ClrDescTSS_Type_B(descriptor))
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
#define _GetDescSeg_G(descriptor)        (CORE_MACHINE_BIT_IS_SET((descriptor), VCPU_DESC_SEG_G))

#define _GetDescUser_Avl(descriptor)     (CORE_MACHINE_BIT_IS_SET((descriptor), VCPU_DESC_USER_AVL))
#define _GetDescUser_Type_A(descriptor)  (CORE_MACHINE_BIT_IS_SET((descriptor), VCPU_DESC_USER_TYPE_A))
#define _SetDescUser_Type_A(descriptor)  (CORE_MACHINE_BIT_SET((descriptor), VCPU_DESC_USER_TYPE_A))
#define _GetDescData_Type_W(descriptor)  (CORE_MACHINE_BIT_IS_SET((descriptor), VCPU_DESC_DATA_TYPE_W))
#define _GetDescData_Type_E(descriptor)  (CORE_MACHINE_BIT_IS_SET((descriptor), VCPU_DESC_DATA_TYPE_E))
#define _GetDescCode_Type_R(descriptor)  (CORE_MACHINE_BIT_IS_SET((descriptor), VCPU_DESC_CODE_TYPE_R))
#define _GetDescCode_Type_C(descriptor)  (CORE_MACHINE_BIT_IS_SET((descriptor), VCPU_DESC_CODE_TYPE_C))
#define _GetDescUser_Type_CD(descriptor) (CORE_MACHINE_BIT_IS_SET((descriptor), VCPU_DESC_USER_TYPE_CD))
#define _GetDescData_B(descriptor)       (CORE_MACHINE_BIT_IS_SET((descriptor), VCPU_DESC_DATA_B))
#define _GetDescCode_D(descriptor)       (CORE_MACHINE_BIT_IS_SET((descriptor), VCPU_DESC_CODE_D))

#define _IsDescSegGranularLarge(descriptor)  (_GetDescSeg_G(descriptor))
#define _IsDescUserAccessed(descriptor)      (_IsDescUser(descriptor) && _GetDescUser_Type_A(descriptor))
#define _SetDescUserAccessed(descriptor)     (_SetDescUser_Type_A(descriptor))
#define _IsDescUserExecutable(descriptor)    (_IsDescUser(descriptor) && _GetDescUser_Type_CD(descriptor))
#define _IsDescCode(descriptor)              (_IsDescUser(descriptor) && _IsDescUserExecutable(descriptor))
#define _IsDescData(descriptor)              (_IsDescUser(descriptor) && !_IsDescUserExecutable(descriptor))
#define _IsDescDataWritable(descriptor)      (_IsDescData(descriptor) && _GetDescData_Type_W(descriptor))
#define _IsDescDataExpDown(descriptor)       (_IsDescData(descriptor) && _GetDescData_Type_E(descriptor))
#define _IsDescCodeReadable(descriptor)      (_IsDescCode(descriptor) && _GetDescCode_Type_R(descriptor))
#define _IsDescCodeConform(descriptor)       (_IsDescCode(descriptor) && _GetDescCode_Type_C(descriptor))
#define _IsDescCodeNonConform(descriptor)    (_IsDescCode(descriptor) && !_GetDescCode_Type_C(descriptor))
#define _IsDescDataBig(descriptor)           (_IsDescData(descriptor) && _GetDescData_B(descriptor))
#define _IsDescCode32(descriptor)            (_IsDescCode(descriptor) && _GetDescCode_D(descriptor))

#define _MakeDescSeg(base, limit, type, s, dpl, p, avl, db, g) \
    (((lib_u64)((base)  & 0xff000000) << 32) | \
     ((lib_u64)((g)     & 0x00000001) << 55) | \
     ((lib_u64)((db)    & 0x00000001) << 54) | \
     ((lib_u64)((avl)   & 0x00000001) << 52) | \
     ((lib_u64)((limit) & 0x000f0000) << 32) | \
     ((lib_u64)((p)     & 0x00000001) << 47) | \
     ((lib_u64)((dpl)   & 0x00000003) << 45) | \
     ((lib_u64)((s)     & 0x00000001) << 44) | \
     ((lib_u64)((type)  & 0x0000000f) << 40) | \
     ((lib_u64)((base)  & 0x00ffffff) << 16) | \
     ((lib_u64)((limit) & 0x0000ffff) << 0))

/* DESCRIPTOR DEFINITION III: System Part */
#define VCPU_DESC_GATE_SELECTOR 0x00000000ffff0000
#define VCPU_DESC_GATE_OFFSET_0 0x000000000000ffff
#define VCPU_DESC_GATE_OFFSET_1 0xffff000000000000 /* offset of call/int/trap gate */
#define VCPU_DESC_CALL_COUNT    0x0000001f00000000 /* parameter count of call gate */

#define _GetDescGate_Selector(descriptor) (((descriptor) & VCPU_DESC_GATE_SELECTOR) >> 16)
#define _GetDescGate_Offset(descriptor) \
    (((descriptor) & VCPU_DESC_GATE_OFFSET_0) | (((descriptor) & VCPU_DESC_GATE_OFFSET_1) >> 32))
#define _GetDescCall_Count(descriptor)    (((descriptor) & VCPU_DESC_CALL_COUNT) >> 32)

#define VCPU_CR0_PE 0x00000001
#define VCPU_CR0_MP 0x00000002
#define VCPU_CR0_EM 0x00000004
#define VCPU_CR0_TS 0x00000008
#define VCPU_CR0_ET 0x00000010
#define VCPU_CR0_PG 0x80000000
#define _GetCR0_PE (CORE_MACHINE_BIT_IS_SET(cpu_state.data.cr0, VCPU_CR0_PE))
#define _GetCR0_MP (CORE_MACHINE_BIT_IS_SET(cpu_state.data.cr0, VCPU_CR0_MP))
#define _GetCR0_EM (CORE_MACHINE_BIT_IS_SET(cpu_state.data.cr0, VCPU_CR0_EM))
#define _GetCR0_TS (CORE_MACHINE_BIT_IS_SET(cpu_state.data.cr0, VCPU_CR0_TS))
#define _GetCR0_ET (CORE_MACHINE_BIT_IS_SET(cpu_state.data.cr0, VCPU_CR0_ET))
#define _GetCR0_PG (CORE_MACHINE_BIT_IS_SET(cpu_state.data.cr0, VCPU_CR0_PG))
#define _SetCR0_TS (CORE_MACHINE_BIT_SET(cpu_state.data.cr0, VCPU_CR0_TS))
#define _ClrCR0_TS (CORE_MACHINE_BIT_CLEAR(cpu_state.data.cr0, VCPU_CR0_TS))

#define VCPU_CR3_BASE   0xfffff000
#define _GetCR3_Base    (cpu_state.data.cr3 & VCPU_CR3_BASE)

#define _IsPaging (_GetCR0_PE && _GetCR0_PG)
#define _IsProtected (_GetCR0_PE && !_GetEFLAGS_VM)
#define _GetCPL  (_GetCR0_PE ? (_GetEFLAGS_VM ? 3 : cpu_state.data.cs.dpl) : 0)
#define _MakeCPL(cpl) (cpu_state.data.cs.dpl = (cpl))

lib_i32 core_machine_cpu_read_linear(core_machine_cpu_execution_context *context,
    lib_u32 linear, void *out_data, lib_u8 size);
lib_i32 core_machine_cpu_write_linear(core_machine_cpu_execution_context *context,
    lib_u32 linear, const void *in_data, lib_u8 size);
lib_i32 core_machine_cpu_get_code_default_size(
    const core_machine_cpu_execution_context *context);
lib_u32 core_machine_cpu_get_code_base(
    const core_machine_cpu_execution_context *context);
void core_machine_cpu_set_watchpoint(core_machine_cpu_execution_context *context,
    core_machine_cpu_watchpoint kind, lib_u32 linear);
void core_machine_cpu_clear_watchpoint(core_machine_cpu_execution_context *context,
    core_machine_cpu_watchpoint kind);
void core_machine_cpu_get_watchpoint(const core_machine_cpu_execution_context *context,
    core_machine_cpu_watchpoint kind, lib_u8 *out_enabled,
    lib_u32 *out_linear);

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
