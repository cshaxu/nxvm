#ifndef CORE_MACHINE_FPU_INTERFACE_H
#define CORE_MACHINE_FPU_INTERFACE_H

#include "type.h"

#include "core/machine/cpu_interface.h"

typedef struct core_machine_fpu core_machine_fpu;

typedef enum core_machine_fpu_profile {
    CORE_MACHINE_FPU_PROFILE_NONE = 0,
    CORE_MACHINE_FPU_PROFILE_8087,
    CORE_MACHINE_FPU_PROFILE_80287,
    CORE_MACHINE_FPU_PROFILE_80387
} core_machine_fpu_profile;

typedef enum core_machine_fpu_operation {
    CORE_MACHINE_FPU_OPERATION_CONSUME_NONE = 0,
    CORE_MACHINE_FPU_OPERATION_FNINIT,
    CORE_MACHINE_FPU_OPERATION_FLD_M32,
    CORE_MACHINE_FPU_OPERATION_FSTP_M32,
    CORE_MACHINE_FPU_OPERATION_FLDCW_M16,
    CORE_MACHINE_FPU_OPERATION_FADD_ST0_STI,
    CORE_MACHINE_FPU_OPERATION_FMUL_ST0_STI,
    CORE_MACHINE_FPU_OPERATION_FSUB_ST0_STI,
    CORE_MACHINE_FPU_OPERATION_FDIV_ST0_STI,
    CORE_MACHINE_FPU_OPERATION_UNSUPPORTED
} core_machine_fpu_operation;

typedef struct core_machine_fpu_operation_metadata {
    core_machine_cpu_profile minimum_cpu;
    core_machine_fpu_profile minimum_fpu;
    core_machine_fpu_operation operation;
    C_INT valid;
} core_machine_fpu_operation_metadata;

typedef struct core_machine_fpu_state {
    uint16_t control_word;
    uint16_t status_word;
    uint8_t top;
    uint8_t tags[8];
    C_INT pending_unmasked_exception;
} core_machine_fpu_state;

const C_CHAR *core_machine_fpu_profile_name(core_machine_fpu_profile profile);
core_machine_fpu_operation_metadata core_machine_fpu_operation_metadata_get(
    uint8_t escape_opcode, uint8_t modrm);

#endif
