#ifndef NTVDM64_CORE_MACHINE_FPU_H
#define NTVDM64_CORE_MACHINE_FPU_H

#include "core/machine/fpu_interface.h"

typedef struct core_machine_fpu {
    core_machine_fpu_profile profile;
} core_machine_fpu;

C_VOID core_machine_fpu_initialize(core_machine_fpu *fpu,
    core_machine_fpu_profile profile);
C_VOID core_machine_fpu_reset(core_machine_fpu *fpu);

#endif
