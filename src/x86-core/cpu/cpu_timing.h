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
/* Sole selected-SX candidate. Incomplete C16 costs stay explicitly unallocated
 * while the existing repeat/result owner records their actual lifetime. */
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

/* E-14 entry subtotal from the mode before actual committed entry. The caller
 * owns event-specific setup and successful-instruction versus event transport. */
lib_u64 x86_core_cpu_80486_interrupt_entry_cost(
    const x86_core_cpu_execution_context *context,
    lib_u32 entry_cr0, lib_u32 entry_flags, lib_u8 entry_cpl, lib_bool task_entry);
lib_u64 x86_core_cpu_80486_exception_setup_cost(
    const x86_core_cpu_execution_context *context, lib_u8 vector);

/* Memory subtotal only; fetch-cache, prefixes, EA and event setup have separate
 * source terms. Optional modeled identifies L2 operand/translation estimates.
 * Unallocated/failed accesses leave both outputs unchanged. */
lib_bool x86_core_cpu_80486_memory_penalty(
    const x86_core_cpu_execution_context *context, lib_u64 *out_ticks, lib_bool *out_modeled);

/* Per decoded instruction, not per REP continuation. E-11 prefixes and the
 * E.2 index/displacement assumptions; nominal LEA already includes its index.
 * AGI uses the actual adjacent GPR-write record, including same-value writes. */
lib_bool x86_core_cpu_80486_decode_penalty(
    const x86_core_cpu_execution_context *context, lib_u64 *out_ticks);

#ifdef __cplusplus
}
#endif

#endif
