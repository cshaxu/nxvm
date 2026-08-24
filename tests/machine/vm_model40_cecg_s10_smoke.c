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

    failed |= vm_model40_fixture_create("t386-s10-even.bin", "t386-s10-odd.bin", &session) !=
        TYPE_STATUS_OK || session == STD_NULL;
    if (!failed) {
        core_machine_port_write(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_COMPAQ_FEATURE_CONTROL, 0x03u);
        failed |= core_machine_port_read(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT) != 0x03u;
    }
    if (!failed) {
        vm_session_reset(session);
        failed |= core_machine_port_read(&session->core_machine->executor_port,
            CORE_MACHINE_VADP_PORT_COMPAQ_ENVIRONMENT) != 0x00u;
    }
    vm_session_destroy(session);
    vm_model40_fixture_remove("t386-s10-even.bin", "t386-s10-odd.bin");
    if (!failed) {
        STD_PRINTF("M5:T386:S10:MODEL40-FEATURE-ENVIRONMENT:OK\n");
        return 0;
    }
    STD_FPRINTF(STD_STDERR, "M5:T386:S10:MODEL40-FEATURE-ENVIRONMENT:FAIL\n");
    return 1;
}
