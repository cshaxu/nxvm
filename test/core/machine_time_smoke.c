#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/machine_interface.h"
#include "support/core_machine_cpu_fixture.h"

static C_INT machine_time_expect(type_status status)
{
    return status == TYPE_STATUS_OK ? 0 : 1;
}

static C_INT machine_time_l1_compatibility(C_VOID)
{
    core_machine_config config = {0};
    core_machine_time_observation observation;
    core_machine *machine = STD_NULL;
    type_bool advanced = TYPE_FALSE;
    C_INT failed = 0;

    config.l1_compatibility_policy = CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= !failed && test_core_machine_fixture_register_reset_mapping(machine,
        0xfffffff0u, 0x000ffff0u, 16u) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != TYPE_STATUS_OK;
    if (!failed) machine->d4_refresh_hold_pending = TYPE_TRUE;
    failed |= !failed && (core_machine_capture_time_observation(machine, &observation) !=
        TYPE_STATUS_OK || observation.next_deadline_valid ||
        observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_L1_COMPATIBILITY);
    failed |= !failed && (core_machine_advance_l1_compatibility(machine, &advanced) !=
        TYPE_STATUS_OK || !advanced || machine->d4_refresh_hold_pending);
    core_machine_destroy(machine);
    return failed;
}

static C_VOID machine_time_timeline_callback(C_VOID *context,
    type_unsigned_64 due_tick)
{
    C_UINT *count = (C_UINT *)context;

    (C_VOID)due_tick;
    if (count != STD_NULL) ++*count;
}

