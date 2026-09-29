/* Copyright 2012-2014 Neko. */

/* VCPUINS implements 8086+ CPU instruction set. */

#ifndef CORE_MACHINE_CPU_INSTRUCTIONS_H
#define CORE_MACHINE_CPU_INSTRUCTIONS_H

#ifdef __cplusplus
extern "C" {
#endif
#include "lib/types/types_interface.h"


#define CORE_MACHINE_CPU_INSTRUCTION_MEMORY_ACCESS_CAPACITY 512u

#include "app-nxvm/devices/cpu.h"
#include "x86/devices/fpu/fpu_interface.h"

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

typedef lib_u8 t_cpuins_data_prefix;

typedef struct {
    t_cpu_data_sreg *rsreg;
    lib_u32 offset;
} t_cpuins_data_logical;

typedef struct {
    lib_u8 flagWrite;
    lib_u32 byte;
    lib_u32 linear;
    lib_u64 data;
} t_cpuins_data_memory;

typedef struct {
    /* prefixes */
    t_cpuins_data_prefix_rep  prefix_rep;
    t_cpuins_data_prefix      prefix_oprsize;
    t_cpuins_data_prefix      prefix_addrsize;
    t_cpu_data_sreg *roverds, *roverss, *rmovsreg;

    /* execution control */
    t_cpu  oldcpu;
    lib_u8 flagInsLoop;
    lib_u8 flagMaskInt; /* if lib_i32 is disabled once */

    /* memory management */
    t_cpuins_data_logical mrm;
    lib_uptr rrm, rr;
    lib_u64 crm, cr, cimm;
    lib_u8 flagMem; /* if rm is in memory */
    lib_u8 flagLock;
    lib_u8 source_lsl_granularity_valid;
    lib_u8 source_lsl_page_granular;

    /* arithmetic operands */
    lib_u64 opr1, opr2, result;
    lib_u32 bit;
    t_cpuins_data_arithtype type;
    lib_u32 udf; /* undefined eflags bits */

    /* exception handler */
    lib_u32 except, excode;

    /* debugger */
    lib_u32 linear;
    lib_u8 flagWR, flagWW, flagWE;
    lib_u32 wrLinear, wwLinear, weLinear;
    lib_u8 watch_hit;
    lib_u8 watch_kind;
    lib_u32 watch_address;

    /* CPU retirement observation */
    lib_u8 flagIgnore;
    /* ENTER accepts an 80186 lexical level up to 255 and performs at most 510
     * recorded stack accesses. This is executor bookkeeping for CPU debug
     * breakpoints, not the copied debugger-observation limit. */
    t_cpuins_data_memory mem[CORE_MACHINE_CPU_INSTRUCTION_MEMORY_ACCESS_CAPACITY];
    lib_u16 msize;
    lib_u8 oplen;
    lib_u8 opcodes[15];
    lib_u16 reccs;
    lib_u32 receip;
} t_cpuins_data;

typedef struct t_cpuins t_cpuins;

typedef void (*core_machine_cpu_instruction_handler)(
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
    core_machine_instruction_timing instruction_timing;
    core_machine_cpu_timing_result timing_result;
    lib_u8 source_repeat_active;
    lib_u16 source_repeat_cs;
    lib_u32 source_repeat_eip;
    lib_u8 source_repeat_opcode;
    lib_u8 source_repeat_prefix;
    lib_u8 source_repeat_operand_size;
    lib_u8 source_repeat_address_size;
    const core_machine_cpu_bus_provider *bus;
    void *bus_context;
    const core_machine_cpu_execution_diagnostic_provider *diagnostic_provider;
    void *diagnostic_context;
    core_machine_cpu_external_cycle_provider external_cycle_provider;
    void *external_cycle_context;
    lib_u8 stop_requested;
    lib_u8 debug_pause_requested;
    lib_u8 reset_requested;
    lib_u8 shutdown_requested;
    /* Private execution-round outcome.  A successfully delivered synchronous
     * exception preserves its architectural delivery but must not be mistaken
     * for retirement of the faulting instruction by the machine clock owner. */
    lib_u8 instruction_in_progress;
    lib_u8 instruction_fault_delivered;
    /* Private CPU-execution state for post-instruction 80386 debug traps. */
    lib_u8 debug_trap_pending;
    lib_u8 debug_tf_before;
    lib_u8 debug_rf_before;
    lib_u32 debug_trap_cause;
    /* A temporary CPU-owned lexical fetch may validate bytes without any
     * architectural, transaction, trace, or diagnostic publication. */
    lib_u8 preview_mode;
    core_machine_cpu_memory_access_provenance memory_access_provenance;
    lib_u32 prefetch_linear;
    lib_u32 prefetch_expected_linear;
    lib_u8 prefetch_bytes[15];
    lib_u8 prefetch_count;
    lib_u8 prefetch_capacity;
    lib_u8 prefetch_valid;
    lib_u8 prefetch_expected_valid;
    lib_u8 prefetch_reservation_valid;
    lib_u32 prefetch_reservation_linear;
    lib_u8 prefetch_reservation_count;
    core_machine_cpu_profile cpu_profile;
    x86_fpu_profile fpu_profile;
    lib_u8 cpu_80386_cr_mov_ignores_mod;
    x86_fpu *fpu;
};

void core_machine_cpu_execution_context_initialize(
    core_machine_cpu_execution_context *context, t_cpu *cpu,
    t_cpuins *instructions, const core_machine_cpu_bus_provider *bus,
    void *bus_context);
lib_u8 core_machine_cpu_execution_load_segment(
    core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg,
    lib_u16 selector);
lib_u8 core_machine_cpu_execution_read_linear(
    core_machine_cpu_execution_context *context, lib_u32 linear,
    lib_uptr rdata, lib_u8 byte);
lib_u8 core_machine_cpu_execution_write_linear(
    core_machine_cpu_execution_context *context, lib_u32 linear,
    lib_uptr rdata, lib_u8 byte);
void core_machine_cpu_execution_initialize(
    core_machine_cpu_execution_context *context);
void core_machine_cpu_execution_reset(
    core_machine_cpu_execution_context *context);
void core_machine_cpu_execution_finalize(
    core_machine_cpu_execution_context *context);


#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
