#ifndef X86_CORE_CPU_TIMING_H
#define X86_CORE_CPU_TIMING_H
#include "lib/types/types_interface.h"


#include "x86-core/cpu/cpu_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* CPU execution selects once before asynchronous interrupt entry. */
lib_i32 x86_core_cpu_timing_select(x86_core_cpu_execution_context *context,
    x86_core_cpu_timing_result *out_result);

/* Legacy rule evaluators remain private to Core. They calculate a candidate
 * only; origin assignment and result publication belong exclusively to the
 * CPU timing selector. */
lib_i32 x86_core_cpu_string_io_source_instruction_cost(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks);
lib_i32 x86_core_cpu_80386_dynamic_multiply_cost(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks);
lib_i32 x86_core_cpu_l2_dynamic_arithmetic_model_cost(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks);
lib_i32 x86_core_cpu_80386_secondary_source_instruction_cost(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks);
lib_i32 x86_core_cpu_80386_privileged_source_instruction_cost(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks);
lib_i32 x86_core_cpu_primary_source_instruction_cost(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks);
lib_i32 x86_core_cpu_control_stack_source_instruction_cost(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks);
lib_i32 x86_core_cpu_8086_source_instruction_cost(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks);
lib_i32 x86_core_cpu_80186_source_instruction_cost(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks);
lib_i32 x86_core_cpu_80286_source_instruction_cost(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks);
lib_i32 x86_core_cpu_80386_source_instruction_cost(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks);
lib_i32 x86_core_cpu_compatibility_instruction_cost(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks);
/* The existing instruction condition owner supplies copied branch facts;
 * false means this is not a Jcc/LOOP/JMP or indirect CALL encoding. */
lib_bool x86_core_cpu_timing_branch_taken(
    const x86_core_cpu_execution_context *context, lib_u32 opcode_index, lib_bool *out_taken);
/* Sole selected-SX aggregate: disjoint nominal/L2, memory, decode and bus
 * terms, after the existing REP owner records this actual primitive. */
lib_i32 x86_core_cpu_80486_source_instruction_cost(x86_core_cpu_execution_context *context,
    lib_u64 *out_ticks);

/* Selected-SX cache-hit instruction cost only. Prefix/access/event penalties
 * belong to the selected timing owner, not this nominal calculation. A missing
 * source form returns false without changing the destination or qualification. */
lib_bool x86_core_cpu_80486_nominal_instruction_cost(
    const x86_core_cpu_execution_context *context, lib_u64 *out_ticks);

/* Explicit L2 arithmetic, alias and no-FPU WAIT models; disjoint from the original
 * nominal rows. Pure query, no access costs or hardware-exact qualification. */
lib_bool x86_core_cpu_80486_l2_instruction_cost(
    const x86_core_cpu_execution_context *context, lib_u64 *out_ticks);

/* E-14 entry plus its scoped accesses, using the mode before committed entry.
 * Zero rejects invalid observations. Task access decomposition is L2.
 * The caller owns event setup and instruction versus event transport. */
lib_u64 x86_core_cpu_80486_interrupt_entry_cost(
    const x86_core_cpu_execution_context *context,
    lib_u32 entry_cr0, lib_u32 entry_flags, lib_u8 entry_cpl, lib_bool task_entry);
lib_u64 x86_core_cpu_80486_exception_setup_cost(
    const x86_core_cpu_execution_context *context, lib_u8 vector);

/* Table 8-1 SX reference intervals and the actual admitted SMM image traffic.
 * Pure L2 zero-wait envelope; the executing receiver owns actual extra waits
 * and prior fetch separately. */
lib_bool x86_core_cpu_80486_smm_cost(
    const x86_core_cpu_execution_context *context, lib_bool entry,
    lib_bool complete, lib_u64 prefix_service, lib_u64 *out_ticks);

/* Memory subtotal only; fetch-cache, prefixes, EA and event setup have separate
 * source terms. Optional modeled identifies L2 operand/translation estimates.
 * Unallocated/failed accesses leave both outputs unchanged. */
lib_bool x86_core_cpu_80486_memory_penalty(
    const x86_core_cpu_execution_context *context, lib_u64 *out_ticks, lib_bool *out_modeled);

/* Per decoded instruction, not per REP continuation. E-11 prefixes and the
 * E.2 index/displacement assumptions; nominal LEA already includes its index.
 * AGI uses the actual adjacent GPR-write record, including same-value writes.
 * Optional modeled identifies redundant/ignored REP prefix estimates. */
lib_bool x86_core_cpu_80486_decode_penalty(
    const x86_core_cpu_execution_context *context, lib_u64 *out_ticks, lib_bool *out_modeled);

/* Private L2 bus observations; only actual CPU execution records transfers.
 * Core is the clock owner; the pure cost query never advances the FIFO. */
void x86_core_cpu_bus_reset(x86_core_cpu_execution_context *context);
void x86_core_cpu_bus_prepare_read(x86_core_cpu_execution_context *context,
    lib_bool ordered);
void x86_core_cpu_bus_prepare_write(x86_core_cpu_execution_context *context);
void x86_core_cpu_bus_read_complete(x86_core_cpu_execution_context *context,
    lib_u32 address, lib_u8 bytes, lib_u64 wait_ticks);
void x86_core_cpu_bus_write_complete(x86_core_cpu_execution_context *context,
    lib_u32 address, lib_u8 bytes, lib_u64 wait_ticks, lib_bool cache_hit,
    lib_bool buffered);
void x86_core_cpu_bus_drain(x86_core_cpu_execution_context *context, lib_bool stalled);
void x86_core_cpu_bus_invalidate(x86_core_cpu_execution_context *context);
/* Observation scope only, never the live FIFO or service credit. Entry and
 * failed-prefix owners consume the same checked bus formula as retirement. */
void x86_core_cpu_bus_clear_observation(x86_core_cpu_execution_context *context);
lib_bool x86_core_cpu_80486_bus_scope_cost(
    const x86_core_cpu_execution_context *context, lib_u64 baseline_ticks, lib_u64 *out_ticks);
lib_bool x86_core_cpu_80486_bus_instruction_cost(
    const x86_core_cpu_execution_context *context, lib_u64 baseline_ticks,
    lib_u64 *out_ticks, lib_bool *out_modeled);

#ifdef __cplusplus
}
#endif

#endif
