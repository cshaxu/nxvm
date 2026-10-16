#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86-core/machine/machine_interface.h"
#include "x86-core/machine/machine.h"
#include "support/memory_alias_fixture.h"

#define RESET_LINEAR 0xfffffff0u
#define RESET_PHYSICAL 0x000ffff0u

static lib_status rejected_firmware(void *owner,
    x86_core_machine_firmware_context *firmware)
{
    const lib_u8 image[] = {0x90u, 0xf4u};

    (void)owner;
    if (x86_core_machine_firmware_register_immutable_rom(firmware,
            0xe0000u, image, sizeof(image)) != LIB_STATUS_OK ||
        x86_core_machine_firmware_register_immutable_rom_alias(firmware,
            0xe0000u, 0xf0000u, sizeof(image)) != LIB_STATUS_OK)
        return LIB_STATUS_INTERNAL_ERROR;
    return LIB_STATUS_IO_ERROR;
}

static lib_status unused_firmware_reset(void *owner,
    x86_core_machine_firmware_context *firmware)
{
    (void)owner;
    (void)firmware;
    return LIB_STATUS_OK;
}

static lib_i32 firmware_rollback_contract(void)
{
    const x86_core_machine_executor_config config = {
        .memory_bytes = 2u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    const x86_core_machine_firmware_provider provider = {
        rejected_firmware, unused_firmware_reset, LIB_NULL
    };
    const lib_u8 prior = 0xebu;
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_memory_route route;
    lib_size providers_before;
    lib_size mappings_before;
    lib_u8 observed = 0u;
    lib_i32 failed;

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK)
        return 1;
    failed = x86_core_machine_register_immutable_rom_mapping(machine,
        0xd0000u, &prior, sizeof(prior)) != LIB_STATUS_OK;
    providers_before = machine->executor_memory.connect.device_provider_count;
    mappings_before = machine->immutable_rom_mapping_count;
    failed |= x86_core_machine_bind_firmware_provider(machine, &provider,
        LIB_NULL) != LIB_STATUS_IO_ERROR ||
        machine->executor_memory.connect.device_provider_count != providers_before ||
        machine->immutable_rom_mapping_count != mappings_before ||
        machine->firmware_provider != LIB_NULL;
    failed |= x86_core_machine_memory_query_physical(&machine->executor_memory,
        0xd0000u, 1u, X86_CORE_MACHINE_MEMORY_ACCESS_READ, &route) != LIB_STATUS_OK ||
        route != X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER;
    failed |= x86_core_machine_memory_read_physical(&machine->executor_memory,
        0xd0000u, (lib_uptr)&observed, 1u) != LIB_STATUS_OK || observed != prior;
    for (lib_u32 address = 0xe0000u; address <= 0xf0000u; address += 0x10000u)
        failed |= x86_core_machine_memory_query_physical(&machine->executor_memory,
            address, 1u, X86_CORE_MACHINE_MEMORY_ACCESS_READ, &route) != LIB_STATUS_OK ||
            route != X86_CORE_MACHINE_MEMORY_ROUTE_ORDINARY_RAM;
    x86_core_machine_destroy(machine);
    return failed;
}

/* Window intersection and registry representation belong to Machine; a board
 * decides whether to request an alias and which physical window to supply. */
