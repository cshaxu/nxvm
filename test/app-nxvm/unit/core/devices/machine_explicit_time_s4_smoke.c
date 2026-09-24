#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/rtc.h"
#include "support/core_machine_cpu_fixture.h"

static lib_i32 machine_explicit_time_prepare(core_machine **out_machine)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80286,
        .ticks_per_instruction = 1u
    };
    const core_machine_rtc_cmos_config rtc = {
        .index_port = 0x70u,
        .data_port = 0x71u,
        .irq = 8u,
        .nmi_mask_bit = 0x80u,
        .ticks_per_second = 1u
    };

    return core_machine_create(&config, out_machine) == LIB_STATUS_OK &&
        core_machine_configure_rtc_cmos(*out_machine, &rtc) == LIB_STATUS_OK &&
        test_core_machine_fixture_register_reset_mapping(*out_machine, 0x00fffff0u,
            0x000ffff0u, 16u) == LIB_STATUS_OK &&
        core_machine_freeze_execution_providers(*out_machine) == LIB_STATUS_OK &&
        core_machine_reset(*out_machine) == LIB_STATUS_OK;
}

static lib_i32 machine_explicit_time_rejects_unstarted_lifecycle(void)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80286
    };
    core_machine *machine = LIB_NULL;
    lib_u64 elapsed = 1u;
    lib_i32 failed = core_machine_create(&config, &machine) != LIB_STATUS_OK ||
        core_machine_advance_time(machine, 1u) != LIB_STATUS_INVALID_STATE ||
        core_machine_get_elapsed_ticks(machine, &elapsed) != LIB_STATUS_INVALID_STATE ||
        machine->elapsed_ticks != 0u;

    core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    const lib_u8 halt = 0xf4u;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result first;
    core_machine_run_result second;
    core_machine_timeline_observation before;
    core_machine_timeline_observation after;
    core_machine *machine = LIB_NULL;
    lib_u64 elapsed = 0u;
    lib_u8 second_before_explicit_time = 0u;
    lib_i32 failed = 0;

    if (machine_explicit_time_rejects_unstarted_lifecycle() ||
        !machine_explicit_time_prepare(&machine) ||
        core_machine_memory_write(machine, 0x00fffff0u, &halt, sizeof(halt)) !=
            LIB_STATUS_OK ||
        core_machine_run(machine, budget, &first) != LIB_STATUS_OK ||
        first.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        first.executed != 1u || first.elapsed_ticks == 0u ||
        core_machine_get_timeline_observation(machine, &before) != LIB_STATUS_OK ||
        core_machine_run(machine, budget, &second) != LIB_STATUS_OK) {
        core_machine_destroy(machine);
        return 1;
    }
    second_before_explicit_time = machine->shared_rtc.calendar.second;

    failed |= second.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        second.executed != 0u || second.ticks != 0u ||
        second.elapsed_ticks != first.elapsed_ticks ||
        core_machine_get_timeline_observation(machine, &after) != LIB_STATUS_OK ||
        after.now != before.now || after.next_sequence != before.next_sequence ||
        core_machine_advance_time(machine, 0u) != LIB_STATUS_INVALID_ARGUMENT ||
        core_machine_get_elapsed_ticks(machine, &elapsed) != LIB_STATUS_OK ||
        elapsed != first.elapsed_ticks;
    failed |= core_machine_advance_time(machine, 5u) != LIB_STATUS_OK ||
        core_machine_get_elapsed_ticks(machine, &elapsed) != LIB_STATUS_OK ||
        elapsed != first.elapsed_ticks + 5u ||
        core_machine_get_timeline_observation(machine, &after) != LIB_STATUS_OK ||
        after.now != before.now + 5u ||
        machine->shared_rtc.calendar.second != second_before_explicit_time + 5u;
    failed |= core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_get_elapsed_ticks(machine, &elapsed) != LIB_STATUS_OK ||
        elapsed != 0u || machine->shared_rtc.calendar.second !=
            second_before_explicit_time + 5u ||
        core_machine_advance_time(machine, 1u) != LIB_STATUS_OK ||
        core_machine_get_elapsed_ticks(machine, &elapsed) != LIB_STATUS_OK ||
        elapsed != 1u;
    machine->elapsed_ticks = UINT64_MAX;
    failed |= core_machine_advance_time(machine, 1u) != LIB_STATUS_INVALID_ARGUMENT ||
        machine->elapsed_ticks != UINT64_MAX;

    core_machine_destroy(machine);
    if (failed) return 1;
    printf("M5:T375:S4:EXPLICIT-MACHINE-TIME:OK\n");
    return 0;
}
