#include "type.h"

#include "core/machine/debug_interface.h"
#include "core/machine/machine_interface.h"
#include "core/machine/port.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session.h"

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    t_port *port;
    uint8_t aperture_value = 0x5au;
    uint8_t read_value = 0u;
    C_INT failed = 0;

    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        session == STD_NULL || !session->active || session->core_machine == STD_NULL) {
        return 1;
    }
    port = core_machine_debug_port_borrow(session->core_machine);
    if (port == STD_NULL) {
        failed = 1;
        goto done;
    }
    failed |= core_machine_port_read(port, 0x03c4u) != 0u;
    failed |= core_machine_port_read(port, 0x03c5u) != 0x03u;
    core_machine_port_write(port, 0x03c4u, 4u);
    core_machine_port_write(port, 0x03c5u, 0xffu);
    failed |= core_machine_port_read(port, 0x03c5u) != 0x0eu;
    failed |= core_machine_memory_write(session->core_machine,
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, &aperture_value,
        sizeof(aperture_value)) != TYPE_STATUS_OK;
    failed |= core_machine_memory_read(session->core_machine,
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, &read_value,
        sizeof(read_value)) != TYPE_STATUS_OK || read_value != aperture_value;

done:
    vm_session_destroy(session);
    if (failed) return 1;
    STD_PRINTF("M5:T235:S3:EGA-SEQUENCER:SYSTEM:OK\n");
    return 0;
}
