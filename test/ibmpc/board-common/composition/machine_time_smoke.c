#include "app-mydeskpro386/profiles/d4_platform_interface.h"
#include "lib/types/types_interface.h"
#include "../../../app-mydeskpro386/unit/profiles/d4_refresh_fixture.h"
#include <stdio.h>

#include "../../../x86/core/composition_fixture.h"
#include "../../../x86/core/time_fixture.h"
#include "ibmpc/board-common/machine_board_interface.h"
#include "../core_machine_board_fixture.h"

static lib_i32 machine_time_rejects(const core_machine_config *config)
{
    /* This non-owned sentinel proves that rejection clears the output. */
    core_machine *sentinel = (core_machine *)config;
    core_machine *rejected = sentinel;
    lib_status status = core_machine_create(config, &rejected, LIB_NULL);
    lib_i32 failed = status != LIB_STATUS_INVALID_ARGUMENT || rejected != LIB_NULL;

    if (rejected != LIB_NULL && rejected != sentinel) core_machine_destroy(rejected);
    return failed;
}

static lib_i32 machine_time_d4_l2_precedes_unrelated_deadline(void)
{
    core_machine_config config = {0};
    const core_machine_d4_platform_config d4 = {CORE_MACHINE_PC_AT_PORT_B, 0u};
    core_machine_time_observation observation;
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_d4_platform *d4_board = LIB_NULL;
    lib_u8 advanced = LIB_FALSE;
    lib_u32 timeline_count = 0u;
    lib_i32 failed = 1;

    config.auxiliary_pit_present = LIB_TRUE;
    config.auxiliary_pit_base_port = 0x48u;
    if (core_machine_create(&config, &machine, &board) != LIB_STATUS_OK ||
        machine == LIB_NULL || board == LIB_NULL ||
        core_machine_d4_platform_attach(board, &d4, &d4_board) != LIB_STATUS_OK ||
        d4_board == LIB_NULL || test_core_machine_fixture_register_reset_mapping(machine,
        0xfffffff0u, 0x000ffff0u, 16u) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK) goto done;
    test_model40_refresh_set_pending(d4_board);
    if (test_core_schedule_counter(machine, 4u, &timeline_count) != LIB_STATUS_OK ||
        core_machine_capture_time_observation(machine, &observation) !=
        LIB_STATUS_OK || !observation.next_deadline_valid ||
        observation.next_deadline_tick != 1u ||
        observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_DEADLINE ||
        core_machine_advance_to_next_deadline(machine, &advanced) !=
        LIB_STATUS_OK || !advanced || test_model40_refresh_is_pending(d4_board) ||
        test_core_elapsed_ticks(machine) != 1u || timeline_count != 0u ||
        core_machine_advance_to_next_deadline(machine, &advanced) !=
        LIB_STATUS_OK || !advanced || test_core_elapsed_ticks(machine) != 4u ||
        timeline_count != 1u) goto done;
    failed = 0;
done:
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
    const lib_u8 nop = 0x90u;
    lib_u64 elapsed = 0u;
    lib_i32 failed = 1;

    config.ticks_per_instruction = 3u;
    config.cpu_profile = CORE_MACHINE_CPU_PROFILE_80286;
    config.time_axis = (core_machine_time_axis) {
        CORE_MACHINE_TIME_AXIS_VERIFIED_PHYSICAL, 8000000u };
    {
        core_machine_config invalid = config;
        invalid.time_axis.ticks_per_second = 0u;
        if (machine_time_rejects(&invalid)) goto done;
        invalid = config;
        invalid.time_axis.kind = (core_machine_time_axis_kind)3;
        if (machine_time_rejects(&invalid)) goto done;
        invalid = config;
        invalid.time_axis = (core_machine_time_axis) {
            CORE_MACHINE_TIME_AXIS_UNQUALIFIED, 0u };
        invalid.retirement_time_contract = CORE_MACHINE_RETIREMENT_TIME_PHYSICAL;
        if (machine_time_rejects(&invalid)) goto done;
        invalid = config;
        invalid.time_axis = (core_machine_time_axis) {
            CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL, 8000000u };
        invalid.retirement_time_contract = CORE_MACHINE_RETIREMENT_TIME_PHYSICAL;
        if (machine_time_rejects(&invalid)) goto done;
    }
    if (core_machine_create(&config, &machine, LIB_NULL) != LIB_STATUS_OK ||
        machine == LIB_NULL || test_core_machine_fixture_register_reset_mapping(machine,
        0xfffffff0u, 0x000ffff0u, 16u) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_get_elapsed_ticks(machine, &elapsed) != LIB_STATUS_OK ||
        elapsed != 0u ||
        core_machine_capture_time_observation(machine, &time_observation) !=
        LIB_STATUS_OK || time_observation.elapsed_ticks != 0u ||
        time_observation.next_deadline_tick != 0u ||
        time_observation.next_deadline_valid || !time_observation.pacing_time_available ||
        time_observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_IDLE ||
        time_observation.pacing_ticks_per_second != 8000000u ||
        !time_observation.physical_time_available ||
        time_observation.physical_ticks_per_second != 8000000u ||
        machine_time_d4_l2_precedes_unrelated_deadline() ||
        core_machine_memory_write(machine, 0xfffffff0u, &nop, sizeof(nop)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0xfffffff1u, &nop, sizeof(nop)) != LIB_STATUS_OK ||
        core_machine_memory_write(machine, 0xfffffff2u, &nop, sizeof(nop)) != LIB_STATUS_OK ||
        core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 2u ||
        result.ticks != 6u || result.elapsed_ticks != 6u ||
        core_machine_capture_observation(machine, &observation) != LIB_STATUS_OK ||
        observation.elapsed_ticks != 6u ||
        core_machine_capture_time_observation(machine, &time_observation) !=
        LIB_STATUS_OK || time_observation.elapsed_ticks != 6u ||
        time_observation.next_deadline_tick != 0u ||
        time_observation.next_deadline_valid ||
        time_observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_IDLE) goto done;
    budget.instructions = 1u;
    budget.ticks = 28u;
    if (core_machine_run(machine, budget, &result) != LIB_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
        result.ticks != 3u || result.elapsed_ticks != 9u ||
        core_machine_reset(machine) != LIB_STATUS_OK ||
        core_machine_get_elapsed_ticks(machine, &elapsed) != LIB_STATUS_OK ||
        elapsed != 0u ||
        core_machine_capture_time_observation(machine, &time_observation) !=
        LIB_STATUS_OK || time_observation.elapsed_ticks != 0u ||
        time_observation.next_deadline_tick != 0u ||
        time_observation.next_deadline_valid ||
        time_observation.progress_disposition != CORE_MACHINE_TIME_PROGRESS_IDLE) goto done;
    failed = 0;
done:
    core_machine_destroy(machine);
    if (failed) return 1;
    printf("M5:T217:S2:TIME:OK\n");
    return 0;
}
