#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/machine/runtime/machine_interface.h"
#include "../support/rom/session_assets.h"
#include "vm/machine/runtime/machine_private.h"

C_INT main(C_VOID)
{
    vm_machine *session = STD_NULL;
    vm_machine *identity;
    core_machine *core_identity;
    t_fdd *fdd_identity;
    t_hdd *hdd_identity;
    t_debug *debug_identity;
    vm_machine_reset_vector vector;
    STD_SIZE_T bytes = 0u;
    C_INT failed = 0;

    if (vm_test_default_pc_at_session_create(STD_NULL, &session) != TYPE_STATUS_OK) return 1;
    identity = session;
    core_identity = session->core_machine;
    fdd_identity = &session->fdd;
    hdd_identity = &session->hdd;
    debug_identity = &session->debug;
    failed |= vm_machine_reconfigure_memory(session, 32u * 1024u * 1024u) !=
        TYPE_STATUS_OK;
    failed |= session != identity;
    failed |= session->core_machine != core_identity || &session->fdd != fdd_identity ||
        &session->hdd != hdd_identity || &session->debug != debug_identity;
    failed |= core_machine_get_memory_bytes(session->core_machine, &bytes) !=
        TYPE_STATUS_OK || bytes != 32u * 1024u * 1024u;
    failed |= vm_machine_get_reset_vector(session, &vector) != TYPE_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u;
    vm_machine_destroy(session);
    if (failed) return 1;
    puts("M5:T173:S1:SESSION-RECONFIGURE:OK");
    return 0;
}
