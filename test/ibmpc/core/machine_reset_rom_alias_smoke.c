#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "x86/core/firmware_interface.h"
#include "x86/core/machine.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "../board-common/absent_memory_fixture.h"

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
    reset_rom_configure, reset_rom_reset, LIB_NULL
};

static lib_i32 reset_rom_run(core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = 0x00100000u,
        .cpu_profile = profile,
        .fpu_profile = X86_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_run_budget budget = {4u, 0u};
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_run_result result;
    const core_machine_absent_memory_config absent_memory = {
        0x00100000u, 0x00f00000u, 0xffu
    };
    lib_u8 reset_byte = 0u;
    lib_i32 failed = 0;

    failed |= core_machine_create(&config, &machine, &board) != LIB_STATUS_OK;
    failed |= !failed && core_machine_configure_absent_memory(board,
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
        lib_c_fprintf(lib_c_stderr, "reset-rom profile=%d run-reason=%d detail=%08x pc=%08x\n",
            (int)profile, (int)result.reason, (unsigned int)result.detail,
            (unsigned int)result.linear_pc);
    }
    core_machine_destroy(machine);
    return failed;
}

static lib_i32 absent_fallback_run(void)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386
    };
    const core_machine_absent_memory_config absent = {
        0x000a0000u, 2u, 0xffu
    };
    const lib_u8 rom_byte = 0x5au;
    core_machine_memory_test_allocation allocation = {LIB_TRUE, 0u};
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    lib_u8 byte = 0u;
    lib_i32 failed = 0;

    if (core_machine_create(&config, &machine, &board) != LIB_STATUS_OK) return 1;
    machine->executor_memory.connect.device_provider_test_allocation = &allocation;
    failed |= core_machine_configure_absent_memory(board, &absent) !=
        LIB_STATUS_NO_MEMORY || allocation.attempts != 1u ||
        test_board_absent_memory_is_configured(board) ||
        machine->executor_memory.connect.device_provider_count != 0u;
    machine->executor_memory.connect.device_provider_test_allocation = LIB_NULL;
    failed |= core_machine_configure_absent_memory(board, &absent) !=
        LIB_STATUS_OK || !test_board_absent_memory_is_configured(board);
    failed |= core_machine_register_immutable_rom_mapping(machine,
        absent.physical_start, &rom_byte, 1u) != LIB_STATUS_OK;
    failed |= core_machine_memory_inspect(machine,
        absent.physical_start, (void *)&byte, 1u) != LIB_STATUS_OK ||
        byte != rom_byte;
    byte = 0u;
    failed |= core_machine_memory_inspect(machine,
        absent.physical_start + 1u, (void *)&byte, 1u) != LIB_STATUS_OK ||
        byte != absent.read_value;
    core_machine_destroy(machine);
    return failed;
}

static lib_status policy_configure(void *opaque,
    core_machine_firmware_context *firmware)
{
    static const lib_u8 source[16] = {0xeau};
    static const lib_u8 high[16] = {0xccu};
    const lib_u32 mode = *(const lib_u32 *)opaque;
    lib_status status = core_machine_firmware_register_immutable_rom(firmware,
        0x000ffff0u, source, mode == 0u ? 14u : mode == 1u ? 15u : 16u);

    if (status != LIB_STATUS_OK) return status;
    if (mode == 2u) return core_machine_firmware_register_immutable_rom(firmware,
        0xfffffff0u, high, sizeof(high));
    if (mode == 3u) return core_machine_firmware_register_immutable_rom(firmware,
        0x000efffeu, source, 4u);
    return LIB_STATUS_OK;
}

static lib_i32 reset_rom_policy(lib_u32 mode)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386
    };
    const core_machine_firmware_provider provider = {
        policy_configure, reset_rom_reset, LIB_NULL
    };
    const lib_size counts[] = {1u, 2u, 2u, 4u};
    core_machine *machine = LIB_NULL;
    lib_u8 byte = 0u;
    lib_i32 failed = 1;

    if (core_machine_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK) goto done;
    if (core_machine_bind_firmware_provider(machine, &provider, &mode) !=
            LIB_STATUS_OK || machine->immutable_rom_mapping_count != counts[mode]) goto done;
    if (mode == 0u) {
        if (core_machine_immutable_rom_mapping_contains(machine, 0xfffffff0u, 1u)) goto done;
    } else {
        if (core_machine_memory_read_reset_physical(&machine->executor_memory,
                0xfffffff0u, (lib_uptr)&byte, 1u) != LIB_STATUS_OK ||
            byte != (mode == 2u ? 0xccu : 0xeau)) goto done;
        if (mode == 1u &&
            (!core_machine_immutable_rom_mapping_contains(machine, 0xfffffff0u, 15u) ||
             core_machine_immutable_rom_mapping_contains(machine, 0xfffffff0u, 16u))) goto done;
        if (mode == 3u &&
            (!core_machine_immutable_rom_mapping_contains(machine, 0xffff0000u, 2u) ||
             core_machine_immutable_rom_mapping_contains(machine, 0xffff0002u, 1u))) goto done;
    }
    failed = 0;
done:
    core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    for (lib_u32 mode = 0u; mode < 4u; ++mode) {
        if (reset_rom_policy(mode)) return 1;
    }
    if (reset_rom_run(CORE_MACHINE_CPU_PROFILE_80286) ||
        reset_rom_run(CORE_MACHINE_CPU_PROFILE_80386) ||
        absent_fallback_run()) return 1;
    lib_c_printf("%s\n", "RESET-ROM-ALIAS:OK");
    return 0;
}
