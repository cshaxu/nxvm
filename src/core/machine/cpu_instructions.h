/* Copyright 2012-2014 Neko. */

/* VCPUINS implements 8086+ CPU instruction set. */

#ifndef CORE_MACHINE_CPU_INSTRUCTIONS_H
#define CORE_MACHINE_CPU_INSTRUCTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "type.h"

#define CORE_MACHINE_CPU_INSTRUCTION_MEMORY_ACCESS_CAPACITY 512u

#include "core/machine/cpu.h"
#include "core/machine/firmware_interface.h"
#include "core/machine/fpu.h"
#include "core/machine/fpu_interface.h"

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

typedef type_bool t_cpuins_data_prefix;

typedef struct {
    t_cpu_data_sreg *rsreg;
    type_unsigned_32 offset;
} t_cpuins_data_logical;

typedef struct {
    type_bool flagWrite;
    type_unsigned_32 byte;
    type_unsigned_32 linear;
    type_unsigned_64 data;
} t_cpuins_data_memory;

typedef struct {
    /* prefixes */
    t_cpuins_data_prefix_rep  prefix_rep;
    t_cpuins_data_prefix      prefix_oprsize;
    t_cpuins_data_prefix      prefix_addrsize;
    t_cpu_data_sreg *roverds, *roverss, *rmovsreg;

    /* execution control */
    t_cpu  oldcpu;
    type_bool flagInsLoop;
    type_bool flagMaskInt; /* if C_INT is disabled once */

    /* memory management */
    t_cpuins_data_logical mrm;
    type_virtual_address rrm, rr;
    type_unsigned_64 crm, cr, cimm;
    type_bool flagMem; /* if rm is in memory */
    type_bool flagLock;
    type_bool source_lsl_granularity_valid;
    type_bool source_lsl_page_granular;

    /* arithmetic operands */
    type_unsigned_64 opr1, opr2, result;
    type_unsigned_32 bit;
    t_cpuins_data_arithtype type;
    type_unsigned_32 udf; /* undefined eflags bits */

    /* exception handler */
    type_unsigned_32 except, excode;

    /* debugger */
    type_unsigned_32 linear;
    type_bool flagWR, flagWW, flagWE;
    type_unsigned_32 wrLinear, wwLinear, weLinear;

    /* cpu recorder */
    type_bool flagIgnore;
    /* ENTER accepts an 80186 lexical level up to 255 and performs at most 510
     * recorded stack accesses. This is executor bookkeeping for CPU debug
     * breakpoints, not the copied debugger-observation limit. */
    t_cpuins_data_memory mem[CORE_MACHINE_CPU_INSTRUCTION_MEMORY_ACCESS_CAPACITY];
    type_unsigned_16 msize;
    type_unsigned_8 oplen;
    type_unsigned_8 opcodes[15];
    type_unsigned_16 reccs;
    type_unsigned_32 receip;
} t_cpuins_data;

typedef struct t_cpuins t_cpuins;
typedef struct t_ram t_ram;
typedef struct t_port t_port;
typedef struct t_pic t_pic;
typedef struct core_machine_transaction_state core_machine_transaction_state;
typedef struct core_machine_cpu_execution_context
    core_machine_cpu_execution_context;

typedef type_status (*core_machine_cpu_firmware_interrupt_provider)(
    C_VOID *opaque, type_unsigned_8 vector,
    const core_machine_firmware_interrupt_frame *frame,
    core_machine_firmware_interrupt_result *result, type_bool *out_handled);
typedef C_VOID (*core_machine_cpu_instruction_handler)(
    core_machine_cpu_execution_context *context);

typedef enum core_machine_cpu_instruction_space {
    CORE_MACHINE_CPU_INSTRUCTION_PRIMARY,
    CORE_MACHINE_CPU_INSTRUCTION_0F,
    CORE_MACHINE_CPU_INSTRUCTION_FPU_ESCAPE
} core_machine_cpu_instruction_space;

/* CPU accesses share one transaction path. This label preserves their
 * architectural source for board-timing consumers without asserting that a
 * logical access is already an external bus cycle. */
