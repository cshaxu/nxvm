/* Copyright 2012-2014 Neko. */

/* VCPUINS implements 8086+ CPU instruction set. */

#ifndef X86_CORE_CPU_INSTRUCTIONS_H
#define X86_CORE_CPU_INSTRUCTIONS_H

#ifdef __cplusplus
extern "C" {
#endif
#include "lib/types/types_interface.h"


#define X86_CORE_CPU_INSTRUCTION_MEMORY_ACCESS_CAPACITY 512u

#include "x86-core/cpu/cpu.h"
#include "x86-core/fpu/fpu_interface.h"

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

typedef enum {
    CPU_INTERRUPT_SHADOW_NONE,
    CPU_INTERRUPT_SHADOW_INTR,
    CPU_INTERRUPT_SHADOW_SEGMENT
} cpu_interrupt_shadow;

typedef enum cpu_wait_state {
    CPU_WAIT_STATE_NONE,
    CPU_WAIT_STATE_TEST,
    CPU_WAIT_STATE_ESC_RESTART
} cpu_wait_state;

/* Architectural masks only: provider/internal failures are not fault pairs. */
static inline lib_bool cpu_exception_requires_double_fault(
    x86_core_cpu_profile profile, lib_u32 first, lib_u32 second)
{
    const lib_u32 contributory = VCPUINS_EXCEPT_DE | VCPUINS_EXCEPT_TS |
        VCPUINS_EXCEPT_NP | VCPUINS_EXCEPT_SS | VCPUINS_EXCEPT_GP;
    const lib_u32 later_contributory = contributory | VCPUINS_EXCEPT_09;

    if (!cpu_profile_has_protected_architecture(profile)) return LIB_FALSE;
    if (profile == X86_CORE_CPU_PROFILE_80286)
        return (first & contributory) != 0u;
    return ((first & later_contributory) != 0u &&
            (second & later_contributory) != 0u) ||
        (first == VCPUINS_EXCEPT_PF &&
            ((second & later_contributory) != 0u || second == VCPUINS_EXCEPT_PF));
}

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

    /* memory management */
    t_cpuins_data_logical mrm;
    lib_uptr rrm, rr;
    lib_u64 crm, cr, cimm;
    lib_u8 flagMem; /* if rm is in memory */
    lib_u8 flagLock;
    /* Selected-SX decode facts, reset with this instruction record. */
    lib_u8 timing_ea_base_mask;
    lib_bool timing_ea_indexed;
    lib_bool timing_ea_displacement;
    lib_bool timing_immediate;
    lib_u8 timing_transfer_type;
    lib_u8 timing_transfer_parameters;
    lib_u8 timing_unaccessed_descriptors;
    lib_bool timing_invlpg_hit;
    /* Actual destination identity and one adjacent instruction, not value deltas. */
    lib_u8 timing_gpr_writes;
    lib_u8 timing_previous_gpr_writes;
    lib_u16 timing_previous_opcode;
    lib_bool timing_stack_access;
    /* Actual completed access facts, not a second cache/TLB state. The walk
     * histogram is indexed by zero, one or two attempted A/D updates in a
     * completed walk. Writes also retain the accepted prefix of failed walks. */
    lib_u32 timing_fetch_bus_ticks; /* L2 serialized 2-1 burst baseline; waits are external. */
    lib_u64 timing_external_wait_ticks;
    lib_bool timing_external_wait_overflow;
    lib_u64 timing_bus_service_ticks;
    lib_u64 timing_bus_stall_ticks;
    lib_u64 timing_bus_wait_ticks;
    lib_u64 timing_bus_invalidation_ticks;
    /* Accepted physical work, excluding Core waits; entry allocation prevents
     * nested handlers from charging the same prefix twice. Not a clock. */
    lib_u64 timing_transfer_ticks;
    lib_u64 timing_entry_transfer_ticks;
    lib_bool timing_cache_used;
    lib_u32 timing_data_misses;
    lib_u32 timing_missed_reads; /* Completed logical DATA reads with a physical miss. */
    lib_u32 timing_descriptor_misses;
    lib_u32 timing_missed_descriptors;
    lib_u32 timing_idt_misses;
    lib_u8 timing_task_gate_miss_mask;
    lib_u32 timing_pair_misses;
    lib_u32 timing_tss_field_misses;
    lib_u32 timing_missed_tss_fields;
    lib_u32 timing_repeat_previous_misses;
    lib_u32 timing_locked_reads;
    lib_u32 timing_table_misses;
    lib_u32 timing_unaligned_accesses;
    lib_u32 timing_tlb_walks[3];
    lib_u32 timing_tlb_updates;
    lib_bool memory_lock_active; /* Actual LOCK signal, not lexical prefix. */
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
    t_cpuins_data_memory mem[X86_CORE_CPU_INSTRUCTION_MEMORY_ACCESS_CAPACITY];
    lib_u16 msize;
    lib_u8 oplen;
    lib_u8 opcodes[15];
    lib_u16 reccs;
    lib_u32 receip;
} t_cpuins_data;

