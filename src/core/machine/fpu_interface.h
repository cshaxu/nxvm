#ifndef NTVDM64_CORE_MACHINE_FPU_INTERFACE_H
#define NTVDM64_CORE_MACHINE_FPU_INTERFACE_H

#include "type.h"

typedef struct core_machine_fpu core_machine_fpu;

typedef enum core_machine_fpu_profile {
    CORE_MACHINE_FPU_PROFILE_NONE = 0,
    CORE_MACHINE_FPU_PROFILE_8087,
    CORE_MACHINE_FPU_PROFILE_80287,
    CORE_MACHINE_FPU_PROFILE_80387
} core_machine_fpu_profile;

const C_CHAR *core_machine_fpu_profile_name(core_machine_fpu_profile profile);

#endif
