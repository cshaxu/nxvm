#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/machine.h"
#include "core/machine/port.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_private.h"
#include "../support/rom/session_assets.h"

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    t_port *port;
    C_INT masked = TYPE_FALSE;
    C_INT failed = 0;

    if (vm_test_default_pc_at_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        session == STD_NULL || !session->active || session->core_machine == STD_NULL ||
        (port = session->core_machine->fdc.connect.port) == STD_NULL) return 1;
    core_machine_port_write(port, 0x0070u, 0x80u);
    if (core_machine_get_nmi_mask(session->core_machine, &masked) != TYPE_STATUS_OK ||
        !masked) {
        failed = 1;
    }
    core_machine_port_write(port, 0x0070u, 0u);
    if (core_machine_get_nmi_mask(session->core_machine, &masked) != TYPE_STATUS_OK ||
        masked) {
        failed = 1;
    }
    vm_session_destroy(session);
    if (failed) return 1;
    puts("M5:T264:S3:PCAT-OWNERSHIP:OK");
    return 0;
}