typedef enum core_machine_cpu_memory_access_provenance {
    CORE_MACHINE_CPU_MEMORY_ACCESS_DATA = 0,
    CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_FETCH,
    CORE_MACHINE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH,
    CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_READ,
    CORE_MACHINE_CPU_MEMORY_ACCESS_PAGE_TABLE_WRITE
} core_machine_cpu_memory_access_provenance;

typedef enum core_machine_cpu_external_cycle_phase {
    CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN = 1,
    CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT,
    CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL,
    /* The Core CPU owner issued this named sequential request while the
     * preceding prefetch cycle was still in flight. */
    CORE_MACHINE_CPU_EXTERNAL_CYCLE_PHASE_OVERLAP_DECLARE
} core_machine_cpu_external_cycle_phase;


typedef C_VOID (*core_machine_cpu_external_cycle_provider)(C_VOID *context,
    core_machine_cpu_external_cycle_phase phase,
    core_machine_cpu_external_cycle_space space, type_unsigned_32 address,
    type_unsigned_8 bytes, type_bool write,
    core_machine_cpu_memory_access_provenance provenance);

typedef struct core_machine_cpu_instruction_metadata {
    core_machine_cpu_profile minimum_cpu;
    core_machine_fpu_profile minimum_fpu;
    C_INT valid;
} core_machine_cpu_instruction_metadata;

/* A lexical result is intentionally narrower than instruction decoding: it
 * names only the byte-layout components used by 80386 Jcc's `m` timing term.
 * It never validates operands or applies instruction semantics. */
typedef struct core_machine_cpu_instruction_lexeme {
    type_unsigned_8 byte_count;
    type_unsigned_8 component_count;
    type_bool available;
} core_machine_cpu_instruction_lexeme;

typedef struct {
    /* instruction dispatch */
    core_machine_cpu_instruction_handler insTable[0x100];
    core_machine_cpu_instruction_handler insTable_0f[0x100];
} t_cpuins_connect;

typedef struct core_machine_cpu_execution_diagnostic_provider {
    C_VOID (*record_instruction)(C_VOID *context, const C_VOID *cpu,
        const t_cpuins *instructions);
    C_VOID (*record_delivered_exception)(C_VOID *context, const C_VOID *cpu,
        const t_cpuins *instructions);
    C_VOID (*record_fault)(C_VOID *context, const C_VOID *cpu,
        const t_cpuins *instructions);
} core_machine_cpu_execution_diagnostic_provider;

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
    core_machine_transaction_state *transaction;
    t_pic *pic_master;
    t_pic *pic_slave;
    type_trace *trace;
    const core_machine_cpu_execution_diagnostic_provider *diagnostic_provider;
    C_VOID *diagnostic_context;
    core_machine_cpu_external_cycle_provider external_cycle_provider;
    C_VOID *external_cycle_context;
    core_machine_cpu_firmware_interrupt_provider firmware_interrupt_provider;
    C_VOID *firmware_interrupt_context;
    type_bool stop_requested;
    type_bool reset_requested;
    type_bool shutdown_requested;
    /* Private execution-round outcome.  A successfully delivered synchronous
     * exception preserves its architectural delivery but must not be mistaken
     * for retirement of the faulting instruction by the machine clock owner. */
    type_bool instruction_in_progress;
    type_bool instruction_fault_delivered;
    /* Private CPU-execution state for post-instruction 80386 debug traps. */
    type_bool debug_trap_pending;
    type_bool debug_tf_before;
    type_bool debug_rf_before;
    type_unsigned_32 debug_trap_cause;
    /* A temporary CPU-owned lexical fetch may validate bytes without any
     * architectural, transaction, trace, or diagnostic publication. */
    type_bool preview_mode;
    core_machine_cpu_memory_access_provenance memory_access_provenance;
    type_unsigned_32 prefetch_linear;
    type_unsigned_32 prefetch_expected_linear;
    type_unsigned_8 prefetch_bytes[15];
    type_unsigned_8 prefetch_count;
    type_unsigned_8 prefetch_capacity;
    type_bool prefetch_valid;
    type_bool prefetch_expected_valid;
    type_bool prefetch_reservation_valid;
    type_unsigned_32 prefetch_reservation_linear;
    type_unsigned_8 prefetch_reservation_count;
    core_machine_cpu_profile cpu_profile;
    core_machine_fpu_profile fpu_profile;
    type_bool cpu_80386_cr_mov_ignores_mod;
    core_machine_fpu *fpu;
};

