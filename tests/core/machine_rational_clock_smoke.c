#include "type.h"

#include "core/machine/clock.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define RATIONAL_CLOCK_STEPS 4u

typedef struct rational_clock_probe {
    type_unsigned_64 ticks[RATIONAL_CLOCK_STEPS];
    type_unsigned_32 count;
} rational_clock_probe;

static C_VOID rational_clock_probe_reset(C_VOID *opaque)
{
    rational_clock_probe *probe = (rational_clock_probe *)opaque;

    if (probe != STD_NULL) STD_MEMSET(probe, 0, sizeof(*probe));
}

static C_VOID rational_clock_probe_advance(C_VOID *opaque,
    type_unsigned_64 elapsed_ticks)
{
    rational_clock_probe *probe = (rational_clock_probe *)opaque;

    if (probe != STD_NULL && probe->count < RATIONAL_CLOCK_STEPS) {
        probe->ticks[probe->count++] = elapsed_ticks;
    }
}

static const core_machine_execution_provider rational_clock_provider = {
    rational_clock_probe_reset,
    rational_clock_probe_advance
};

static C_INT rational_clock_prepare(core_machine **out_machine,
    rational_clock_probe *probe)
{
    const type_unsigned_8 program[RATIONAL_CLOCK_STEPS] = {
        0x90u, 0x90u, 0x90u, 0x90u
    };
    core_machine_config config = { 0 };

    config.ticks_per_instruction = 1u;
    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80286;
    config.clock_plan.provider.numerator = 3u;
    config.clock_plan.provider.denominator = 2u;
    config.clock_plan.provider.reset_phase = 1u;
    if (core_machine_create(&config, out_machine) != TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(*out_machine, 0xfffffff0u,
            0x000ffff0u, sizeof(program)) != TYPE_STATUS_OK ||
        core_machine_bind_execution_provider(*out_machine, &rational_clock_provider,
            probe) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(*out_machine) != TYPE_STATUS_OK ||
        core_machine_reset(*out_machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(*out_machine, 0xfffffff0u, program,
            sizeof(program)) != TYPE_STATUS_OK) {
        core_machine_destroy(*out_machine);
        *out_machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT rational_clock_restart(core_machine *machine)
{
    const type_unsigned_8 program[RATIONAL_CLOCK_STEPS] = {
        0x90u, 0x90u, 0x90u, 0x90u
    };

    return core_machine_reset(machine) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, 0xfffffff0u, program,
            sizeof(program)) == TYPE_STATUS_OK;
}

static C_INT rational_clock_run(core_machine *machine, type_unsigned_32 quantum)
{
    core_machine_run_budget budget = { quantum, 0u };
    core_machine_run_result result;
    type_unsigned_32 remaining = RATIONAL_CLOCK_STEPS;

    while (remaining != 0u) {
        if (core_machine_run(machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            result.executed != quantum) return 0;
        remaining -= quantum;
    }
    return 1;
}

C_INT main(C_VOID)
{
    core_machine_clock_domain domain;
    core_machine_clock_ratio ratio = { 3u, 2u, 1u };
    core_machine_clock_ratio invalid_phase = { 1u, 2u, 2u };
    core_machine_clock_ratio invalid_zero = { 1u, 0u, 0u };
    core_machine_clock_ratio identity = { 0u, 0u, 0u };
    rational_clock_probe single = { { 0u }, 0u };
    rational_clock_probe reset = { { 0u }, 0u };
    rational_clock_probe split = { { 0u }, 0u };
    core_machine *machine = STD_NULL;
    C_INT failed = 0;

    failed |= core_machine_clock_domain_initialize(&domain, &ratio) !=
        TYPE_STATUS_OK;
    failed |= core_machine_clock_domain_advance(&domain, 1u) != 2u;
    failed |= core_machine_clock_domain_advance(&domain, 1u) != 1u;
    failed |= core_machine_clock_domain_advance(&domain, 2u) != 3u;
    core_machine_clock_domain_reset(&domain);
    failed |= core_machine_clock_domain_advance(&domain, 1u) != 2u;
    failed |= core_machine_clock_domain_initialize(&domain, &identity) !=
        TYPE_STATUS_OK || core_machine_clock_domain_advance(&domain, 5u) != 5u;
    failed |= core_machine_clock_domain_initialize(&domain, &invalid_phase) !=
        TYPE_STATUS_INVALID_ARGUMENT;
    failed |= core_machine_clock_domain_initialize(&domain, &invalid_zero) !=
        TYPE_STATUS_INVALID_ARGUMENT;

    if (!failed && rational_clock_prepare(&machine, &single)) {
        failed |= !rational_clock_run(machine, RATIONAL_CLOCK_STEPS);
        reset = single;
        failed |= !rational_clock_restart(machine);
        failed |= !rational_clock_run(machine, 2u);
        core_machine_destroy(machine);
        machine = STD_NULL;
    } else {
        failed = 1;
    }
    if (!failed && rational_clock_prepare(&machine, &split)) {
        failed |= !rational_clock_run(machine, 1u);
        core_machine_destroy(machine);
        machine = STD_NULL;
    } else {
        failed = 1;
    }
    failed |= single.count != RATIONAL_CLOCK_STEPS ||
        reset.count != RATIONAL_CLOCK_STEPS ||
        split.count != RATIONAL_CLOCK_STEPS ||
        single.ticks[0] != 5u || single.ticks[1] != 4u ||
        single.ticks[2] != 5u || single.ticks[3] != 4u ||
        STD_MEMCMP(reset.ticks, single.ticks, sizeof(single.ticks)) != 0 ||
        STD_MEMCMP(reset.ticks, split.ticks, sizeof(single.ticks)) != 0;

    core_machine_destroy(machine);
    if (failed) return 1;
    STD_PRINTF("M5:T256:S3:RATIONAL-CLOCK:OK\n");
    return 0;
}
