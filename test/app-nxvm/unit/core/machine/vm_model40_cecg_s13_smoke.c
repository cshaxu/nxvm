#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/port.h"
#include "app-nxvm/devices/vadp.h"
#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/machine/machine_private.h"
#include "app-nxvm/machine/machine_interface.h"
#include "support/rom/model40_session_assets.h"

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    t_port *port;
    lib_i32 failed = 0;

    failed |= vm_model40_fixture_create(&session) !=
        LIB_STATUS_OK || session == LIB_NULL;
    if (!failed) {
        port = &session->core_machine->executor_port;
        failed |= core_machine_port_read(port,
            CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT) != 0xe0u;
        core_machine_port_write(port, CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT,
            0x00u);
        failed |= core_machine_port_read(port,
            CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT) != 0xf0u;
    }
    if (!failed) {
        vm_machine_reset(session);
        port = &session->core_machine->executor_port;
        failed |= core_machine_port_read(port,
            CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT) != 0xe0u;
    }
    vm_machine_destroy(session);
    if (!failed) {
        printf("M5:T386:S13:MODEL40-INPUT-STATUS-0:OK\n");
        return 0;
    }
    fprintf(stderr, "M5:T386:S13:MODEL40-INPUT-STATUS-0:FAIL\n");
    return 1;
}