C_VOID core_machine_cpu_execution_context_initialize(
    core_machine_cpu_execution_context *context, t_cpu *cpu,
    t_cpuins *instructions, t_ram *memory, t_port *port);
C_VOID core_machine_cpu_execution_context_bind_pic(
    core_machine_cpu_execution_context *context, t_pic *master,
    t_pic *slave);
C_VOID core_machine_cpu_execution_context_bind_diagnostic_provider(
    core_machine_cpu_execution_context *context,
    const core_machine_cpu_execution_diagnostic_provider *provider,
    C_VOID *provider_context);
C_VOID core_machine_cpu_execution_context_bind_fpu(
    core_machine_cpu_execution_context *context, core_machine_fpu *fpu);
C_VOID core_machine_cpu_execution_context_bind_external_cycle_provider(
    core_machine_cpu_execution_context *context,
    core_machine_cpu_external_cycle_provider provider, C_VOID *provider_context);
C_VOID core_machine_cpu_execution_context_bind_firmware_interrupt_provider(
    core_machine_cpu_execution_context *context,
    core_machine_cpu_firmware_interrupt_provider provider, C_VOID *provider_context);
C_VOID core_machine_cpu_execution_context_bind_transaction(
    core_machine_cpu_execution_context *context,
    core_machine_transaction_state *transaction);
type_bool core_machine_cpu_execution_load_segment(
    core_machine_cpu_execution_context *context, t_cpu_data_sreg *rsreg,
    type_unsigned_16 selector);
type_bool core_machine_cpu_execution_read_linear(
    core_machine_cpu_execution_context *context, type_unsigned_32 linear,
    type_virtual_address rdata, type_unsigned_8 byte);
type_bool core_machine_cpu_execution_write_linear(
    core_machine_cpu_execution_context *context, type_unsigned_32 linear,
    type_virtual_address rdata, type_unsigned_8 byte);
C_VOID core_machine_cpu_execution_initialize(
    core_machine_cpu_execution_context *context);
/* Core invalidates queued instruction bytes after a stopped-state physical write.
 * The caller does not need CPU or prefetch storage access. */
C_VOID core_machine_cpu_execution_reserve_prefetch(
    core_machine_cpu_execution_context *context);
C_VOID core_machine_cpu_execution_advance_prefetch_reservation(
    core_machine_cpu_execution_context *context);
C_VOID core_machine_cpu_execution_invalidate_prefetch(
    core_machine_cpu_execution_context *context);
C_VOID core_machine_cpu_execution_reset(
    core_machine_cpu_execution_context *context);
C_VOID core_machine_cpu_execution_refresh(
    core_machine_cpu_execution_context *context);
type_bool core_machine_cpu_execution_consume_instruction_fault_delivery(
    core_machine_cpu_execution_context *context);
C_VOID core_machine_cpu_execution_finalize(
    core_machine_cpu_execution_context *context);
core_machine_cpu_instruction_metadata core_machine_cpu_instruction_metadata_get(
    core_machine_cpu_instruction_space space, type_unsigned_8 opcode, type_unsigned_8 modrm);
type_bool core_machine_cpu_instruction_lexeme_scan(
    const type_unsigned_8 *bytes, type_unsigned_8 available_bytes,
    core_machine_cpu_profile profile, type_bool code_32,
    core_machine_cpu_instruction_lexeme *out_lexeme);
type_bool core_machine_cpu_execution_preview_lexeme(
    const core_machine_cpu_execution_context *context,
    core_machine_cpu_instruction_lexeme *out_lexeme);

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

#define VCPUINS_EXCEPT_FPU_UNSUPPORTED 0x40000000 /* internal FPU model stop */

/* 80386 real-address stack-limit wrap is an architectural shutdown, not an
 * interrupt-deliverable exception. */
#define VCPUINS_EXCEPT_SHUTDOWN 0x20000000

#define VCPUINS_EXCEPT_CE  0x80000000 /* 31 - internal case error */

#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
