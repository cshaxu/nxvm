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
    type_unsigned_32 significand;
} core_machine_fpu_value;

struct core_machine_fpu {
    core_machine_fpu_profile profile;
    type_unsigned_16 control_word;
    type_unsigned_16 status_word;
    type_unsigned_8 top;
    core_machine_fpu_tag tags[8];
    core_machine_fpu_value registers[8];
    type_bool pending_unmasked_exception;
    /* BUSY and ERROR are independent processor-extension signals.  The
     * operation interval is source data for a later FPU semantic/timeline
     * owner; it is never folded into CPU ESC retirement time. */
    type_bool busy;
    type_unsigned_8 last_escape_opcode;
    type_unsigned_8 last_escape_modrm;
    type_unsigned_32 operation_ticks_min;
    type_unsigned_32 operation_ticks_max;
    /* Core-private TEST-pin wait work.  A timing-capable FPU owner supplies
     * the remaining iterations; FWAIT consumes them atomically on successful
     * retirement and retains the consumed count for CPU timing publication. */
    type_unsigned_32 wait_iterations;
    type_unsigned_32 last_wait_iterations;
};

typedef enum core_machine_fpu_escape_action {
    CORE_MACHINE_FPU_ESCAPE_CONSUME_NONE,
    CORE_MACHINE_FPU_ESCAPE_HANDOFF,
    CORE_MACHINE_FPU_ESCAPE_EXECUTE_8087,
    CORE_MACHINE_FPU_ESCAPE_UNSUPPORTED
} core_machine_fpu_escape_action;

typedef enum core_machine_fpu_execute_result {
    CORE_MACHINE_FPU_EXECUTE_COMPLETED,
    CORE_MACHINE_FPU_EXECUTE_UNSUPPORTED
} core_machine_fpu_execute_result;

C_VOID core_machine_fpu_initialize(core_machine_fpu *fpu,
    core_machine_fpu_profile profile);
C_VOID core_machine_fpu_reset(core_machine_fpu *fpu);
type_bool core_machine_fpu_profile_allows_cpu(core_machine_cpu_profile cpu,
    core_machine_fpu_profile fpu);
core_machine_fpu_escape_action core_machine_fpu_escape_dispatch(
    core_machine_fpu *fpu, core_machine_cpu_profile cpu,
    C_UCHAR escape_opcode, C_UCHAR modrm);
type_bool core_machine_fpu_busy(const core_machine_fpu *fpu);
C_VOID core_machine_fpu_get_state(const core_machine_fpu *fpu,
    core_machine_fpu_state *out_state);
core_machine_fpu_execute_result core_machine_fpu_load_m32(core_machine_fpu *fpu,
    type_unsigned_32 bits);
core_machine_fpu_execute_result core_machine_fpu_store_m32(core_machine_fpu *fpu,
    type_unsigned_32 *out_bits);
C_VOID core_machine_fpu_load_control_word(core_machine_fpu *fpu,
    type_unsigned_16 control_word);
core_machine_fpu_execute_result core_machine_fpu_binary_st0_sti(core_machine_fpu *fpu,
    core_machine_fpu_operation operation, type_unsigned_8 index);
type_bool core_machine_fpu_wait_pending(const core_machine_fpu *fpu);
type_unsigned_32 core_machine_fpu_complete_wait(core_machine_fpu *fpu);
type_unsigned_32 core_machine_fpu_last_wait_iterations(const core_machine_fpu *fpu);

#endif
