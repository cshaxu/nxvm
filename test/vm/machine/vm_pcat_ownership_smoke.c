#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/machine.h"
#include "core/machine/port.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_private.h"

C_INT main(C_VOID)
{
    vm_session *session = (vm_session *)STD_CALLOC(1u, sizeof(*session));
    t_port *port;
    C_INT masked = TYPE_FALSE;
    C_INT failed = 0;

    if (session == STD_NULL) return 1;
    vm_session_initialize(session);
    port = session->core_machine->fdc.connect.port;
    if (!session->active || session->core_machine == STD_NULL ||
        port == STD_NULL) {
        failed = 1;
    }
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
    vm_session_finalize(session);
    STD_FREE(session);
    if (failed) return 1;
    puts("M5:T264:S3:PCAT-OWNERSHIP:OK");
    return 0;
}
