#include "type.h"
#include "tests/support/vm_session_fixture.h"




#include "vm/composition/session/session.h"

#include "vm/composition/session/lifecycle.h"

#include "vm/machine/debug.h"

C_INT main(C_VOID)
{
    vm_session *session;
    vm_session *machine;

    session = vm_session_fixture_allocate();
    if (session == STD_NULL) return 1;
    vm_session_initialize(session);
    machine = session;
    if (machine == STD_NULL) {
        vm_session_finalize(session);
        vm_session_fixture_free(session);
        return 1;
    }
    vm_machine_debug_set_trace(&vm_session_fixture_debug(machine), 2u);
    if (!vm_session_fixture_debug(machine).data.flagTrace ||
        vm_session_fixture_debug(machine).data.traceCount != 2u) {
        vm_session_finalize(session);
        vm_session_fixture_free(session);
        return 1;
    }
    vm_machine_debug_clear_trace(&vm_session_fixture_debug(machine));
    vm_session_finalize(session);
    vm_session_fixture_free(session);
    puts("M5:T43:S1:DEBUG-AUTHORITY:OK");
    return 0;
}
