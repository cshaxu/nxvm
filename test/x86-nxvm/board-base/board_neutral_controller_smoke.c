#include "lib/types/test.h"
#include "x86-nxvm/board-base/machine_board_interface.h"
#include "x86-nxvm/board-base/machine_board_state.h"

int main(void)
{
    const x86_nxvm_board_config configuration = {
        .memory_bytes = 2u * 1024u * 1024u,
        .time_axis = {X86_CORE_MACHINE_TIME_AXIS_MACRO_PROPORTIONAL, 1000000u},
        .keyboard_topology = X86_NXVM_BOARD_KEYBOARD_TOPOLOGY_NONE,
        .display_topology = X86_NXVM_BOARD_DISPLAY_TOPOLOGY_NONE,
        .system_facade = X86_NXVM_BOARD_SYSTEM_FACADE_NONE
    };
    x86_core_machine *machine = LIB_NULL;
    x86_nxvm_board_state *board = LIB_NULL;
    x86_nxvm_board_plan *plan = LIB_NULL;
    lib_u32 value = 0u;
    lib_u8 scan_set = 0u;
    x86_core_machine_attachment_deadline_observation deadline = {0};
    lib_i32 failed = 0;

    failed |= x86_nxvm_board_plan_create(&configuration, &plan) != LIB_STATUS_OK;
    failed |= !failed && x86_nxvm_board_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_OK;
    failed |= !failed && x86_core_machine_freeze_execution_providers(machine) !=
        LIB_STATUS_OK;
    failed |= !failed && x86_core_machine_reset(machine) != LIB_STATUS_OK;
    /* Standard controller routes remain; forced PC/AT endpoint routes do not. */
    failed |= !failed && (x86_core_machine_bus_read(machine, 0x20u, &value) !=
        LIB_STATUS_OK || x86_core_machine_bus_read(machine, 0x40u, &value) !=
        LIB_STATUS_OK || x86_core_machine_bus_read(machine, 0x60u, &value) !=
        LIB_STATUS_UNSUPPORTED || x86_core_machine_bus_read(machine, 0x92u, &value) !=
        LIB_STATUS_UNSUPPORTED || x86_core_machine_bus_read(machine, 0x3dau, &value) !=
        LIB_STATUS_UNSUPPORTED);
    failed |= !failed && x86_nxvm_board_keyboard_get_native_scan_set(board, &scan_set) !=
        LIB_STATUS_UNSUPPORTED;
    /* A neutral board has no VADP.  Its ordinary time callbacks must still
     * serve the controllers it does own without probing a PC display. */
    if (!failed) {
        /* PIT wake edges are board-owned even when a profile has no VADP. */
        failed |= x86_core_machine_bus_write(machine, 0x43u, 0x30u) !=
                LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x40u, 3u) != LIB_STATUS_OK ||
            x86_core_machine_bus_write(machine, 0x40u, 0u) != LIB_STATUS_OK;
        x86_nxvm_board_deadline_observe(board, 0u, &deadline);
        failed |= deadline.source_ticks == 0u;
        x86_nxvm_board_after_internal_advance(board, 1u, 1u);
    }

    x86_core_machine_destroy(machine);
    x86_nxvm_board_plan_destroy(plan);

    /* Explicit system control does not require a PC/AT facade or display. */
    x86_nxvm_board_config present = configuration;
    present.system_control_port_present = LIB_TRUE;
    machine = LIB_NULL;
    plan = LIB_NULL;
    failed |= !failed && x86_nxvm_board_plan_create(&present, &plan) != LIB_STATUS_OK;
    failed |= !failed && x86_nxvm_board_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_OK;
    failed |= !failed && (x86_core_machine_freeze_execution_providers(machine) !=
        LIB_STATUS_OK || x86_core_machine_reset(machine) != LIB_STATUS_OK ||
        x86_core_machine_bus_write(machine, 0x92u, 2u) != LIB_STATUS_OK ||
        x86_core_machine_bus_read(machine, 0x92u, &value) != LIB_STATUS_OK || value != 2u ||
        x86_core_machine_bus_read(machine, 0x3dau, &value) != LIB_STATUS_UNSUPPORTED);
    x86_core_machine_destroy(machine);
    x86_nxvm_board_plan_destroy(plan);
    plan = LIB_NULL;
    machine = LIB_NULL;
    board = LIB_NULL;
    present.system_control_port_present = 2u;
    lib_test_assert(!failed);
    failed |= !failed && x86_nxvm_board_plan_create(&present, &plan) != LIB_STATUS_OK;
    failed |= !failed && (x86_nxvm_board_create_from_plan(plan, &machine, &board) !=
        LIB_STATUS_INVALID_ARGUMENT || machine != LIB_NULL || board != LIB_NULL);
    x86_core_machine_destroy(machine);
    x86_nxvm_board_plan_destroy(plan);
    return failed;
}
