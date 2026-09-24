#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/cpu_instructions.h"
#include "app-nxvm/devices/machine_interface.h"

static lib_i32 verify_machine_profiles(core_machine_cpu_profile cpu_profile,
    core_machine_fpu_profile fpu_profile)
{
    core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = cpu_profile,
        .fpu_profile = fpu_profile
    };
    core_machine *machine = LIB_NULL;
    core_machine_cpu_profile observed_cpu;
    core_machine_fpu_profile observed_fpu;
    lib_i32 failed = core_machine_create(&config, &machine) != LIB_STATUS_OK ||
        core_machine_get_cpu_profile(machine, &observed_cpu) != LIB_STATUS_OK ||
        core_machine_get_fpu_profile(machine, &observed_fpu) != LIB_STATUS_OK ||
        observed_cpu != cpu_profile || observed_fpu != fpu_profile;

    core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    core_machine_config default_config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES
    };
    core_machine_cpu_instruction_metadata metadata;
    core_machine *machine = LIB_NULL;
    core_machine_cpu_profile cpu_profile;
    core_machine_fpu_profile fpu_profile;
    lib_i32 failed = 0;

    failed |= verify_machine_profiles(CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_FPU_PROFILE_NONE);
    failed |= verify_machine_profiles(CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_FPU_PROFILE_NONE);
    failed |= verify_machine_profiles(CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_FPU_PROFILE_8087);
    failed |= verify_machine_profiles(CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_FPU_PROFILE_80287);
    failed |= verify_machine_profiles(CORE_MACHINE_CPU_PROFILE_80386,
        CORE_MACHINE_FPU_PROFILE_80387);
    failed |= core_machine_create(&default_config, &machine) != LIB_STATUS_OK;
    failed |= core_machine_get_cpu_profile(machine, &cpu_profile) != LIB_STATUS_OK ||
        cpu_profile != CORE_MACHINE_CPU_PROFILE_80386;
    failed |= core_machine_get_fpu_profile(machine, &fpu_profile) != LIB_STATUS_OK ||
        fpu_profile != CORE_MACHINE_FPU_PROFILE_NONE;
    core_machine_destroy(machine);

    metadata = core_machine_cpu_instruction_metadata_get(
        CORE_MACHINE_CPU_INSTRUCTION_PRIMARY, 0x60u, 0u);
    failed |= !metadata.valid || metadata.minimum_cpu != CORE_MACHINE_CPU_PROFILE_80186;
    metadata = core_machine_cpu_instruction_metadata_get(
        CORE_MACHINE_CPU_INSTRUCTION_0F, 0x80u, 0u);
    failed |= !metadata.valid || metadata.minimum_cpu != CORE_MACHINE_CPU_PROFILE_80386;
    metadata = core_machine_cpu_instruction_metadata_get(
        CORE_MACHINE_CPU_INSTRUCTION_FPU_ESCAPE, 0xdbu, 0xe3u);
    failed |= !metadata.valid || metadata.minimum_fpu != CORE_MACHINE_FPU_PROFILE_8087;
    metadata = core_machine_cpu_instruction_metadata_get(
        CORE_MACHINE_CPU_INSTRUCTION_PRIMARY, 0xa4u, 0u);
    failed |= !metadata.valid || metadata.minimum_cpu != CORE_MACHINE_CPU_PROFILE_8086;
    metadata = core_machine_cpu_instruction_metadata_get(
        CORE_MACHINE_CPU_INSTRUCTION_0F, 0xa2u, 0u);
    failed |= metadata.valid;

    if (failed) return 1;
    printf("M5:T154:S1:CPU-FPU-PROFILES:OK\n");
    return 0;
}
