#include "absent_memory_fixture.h"
#include "x86-nxvm/board-base/machine_board_state.h"

static lib_size *registration_attempts;

static lib_status absent_test_install_routes(x86_core_machine *machine,
    const x86_core_machine_memory_device_route *routes, lib_size count,
    x86_core_machine_memory_write_observer observer,
    const x86_core_machine_memory_parity_config *parity, void *owner)
{
    if (registration_attempts != LIB_NULL) {
        ++*registration_attempts;
        return LIB_STATUS_NO_MEMORY;
    }
    return x86_core_machine_install_memory_device_routes(machine, routes, count,
        observer, parity, owner);
}

/* Exercise Board's real publication/rollback at its public Machine boundary. */
#define x86_core_machine_install_memory_device_routes absent_test_install_routes
#include "x86-nxvm/board-base/machine_board.c"
#undef x86_core_machine_install_memory_device_routes

lib_status test_board_absent_memory_registration_failure(x86_nxvm_board_state *board,
    const x86_nxvm_board_absent_memory_config *config, lib_size *out_attempts)
{
    lib_status status;

    *out_attempts = 0u;
    registration_attempts = out_attempts;
    status = x86_nxvm_board_configure_absent_memory(board, config);
    registration_attempts = LIB_NULL;
    return status;
}

lib_bool test_board_absent_memory_is_configured(x86_nxvm_board_state *board)
{
    return board->absent_memory[0].configured;
}
