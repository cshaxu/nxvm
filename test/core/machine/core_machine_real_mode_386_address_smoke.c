#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/debug_interface.h"
#include "core/machine/machine_interface.h"
#include "core/machine/memory_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define TEST_RESET_LINEAR 0xfffffff0u
#define TEST_RESET_PHYSICAL 0x000ffff0u
#define TEST_RESET_WINDOW 16u
#define TEST_SOURCE 0x00020000u
#define TEST_DESTINATION 0x00030000u

static C_INT real_mode_386_prepare(core_machine **out_machine)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 1u
    };
    static const type_unsigned_8 reset_jump[] = {0xeau, 0x00u, 0x00u, 0x00u, 0x00u};
    core_machine *machine = STD_NULL;

    if (out_machine == STD_NULL || core_machine_create(&config, &machine) !=
            TYPE_STATUS_OK || test_core_machine_fixture_register_reset_mapping(
            machine, TEST_RESET_LINEAR,
            TEST_RESET_PHYSICAL, TEST_RESET_WINDOW) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK ||
        core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, TEST_RESET_LINEAR, reset_jump,
            sizeof(reset_jump)) != TYPE_STATUS_OK) {
        core_machine_destroy(machine);
        return 0;
    }
    *out_machine = machine;
    return 1;
}

C_INT main(C_VOID)
{
    static const type_unsigned_8 program[] = {
        0xb8u, 0x00u, 0x00u,
        0x8eu, 0xd8u,
        0x8eu, 0xc0u,
        0x66u, 0xbeu, 0x00u, 0x00u, 0x02u, 0x00u,
        0x66u, 0xbfu, 0x00u, 0x00u, 0x03u, 0x00u,
        0x66u, 0xb9u, 0x01u, 0x00u, 0x00u, 0x00u,
        0xfcu,
        0xf3u, 0x67u, 0x66u, 0xa5u,
        0xf4u
    };
    static const type_unsigned_8 source[] = {0x41u, 0x42u, 0x43u, 0x44u};
    type_unsigned_8 destination[sizeof(source)] = {0};
    core_machine_cpu_state state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    const core_machine_run_budget budget = {256u, 0u};
    core_machine *machine = STD_NULL;
    C_INT failed = !real_mode_386_prepare(&machine);

    if (!failed) {
        failed |= core_machine_memory_write(machine, 0u, program,
            sizeof(program)) != TYPE_STATUS_OK ||
            core_machine_memory_write(machine, TEST_SOURCE, source,
            sizeof(source)) != TYPE_STATUS_OK ||
            core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            core_machine_memory_read(machine, TEST_DESTINATION, destination,
            sizeof(destination)) != TYPE_STATUS_OK ||
            core_machine_debug_read_cpu(machine, &state) != TYPE_STATUS_OK ||
            core_machine_get_cpu_diagnostic(machine, &diagnostic) != TYPE_STATUS_OK ||
            diagnostic.first_fault.valid ||
            STD_MEMCMP(destination, source, sizeof(source)) != 0 ||
            state.eip == 0u;
    }
    core_machine_destroy(machine);
    if (failed) return 1;
    STD_PRINTF("M5:T287:S24:REAL-MODE-386-ADDR32:OK\n");
    return 0;
}
