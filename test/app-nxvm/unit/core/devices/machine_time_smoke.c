#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_cpu_fixture.h"

static lib_i32 machine_time_expect(lib_status status)
{
    return status == LIB_STATUS_OK ? 0 : 1;
}

static void machine_time_timeline_callback(void *context,
    lib_u64 due_tick)
{
    lib_u32 *count = (lib_u32 *)context;

    (void)due_tick;
    if (count != LIB_NULL) ++*count;
}

static lib_i32 machine_time_d4_l2_precedes_unrelated_deadline(void)
{
    core_machine_config config = {0};
    core_machine_time_observation observation;
    core_machine *machine = LIB_NULL;
    lib_u8 advanced = LIB_FALSE;
    lib_u32 timeline_count = 0u;
    core_machine_timeline_token token;
    lib_i32 failed = 0;

    failed |= core_machine_create(&config, &machine) != LIB_STATUS_OK;
    failed |= !failed && test_core_machine_fixture_register_reset_mapping(machine,
        0xfffffff0u, 0x000ffff0u, 16u) != LIB_STATUS_OK;
    failed |= !failed && core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != LIB_STATUS_OK;
    if (!failed) {
        machine->d4_refresh_hold_pending = LIB_TRUE;
        failed |= core_machine_timeline_schedule(&machine->timeline, 4u,
            machine_time_timeline_callback, &timeline_count, &token) != LIB_STATUS_OK;
    }
    failed |= !failed && (core_machine_capture_time_observation(machine, &observation) !=
        LIB_STATUS_OK || !observation.next_deadline_valid ||
        observation.next_deadline_tick != 1u ||
        observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_DEADLINE);
    failed |= !failed && (core_machine_advance_to_next_deadline(machine, &advanced) !=
        LIB_STATUS_OK || !advanced || machine->d4_refresh_hold_pending ||
        machine->elapsed_ticks != 1u || timeline_count != 0u);
    failed |= !failed && (core_machine_advance_to_next_deadline(machine, &advanced) !=
        LIB_STATUS_OK || !advanced || machine->elapsed_ticks != 4u ||
        timeline_count != 1u);
    core_machine_destroy(machine);
    return failed;
}

lib_i32 main(void)
{
    core_machine_config config = { 0 };
    core_machine_run_budget budget = { 2u, 0u };
    core_machine_run_result result;
    core_machine_observation observation;
    core_machine_time_observation time_observation;
    core_machine *machine = LIB_NULL;
    core_machine *rejected = LIB_NULL;
    const lib_u8 nop = 0x90u;
    lib_u64 elapsed = 0u;
    lib_i32 failed = 0;

    config.ticks_per_instruction = 3u;
    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80286;
    config.time_axis = (core_machine_time_axis) {
        CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL, 8000000u };
    {
        core_machine_config invalid = config;
        core_machine rejected_sentinel = {0};

        rejected = &rejected_sentinel;

        invalid.time_axis.ticks_per_second = 0u;
        failed |= core_machine_create(&invalid, &rejected) != LIB_STATUS_INVALID_ARGUMENT ||
            rejected != LIB_NULL;
        invalid = config;
        invalid.time_axis.kind = (core_machine_time_axis_kind)3;
        failed |= core_machine_create(&invalid, &rejected) != LIB_STATUS_INVALID_ARGUMENT ||
            rejected != LIB_NULL;
        invalid = config;
        invalid.time_axis = (core_machine_time_axis) {
            CORE_MACHINE_TIME_AXIS_UNQUALIFIED, 0u };
        invalid.retirement_time_contract = CORE_MACHINE_RETIREMENT_TIME_PHYSICAL;
        failed |= core_machine_create(&invalid, &rejected) != LIB_STATUS_INVALID_ARGUMENT ||
            rejected != LIB_NULL;
        invalid = config;
        invalid.time_axis = (core_machine_time_axis) {
            CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL, 8000000u };
        invalid.retirement_time_contract = CORE_MACHINE_RETIREMENT_TIME_PHYSICAL;
        failed |= core_machine_create(&invalid, &rejected) != LIB_STATUS_INVALID_ARGUMENT ||
            rejected != LIB_NULL;
    }
    failed |= machine_time_expect(core_machine_create(&config, &machine));
    failed |= test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, 16u) != LIB_STATUS_OK;
    failed |= machine_time_expect(core_machine_freeze_execution_providers(machine));
    failed |= machine_time_expect(core_machine_reset(machine));
    failed |= machine_time_expect(core_machine_get_elapsed_ticks(machine, &elapsed));
    failed |= elapsed != 0u;
    failed |= core_machine_capture_time_observation(machine, &time_observation) !=
        LIB_STATUS_OK || time_observation.elapsed_ticks != 0u ||
        time_observation.next_deadline_tick != 0u ||
        time_observation.next_deadline_valid || !time_observation.pacing_time_available ||
        time_observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_IDLE ||
        time_observation.pacing_ticks_per_second != 8000000u ||
        !time_observation.physical_time_available ||
        time_observation.physical_ticks_per_second != 8000000u;
    failed |= machine_time_d4_l2_precedes_unrelated_deadline();
    failed |= core_machine_memory_write(machine, 0xfffffff0u, &nop, sizeof(nop)) !=
        LIB_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0xfffffff1u, &nop, sizeof(nop)) !=
        LIB_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0xfffffff2u, &nop, sizeof(nop)) !=
        LIB_STATUS_OK;
    failed |= core_machine_run(machine, budget, &result) != LIB_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 2u ||
        result.ticks != 6u || result.elapsed_ticks != 6u;
    failed |= core_machine_capture_observation(machine, &observation) != LIB_STATUS_OK;
    failed |= observation.elapsed_ticks != 6u;
    failed |= core_machine_capture_time_observation(machine, &time_observation) !=
        LIB_STATUS_OK || time_observation.elapsed_ticks != 6u ||
        time_observation.next_deadline_tick != 0u ||
        time_observation.next_deadline_valid ||
        time_observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_IDLE;
    budget.instructions = 1u;
    budget.ticks = 28u;
    failed |= core_machine_run(machine, budget, &result) != LIB_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
        result.ticks != 3u || result.elapsed_ticks != 9u;
    failed |= machine_time_expect(core_machine_reset(machine));
    failed |= machine_time_expect(core_machine_get_elapsed_ticks(machine, &elapsed));
    failed |= elapsed != 0u;
    failed |= core_machine_capture_time_observation(machine, &time_observation) !=
        LIB_STATUS_OK || time_observation.elapsed_ticks != 0u ||
        time_observation.next_deadline_tick != 0u ||
        time_observation.next_deadline_valid ||
        time_observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_IDLE;
    core_machine_destroy(machine);
    core_machine_destroy(rejected);
    if (failed) return 1;
    printf("M5:T217:S2:TIME:OK\n");
    return 0;
}
