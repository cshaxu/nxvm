#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/firmware_interface.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/machine_interface.h"

static lib_status reset_rom_configure(void *opaque,
    core_machine_firmware_context *firmware)
{
    static const lib_u8 halt[] = {0xf4u};
    static const lib_u8 reset_jump[] = {
        0xeau, 0x00u, 0x00u, 0x00u, 0xf0u,
        0x90u, 0x90u, 0x90u, 0x90u, 0x90u, 0x90u, 0x90u,
        0x90u, 0x90u, 0x90u, 0x90u
    };

    (void)opaque;
    if (core_machine_firmware_register_immutable_rom(firmware, 0x000f0000u,
            halt, sizeof(halt)) != LIB_STATUS_OK) return LIB_STATUS_INTERNAL_ERROR;
    return core_machine_firmware_register_immutable_rom(firmware, 0x000ffff0u,
        reset_jump, sizeof(reset_jump));
}

static lib_status reset_rom_reset(void *opaque,
    core_machine_firmware_context *firmware)
{
    (void)opaque;
    (void)firmware;
    return LIB_STATUS_OK;
}

static const core_machine_firmware_provider reset_rom_provider = {
    reset_rom_configure, reset_rom_reset, LIB_NULL, LIB_NULL
};

static lib_i32 reset_rom_run(core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = 0x00100000u,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_run_budget budget = {4u, 0u};
    core_machine *machine = LIB_NULL;
    core_machine_run_result result;
    const core_machine_absent_memory_config absent_memory = {
        0x00100000u, 0x00f00000u, 0xffu
    };
    lib_u8 reset_byte = 0u;
    lib_i32 failed = 0;

    failed |= core_machine_create(&config, &machine) != LIB_STATUS_OK;
    failed |= !failed && core_machine_configure_absent_memory(machine,
        &absent_memory) != LIB_STATUS_OK;
    failed |= !failed && core_machine_bind_firmware_provider(machine,
        &reset_rom_provider, LIB_NULL) != LIB_STATUS_OK;
    failed |= !failed && core_machine_memory_read_reset_physical(
        &machine->executor_memory,
        profile == CORE_MACHINE_CPU_PROFILE_80286 ? 0x00fffff0u : 0xfffffff0u,
        (lib_uptr)&reset_byte, 1u) != LIB_STATUS_OK;
    failed |= !failed && reset_byte != 0xeau;
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        LIB_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != LIB_STATUS_OK;
    failed |= !failed && core_machine_run(machine, budget, &result) !=
        LIB_STATUS_OK;
    failed |= !failed && result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
    if (failed) {
        fprintf(stderr, "reset-rom profile=%d run-reason=%d detail=%08x pc=%08x\n",
            (int)profile, (int)result.reason, (unsigned int)result.detail,
            (unsigned int)result.linear_pc);
    }
    core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    if (reset_rom_run(CORE_MACHINE_CPU_PROFILE_80286) ||
        reset_rom_run(CORE_MACHINE_CPU_PROFILE_80386)) return 1;
    puts("M5:T496:S7:RESET-ROM-ALIAS:OK");
    return 0;
}
