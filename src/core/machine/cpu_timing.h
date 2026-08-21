#ifndef CORE_MACHINE_CPU_TIMING_H
#define CORE_MACHINE_CPU_TIMING_H

#include "type.h"

#include "core/machine/retirement_observation_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

/* A result is built once for every successful CPU timing selection.  The
 * decoder-form key is Core-private; the S3 result verifier maps it to the
 * corresponding manifest record without exposing machine storage. */
typedef struct core_machine_cpu_timing_result {
    type_unsigned_64 ticks;
    type_unsigned_32 key_id;
    type_unsigned_32 formula_inputs;
    core_machine_retirement_timing_origin retirement_origin;
    type_bool source_timing_unallocated;
} core_machine_cpu_timing_result;

#define CORE_MACHINE_CPU_TIMING_INPUT_MODRM      (1u << 0)
#define CORE_MACHINE_CPU_TIMING_INPUT_CONTROL    (1u << 1)
#define CORE_MACHINE_CPU_TIMING_INPUT_REPEAT     (1u << 2)
#define CORE_MACHINE_CPU_TIMING_INPUT_MODE       (1u << 3)
#define CORE_MACHINE_CPU_TIMING_INPUT_SIZE       (1u << 4)
#define CORE_MACHINE_CPU_TIMING_INPUT_LOCK       (1u << 5)

/* B0's only successful-retirement CPU timing selection entry. */
C_INT core_machine_cpu_timing_select(core_machine *machine,
    core_machine_cpu_timing_result *out_result);

/* Legacy rule evaluators remain private to Core. They calculate a candidate
 * only; origin assignment and result publication belong exclusively to the
 * selector above. */
C_INT core_machine_string_io_source_instruction_cost(core_machine *machine,
    type_unsigned_64 *out_ticks);
C_INT core_machine_legacy_dynamic_arithmetic_model_cost(core_machine *machine,
    type_unsigned_64 *out_ticks);
C_INT core_machine_80386_secondary_source_instruction_cost(core_machine *machine,
    type_unsigned_64 *out_ticks);
C_INT core_machine_80386_privileged_source_instruction_cost(core_machine *machine,
    type_unsigned_64 *out_ticks);
C_INT core_machine_primary_source_instruction_cost(core_machine *machine,
    type_unsigned_64 *out_ticks);
C_INT core_machine_control_stack_source_instruction_cost(core_machine *machine,
    type_unsigned_64 *out_ticks);
C_INT core_machine_8086_source_instruction_cost(core_machine *machine,
    type_unsigned_64 *out_ticks);
C_INT core_machine_80186_source_instruction_cost(core_machine *machine,
    type_unsigned_64 *out_ticks);
C_INT core_machine_80286_source_instruction_cost(core_machine *machine,
    type_unsigned_64 *out_ticks);
C_INT core_machine_80386_source_instruction_cost(core_machine *machine,
    type_unsigned_64 *out_ticks);
C_INT core_machine_compatibility_instruction_cost(core_machine *machine,
    type_unsigned_64 *out_ticks);

#ifdef __cplusplus
}
#endif

#endif
