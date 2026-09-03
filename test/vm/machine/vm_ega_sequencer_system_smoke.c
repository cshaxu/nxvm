#include "type.h"

#include "core/machine/machine_interface.h"
#include "core/machine/debug_interface.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_interface.h"
#include "../support/rom/session_assets.h"
#include "vm/composition/session/session_private.h"

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    type_unsigned_8 aperture_value = 0x5au;
    type_unsigned_8 read_value = 0u;
    type_unsigned_32 port_value = 0u;
    C_INT failed = 0;

    if (vm_test_default_pc_at_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        session == STD_NULL || !session->active || session->core_machine == STD_NULL) {
        return 1;
    }
    vm_session_reset(session);
    failed |= core_machine_debug_read_port(session->core_machine, 0x03c4u, &port_value) != TYPE_STATUS_OK || port_value != 0u;
    failed |= core_machine_debug_read_port(session->core_machine, 0x03c5u, &port_value) != TYPE_STATUS_OK || port_value != 0x03u;
    failed |= core_machine_debug_write_port(session->core_machine, 0x03c4u, 4u) != TYPE_STATUS_OK || core_machine_debug_write_port(session->core_machine, 0x03c5u, 0xffu) != TYPE_STATUS_OK;
    failed |= core_machine_debug_read_port(session->core_machine, 0x03c5u, &port_value) != TYPE_STATUS_OK || port_value != 0x0eu;
    failed |= core_machine_memory_write(session->core_machine,
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, &aperture_value,
        sizeof(aperture_value)) != TYPE_STATUS_OK;
    failed |= core_machine_memory_read(session->core_machine,
        CORE_MACHINE_VADP_EGA_APERTURE_BASE, &read_value,
        sizeof(read_value)) != TYPE_STATUS_OK || read_value != aperture_value;

    vm_session_destroy(session);
    if (failed) return 1;
    STD_PRINTF("M5:T235:S3:EGA-SEQUENCER:SYSTEM:OK\n");
    return 0;
}
