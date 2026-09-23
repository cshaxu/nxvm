#include "type.h"

#include "app-nxvm/devices/machine.h"
#include "app-nxvm/devices/port.h"
#include "app-nxvm/devices/vadp.h"
#include "app-nxvm/machine/lifecycle.h"
#include "app-nxvm/machine/machine_private.h"
#include "app-nxvm/machine/machine_interface.h"
#include "support/rom/model40_session_assets.h"

C_INT main(C_VOID)
{
    vm_machine *session = STD_NULL;
    C_INT failed = 0;

    failed |= vm_model40_fixture_create(&session) !=
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
            CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE) != 0x7fu ||
            (core_machine_port_read(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_STATUS) & 0x06u) != 0x06u;
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE, 0xa5u);
        failed |= core_machine_port_read(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE) != 0xa5u;
    }
    if (!failed) {
        vm_machine_reset(session);
        failed |= core_machine_port_read(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_COMPAQ_CONTROL_MODE) != 0x40u ||
            (core_machine_port_read(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_STATUS) & 0x06u) != 0x04u;
    }
    vm_machine_destroy(session);
    if (!failed) {
        STD_PRINTF("M5:T386:S9:MODEL40-CECG:OK\n");
        return 0;
    }
    STD_FPRINTF(STD_STDERR, "M5:T386:S9:MODEL40-CECG:FAIL\n");
    return 1;
}
