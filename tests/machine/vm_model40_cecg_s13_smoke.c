#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session.h"
#include "vm/composition/session/session_interface.h"

C_INT main(C_VOID)
{
    static type_unsigned_8 even[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    static type_unsigned_8 odd[VM_PROFILE_MODEL40_ROM_CHIP_BYTES];
    vm_profile_model40_external_rom external_rom = { even, odd, sizeof(even) };
    vm_session *session = STD_NULL;
    t_port *port;
    C_INT failed = 0;

    failed |= vm_session_create_model40_private(&external_rom, &session) !=
        TYPE_STATUS_OK || session == STD_NULL;
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
        vm_session_reset(session);
        port = &session->core_machine->executor_port;
        failed |= core_machine_port_read(port,
            CORE_MACHINE_VADP_PORT_COMPAQ_MISCELLANEOUS_OUTPUT) != 0xe0u;
    }
    vm_session_destroy(session);
    if (!failed) {
        STD_PRINTF("M5:T386:S13:MODEL40-INPUT-STATUS-0:OK\n");
        return 0;
    }
    STD_FPRINTF(STD_STDERR, "M5:T386:S13:MODEL40-INPUT-STATUS-0:FAIL\n");
    return 1;
}
