#include "type.h"

#include "core/machine/machine.h"
#include "core/machine/port.h"
#include "core/machine/vadp.h"
#include "vm/composition/session/lifecycle.h"
#include "vm/composition/session/session_private.h"
#include "vm/composition/session/session_interface.h"
#include "../support/rom/model40_session_assets.h"

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    core_machine_display_snapshot snapshot;
    static const type_unsigned_8 text[] = { 'O', 0x07u, 'K', 0x07u };
    C_INT failed = 0;

    failed |= vm_model40_fixture_create(&session) !=
        TYPE_STATUS_OK || session == STD_NULL;
    if (!failed) {
        failed |= core_machine_memory_write(session->core_machine,
            CORE_MACHINE_VADP_TEXT_BASE, text, sizeof(text)) != TYPE_STATUS_OK ||
            core_machine_capture_display_snapshot(session->core_machine, &snapshot) !=
                TYPE_STATUS_OK || snapshot.kind != CORE_MACHINE_DISPLAY_KIND_TEXT ||
            snapshot.characters[0] != 'O' || snapshot.characters[1] != 'K';
    }
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
    if (!failed) {
        STD_PRINTF("M5:T386:S10:MODEL40-FEATURE-ENVIRONMENT:OK\n");
        return 0;
    }
    STD_FPRINTF(STD_STDERR, "M5:T386:S10:MODEL40-FEATURE-ENVIRONMENT:FAIL\n");
    return 1;
}
