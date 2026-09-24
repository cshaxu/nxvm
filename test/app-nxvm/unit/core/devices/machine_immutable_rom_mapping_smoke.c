#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_cpu_fixture.h"

#define RESET_LINEAR 0xfffffff0u
#define RESET_PHYSICAL 0x000ffff0u

lib_i32 main(void)
{
    static const lib_u8 image[] = { 0xf4u, 0x90u, 0x90u };
    static const lib_u8 high_image[] = { 0x5au };
    static const lib_u8 reset_jump[] = { 0xeau, 0x00u, 0x10u, 0x00u, 0x00u };
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    const core_machine_run_budget budget = { 16u, 0u };
    core_machine *machine = LIB_NULL;
    core_machine_memory_route route;
    core_machine_run_result result;
    lib_u8 observed[sizeof(image)] = { 0u };
    lib_u8 overwrite = 0u;
    lib_i32 failed = 0;

    failed |= core_machine_create(&config, &machine) != LIB_STATUS_OK;
    failed |= machine == LIB_NULL;
    if (!failed) {
        failed |= test_core_machine_fixture_register_reset_mapping(machine, RESET_LINEAR,
            RESET_PHYSICAL, 16u) != LIB_STATUS_OK;
        failed |= core_machine_register_immutable_rom_mapping(machine, 0x1000u,
            image, sizeof(image)) != LIB_STATUS_OK;
        failed |= core_machine_register_immutable_rom_mapping(machine, 0xf0000u,
            high_image, sizeof(high_image)) != LIB_STATUS_OK;
        failed |= core_machine_register_immutable_rom_mapping(machine, 0x1001u,
            image, 1u) != LIB_STATUS_INVALID_ARGUMENT;
        failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
        failed |= core_machine_register_immutable_rom_mapping(machine, 0x2000u,
            image, 1u) != LIB_STATUS_INVALID_STATE;
        failed |= core_machine_reset(machine) != LIB_STATUS_OK;
        failed |= core_machine_memory_query(machine, 0x1000u, sizeof(image),
            CORE_MACHINE_MEMORY_ACCESS_READ, &route) != LIB_STATUS_OK ||
            route != CORE_MACHINE_MEMORY_ROUTE_PROVIDER;
        failed |= core_machine_memory_query(machine, 0x1000u, 1u,
            CORE_MACHINE_MEMORY_ACCESS_WRITE, &route) != LIB_STATUS_OK ||
            route != CORE_MACHINE_MEMORY_ROUTE_PROVIDER;
        failed |= core_machine_memory_read(machine, 0x1000u, observed,
            sizeof(observed)) != LIB_STATUS_OK ||
            lib_memory_compare(image, observed, sizeof(image)) != 0;
        failed |= core_machine_memory_write(machine, 0x1000u, &overwrite, 1u) !=
            LIB_STATUS_OK;
        failed |= core_machine_memory_read(machine, 0x1000u, observed,
            sizeof(observed)) != LIB_STATUS_OK ||
            lib_memory_compare(image, observed, sizeof(image)) != 0;
        failed |= core_machine_memory_write(machine, 0xf0000u, &overwrite, 1u) !=
            LIB_STATUS_OK;
        failed |= core_machine_memory_read(machine, 0xf0000u, observed, 1u) !=
            LIB_STATUS_OK || observed[0u] != high_image[0u];
        failed |= core_machine_memory_write(machine, RESET_LINEAR, reset_jump,
            sizeof(reset_jump)) != LIB_STATUS_OK;
        failed |= core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= core_machine_reset(machine) != LIB_STATUS_OK;
        lib_memory_set(observed, 0, sizeof(observed));
        failed |= core_machine_memory_read(machine, 0x1000u, observed,
            sizeof(observed)) != LIB_STATUS_OK ||
            lib_memory_compare(image, observed, sizeof(image)) != 0;
    }
    core_machine_destroy(machine);
    if (failed) return 1;
    puts("M5:T245:S2:IMMUTABLE-ROM-MAPPING:OK");
    puts("M5:T419:S2:ROM-PREFETCH-BOUNDARY:OK");
    return 0;
}
