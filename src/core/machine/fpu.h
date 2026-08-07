#ifndef CORE_MACHINE_FPU_H
#define CORE_MACHINE_FPU_H

#include "core/machine/fpu_interface.h"

typedef enum core_machine_fpu_tag {
    CORE_MACHINE_FPU_TAG_EMPTY = 3,
    CORE_MACHINE_FPU_TAG_VALID = 0
} core_machine_fpu_tag;

typedef enum core_machine_fpu_value_kind {
    CORE_MACHINE_FPU_VALUE_ZERO,
    CORE_MACHINE_FPU_VALUE_FINITE,
    CORE_MACHINE_FPU_VALUE_INFINITY
} core_machine_fpu_value_kind;

typedef struct core_machine_fpu_value {
    core_machine_fpu_value_kind kind;
    type_bool negative;
    type_signed_16 exponent;
    uint32_t significand;
} core_machine_fpu_value;

struct core_machine_fpu {
    core_machine_fpu_profile profile;
    uint16_t control_word;
    uint16_t status_word;
    uint8_t top;
    core_machine_fpu_tag tags[8];
    core_machine_fpu_value registers[8];
    type_bool pending_unmasked_exception;
};

typedef enum core_machine_fpu_escape_action {
    CORE_MACHINE_FPU_ESCAPE_CONSUME_NONE,
    CORE_MACHINE_FPU_ESCAPE_UNSUPPORTED
} core_machine_fpu_escape_action;

typedef enum core_machine_fpu_execute_result {
    CORE_MACHINE_FPU_EXECUTE_COMPLETED,
    CORE_MACHINE_FPU_EXECUTE_UNSUPPORTED
} core_machine_fpu_execute_result;

C_VOID core_machine_fpu_initialize(core_machine_fpu *fpu,
    core_machine_fpu_profile profile);
C_VOID core_machine_fpu_reset(core_machine_fpu *fpu);
core_machine_fpu_escape_action core_machine_fpu_escape_dispatch(
    const core_machine_fpu *fpu, C_UCHAR escape_opcode, C_UCHAR modrm);
C_VOID core_machine_fpu_get_state(const core_machine_fpu *fpu,
    core_machine_fpu_state *out_state);
core_machine_fpu_execute_result core_machine_fpu_load_m32(core_machine_fpu *fpu,
    uint32_t bits);
core_machine_fpu_execute_result core_machine_fpu_store_m32(core_machine_fpu *fpu,
    uint32_t *out_bits);
C_VOID core_machine_fpu_load_control_word(core_machine_fpu *fpu,
    uint16_t control_word);
core_machine_fpu_execute_result core_machine_fpu_binary_st0_sti(core_machine_fpu *fpu,
    core_machine_fpu_operation operation, uint8_t index);
type_bool core_machine_fpu_wait_pending(const core_machine_fpu *fpu);

#endif
