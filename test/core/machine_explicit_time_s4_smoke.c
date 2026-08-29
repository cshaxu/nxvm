#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "core/machine/rtc.h"
#include "support/core_machine_cpu_fixture.h"

static C_INT machine_explicit_time_prepare(core_machine **out_machine)
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

    return core_machine_create(&config, out_machine) == TYPE_STATUS_OK &&
        core_machine_configure_rtc_cmos(*out_machine, &rtc) == TYPE_STATUS_OK &&
        test_core_machine_fixture_register_reset_mapping(*out_machine, 0x00fffff0u,
            0x000ffff0u, 16u) == TYPE_STATUS_OK &&
        core_machine_freeze_execution_providers(*out_machine) == TYPE_STATUS_OK &&
        core_machine_reset(*out_machine) == TYPE_STATUS_OK;
}

static C_INT machine_explicit_time_rejects_unstarted_lifecycle(C_VOID)
{
    const core_machine_config config = {
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80286
    };
    core_machine *machine = STD_NULL;
    type_unsigned_64 elapsed = 1u;
    C_INT failed = core_machine_create(&config, &machine) != TYPE_STATUS_OK ||
        core_machine_advance_time(machine, 1u) != TYPE_STATUS_INVALID_STATE ||
        core_machine_get_elapsed_ticks(machine, &elapsed) != TYPE_STATUS_INVALID_STATE ||
        machine->elapsed_ticks != 0u;

    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    const type_unsigned_8 halt = 0xf4u;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result first;
    core_machine_run_result second;
    core_machine_timeline_observation before;
    core_machine_timeline_observation after;
    core_machine *machine = STD_NULL;
    type_unsigned_64 elapsed = 0u;
    type_unsigned_8 second_before_explicit_time = 0u;
    C_INT failed = 0;

    if (machine_explicit_time_rejects_unstarted_lifecycle() ||
        !machine_explicit_time_prepare(&machine) ||
        core_machine_memory_write(machine, 0x00fffff0u, &halt, sizeof(halt)) !=
            TYPE_STATUS_OK ||
        core_machine_run(machine, budget, &first) != TYPE_STATUS_OK ||
        first.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        first.executed != 1u || first.elapsed_ticks == 0u ||
        core_machine_get_timeline_observation(machine, &before) != TYPE_STATUS_OK ||
        core_machine_run(machine, budget, &second) != TYPE_STATUS_OK) {
        core_machine_destroy(machine);
        return 1;
    }
    second_before_explicit_time = machine->shared_rtc.calendar.second;

    failed |= second.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
        second.executed != 0u || second.ticks != 0u ||
        second.elapsed_ticks != first.elapsed_ticks ||
        core_machine_get_timeline_observation(machine, &after) != TYPE_STATUS_OK ||
        after.now != before.now || after.next_sequence != before.next_sequence ||
        core_machine_advance_time(machine, 0u) != TYPE_STATUS_INVALID_ARGUMENT ||
        core_machine_get_elapsed_ticks(machine, &elapsed) != TYPE_STATUS_OK ||
        elapsed != first.elapsed_ticks;
    failed |= core_machine_advance_time(machine, 5u) != TYPE_STATUS_OK ||
        core_machine_get_elapsed_ticks(machine, &elapsed) != TYPE_STATUS_OK ||
        elapsed != first.elapsed_ticks + 5u ||
        core_machine_get_timeline_observation(machine, &after) != TYPE_STATUS_OK ||
        after.now != before.now + 5u ||
        machine->shared_rtc.calendar.second != second_before_explicit_time + 5u;
    failed |= core_machine_reset(machine) != TYPE_STATUS_OK ||
        core_machine_get_elapsed_ticks(machine, &elapsed) != TYPE_STATUS_OK ||
        elapsed != 0u || machine->shared_rtc.calendar.second !=
            second_before_explicit_time + 5u ||
        core_machine_advance_time(machine, 1u) != TYPE_STATUS_OK ||
        core_machine_get_elapsed_ticks(machine, &elapsed) != TYPE_STATUS_OK ||
        elapsed != 1u;
    machine->elapsed_ticks = UINT64_MAX;
    failed |= core_machine_advance_time(machine, 1u) != TYPE_STATUS_INVALID_ARGUMENT ||
        machine->elapsed_ticks != UINT64_MAX;

    core_machine_destroy(machine);
    if (failed) return 1;
    STD_PRINTF("M5:T375:S4:EXPLICIT-MACHINE-TIME:OK\n");
    return 0;
}
