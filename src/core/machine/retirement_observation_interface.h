#ifndef CORE_MACHINE_RETIREMENT_OBSERVATION_INTERFACE_H
#define CORE_MACHINE_RETIREMENT_OBSERVATION_INTERFACE_H

#include "core/machine/cpu_interface.h"
#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

typedef enum core_machine_retirement_timing_disposition {
    CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED = 1,
    CORE_MACHINE_RETIREMENT_TIMING_SOURCE_UNALLOCATED
} core_machine_retirement_timing_disposition;

typedef enum core_machine_retirement_timing_origin {
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED = 0,
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_STRING_IO,
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80386_DYNAMIC_MULTIPLY,
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC,
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80386_SECONDARY,
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80386_PRIVILEGED,
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_PRIMARY,
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_CONTROL_STACK,
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80186_FALLBACK,
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80286_FALLBACK,
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_80386_FALLBACK,
    CORE_MACHINE_RETIREMENT_TIMING_ORIGIN_COMPATIBILITY
} core_machine_retirement_timing_origin;

typedef enum core_machine_retirement_modrm_form {
    CORE_MACHINE_RETIREMENT_MODRM_UNAVAILABLE = 0,
    CORE_MACHINE_RETIREMENT_MODRM_REGISTER,
    CORE_MACHINE_RETIREMENT_MODRM_MEMORY
} core_machine_retirement_modrm_form;

typedef enum core_machine_retirement_control_outcome {
    CORE_MACHINE_RETIREMENT_CONTROL_NONE = 0,
    CORE_MACHINE_RETIREMENT_CONTROL_FALLTHROUGH,
    CORE_MACHINE_RETIREMENT_CONTROL_TAKEN
} core_machine_retirement_control_outcome;

typedef enum core_machine_retirement_repeat_phase {
    CORE_MACHINE_RETIREMENT_REPEAT_NONE = 0,
    CORE_MACHINE_RETIREMENT_REPEAT_PRIMITIVE,
    CORE_MACHINE_RETIREMENT_REPEAT_ZERO_COUNT,
    CORE_MACHINE_RETIREMENT_REPEAT_FIRST,
    CORE_MACHINE_RETIREMENT_REPEAT_CONTINUATION
} core_machine_retirement_repeat_phase;

typedef enum core_machine_retirement_io_direction {
    CORE_MACHINE_RETIREMENT_IO_NONE = 0,
    CORE_MACHINE_RETIREMENT_IO_READ,
    CORE_MACHINE_RETIREMENT_IO_WRITE
} core_machine_retirement_io_direction;

/* The timing form is an opaque Core-owned identifier. A classified path
 * without a ledger lookup intentionally reports this sentinel. */
#define CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ((type_unsigned_32)-1)
#define CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE ((type_unsigned_8)-1)

/* A semantic retirement identity; it deliberately contains no instruction
 * addresses, literal operands, firmware identity, or elapsed-time result. */
typedef struct core_machine_retirement_eligibility_key {
    core_machine_cpu_profile cpu_profile;
    core_machine_retirement_timing_origin timing_origin;
    type_unsigned_32 source_timing_form_id;
    type_unsigned_8 opcode;
    type_unsigned_8 escape_opcode;
    core_machine_retirement_modrm_form modrm_form;
    type_unsigned_8 modrm_extension;
    core_machine_retirement_control_outcome control_outcome;
    type_unsigned_8 next_lexeme_components;
    core_machine_retirement_repeat_phase repeat_phase;
    type_unsigned_8 cpl;
    type_bool protected_mode;
    type_bool virtual_8086_mode;
    type_bool operand_size_32;
    type_bool address_size_32;
    type_bool lock_prefix;
    type_unsigned_8 repeat_prefix;
} core_machine_retirement_eligibility_key;

typedef struct core_machine_retirement_qualification_descriptor {
    const core_machine_retirement_eligibility_key *entries;
    STD_SIZE_T entry_count;
} core_machine_retirement_qualification_descriptor;

typedef struct core_machine_retirement_observation {
    type_unsigned_64 sequence;
    type_unsigned_64 elapsed_ticks;
    type_unsigned_64 timeline_ticks;
    type_unsigned_64 source_ticks;
    core_machine_retirement_eligibility_key eligibility_key;
    core_machine_cpu_execution_point point;
    core_machine_cpu_profile cpu_profile;
    core_machine_retirement_timing_disposition timing_disposition;
    core_machine_retirement_timing_origin timing_origin;
    type_unsigned_32 source_timing_form_id;
    type_unsigned_32 timing_key_id;
    type_unsigned_32 formula_inputs;
    core_machine_retirement_modrm_form modrm_form;
    type_unsigned_8 modrm_extension;
    core_machine_retirement_control_outcome control_outcome;
    type_unsigned_8 next_lexeme_components;
    core_machine_retirement_repeat_phase repeat_phase;
    core_machine_retirement_io_direction io_direction;
    type_unsigned_16 io_port;
    type_unsigned_8 io_bytes;
    type_unsigned_32 io_value;
    type_unsigned_8 cpl;
    type_bool protected_mode;
    type_bool virtual_8086_mode;
    type_bool operand_size_32;
    type_bool address_size_32;
    type_bool lock_prefix;
    type_unsigned_8 repeat_prefix;
} core_machine_retirement_observation;

typedef C_VOID (*core_machine_retirement_observation_callback)(
    C_VOID *context, const core_machine_retirement_observation *observation);

typedef struct core_machine_retirement_observation_provider {
    core_machine_retirement_observation_callback callback;
    C_VOID *context;
} core_machine_retirement_observation_provider;

/* Installs or removes the optional copied retirement observer while stopped or
 * paused. The callback observes only Core-owned copied state and is invoked
 * before physical-contract rejection or elapsed-time publication. */
type_status core_machine_set_retirement_observation_provider(
    core_machine *machine,
    const core_machine_retirement_observation_provider *provider);

#ifdef __cplusplus
}
#endif

#endif
