/* Copyright 2012-2014 Neko. */

/* VCPUINS implements 8086+ CPU instruction set. */

#ifndef NXVM_VCPUINS_H
#define NXVM_VCPUINS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

#include "core/machine/cpu.h"

typedef enum {
    ARITHTYPE_NULL,
    ADD8,ADD16,ADD32,
    OR8,OR16,OR32,
    ADC8,ADC16,ADC32,
    SBB8,SBB16,SBB32,
    AND8,AND16,AND32,
    SUB8,SUB16,SUB32,
    XOR8,XOR16,XOR32,
    CMP8,CMP16,CMP32,
    TEST8,TEST16,TEST32
} t_cpuins_data_arithtype;

typedef enum {
    PREFIX_REP_NONE,
    PREFIX_REP_REPZ,
    PREFIX_REP_REPZNZ
} t_cpuins_data_prefix_rep;

typedef enum {
    PREFIX_SREG_NONE,
    PREFIX_SREG_CS, PREFIX_SREG_SS,
    PREFIX_SREG_DS, PREFIX_SREG_ES,
    PREFIX_SREG_FS, PREFIX_SREG_GS
} t_cpuins_data_prefix_sreg;

typedef ntvdm64_type_bool t_cpuins_data_prefix;

typedef struct {
    t_cpu_data_sreg *rsreg;
    ntvdm64_type_unsigned_32 offset;
} t_cpuins_data_logical;

typedef struct {
    ntvdm64_type_bool flagWrite;
    ntvdm64_type_unsigned_32 byte;
    ntvdm64_type_unsigned_32 linear;
    ntvdm64_type_unsigned_64 data;
} t_cpuins_data_memory;

typedef struct {
    /* prefixes */
    t_cpuins_data_prefix_rep  prefix_rep;
    t_cpuins_data_prefix      prefix_oprsize;
    t_cpuins_data_prefix      prefix_addrsize;
    t_cpu_data_sreg *roverds, *roverss, *rmovsreg;

    /* execution control */
    t_cpu  oldcpu;
    ntvdm64_type_bool flagInsLoop;
    ntvdm64_type_bool flagMaskInt; /* if C_INT is disabled once */

    /* memory management */
    t_cpuins_data_logical mrm;
    ntvdm64_type_virtual_address rrm, rr;
    ntvdm64_type_unsigned_64 crm, cr, cimm;
    ntvdm64_type_bool flagMem; /* if rm is in memory */
    ntvdm64_type_bool flagLock;

    /* arithmetic operands */
    ntvdm64_type_unsigned_64 opr1, opr2, result;
    ntvdm64_type_unsigned_32 bit;
    t_cpuins_data_arithtype type;
    ntvdm64_type_unsigned_32 udf; /* undefined eflags bits */

    /* exception handler */
    ntvdm64_type_unsigned_32 except, excode;

    /* debugger */
    ntvdm64_type_unsigned_32 linear;
    ntvdm64_type_bool flagWR, flagWW, flagWE;
    ntvdm64_type_unsigned_32 wrLinear, wwLinear, weLinear;

    /* cpu recorder */
    ntvdm64_type_bool flagIgnore;
    t_cpuins_data_memory mem[0x20];
    ntvdm64_type_unsigned_8 msize;
    ntvdm64_type_unsigned_8 oplen;
    ntvdm64_type_unsigned_8 opcodes[15];
    ntvdm64_type_unsigned_16 reccs;
    ntvdm64_type_unsigned_32 receip;
} t_cpuins_data;

typedef struct t_cpuins t_cpuins;
typedef struct t_ram t_ram;
typedef struct t_port t_port;
typedef struct t_pic t_pic;
typedef struct core_machine_cpu_execution_context
    core_machine_cpu_execution_context;
typedef C_VOID (*core_machine_cpu_instruction_handler)(
    core_machine_cpu_execution_context *context);

typedef struct {
    /* instruction dispatch */
    core_machine_cpu_instruction_handler insTable[0x100];
    core_machine_cpu_instruction_handler insTable_0f[0x100];
} t_cpuins_connect;

