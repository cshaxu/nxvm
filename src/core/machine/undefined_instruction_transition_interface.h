#ifndef CORE_MACHINE_UNDEFINED_INSTRUCTION_TRANSITION_INTERFACE_H
#define CORE_MACHINE_UNDEFINED_INSTRUCTION_TRANSITION_INTERFACE_H

#include "type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct core_machine core_machine;

#define CORE_MACHINE_UNDEFINED_INSTRUCTION_MAX_BYTES 15u

typedef enum core_machine_undefined_instruction_outcome {
    CORE_MACHINE_UNDEFINED_INSTRUCTION_UNHANDLED = 0,
    CORE_MACHINE_UNDEFINED_INSTRUCTION_HANDLED_RESUME,
    CORE_MACHINE_UNDEFINED_INSTRUCTION_STOP,
    CORE_MACHINE_UNDEFINED_INSTRUCTION_FAULT
} core_machine_undefined_instruction_outcome;

typedef struct core_machine_undefined_instruction_input {
    uint16_t cs;
    uint32_t eip;
    uint8_t bytes[CORE_MACHINE_UNDEFINED_INSTRUCTION_MAX_BYTES];
    uint8_t byte_count;
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t eflags;
} core_machine_undefined_instruction_input;

typedef struct core_machine_undefined_instruction_patch {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t ebp;
    uint32_t eflags;
} core_machine_undefined_instruction_patch;

typedef struct core_machine_undefined_instruction_response {
    core_machine_undefined_instruction_outcome outcome;
    uint32_t fault_detail;
    uint8_t consumed_bytes;
    core_machine_undefined_instruction_patch patch;
} core_machine_undefined_instruction_response;

typedef C_VOID (*core_machine_undefined_instruction_consumer)(C_VOID *owner,
    const core_machine_undefined_instruction_input *input,
    core_machine_undefined_instruction_response *out_response);

type_status core_machine_register_undefined_instruction_transition(
    core_machine *machine,
    const uint8_t *pattern,
    uint8_t length,
    core_machine_undefined_instruction_consumer consumer,
    C_VOID *owner);

#ifdef __cplusplus
}
#endif

#endif
