#ifndef CORE_MACHINE_RETIREMENT_OBSERVATION_INTERFACE_H
#define CORE_MACHINE_RETIREMENT_OBSERVATION_INTERFACE_H

#include "app-nxvm/devices/cpu_interface.h"
#include "lib/types/types_interface.h"

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
#define CORE_MACHINE_RETIREMENT_SOURCE_FORM_UNATTRIBUTED ((lib_u32)-1)
#define CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE ((lib_u8)-1)

/* A semantic retirement identity; it deliberately contains no instruction
 * addresses, literal operands, firmware identity, or elapsed-time result. */
typedef struct core_machine_retirement_eligibility_key {
    core_machine_cpu_profile cpu_profile;
    core_machine_retirement_timing_origin timing_origin;
    lib_u32 source_timing_form_id;
    lib_u8 opcode;
    lib_u8 escape_opcode;
    core_machine_retirement_modrm_form modrm_form;
    lib_u8 modrm_extension;
    core_machine_retirement_control_outcome control_outcome;
    lib_u8 next_lexeme_components;
    core_machine_retirement_repeat_phase repeat_phase;
    lib_u8 cpl;
    lib_u8 protected_mode;
    lib_u8 virtual_8086_mode;
    lib_u8 operand_size_32;
    lib_u8 address_size_32;
    lib_u8 lock_prefix;
    lib_u8 repeat_prefix;
} core_machine_retirement_eligibility_key;

typedef struct core_machine_retirement_qualification_descriptor {
    const core_machine_retirement_eligibility_key *entries;
    lib_size entry_count;
} core_machine_retirement_qualification_descriptor;

typedef struct core_machine_retirement_observation {
    lib_u64 sequence;
    lib_u64 elapsed_ticks;
    lib_u64 timeline_ticks;
    lib_u64 source_ticks;
    core_machine_retirement_eligibility_key eligibility_key;
    core_machine_cpu_execution_point point;
    core_machine_cpu_profile cpu_profile;
    core_machine_retirement_timing_disposition timing_disposition;
    core_machine_retirement_timing_origin timing_origin;
    lib_u32 source_timing_form_id;
    lib_u32 timing_key_id;
    lib_u32 formula_inputs;
    core_machine_retirement_modrm_form modrm_form;
    lib_u8 modrm_extension;
    core_machine_retirement_control_outcome control_outcome;
    lib_u8 next_lexeme_components;
    core_machine_retirement_repeat_phase repeat_phase;
    core_machine_retirement_io_direction io_direction;
    lib_u16 io_port;
    lib_u8 io_bytes;
    lib_u32 io_value;
    lib_u8 cpl;
    lib_u8 protected_mode;
    lib_u8 virtual_8086_mode;
    lib_u8 operand_size_32;
    lib_u8 address_size_32;
    lib_u8 lock_prefix;
    lib_u8 repeat_prefix;
} core_machine_retirement_observation;

typedef void (*core_machine_retirement_observation_callback)(
    void *context, const core_machine_retirement_observation *observation);

typedef struct core_machine_retirement_observation_provider {
    core_machine_retirement_observation_callback callback;
    void *context;
} core_machine_retirement_observation_provider;

/* Installs or removes the optional copied retirement observer while stopped or
 * paused. The callback observes only Core-owned copied state and is invoked
 * before physical-contract rejection or elapsed-time publication. */
lib_status core_machine_set_retirement_observation_provider(
    core_machine *machine,
    const core_machine_retirement_observation_provider *provider);

#ifdef __cplusplus
}
#endif

#endif
