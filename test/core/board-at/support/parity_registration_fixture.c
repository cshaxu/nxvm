#include "parity_registration_fixture.h"
#include "x86-core/machine/port_interface.h"

static lib_bool reject_ports;
static lib_bool rejection_reached;

static lib_status parity_test_ports(x86_core_machine *machine,
    const x86_core_machine_port_route *routes, lib_size count)
{
    if (reject_ports) {
        rejection_reached = count == 1u;
        return rejection_reached ? LIB_STATUS_NO_MEMORY : LIB_STATUS_INTERNAL_ERROR;
    }
    return x86_core_machine_install_port_routes(machine, routes, count);
}

/* Exercise the actual AT parity cleanup after its memory registration. */
#define x86_core_machine_install_port_routes parity_test_ports
#include "core/board-at/parity.c"
#undef x86_core_machine_install_port_routes

lib_status test_at_parity_try(core_machine_board_state *board,
    const core_machine_planar_parity_config *config, lib_bool reject)
{
    reject_ports = reject;
    rejection_reached = LIB_FALSE;
    const lib_status status = core_machine_configure_planar_parity(board, config);
    reject_ports = LIB_FALSE;
    return reject && !rejection_reached ? LIB_STATUS_INTERNAL_ERROR : status;
}
