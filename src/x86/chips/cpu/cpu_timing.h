#ifndef CORE_MACHINE_CPU_TIMING_H
#define CORE_MACHINE_CPU_TIMING_H
#include "lib/types/types_interface.h"


#include "x86/chips/cpu/cpu_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Legacy rule evaluators remain private to Core. They calculate a candidate
 * only; origin assignment and result publication belong exclusively to the
 * selector above. */
lib_i32 core_machine_string_io_source_instruction_cost(core_machine_cpu_execution_context *context,
    lib_u64 *out_ticks);
lib_i32 core_machine_80386_dynamic_multiply_cost(core_machine_cpu_execution_context *context,
    lib_u64 *out_ticks);
lib_i32 core_machine_l2_dynamic_arithmetic_model_cost(core_machine_cpu_execution_context *context,
    lib_u64 *out_ticks);
lib_i32 core_machine_80386_secondary_source_instruction_cost(core_machine_cpu_execution_context *context,
    lib_u64 *out_ticks);
lib_i32 core_machine_80386_privileged_source_instruction_cost(core_machine_cpu_execution_context *context,
    lib_u64 *out_ticks);
lib_i32 core_machine_primary_source_instruction_cost(core_machine_cpu_execution_context *context,
    lib_u64 *out_ticks);
lib_i32 core_machine_control_stack_source_instruction_cost(core_machine_cpu_execution_context *context,
    lib_u64 *out_ticks);
lib_i32 core_machine_8086_source_instruction_cost(core_machine_cpu_execution_context *context,
    lib_u64 *out_ticks);
lib_i32 core_machine_80186_source_instruction_cost(core_machine_cpu_execution_context *context,
    lib_u64 *out_ticks);
lib_i32 core_machine_80286_source_instruction_cost(core_machine_cpu_execution_context *context,
    lib_u64 *out_ticks);
lib_i32 core_machine_80386_source_instruction_cost(core_machine_cpu_execution_context *context,
    lib_u64 *out_ticks);
lib_i32 core_machine_compatibility_instruction_cost(core_machine_cpu_execution_context *context,
    lib_u64 *out_ticks);

#ifdef __cplusplus
}
#endif

#endif
