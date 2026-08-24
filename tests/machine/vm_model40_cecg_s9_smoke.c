#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/session_interface.h"
#include "../support/vm_model40_byob_fixture.h"

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    C_INT failed = 0;

    failed |= vm_model40_fixture_create("t386-s9-even.bin", "t386-s9-odd.bin", &session) !=
        TYPE_STATUS_OK || session == STD_NULL;
    if (!failed) {
        failed |= core_machine_port_read(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE) != 0x40u ||
            !core_machine_port_has_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_COMPAQ_LIGHTPEN_LATCH_SET);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE, 0x7fu);
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_COMPAQ_LIGHTPEN_LATCH_SET, 0u);
        failed |= core_machine_port_read(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE) != 0x5fu ||
            (core_machine_port_read(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_STATUS) & 0x06u) != 0x06u;
    }
    if (!failed) {
        vm_session_reset(session);
        failed |= core_machine_port_read(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE) != 0x40u ||
            (core_machine_port_read(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_STATUS) & 0x06u) != 0x04u;
    }
    vm_session_destroy(session);
    vm_model40_fixture_remove("t386-s9-even.bin", "t386-s9-odd.bin");
    if (!failed) {
        STD_PRINTF("M5:T386:S9:MODEL40-CECG:OK\n");
        return 0;
    }
    STD_FPRINTF(STD_STDERR, "M5:T386:S9:MODEL40-CECG:FAIL\n");
    return 1;
}