typedef struct t_cpuins t_cpuins;

typedef void (*x86_core_cpu_instruction_handler)(
    x86_core_cpu_execution_context *context);

typedef struct {
    /* instruction dispatch */
    x86_core_cpu_instruction_handler insTable[0x100];
    x86_core_cpu_instruction_handler insTable_0f[0x100];
} t_cpuins_connect;

struct t_cpuins {
    t_cpuins_data data;
    t_cpuins_connect connect;
};

#define X86_CPU_TLB_SETS 8u
#define X86_CPU_TLB_WAYS 4u
#define X86_CPU_CACHE_SETS 128u
#define X86_CPU_CACHE_WAYS 4u
#define X86_CPU_CACHE_LINE_DWORDS 4u
#define X86_CPU_WRITE_BUFFER_CAPACITY 4u

typedef struct cpu_write_buffer {
    lib_u64 ticks;
    lib_bool cache_hit;
} cpu_write_buffer;

typedef struct cpu_bus_timing {
    cpu_write_buffer writes[X86_CPU_WRITE_BUFFER_CAPACITY];
    /* Predicted service already consumed by the atomic execution owner. */
    lib_u64 service_credit;
    lib_u8 head;
    lib_u8 count;
    lib_bool write_active;
    lib_bool invalid;
} cpu_bus_timing;

typedef struct cpu_cache_line {
    lib_u32 tag;
    lib_u32 data[X86_CPU_CACHE_LINE_DWORDS];
    lib_bool valid;
} cpu_cache_line;

typedef struct cpu_tlb_entry {
    lib_u32 linear;
    lib_u32 physical;
    lib_u32 attributes;
    lib_bool valid;
} cpu_tlb_entry;

