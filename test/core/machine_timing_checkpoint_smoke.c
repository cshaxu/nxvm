#include "type.h"

#include "core/machine/machine_interface.h"
#include "support/core_machine_cpu_fixture.h"

#define CHECKPOINTS 16u

static C_INT timing_checkpoint_run(core_machine *machine,
    const type_unsigned_8 *program, type_unsigned_8 *statuses)
{
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    type_unsigned_32 index;

    if (core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(machine, 0xfffffff0u, program, CHECKPOINTS) !=
            TYPE_STATUS_OK) {
        STD_FPRINTF(STD_STDERR, "T221 setup failed\n");
        return 1;
    }
    for (index = 0u; index < CHECKPOINTS; ++index) {
        type_unsigned_64 ticks = 3u;
        type_unsigned_64 elapsed = (type_unsigned_64)(index + 1u) * 3u;

        if (core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
            result.ticks != ticks || result.elapsed_ticks != elapsed) {
            STD_FPRINTF(STD_STDERR,
                "T221 run failed index=%u reason=%d executed=%llu ticks=%llu elapsed=%llu\n",
                index, (C_INT)result.reason, (unsigned long long)result.executed,
                (unsigned long long)result.ticks,
                (unsigned long long)result.elapsed_ticks);
            return 1;
        }
        statuses[index] = test_core_machine_fixture_read_port(machine, 0x03dau);
    }
    return 0;
}

C_INT main(C_VOID)
{
    core_machine *machine = STD_NULL;
    core_machine_config config = { 0 };
    type_unsigned_8 program[CHECKPOINTS];
    type_unsigned_8 first[CHECKPOINTS];
    type_unsigned_8 second[CHECKPOINTS];
    C_INT failed = 0;

    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80286;
    STD_MEMSET(program, 0x90, sizeof(program));
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, CHECKPOINTS) != TYPE_STATUS_OK;
    failed |= core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= timing_checkpoint_run(machine, program, first);
    failed |= timing_checkpoint_run(machine, program, second);
    failed |= STD_MEMCMP(first, second, sizeof(first)) != 0;

    if (failed) {
        STD_FPRINTF(STD_STDERR,
            "M5:T221:S2:TIMING-CHECKPOINT:FAIL first=%u,%u,%u,%u,%u,%u,%u "
            "second=%u,%u,%u,%u,%u,%u,%u\n",
            first[0u], first[15u], 0u, 0u, 0u, 0u, 0u,
            second[0u], second[15u], 0u, 0u, 0u, 0u, 0u);
        core_machine_destroy(machine);
        return 1;
    }
    core_machine_destroy(machine);
    STD_PRINTF("M5:T221:S2:TIMING-CHECKPOINT:OK\n");
    return 0;
}