struct t_cpuins {
    t_cpuins_data data;
    t_cpuins_connect connect;
};

/* One composition-owned executor context names the existing CPU and decoder. */
struct core_machine_cpu_execution_context {
    t_cpu *cpu;
    t_cpuins *instructions;
    t_ram *memory;
    t_port *port;
    t_pic *pic_master;
    t_pic *pic_slave;
    ntvdm64_type_trace *trace;
    C_VOID *extension_context;
    ntvdm64_type_bool stop_requested;
    ntvdm64_type_bool reset_requested;
};

C_VOID core_machine_cpu_execution_context_initialize(
    core_machine_cpu_execution_context *context, t_cpu *cpu,
    t_cpuins *instructions, t_ram *memory, t_port *port);
C_VOID core_machine_cpu_execution_context_bind_pic(
    core_machine_cpu_execution_context *context, t_pic *master,
    t_pic *slave);
C_VOID core_machine_cpu_execution_context_bind_extension(
    core_machine_cpu_execution_context *context, C_VOID *extension_context);
C_VOID *core_machine_cpu_execution_context_extension(
    const core_machine_cpu_execution_context *context);
ntvdm64_type_bool core_machine_cpu_execution_load_segment(
    core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg,
    ntvdm64_type_unsigned_16 selector);
ntvdm64_type_bool core_machine_cpu_execution_read_linear(
    core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 linear,
    ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte);
ntvdm64_type_bool core_machine_cpu_execution_write_linear(
    core_machine_cpu_execution_context *context, ntvdm64_type_unsigned_32 linear,
    ntvdm64_type_virtual_address rdata, ntvdm64_type_unsigned_8 byte);
C_VOID core_machine_cpu_execution_initialize(
    core_machine_cpu_execution_context *context);
C_VOID core_machine_cpu_execution_reset(
    core_machine_cpu_execution_context *context);
C_VOID core_machine_cpu_execution_refresh(
    core_machine_cpu_execution_context *context);
C_VOID core_machine_cpu_execution_finalize(
    core_machine_cpu_execution_context *context);

#define VCPUINS_EXCEPT_DE  0x00000001 /* 00 - fault: divide error */
#define VCPUINS_EXCEPT_DB  0x00000002 /* 01 - trap/fault: debug exception */
#define VCPUINS_EXCEPT_NMI 0x00000004 /* 02 - n/a:   non-maskable interrupt */
#define VCPUINS_EXCEPT_BP  0x00000008 /* 03 - trap:  break point */
#define VCPUINS_EXCEPT_OF  0x00000010 /* 04 - trap:  overflow exception */
#define VCPUINS_EXCEPT_BR  0x00000020 /* 05 - fault: boundary check fail */
#define VCPUINS_EXCEPT_UD  0x00000040 /* 06 - fault: invalid opcode */
#define VCPUINS_EXCEPT_NM  0x00000080 /* 07 - fault: coprocessor not available */
#define VCPUINS_EXCEPT_DF  0x00000100 /* 08 - abort: C_DOUBLE fault */
#define VCPUINS_EXCEPT_09  0x00000200 /* 09 - abort: reserved */
#define VCPUINS_EXCEPT_TS  0x00000400 /* 10 - fault: task state segment fail */
#define VCPUINS_EXCEPT_NP  0x00000800 /* 11 - fault: segment not present */
#define VCPUINS_EXCEPT_SS  0x00001000 /* 12 - fault: stack segment fault */
#define VCPUINS_EXCEPT_GP  0x00002000 /* 13 - fault: general protection */
#define VCPUINS_EXCEPT_PF  0x00004000 /* 14 - fault: page fault */
#define VCPUINS_EXCEPT_15  0x00008000 /* 15 - n/a:   reserved */
#define VCPUINS_EXCEPT_MF  0x00010000 /* 16 - fault: x87 fpu floating point error */

#define VCPUINS_EXCEPT_CE  0x80000000 /* 31 - internal case error */

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