static C_INT machine_time_l1_precedes_unrelated_deadline(C_VOID)
{
    core_machine_config config = {0};
    core_machine_time_observation observation;
    core_machine *machine = STD_NULL;
    type_bool advanced = TYPE_FALSE;
    C_UINT timeline_count = 0u;
    core_machine_timeline_token token;
    C_INT failed = 0;

    config.l1_compatibility_policy = CORE_MACHINE_L1_COMPATIBILITY_BOUNDED_PROGRESS;
    failed |= core_machine_create(&config, &machine) != TYPE_STATUS_OK;
    failed |= !failed && test_core_machine_fixture_register_reset_mapping(machine,
        0xfffffff0u, 0x000ffff0u, 16u) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_freeze_execution_providers(machine) != TYPE_STATUS_OK;
    failed |= !failed && core_machine_reset(machine) != TYPE_STATUS_OK;
    if (!failed) {
        machine->d4_refresh_hold_pending = TYPE_TRUE;
        failed |= core_machine_timeline_schedule(&machine->timeline, 4u,
            machine_time_timeline_callback, &timeline_count, &token) != TYPE_STATUS_OK;
    }
    failed |= !failed && (core_machine_capture_time_observation(machine, &observation) !=
        TYPE_STATUS_OK || observation.next_deadline_valid ||
        observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_L1_COMPATIBILITY);
    failed |= !failed && (core_machine_advance_l1_compatibility(machine, &advanced) !=
        TYPE_STATUS_OK || !advanced || machine->d4_refresh_hold_pending ||
        machine->elapsed_ticks != 1u || timeline_count != 0u);
    failed |= !failed && (core_machine_advance_to_next_deadline(machine, &advanced) !=
        TYPE_STATUS_OK || !advanced || machine->elapsed_ticks != 4u ||
        timeline_count != 1u);
    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    core_machine_config config = { 0 };
    core_machine_run_budget budget = { 2u, 0u };
    core_machine_run_result result;
    core_machine_observation observation;
    core_machine_time_observation time_observation;
    core_machine *machine = STD_NULL;
    core_machine *rejected = STD_NULL;
    const type_unsigned_8 nop = 0x90u;
    type_unsigned_64 elapsed = 0u;
    C_INT failed = 0;

    config.ticks_per_instruction = 3u;
    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80286;
    config.time_axis = (core_machine_time_axis) {
        CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL, 8000000u };
    {
        core_machine_config invalid = config;

        invalid.time_axis.ticks_per_second = 0u;
        failed |= core_machine_create(&invalid, &rejected) != TYPE_STATUS_INVALID_ARGUMENT ||
            rejected != STD_NULL;
        invalid = config;
        invalid.time_axis.kind = (core_machine_time_axis_kind)3;
        failed |= core_machine_create(&invalid, &rejected) != TYPE_STATUS_INVALID_ARGUMENT ||
            rejected != STD_NULL;
        invalid = config;
        invalid.time_axis = (core_machine_time_axis) {
            CORE_MACHINE_TIME_AXIS_UNQUALIFIED, 0u };
        invalid.retirement_time_contract = CORE_MACHINE_RETIREMENT_TIME_PHYSICAL;
        failed |= core_machine_create(&invalid, &rejected) != TYPE_STATUS_INVALID_ARGUMENT ||
            rejected != STD_NULL;
        invalid = config;
        invalid.time_axis = (core_machine_time_axis) {
            CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL, 8000000u };
        invalid.retirement_time_contract = CORE_MACHINE_RETIREMENT_TIME_PHYSICAL;
        failed |= core_machine_create(&invalid, &rejected) != TYPE_STATUS_INVALID_ARGUMENT ||
            rejected != STD_NULL;
    }
    failed |= machine_time_expect(core_machine_create(&config, &machine));
    failed |= test_core_machine_fixture_register_reset_mapping(machine, 0xfffffff0u,
        0x000ffff0u, 16u) != TYPE_STATUS_OK;
    failed |= machine_time_expect(core_machine_freeze_execution_providers(machine));
    failed |= machine_time_expect(core_machine_reset(machine));
    failed |= machine_time_expect(core_machine_get_elapsed_ticks(machine, &elapsed));
    failed |= elapsed != 0u;
    failed |= core_machine_capture_time_observation(machine, &time_observation) !=
        TYPE_STATUS_OK || time_observation.elapsed_ticks != 0u ||
        time_observation.next_deadline_tick != 0u ||
        time_observation.next_deadline_valid || !time_observation.pacing_time_available ||
        time_observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_IDLE ||
        time_observation.pacing_ticks_per_second != 8000000u ||
        !time_observation.physical_time_available ||
        time_observation.physical_ticks_per_second != 8000000u;
    failed |= machine_time_l1_compatibility();
    failed |= machine_time_l1_precedes_unrelated_deadline();
    failed |= core_machine_memory_write(machine, 0xfffffff0u, &nop, sizeof(nop)) !=
        TYPE_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0xfffffff1u, &nop, sizeof(nop)) !=
        TYPE_STATUS_OK;
    failed |= core_machine_memory_write(machine, 0xfffffff2u, &nop, sizeof(nop)) !=
        TYPE_STATUS_OK;
    failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 2u ||
        result.ticks != 6u || result.elapsed_ticks != 6u;
    failed |= core_machine_capture_observation(machine, &observation) != TYPE_STATUS_OK;
    failed |= observation.elapsed_ticks != 6u;
    failed |= core_machine_capture_time_observation(machine, &time_observation) !=
        TYPE_STATUS_OK || time_observation.elapsed_ticks != 6u ||
        time_observation.next_deadline_tick != 0u ||
        time_observation.next_deadline_valid ||
        time_observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_IDLE;
    budget.instructions = 1u;
    budget.ticks = 28u;
    failed |= core_machine_run(machine, budget, &result) != TYPE_STATUS_OK;
    failed |= result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
        result.ticks != 3u || result.elapsed_ticks != 9u;
    failed |= machine_time_expect(core_machine_reset(machine));
    failed |= machine_time_expect(core_machine_get_elapsed_ticks(machine, &elapsed));
    failed |= elapsed != 0u;
    failed |= core_machine_capture_time_observation(machine, &time_observation) !=
        TYPE_STATUS_OK || time_observation.elapsed_ticks != 0u ||
        time_observation.next_deadline_tick != 0u ||
        time_observation.next_deadline_valid ||
        time_observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_IDLE;
    core_machine_destroy(machine);
    core_machine_destroy(rejected);
    if (failed) return 1;
    STD_PRINTF("M5:T217:S2:TIME:OK\n");
    return 0;
}