/* One composition-owned executor context names the existing CPU and decoder. */
struct x86_core_cpu_execution_context {
    t_cpu *cpu;
    t_cpuins *instructions;
    x86_core_cpu_instruction_timing instruction_timing;
    x86_core_cpu_timing_result timing_result;
    /* Completed facts survive asynchronous decoder reuse until the next round. */
    x86_core_cpu_instruction_observation completed_observation;
    lib_bool completed_observation_valid;
    lib_bool source_repeat_active;
    lib_u16 source_repeat_cs;
    lib_u32 source_repeat_eip;
    lib_u8 source_repeat_opcode;
    lib_u8 source_repeat_prefix;
    lib_u8 source_repeat_operand_size;
    lib_u8 source_repeat_address_size;
    /* Selected-SX identity includes segment/redundant prefixes, not a second
     * repeat lifecycle. Older profiles retain their existing identity. */
    lib_u8 source_repeat_byte_count;
    lib_u8 source_repeat_bytes[15];
    lib_u32 source_repeat_data_misses;
    cpu_bus_timing bus_timing;
    lib_bool timing_bus_active;
    const x86_core_cpu_bus_provider *bus;
    void *bus_context;
    const x86_core_cpu_execution_diagnostic_provider *diagnostic_provider;
    void *diagnostic_context;
    x86_core_cpu_external_cycle_provider external_cycle_provider;
    void *external_cycle_context;
    lib_u8 stop_requested;
    lib_u8 debug_pause_requested;
    x86_core_cpu_reset_kind reset_requested;
    lib_u32 smbase;
    lib_bool smm_active;
    lib_bool smi_asserted;
    lib_bool smi_pending;
    x86_core_cpu_special_cycle special_cycle;
    lib_bool stop_clock_asserted;
    lib_bool stop_granted;
    lib_bool clock_running;
    lib_bool flush_asserted;
    lib_bool flush_pending;
    lib_bool nmi_asserted;
    lib_bool io_restart_valid;
    lib_u32 io_restart_eip;
    lib_u32 io_restart_esi;
    lib_u32 io_restart_ecx;
    lib_u32 io_restart_edi;
    lib_u8 shutdown_requested;
    /* Resident chip state is independent of the consumable board signal. */
    enum { CPU_SHUTDOWN_NONE, CPU_SHUTDOWN_WAITING,
        CPU_SHUTDOWN_RESET_ONLY, CPU_SHUTDOWN_SMM } shutdown_state;
    /* Delivery without retirement: a synchronous fault or interrupt waking an
     * already halted CPU preserves entry but publishes no instruction tick. */
    lib_u8 instruction_in_progress;
    lib_u8 instruction_fault_delivered;
    /* REP compare faults restore entry flags, not the last iteration's result. */
    lib_bool repeat_compare_flags_valid;
    lib_u32 repeat_compare_flags;
    /* A 286 automatic ESC wait restores the unretired ESC and sleeps until
     * TEST/BUSY releases. This is distinct from an already retired WAIT or
     * HLT instruction. */
    lib_bool instruction_wait_stall;
    cpu_wait_state wait_state;
    lib_u64 wait_poll_remaining_ticks;
    lib_bool instruction_task_switched;
    t_cpu instruction_task_checkpoint;
    cpu_interrupt_shadow interrupt_shadow;
    lib_bool debug_segment_shadow_before;
    lib_bool nmi_in_service;
    lib_bool nmi_masked;
    lib_bool nmi_pending;
    /* Private CPU-execution state for post-instruction 80386 debug traps. */
    lib_u8 debug_trap_pending;
    lib_u8 debug_tf_before;
    lib_u8 debug_rf_before;
    lib_u32 debug_trap_cause;
    /* A temporary CPU-owned lexical fetch may validate bytes without any
     * architectural, transaction, trace, or diagnostic publication. */
    lib_u8 preview_mode;
    cpu_tlb_entry tlb[X86_CPU_TLB_SETS][X86_CPU_TLB_WAYS];
    lib_u8 tlb_lru[X86_CPU_TLB_SETS];
    cpu_cache_line cache[X86_CPU_CACHE_SETS][X86_CPU_CACHE_WAYS];
    lib_u8 cache_lru[X86_CPU_CACHE_SETS];
    lib_u32 cache_fill[X86_CPU_CACHE_LINE_DWORDS];
    lib_u32 cache_read[X86_CPU_CACHE_LINE_DWORDS];
    x86_core_cpu_memory_access_provenance memory_access_provenance;
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
    x86_core_cpu_profile cpu_profile;
    x86_fpu_profile fpu_profile;
    lib_u8 cpu_80386_cr_mov_ignores_mod;
    x86_fpu *fpu;
};

void x86_core_cpu_execution_context_initialize(
    x86_core_cpu_execution_context *context, t_cpu *cpu,
    t_cpuins *instructions, const x86_core_cpu_bus_provider *bus,
    void *bus_context);
lib_u8 x86_core_cpu_execution_load_segment(
    x86_core_cpu_execution_context *context, t_cpu_data_sreg *rsreg,
    lib_u16 selector);
lib_u8 x86_core_cpu_execution_read_linear(
    x86_core_cpu_execution_context *context, lib_u32 linear,
    lib_uptr rdata, lib_u8 byte);
lib_u8 x86_core_cpu_execution_write_linear(
    x86_core_cpu_execution_context *context, lib_u32 linear,
    lib_uptr rdata, lib_u8 byte);
void x86_core_cpu_execution_initialize(
    x86_core_cpu_execution_context *context);
void x86_core_cpu_execution_reset(
    x86_core_cpu_execution_context *context);
void x86_core_cpu_execution_invalidate_tlb(
    x86_core_cpu_execution_context *context);
lib_u32 x86_core_cpu_instruction_prefix_count(const t_cpuins_data *data);


#ifdef __cplusplus
}/*_EOCD_*/
#endif

#endif
