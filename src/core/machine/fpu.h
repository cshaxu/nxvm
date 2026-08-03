#ifndef NTVDM64_CORE_MACHINE_FPU_H
#define NTVDM64_CORE_MACHINE_FPU_H

#include "core/machine/fpu_interface.h"

struct core_machine_fpu {
    core_machine_fpu_profile profile;
};

typedef enum core_machine_fpu_escape_action {
    CORE_MACHINE_FPU_ESCAPE_CONSUME_NONE,
    CORE_MACHINE_FPU_ESCAPE_UNSUPPORTED
} core_machine_fpu_escape_action;

C_VOID core_machine_fpu_initialize(core_machine_fpu *fpu,
    core_machine_fpu_profile profile);
C_VOID core_machine_fpu_reset(core_machine_fpu *fpu);
core_machine_fpu_escape_action core_machine_fpu_escape_dispatch(
    const core_machine_fpu *fpu, C_UCHAR escape_opcode, C_UCHAR modrm);

#endif
