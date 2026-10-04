#include "x86/ibmpc-common/machine_board_interface.h"
#include "lib/types/test.h"

/* The complete board links without any App, firmware or media inputs. */
static lib_i32 check_board(core_machine_keyboard_topology keyboard,
    x86_pit_personality pit)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .time_axis = {CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL, 1000000u},
        .shared_pit_personality = pit,
        .keyboard_topology = keyboard,
        .xt_ppi_keyboard = {0x60u, 0x61u, 0x62u, 0x63u, 1u, 0x0du, 0x02u}
    };
    core_machine *machine = LIB_NULL;
    core_machine_board_state *board = LIB_NULL;
    core_machine_plan *plan = LIB_NULL;
    lib_u32 value = 0u;
    lib_u8 scan_set = 0u;
    lib_i32 failed = 1;

    lib_test_assert(core_machine_plan_create(&config, &plan) == LIB_STATUS_OK);
    lib_test_assert(core_machine_create_from_plan(plan, &machine, &board) == LIB_STATUS_OK);
    lib_test_assert(machine != LIB_NULL && board != LIB_NULL);
    lib_test_assert(core_machine_freeze_execution_providers(machine) == LIB_STATUS_OK);
    lib_test_assert(core_machine_reset(machine) == LIB_STATUS_OK);
    lib_test_assert(core_machine_keyboard_get_native_scan_set(board, &scan_set) == LIB_STATUS_OK);
    lib_test_assert(scan_set == (keyboard == CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI ?
        CORE_MACHINE_KEYBOARD_SCAN_SET_1 : CORE_MACHINE_KEYBOARD_SCAN_SET_2));
    /* CR is not CE until the next input clock; pit_ports owns the exact count
     * assertion. This matrix proves board route installation and reset. */
    if (core_machine_bus_write(machine, 0x43u, 0x34u) != LIB_STATUS_OK ||
        core_machine_bus_write(machine, 0x40u, 18u) != LIB_STATUS_OK ||
        core_machine_bus_write(machine, 0x40u, 0u) != LIB_STATUS_OK ||
        core_machine_bus_write(machine, 0x43u, 0u) != LIB_STATUS_OK ||
        core_machine_bus_read(machine, 0x40u, &value) != LIB_STATUS_OK ||
        core_machine_bus_read(machine, 0x40u, &value) != LIB_STATUS_OK ||
        core_machine_reset(machine) != LIB_STATUS_OK) goto done;
    failed = 0;
done:
    core_machine_destroy(machine);
    core_machine_plan_destroy(plan);
    return failed;
}

lib_i32 main(void)
{
    return check_board(CORE_MACHINE_KEYBOARD_TOPOLOGY_8042, X86_PIT_PERSONALITY_8254) |
        check_board(CORE_MACHINE_KEYBOARD_TOPOLOGY_8042, X86_PIT_PERSONALITY_8253) |
        check_board(CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI, X86_PIT_PERSONALITY_8254) |
        check_board(CORE_MACHINE_KEYBOARD_TOPOLOGY_XT_PPI, X86_PIT_PERSONALITY_8253);
}