static lib_i32 reset_window_segments(lib_u32 target, lib_u32 mode)
{
    static const lib_u8 source[16] = {0xeau};
    static const lib_u8 high[16] = {0xccu};
    const lib_size counts[] = {1u, 2u, 2u, 4u};
    const lib_size bytes = mode == 0u ? 14u : mode == 1u ? 15u : 16u;
    const x86_core_machine_executor_config config = {
        .memory_bytes = 2u * 1024u * 1024u,
        .cpu_profile = X86_CORE_CPU_PROFILE_80386
    };
    x86_core_machine *machine = LIB_NULL;
    lib_u8 byte = 0u;
    lib_i32 failed = 1;

    if (x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK)
        return 1;
    if (x86_core_machine_register_immutable_rom_mapping(machine, 0xffff0u,
            source, bytes) != LIB_STATUS_OK) goto done;
    if (mode == 2u && x86_core_machine_register_immutable_rom_mapping(machine,
            target + 0xfff0u, high, sizeof(high)) != LIB_STATUS_OK) goto done;
    if (mode == 3u && x86_core_machine_register_immutable_rom_mapping(machine,
            0xefffeu, source, 4u) != LIB_STATUS_OK) goto done;
    if ((mode == 1u || mode == 3u) &&
        x86_core_machine_register_immutable_rom_mapping_reset_window(machine,
            0xf0000u, target, 0x10000u) != LIB_STATUS_OK) goto done;
    if (machine->immutable_rom_mapping_count != counts[mode]) goto done;
    if (mode == 0u) {
        if (x86_core_machine_immutable_rom_mapping_contains(machine,
                target + 0xfff0u, 1u)) goto done;
    } else {
        if (x86_core_machine_memory_read_reset_physical(&machine->executor_memory,
                target + 0xfff0u, (lib_uptr)&byte, 1u) != LIB_STATUS_OK ||
            byte != (mode == 2u ? 0xccu : 0xeau)) goto done;
        if (mode == 1u &&
            (!x86_core_machine_immutable_rom_mapping_contains(machine,
                target + 0xfff0u, 15u) ||
             x86_core_machine_immutable_rom_mapping_contains(machine,
                target + 0xfff0u, 16u))) goto done;
        if (mode == 3u &&
            (!x86_core_machine_immutable_rom_mapping_contains(machine, target, 2u) ||
             x86_core_machine_immutable_rom_mapping_contains(machine,
                target + 2u, 1u))) goto done;
    }
    failed = 0;
done:
    x86_core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    static const lib_u8 image[] = { 0xf4u, 0x90u, 0x90u };
    static const lib_u8 high_image[] = { 0x5au };
    static const lib_u8 reset_jump[] = { 0xeau, 0x00u, 0x10u, 0x00u, 0x00u };
    const x86_core_machine_executor_config config = {
        .memory_bytes = (2u * 1024u * 1024u),
        .cpu_profile = X86_CORE_CPU_PROFILE_8086,
        .fpu_profile = X86_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const x86_core_machine_run_budget budget = { 16u, 0u };
    x86_core_machine *machine = LIB_NULL;
    x86_core_machine_memory_route route;
    x86_core_machine_run_result result;
    lib_u8 observed[sizeof(image)] = { 0u };
    lib_u8 overwrite = 0u;
    lib_i32 failed = 0;

    failed |= x86_core_machine_neutral_create(&config, &machine) != LIB_STATUS_OK;
    failed |= machine == LIB_NULL;
    if (!failed) {
        failed |= test_x86_core_machine_fixture_register_reset_mapping(machine, RESET_LINEAR,
            RESET_PHYSICAL, 16u) != LIB_STATUS_OK;
        failed |= x86_core_machine_register_immutable_rom_mapping(machine, 0x1000u,
            image, sizeof(image)) != LIB_STATUS_OK;
        failed |= x86_core_machine_register_immutable_rom_mapping(machine, 0xf0000u,
            high_image, sizeof(high_image)) != LIB_STATUS_OK;
        failed |= x86_core_machine_register_immutable_rom_mapping(machine, 0x1001u,
            image, 1u) != LIB_STATUS_INVALID_ARGUMENT;
        failed |= x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
        failed |= x86_core_machine_register_immutable_rom_mapping(machine, 0x2000u,
            image, 1u) != LIB_STATUS_INVALID_STATE;
        failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
        failed |= x86_core_machine_memory_query(machine, 0x1000u, sizeof(image),
            X86_CORE_MACHINE_MEMORY_ACCESS_READ, &route) != LIB_STATUS_OK ||
            route != X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER;
        failed |= x86_core_machine_memory_query(machine, 0x1000u, 1u,
            X86_CORE_MACHINE_MEMORY_ACCESS_WRITE, &route) != LIB_STATUS_OK ||
            route != X86_CORE_MACHINE_MEMORY_ROUTE_PROVIDER;
        failed |= x86_core_machine_memory_read(machine, 0x1000u, observed,
            sizeof(observed)) != LIB_STATUS_OK ||
            lib_memory_compare(image, observed, sizeof(image)) != 0;
        failed |= x86_core_machine_memory_write(machine, 0x1000u, &overwrite, 1u) !=
            LIB_STATUS_OK;
        failed |= x86_core_machine_memory_read(machine, 0x1000u, observed,
            sizeof(observed)) != LIB_STATUS_OK ||
            lib_memory_compare(image, observed, sizeof(image)) != 0;
        failed |= x86_core_machine_memory_write(machine, 0xf0000u, &overwrite, 1u) !=
            LIB_STATUS_OK;
        failed |= x86_core_machine_memory_read(machine, 0xf0000u, observed, 1u) !=
            LIB_STATUS_OK || observed[0u] != high_image[0u];
        failed |= x86_core_machine_memory_write(machine, RESET_LINEAR, reset_jump,
            sizeof(reset_jump)) != LIB_STATUS_OK;
        failed |= x86_core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
            result.reason != X86_CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= x86_core_machine_reset(machine) != LIB_STATUS_OK;
        lib_memory_set(observed, 0, sizeof(observed));
        failed |= x86_core_machine_memory_read(machine, 0x1000u, observed,
            sizeof(observed)) != LIB_STATUS_OK ||
            lib_memory_compare(image, observed, sizeof(image)) != 0;
    }
    x86_core_machine_destroy(machine);
    if (failed || firmware_rollback_contract()) return 1;
    for (lib_u32 mode = 0u; mode < 4u; ++mode)
        if (reset_window_segments(0xffff0000u, mode) ||
            reset_window_segments(0x00ff0000u, mode)) return 1;
    lib_c_printf("%s\n", "IMMUTABLE-ROM-MAPPING:OK");
    lib_c_printf("%s\n", "ROM-PREFETCH-BOUNDARY:OK");
    return 0;
}
