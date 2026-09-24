#ifndef CORE_MACHINE_CPU_TIMING_H
#define CORE_MACHINE_CPU_TIMING_H
#include "lib/types/types_interface.h"


#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/retirement_observation_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

/* A result is built once for every successful CPU timing selection.  The
 * decoder-form key is Core-private; the S3 result verifier maps it to the
 * corresponding manifest record without exposing machine storage. */
typedef struct core_machine_cpu_timing_result {
    lib_u64 ticks;
    lib_u32 key_id;
    lib_u32 formula_inputs;
    core_machine_retirement_timing_origin retirement_origin;
    lib_u8 source_timing_unallocated;
} core_machine_cpu_timing_result;

#define CORE_MACHINE_CPU_TIMING_INPUT_MODRM      (1u << 0)
#define CORE_MACHINE_CPU_TIMING_INPUT_CONTROL    (1u << 1)
#define CORE_MACHINE_CPU_TIMING_INPUT_REPEAT     (1u << 2)
#define CORE_MACHINE_CPU_TIMING_INPUT_MODE       (1u << 3)
#define CORE_MACHINE_CPU_TIMING_INPUT_SIZE       (1u << 4)
#define CORE_MACHINE_CPU_TIMING_INPUT_LOCK       (1u << 5)
#define CORE_MACHINE_CPU_TIMING_INPUT_EFFECTIVE_ADDRESS (1u << 6)
#define CORE_MACHINE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE (1u << 7)
#define CORE_MACHINE_CPU_TIMING_INPUT_ODD_WORD   (1u << 8)
#define CORE_MACHINE_CPU_TIMING_INPUT_REPEAT_PHASE (1u << 9)
#define CORE_MACHINE_CPU_TIMING_INPUT_GROUP3_OPERAND (1u << 10)
#define CORE_MACHINE_CPU_TIMING_INPUT_WAIT_TICKS      (1u << 11)

/* B0's only successful-retirement CPU timing selection entry. */
lib_i32 core_machine_cpu_timing_select(core_machine *machine,
    core_machine_cpu_timing_result *out_result);
/* Shared checked accumulation for timing selection and the retained run loop. */
lib_i32 core_machine_timing_add_ticks(lib_u64 *value,
    lib_u64 delta);
lib_u64 core_machine_cpu_timing_maximum_ticks(
    core_machine_cpu_profile profile,
    const core_machine_instruction_timing *timing);

/* Legacy rule evaluators remain private to Core. They calculate a candidate
 * only; origin assignment and result publication belong exclusively to the
 * selector above. */
lib_i32 core_machine_string_io_source_instruction_cost(core_machine *machine,
    lib_u64 *out_ticks);
lib_i32 core_machine_80386_dynamic_multiply_cost(core_machine *machine,
    lib_u64 *out_ticks);
lib_i32 core_machine_l2_dynamic_arithmetic_model_cost(core_machine *machine,
    lib_u64 *out_ticks);
lib_i32 core_machine_80386_secondary_source_instruction_cost(core_machine *machine,
    lib_u64 *out_ticks);
lib_i32 core_machine_80386_privileged_source_instruction_cost(core_machine *machine,
    lib_u64 *out_ticks);
lib_i32 core_machine_primary_source_instruction_cost(core_machine *machine,
    lib_u64 *out_ticks);
lib_i32 core_machine_control_stack_source_instruction_cost(core_machine *machine,
    lib_u64 *out_ticks);
lib_i32 core_machine_8086_source_instruction_cost(core_machine *machine,
    lib_u64 *out_ticks);
lib_i32 core_machine_80186_source_instruction_cost(core_machine *machine,
    lib_u64 *out_ticks);
lib_i32 core_machine_80286_source_instruction_cost(core_machine *machine,
    lib_u64 *out_ticks);
lib_i32 core_machine_80386_source_instruction_cost(core_machine *machine,
    lib_u64 *out_ticks);
lib_i32 core_machine_compatibility_instruction_cost(core_machine *machine,
    lib_u64 *out_ticks);

#ifdef __cplusplus
}
#endif

#endif
