#include "type.h"

#include "core/machine/firmware_interface.h"
#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"

static type_status reset_rom_configure(C_VOID *opaque,
    core_machine_firmware_context *firmware)
{
    static const type_unsigned_8 halt[] = {0xf4u};
    static const type_unsigned_8 reset_jump[] = {
        0xeau, 0x00u, 0x00u, 0x00u, 0xf0u,
        0x90u, 0x90u, 0x90u, 0x90u, 0x90u, 0x90u, 0x90u,
        0x90u, 0x90u, 0x90u, 0x90u
    };

    (C_VOID)opaque;
    if (core_machine_firmware_register_immutable_rom(firmware, 0x000f0000u,
            halt, sizeof(halt)) != TYPE_STATUS_OK) return TYPE_STATUS_FAULT;
    return core_machine_firmware_register_immutable_rom(firmware, 0x000ffff0u,
        reset_jump, sizeof(reset_jump));
}

static type_status reset_rom_reset(C_VOID *opaque,
    core_machine_firmware_context *firmware)
{
    (C_VOID)opaque;
    (C_VOID)firmware;
    return TYPE_STATUS_OK;
}

static const core_machine_firmware_provider reset_rom_provider = {
    reset_rom_configure, reset_rom_reset, STD_NULL
};

static C_INT reset_rom_run(core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = 0x00100000u,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_run_budget budget = {4u, 0u};
    core_machine *machine = STD_NULL;
    core_machine_run_result result;
    type_unsigned_8 reset_byte = 0u;
    C_INT failed = 0;

    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_bind_firmware_provider(machine,
        &reset_rom_provider, STD_NULL) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_memory_read_reset_physical(
        &machine->executor_memory,
        profile == CORE_MACHINE_CPU_PROFILE_80286 ? 0x00fffff0u : 0xfffffff0u,
        (type_virtual_address)&reset_byte, 1u) != TYPE_STATUS_OK;
    failed |= !failed && reset_byte != 0xeau;
    failed |= !failed && core_machine_freeze_execution_providers(machine) !=
        TYPE_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_run(machine, budget, &result) !=
        TYPE_STATUS_OK;
    failed |= !failed && result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
    if (failed) {
        fprintf(stderr, "reset-rom profile=%d run-reason=%d detail=%08x pc=%08x\n",
            (int)profile, (int)result.reason, (unsigned int)result.detail,
            (unsigned int)result.linear_pc);
    }
    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    if (reset_rom_run(CORE_MACHINE_CPU_PROFILE_80286) ||
        reset_rom_run(CORE_MACHINE_CPU_PROFILE_80386)) return 1;
    puts("M5:T496:S7:RESET-ROM-ALIAS:OK");
    return 0;
}
