#include "app-mydeskpro386/d4_platform_interface.h"
#include "../../x86-nxvm/board-base/composition/port_assembly_fixture.h"
#include "lib/types/file.h"

static lib_bool reject_ports;
static lib_bool rejection_reached;

static lib_status d4_test_ports(x86_core_machine *machine,
    const x86_core_machine_port_route *routes, lib_size count)
{
    if (reject_ports) {
        rejection_reached = count == 1u;
        return rejection_reached ? LIB_STATUS_NO_MEMORY : LIB_STATUS_INTERNAL_ERROR;
    }
    return x86_core_machine_install_port_routes(machine, routes, count);
}

#define x86_core_machine_install_port_routes d4_test_ports
#include "app-mydeskpro386/d4_platform.c"
#undef x86_core_machine_install_port_routes

static lib_status port_assembly_attach_d4(x86_nxvm_board_state *board, lib_bool reject)
{
    const mydeskpro386_d4_platform_config config = {X86_NXVM_BOARD_PC_AT_PORT_B, 0u, 2u, 18u};
    mydeskpro386_d4_platform *platform = LIB_NULL;
    reject_ports = reject;
    rejection_reached = LIB_FALSE;
    const lib_status status = mydeskpro386_d4_platform_attach(board, &config, &platform);
    reject_ports = LIB_FALSE;
    return reject && !rejection_reached ? LIB_STATUS_INTERNAL_ERROR : status;
}

lib_i32 main(void)
{
    const lib_i32 failed = port_assembly_port_b_transaction(port_assembly_attach_d4);

    if (failed) return 1;
    lib_c_printf("D4-PORT-B-ROLLBACK:OK\n");
    return 0;
}
