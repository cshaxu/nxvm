#ifndef X86_CORE_MACHINE_RETIREMENT_OBSERVATION_INTERFACE_H
#define X86_CORE_MACHINE_RETIREMENT_OBSERVATION_INTERFACE_H

#include "x86-core/cpu/cpu_interface.h"
#include "lib/types/types_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct x86_core_machine x86_core_machine;

typedef enum x86_core_machine_retirement_timing_disposition {
    X86_CORE_MACHINE_RETIREMENT_TIMING_CLASSIFIED = 1,
    X86_CORE_MACHINE_RETIREMENT_TIMING_SOURCE_UNALLOCATED
} x86_core_machine_retirement_timing_disposition;


typedef enum x86_core_machine_retirement_modrm_form {
    X86_CORE_MACHINE_RETIREMENT_MODRM_UNAVAILABLE = 0,
    X86_CORE_MACHINE_RETIREMENT_MODRM_REGISTER,
    X86_CORE_MACHINE_RETIREMENT_MODRM_MEMORY
} x86_core_machine_retirement_modrm_form;

typedef enum x86_core_machine_retirement_control_outcome {
    X86_CORE_MACHINE_RETIREMENT_CONTROL_NONE = 0,
    X86_CORE_MACHINE_RETIREMENT_CONTROL_FALLTHROUGH,
    X86_CORE_MACHINE_RETIREMENT_CONTROL_TAKEN
} x86_core_machine_retirement_control_outcome;


typedef enum x86_core_machine_retirement_io_direction {
    X86_CORE_MACHINE_RETIREMENT_IO_NONE = 0,
    X86_CORE_MACHINE_RETIREMENT_IO_READ,
    X86_CORE_MACHINE_RETIREMENT_IO_WRITE
} x86_core_machine_retirement_io_direction;

#define X86_CORE_MACHINE_RETIREMENT_CONTEXT_UNAVAILABLE ((lib_u8)-1)

/* A semantic retirement identity; it deliberately contains no instruction
 * addresses, literal operands, firmware identity, or elapsed-time result. */
typedef struct x86_core_machine_retirement_eligibility_key {
    x86_core_cpu_profile cpu_profile;
    x86_core_cpu_retirement_timing_origin timing_origin;
    lib_u32 source_timing_form_id;
    lib_u8 opcode;
    lib_u8 escape_opcode;
    x86_core_machine_retirement_modrm_form modrm_form;
    lib_u8 modrm_extension;
    x86_core_machine_retirement_control_outcome control_outcome;
    lib_u8 next_lexeme_components;
    x86_core_cpu_retirement_repeat_phase repeat_phase;
    lib_u8 cpl;
    lib_u8 protected_mode;
    lib_u8 virtual_8086_mode;
    lib_u8 operand_size_32;
    lib_u8 address_size_32;
    lib_u8 lock_prefix;
    lib_u8 repeat_prefix;
} x86_core_machine_retirement_eligibility_key;

typedef struct x86_core_machine_retirement_qualification_descriptor {
    const x86_core_machine_retirement_eligibility_key *entries;
    lib_size entry_count;
} x86_core_machine_retirement_qualification_descriptor;

typedef struct x86_core_machine_retirement_observation {
    lib_u64 sequence;
    lib_u64 elapsed_ticks;
    lib_u64 timeline_ticks;
    lib_u64 source_ticks;
    x86_core_machine_retirement_eligibility_key eligibility_key;
    x86_core_cpu_execution_point point;
    x86_core_cpu_profile cpu_profile;
    x86_core_machine_retirement_timing_disposition timing_disposition;
    x86_core_cpu_retirement_timing_origin timing_origin;
    lib_u32 source_timing_form_id;
    lib_u32 timing_key_id;
    lib_u32 formula_inputs;
    x86_core_machine_retirement_modrm_form modrm_form;
    lib_u8 modrm_extension;
    x86_core_machine_retirement_control_outcome control_outcome;
    lib_u8 next_lexeme_components;
    x86_core_cpu_retirement_repeat_phase repeat_phase;
    x86_core_machine_retirement_io_direction io_direction;
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
    /* Copies made only for an installed callback; no private CPU pointers. */
    x86_core_cpu_debug_cpu_snapshot instruction_entry_cpu;
    x86_core_cpu_debug_cpu_snapshot current_cpu;
} x86_core_machine_retirement_observation;

typedef void (*x86_core_machine_retirement_observation_callback)(
    void *context, const x86_core_machine_retirement_observation *observation);

typedef struct x86_core_machine_retirement_observation_provider {
    x86_core_machine_retirement_observation_callback callback;
    void *context;
} x86_core_machine_retirement_observation_provider;

/* Installs or removes the optional copied retirement observer while stopped or
 * paused. The callback observes only Core-owned copied state and is invoked
 * before physical-contract rejection or elapsed-time publication. */
lib_status x86_core_machine_set_retirement_observation_provider(
    x86_core_machine *machine,
    const x86_core_machine_retirement_observation_provider *provider);

#ifdef __cplusplus
}
#endif

#endif
