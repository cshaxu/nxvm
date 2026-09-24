#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/cpu_instructions.h"

static lib_i32 valid_cpu_profile(core_machine_cpu_profile profile)
{
    return profile >= CORE_MACHINE_CPU_PROFILE_8086 &&
        profile <= CORE_MACHINE_CPU_PROFILE_80386;
}

static lib_i32 valid_fpu_profile(core_machine_fpu_profile profile)
{
    return profile >= CORE_MACHINE_FPU_PROFILE_NONE &&
        profile <= CORE_MACHINE_FPU_PROFILE_80387;
}

static lib_i32 verify_metadata(core_machine_cpu_instruction_space space,
    lib_u8 opcode, lib_u8 modrm)
{
    core_machine_cpu_instruction_metadata metadata =
        core_machine_cpu_instruction_metadata_get(space, opcode, modrm);

    return !valid_cpu_profile(metadata.minimum_cpu) ||
        !valid_fpu_profile(metadata.minimum_fpu);
}

lib_i32 main(void)
{
    lib_u32 opcode;
    lib_u32 modrm;
    lib_i32 failed = 0;

    for (opcode = 0u; opcode <= 0xffu; ++opcode) {
        failed |= verify_metadata(CORE_MACHINE_CPU_INSTRUCTION_PRIMARY,
            (lib_u8)opcode, 0u);
        for (modrm = 0u; modrm <= 0xffu; ++modrm) {
            failed |= verify_metadata(CORE_MACHINE_CPU_INSTRUCTION_0F,
                (lib_u8)opcode, (lib_u8)modrm);
        }
    }
    for (opcode = 0xd8u; opcode <= 0xdfu; ++opcode) {
        for (modrm = 0u; modrm <= 0xffu; ++modrm) {
            core_machine_cpu_instruction_metadata metadata =
                core_machine_cpu_instruction_metadata_get(
                    CORE_MACHINE_CPU_INSTRUCTION_FPU_ESCAPE,
                    (lib_u8)opcode, (lib_u8)modrm);
            failed |= !metadata.valid ||
                metadata.minimum_cpu != CORE_MACHINE_CPU_PROFILE_8086 ||
                metadata.minimum_fpu != CORE_MACHINE_FPU_PROFILE_8087;
        }
    }
    for (opcode = 0u; opcode <= 0xffu; ++opcode) {
        core_machine_cpu_instruction_metadata metadata =
            core_machine_cpu_instruction_metadata_get(
                CORE_MACHINE_CPU_INSTRUCTION_FPU_ESCAPE, (lib_u8)opcode, 0u);
        if (opcode < 0xd8u || opcode > 0xdfu) failed |= metadata.valid;
    }
    if (failed) return 1;
    printf("M5:T158:S1:CPU-FPU-METADATA-CLOSURE:OK\n");
    return 0;
}
