#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "core/board-base/machine_board_interface.h"
#include "core_machine_board_fixture.h"

#define CHECKPOINTS 16u

static lib_i32 timing_checkpoint_run(core_machine *machine,
    const lib_u8 *program, lib_u8 *statuses)
{
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    lib_u32 index;

    if (core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0xfffffff0u, program, CHECKPOINTS) !=
            LIB_STATUS_OK) {
        lib_c_fprintf(lib_c_stderr, "setup failed\n");
        return 1;
    }
    for (index = 0u; index < CHECKPOINTS; ++index) {
        lib_u64 ticks = 3u;
        lib_u64 elapsed = (lib_u64)(index + 1u) * 3u;

        if (core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            result.ticks != ticks || result.elapsed_ticks != elapsed) {
            lib_c_fprintf(lib_c_stderr,
                "run failed index=%u reason=%d executed=%llu ticks=%llu elapsed=%llu\n",
                index, (lib_i32)result.reason, (unsigned long long)result.executed,
                (unsigned long long)result.ticks,
                (unsigned long long)result.elapsed_ticks);
            return 1;
        }
        statuses[index] = test_core_machine_fixture_read_port(machine, 0x03dau);
    }
    return 0;
}

lib_i32 main(void)
{
    core_machine *machine = LIB_NULL;
    core_machine_config config = { 0 };
    lib_u8 program[CHECKPOINTS];
    lib_u8 first[CHECKPOINTS];
    lib_u8 second[CHECKPOINTS];
    lib_i32 failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80286;
    lib_memory_set(program, 0x90, sizeof(program));
    failed |= core_machine_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK;
    failed |= test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, CHECKPOINTS) != LIB_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= timing_checkpoint_run(machine, program, first);
    failed |= timing_checkpoint_run(machine, program, second);
    failed |= lib_memory_compare(first, second, sizeof(first)) != 0;

    if (failed) {
        lib_c_fprintf(lib_c_stderr,
            "TIMING-CHECKPOINT:FAIL first=%u,%u,%u,%u,%u,%u,%u "
            "second=%u,%u,%u,%u,%u,%u,%u\n",
            first[0u], first[15u], 0u, 0u, 0u, 0u, 0u,
            second[0u], second[15u], 0u, 0u, 0u, 0u, 0u);
        core_machine_destroy(machine);
        return 1;
    }
    core_machine_destroy(machine);
    lib_c_printf("TIMING-CHECKPOINT:OK\n");
    return 0;
}
