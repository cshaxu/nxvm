#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/debug_interface.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    type_unsigned_8 value = 0x5au;
    type_unsigned_8 read_value = 0u;
    type_unsigned_32 port_value = 0u;
    C_INT failed = 0;

    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        session == STD_NULL || !session->active || session->core_machine == STD_NULL) {
        return 1;
    }
    vm_session_reset(session);
    failed |= core_machine_debug_write_port(session->core_machine, 0x03ceu, 6u) != TYPE_STATUS_OK;
    failed |= core_machine_debug_read_port(session->core_machine, 0x03cfu, &port_value) != TYPE_STATUS_OK || port_value != 0x05u;
    failed |= core_machine_debug_write_port(session->core_machine, 0x03cfu, 0x09u) != TYPE_STATUS_OK;
    failed |= core_machine_debug_read_port(session->core_machine, 0x03cfu, &port_value) != TYPE_STATUS_OK || port_value != 0x09u;
    failed |= core_machine_memory_write(session->core_machine, 0x000b0000u,
        &value, sizeof(value)) != TYPE_STATUS_OK;
    failed |= core_machine_memory_read(session->core_machine, 0x000b0000u,
        &read_value, sizeof(read_value)) != TYPE_STATUS_OK || read_value != value;
    failed |= core_machine_debug_write_port(session->core_machine, 0x03c0u, 0x32u) != TYPE_STATUS_OK || core_machine_debug_write_port(session->core_machine, 0x03c0u, 0xffu) != TYPE_STATUS_OK;
    failed |= core_machine_debug_read_port(session->core_machine, 0x03c1u, &port_value) != TYPE_STATUS_OK || port_value != 0x0fu;
    (C_VOID)core_machine_debug_read_port(session->core_machine, 0x03dau, &port_value);
    failed |= core_machine_debug_write_port(session->core_machine, 0x03c0u, 0x10u) != TYPE_STATUS_OK || core_machine_debug_read_port(session->core_machine, 0x03c1u, &port_value) != TYPE_STATUS_OK || port_value != 0x01u;
    vm_session_destroy(session);
    if (failed) return 1;
    STD_PRINTF("M5:T236:S3:EGA-CONTROLLER:SYSTEM:OK\n");
    STD_PRINTF("M5:T480:S3:REGRESSIONS:OK\n");
    return 0;
}
