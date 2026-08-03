#include "type.h"

#include "core/machine/fpu.h"

const C_CHAR *core_machine_fpu_profile_name(core_machine_fpu_profile profile)
{
    switch (profile) {
    case CORE_MACHINE_FPU_PROFILE_NONE: return "none";
    case CORE_MACHINE_FPU_PROFILE_8087: return "8087";
    case CORE_MACHINE_FPU_PROFILE_80287: return "80287";
    case CORE_MACHINE_FPU_PROFILE_80387: return "80387";
    }
    return "invalid";
}

C_VOID core_machine_fpu_initialize(core_machine_fpu *fpu,
    core_machine_fpu_profile profile)
{
    if (fpu == STD_NULL) return;
    fpu->profile = profile;
}

C_VOID core_machine_fpu_reset(core_machine_fpu *fpu)
{
    (C_VOID)fpu;
}
