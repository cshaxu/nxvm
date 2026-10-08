#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "x86/core/clock_interface.h"
#include "x86/core/machine_interface.h"
#include "memory_alias_fixture.h"

#define RATIONAL_CLOCK_STEPS 4u

typedef struct rational_clock_probe {
    lib_u64 ticks[RATIONAL_CLOCK_STEPS];
    lib_u32 count;
} rational_clock_probe;

static void rational_clock_probe_reset(void *opaque)
{
    rational_clock_probe *probe = (rational_clock_probe *)opaque;

    if (probe != LIB_NULL) lib_memory_set(probe, 0, sizeof(*probe));
}

static void rational_clock_probe_advance(void *opaque,
    lib_u64 elapsed_ticks)
{
    rational_clock_probe *probe = (rational_clock_probe *)opaque;

    if (probe != LIB_NULL && probe->count < RATIONAL_CLOCK_STEPS) {
        probe->ticks[probe->count++] = elapsed_ticks;
    }
}

static const core_machine_execution_provider rational_clock_provider = {
    rational_clock_probe_reset,
    rational_clock_probe_advance
};

static lib_i32 rational_clock_prepare(core_machine **out_machine,
    rational_clock_probe *probe)
{
    const lib_u8 program[RATIONAL_CLOCK_STEPS] = {
        0x90u, 0x90u, 0x90u, 0x90u
    };
    core_machine_executor_config config = { 0 };

    config.ticks_per_instruction = 1u;
    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80286;
    config.provider_clock.numerator = 3u;
    config.provider_clock.denominator = 2u;
    config.provider_clock.reset_phase = 1u;
    if (core_machine_neutral_create(&config, out_machine) != LIB_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(*out_machine, 0xfffffff0u,
            0x000ffff0u, sizeof(program)) != LIB_STATUS_OK ||
        core_machine_bind_execution_provider(*out_machine, &rational_clock_provider,
            probe) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(*out_machine) != LIB_STATUS_OK ||
        core_machine_reset(*out_machine) != LIB_STATUS_OK ||
        core_machine_memory_write(*out_machine, 0xfffffff0u, program,
            sizeof(program)) != LIB_STATUS_OK) {
        core_machine_destroy(*out_machine);
        *out_machine = LIB_NULL;
        return 0;
    }
    return 1;
}

static lib_i32 rational_clock_restart(core_machine *machine)
{
    const lib_u8 program[RATIONAL_CLOCK_STEPS] = {
        0x90u, 0x90u, 0x90u, 0x90u
    };

    return core_machine_reset(machine) == LIB_STATUS_OK &&
        core_machine_memory_write(machine, 0xfffffff0u, program,
            sizeof(program)) == LIB_STATUS_OK;
}

static lib_i32 rational_clock_run(core_machine *machine, lib_u32 quantum)
{
    core_machine_run_budget budget = { quantum, 0u };
    core_machine_run_result result;
    lib_u32 remaining = RATIONAL_CLOCK_STEPS;

    while (remaining != 0u) {
        if (core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            result.executed != quantum) return 0;
        remaining -= quantum;
    }
    return 1;
}

