#include "type.h"

#include "core/machine/machine_interface.h"
#include "vm/composition/session/session.h"
#include "tests/support/vm_session_fixture.h"

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    vm_session *identity;
    core_machine *core_identity;
    t_fdd *fdd_identity;
    t_hdd *hdd_identity;
    t_debug *debug_identity;
    vm_platform_run_context *platform_identity;
    vm_session_reset_vector vector;
    STD_SIZE_T bytes = 0u;
    C_INT failed = 0;

    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK) return 1;
    identity = session;
    core_identity = vm_session_fixture_machine(session);
    fdd_identity = vm_session_fixture_fdd(session);
    hdd_identity = vm_session_fixture_hdd(session);
    debug_identity = vm_session_fixture_debug(session);
    platform_identity = vm_session_fixture_platform_run_context(session);
    failed |= vm_session_reconfigure_memory(session, 32u * 1024u * 1024u) !=
        TYPE_STATUS_OK;
    failed |= session != identity;
    failed |= vm_session_fixture_machine(session) != core_identity || vm_session_fixture_fdd(session) != fdd_identity ||
        vm_session_fixture_hdd(session) != hdd_identity || vm_session_fixture_debug(session) != debug_identity ||
        vm_session_fixture_platform_run_context(session) != platform_identity;
    failed |= core_machine_get_memory_bytes(vm_session_fixture_machine(session), &bytes) !=
        TYPE_STATUS_OK || bytes != 32u * 1024u * 1024u;
    failed |= vm_session_get_reset_vector(session, &vector) != TYPE_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u;
    vm_session_destroy(session);
    if (failed) return 1;
    puts("M5:T173:S1:SESSION-RECONFIGURE:OK");
    return 0;
}
