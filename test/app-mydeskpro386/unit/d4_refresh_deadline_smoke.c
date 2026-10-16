#include "app-mydeskpro386/d4_platform_interface.h"
#include "app-mydeskpro386/d4_platform.h"
#include "lib/types/types_interface.h"
#include "lib/types/file.h"

#include "../../x86-nxvm/board-base/composition/composition_fixture.h"
#include "../../x86-nxvm/board-base/board_board_fixture.h"
#include "d4_refresh_fixture.h"

lib_i32 main(void)
{
    x86_nxvm_board_config config = {0};
    const mydeskpro386_d4_platform_config d4 = {X86_NXVM_BOARD_PC_AT_PORT_B, 0u, 2u, 18u};
    x86_core_machine_time_observation observation;
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_plan *plan = LIB_NULL;
    const x86_nxvm_board_plan_topology topology = {0};
    x86_nxvm_board_state *board = LIB_NULL;
    mydeskpro386_d4_platform *d4_board = LIB_NULL;
    lib_u8 advanced = LIB_FALSE;
    lib_i32 failed = 1;

    config.auxiliary_pit_present = LIB_TRUE;
    config.auxiliary_pit_base_port = 0x48u;
    if (x86_nxvm_board_plan_create(&config, &plan) != LIB_STATUS_OK ||
        x86_nxvm_board_plan_set_topology(plan, &topology) != LIB_STATUS_OK ||
        x86_nxvm_board_create_from_plan(plan, &machine, &board) != LIB_STATUS_OK ||
        machine == LIB_NULL || board == LIB_NULL ||
        mydeskpro386_d4_platform_attach(board, &d4, &d4_board) != LIB_STATUS_OK ||
        d4_board == LIB_NULL || test_x86_nxvm_machine_fixture_register_reset_mapping(machine,
        0xfffffff0u, 0x000ffff0u, 16u) != LIB_STATUS_OK ||
        x86_core_machine_freeze_execution_providers(machine) != LIB_STATUS_OK ||
        x86_core_machine_reset(machine) != LIB_STATUS_OK) goto done;
    test_model40_refresh_set_pending(d4_board);
    /* Real PIT load at tick 1, terminal count at tick 4. The D4 refresh
     * request must complete first without dropping this later source. */
    if (x86_core_machine_bus_write(machine, 0x43u, 0x30u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x40u, 3u) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x40u, 0u) != LIB_STATUS_OK ||
        x86_core_machine_capture_time_observation(machine, &observation) !=
        LIB_STATUS_OK || !observation.next_deadline_valid ||
        observation.next_deadline_tick != 1u ||
        observation.progress_disposition != X86_CORE_MACHINE_TIME_PROGRESS_DEADLINE ||
        x86_core_machine_advance_to_next_deadline(machine, &advanced) !=
        LIB_STATUS_OK || !advanced || test_model40_refresh_is_pending(d4_board) ||
        test_x86_nxvm_elapsed_ticks(machine) != 1u ||
        x86_pit_get_output(d4_board->shared_pit, 0u) ||
        x86_core_machine_capture_time_observation(machine, &observation) !=
        LIB_STATUS_OK || !observation.next_deadline_valid ||
        observation.next_deadline_tick != 4u ||
        x86_core_machine_advance_to_next_deadline(machine, &advanced) !=
        LIB_STATUS_OK || !advanced || test_x86_nxvm_elapsed_ticks(machine) != 4u ||
        !x86_pit_get_output(d4_board->shared_pit, 0u)) goto done;
    failed = 0;
done:
    x86_core_machine_destroy(machine);
    x86_nxvm_board_plan_destroy(plan);
    if (failed) return 1;
    lib_c_printf("D4-REFRESH-DEADLINE:OK\n");
    return 0;
}
