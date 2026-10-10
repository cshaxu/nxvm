#include "app-mydeskpro386/profiles/d4_platform_interface.h"
#include "lib/types/types_interface.h"
#include <stdio.h>

#include "../../../core/board-base/composition/composition_fixture.h"
#include "../../../core/board-base/composition/time_fixture.h"
#include "../../../core/board-base/core_machine_board_fixture.h"
#include "d4_refresh_fixture.h"

lib_i32 main(void)
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
    if (failed) return 1;
    printf("D4-REFRESH-DEADLINE:OK\n");
    return 0;
}