static lib_i32 rational_clock_deadline_contract(void)
{
    const struct { core_machine_clock_ratio ratio; lib_u64 delivered, source; } cases[] = {
        {{3u, 2u, 1u}, 1u, 1u}, {{3u, 2u, 1u}, 2u, 1u},
        {{3u, 2u, 1u}, 3u, 2u}, {{1u, 3u, 2u}, 1u, 1u},
        {{1u, 3u, 2u}, 2u, 4u}, {{0u, 0u, 0u}, 5u, 5u}
    };
    core_machine_clock_domain domain = {0};
    lib_u64 source = 99u;
    if (core_machine_clock_ratio_is_valid(LIB_NULL) ||
        core_machine_clock_domain_source_ticks_until(LIB_NULL, 1u, &source) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_clock_domain_source_ticks_until(&domain, 1u, &source) != LIB_STATUS_INVALID_ARGUMENT ||
        source != 99u) return 1;
    for (lib_size i = 0u; i < sizeof(cases) / sizeof(cases[0]); ++i) {
        if (!core_machine_clock_ratio_is_valid(&cases[i].ratio) ||
            core_machine_clock_domain_initialize(&domain, &cases[i].ratio) != LIB_STATUS_OK ||
            core_machine_clock_domain_source_ticks_until(&domain, cases[i].delivered, &source) != LIB_STATUS_OK ||
            source != cases[i].source || domain.phase != domain.reset_phase || domain.delivered_ticks != 0u ||
            core_machine_clock_domain_source_ticks_until(&domain, 1u, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT) return 1;
    }
    if (core_machine_clock_domain_initialize(&domain, &cases[0].ratio) != LIB_STATUS_OK) return 1;
    source = 99u;
    return core_machine_clock_domain_source_ticks_until(&domain, LIB_UINT64_MAX, &source) !=
        LIB_STATUS_INVALID_ARGUMENT || source != 99u;
}

lib_i32 main(void)
{
    core_machine_clock_domain domain;
    core_machine_clock_ratio ratio = { 3u, 2u, 1u };
    core_machine_clock_ratio invalid_phase = { 1u, 2u, 2u };
    core_machine_clock_ratio invalid_zero = { 1u, 0u, 0u };
    core_machine_clock_ratio identity = { 0u, 0u, 0u };
    rational_clock_probe single = { { 0u }, 0u };
    rational_clock_probe reset = { { 0u }, 0u };
    rational_clock_probe split = { { 0u }, 0u };
    core_machine *machine = LIB_NULL;
    lib_i32 failed = rational_clock_deadline_contract();

    failed |= core_machine_clock_domain_initialize(&domain, &ratio) !=
        LIB_STATUS_OK;
    failed |= core_machine_clock_domain_advance(&domain, 1u) != 2u;
    failed |= core_machine_clock_domain_advance(&domain, 1u) != 1u;
    failed |= core_machine_clock_domain_advance(&domain, 2u) != 3u;
    core_machine_clock_domain_reset(&domain);
    failed |= core_machine_clock_domain_advance(&domain, 1u) != 2u;
    failed |= core_machine_clock_domain_initialize(&domain, &identity) !=
        LIB_STATUS_OK || core_machine_clock_domain_advance(&domain, 5u) != 5u;
    failed |= core_machine_clock_domain_initialize(&domain, &invalid_phase) !=
        LIB_STATUS_INVALID_ARGUMENT;
    failed |= core_machine_clock_domain_initialize(&domain, &invalid_zero) !=
        LIB_STATUS_INVALID_ARGUMENT;

    if (!failed && rational_clock_prepare(&machine, &single)) {
        failed |= !rational_clock_run(machine, RATIONAL_CLOCK_STEPS);
        reset = single;
        failed |= !rational_clock_restart(machine);
        failed |= !rational_clock_run(machine, 2u);
        core_machine_destroy(machine);
        machine = LIB_NULL;
    } else {
        failed = 1;
    }
    if (!failed && rational_clock_prepare(&machine, &split)) {
        failed |= !rational_clock_run(machine, 1u);
        core_machine_destroy(machine);
        machine = LIB_NULL;
    } else {
        failed = 1;
    }
    failed |= single.count != RATIONAL_CLOCK_STEPS ||
        reset.count != RATIONAL_CLOCK_STEPS ||
        split.count != RATIONAL_CLOCK_STEPS ||
        single.ticks[0] != 5u || single.ticks[1] != 4u ||
        single.ticks[2] != 5u || single.ticks[3] != 4u ||
        lib_memory_compare(reset.ticks, single.ticks, sizeof(single.ticks)) != 0 ||
        lib_memory_compare(reset.ticks, split.ticks, sizeof(single.ticks)) != 0;

    core_machine_destroy(machine);
    if (failed) return 1;
    lib_c_printf("RATIONAL-CLOCK:OK\n");
    return 0;
}
