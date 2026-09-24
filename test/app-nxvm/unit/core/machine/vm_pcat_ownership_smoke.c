#include "lib/types/types_interface.h"
#include <stdio.h>

#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/port.h"
#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/machine/machine_private.h"
#include "support/rom/session_assets.h"

lib_i32 main(void)
{
    vm_machine *session = LIB_NULL;
    t_port *port;
    lib_i32 masked = LIB_FALSE;
    lib_i32 failed = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK ||
        session == LIB_NULL || !session->active || session->core_machine == LIB_NULL ||
        (port = session->core_machine->fdc.connect.port) == LIB_NULL) return 1;
    core_machine_port_write(port, 0x0070u, 0x80u);
    if (core_machine_get_nmi_mask(session->core_machine, &masked) != LIB_STATUS_OK ||
        !masked) {
        failed = 1;
    }
    core_machine_port_write(port, 0x0070u, 0u);
    if (core_machine_get_nmi_mask(session->core_machine, &masked) != LIB_STATUS_OK ||
        masked) {
        failed = 1;
    }
    vm_machine_destroy(session);
    if (failed) return 1;
    puts("M5:T264:S3:PCAT-OWNERSHIP:OK");
    return 0;
}
