#include "type.h"

#include "core/machine/cpu_instructions.h"

static C_INT valid_cpu_profile(core_machine_cpu_profile profile)
{
    return profile >= CORE_MACHINE_CPU_PROFILE_8086 &&
        profile <= CORE_MACHINE_CPU_PROFILE_80386;
}

static C_INT valid_fpu_profile(core_machine_fpu_profile profile)
{
    return profile >= CORE_MACHINE_FPU_PROFILE_NONE &&
        profile <= CORE_MACHINE_FPU_PROFILE_80387;
}

static C_INT verify_metadata(core_machine_cpu_instruction_space space,
    uint8_t opcode, uint8_t modrm)
{
    core_machine_cpu_instruction_metadata metadata =
        core_machine_cpu_instruction_metadata_get(space, opcode, modrm);

    return !valid_cpu_profile(metadata.minimum_cpu) ||
        !valid_fpu_profile(metadata.minimum_fpu);
}

C_INT main(C_VOID)
{
    uint32_t opcode;
    uint32_t modrm;
    C_INT failed = 0;

    for (opcode = 0u; opcode <= 0xffu; ++opcode) {
        failed |= verify_metadata(CORE_MACHINE_CPU_INSTRUCTION_PRIMARY,
            (uint8_t)opcode, 0u);
        for (modrm = 0u; modrm <= 0xffu; ++modrm) {
            failed |= verify_metadata(CORE_MACHINE_CPU_INSTRUCTION_0F,
                (uint8_t)opcode, (uint8_t)modrm);
        }
    }
    for (opcode = 0xd8u; opcode <= 0xdfu; ++opcode) {
        for (modrm = 0u; modrm <= 0xffu; ++modrm) {
            core_machine_cpu_instruction_metadata metadata =
                core_machine_cpu_instruction_metadata_get(
                    CORE_MACHINE_CPU_INSTRUCTION_FPU_ESCAPE,
                    (uint8_t)opcode, (uint8_t)modrm);
            failed |= !metadata.valid ||
                metadata.minimum_cpu != CORE_MACHINE_CPU_PROFILE_8086 ||
                metadata.minimum_fpu != CORE_MACHINE_FPU_PROFILE_8087;
        }
    }
    for (opcode = 0u; opcode <= 0xffu; ++opcode) {
        core_machine_cpu_instruction_metadata metadata =
            core_machine_cpu_instruction_metadata_get(
                CORE_MACHINE_CPU_INSTRUCTION_FPU_ESCAPE, (uint8_t)opcode, 0u);
        if (opcode < 0xd8u || opcode > 0xdfu) failed |= metadata.valid;
    }
    if (failed) return 1;
    STD_PRINTF("M5:T158:S1:CPU-FPU-METADATA-CLOSURE:OK\n");
    return 0;
}
