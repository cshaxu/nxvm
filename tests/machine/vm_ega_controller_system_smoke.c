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
    uint8_t value = 0x5au;
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
    core_machine_port_write(port, 0x03ceu, 6u);
    failed |= core_machine_port_read(port, 0x03cfu) != 0x05u;
    core_machine_port_write(port, 0x03cfu, 0x09u);
    failed |= core_machine_port_read(port, 0x03cfu) != 0x09u;
    failed |= core_machine_memory_write(session->core_machine, 0x000b0000u,
        &value, sizeof(value)) != TYPE_STATUS_OK;
    failed |= core_machine_memory_read(session->core_machine, 0x000b0000u,
        &read_value, sizeof(read_value)) != TYPE_STATUS_OK || read_value != value;
    core_machine_port_write(port, 0x03c0u, 0x32u);
    core_machine_port_write(port, 0x03c0u, 0xffu);
    failed |= core_machine_port_read(port, 0x03c1u) != 0x0fu;
    (C_VOID)core_machine_port_read(port, 0x03dau);
    core_machine_port_write(port, 0x03c0u, 0x10u);
    failed |= core_machine_port_read(port, 0x03c1u) != 0x01u;

done:
    vm_session_destroy(session);
    if (failed) return 1;
    STD_PRINTF("M5:T236:S3:EGA-CONTROLLER:SYSTEM:OK\n");
    return 0;
}
