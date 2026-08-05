#include "type.h"
#include "tests/support/vm_session_fixture.h"




#include "vm/composition/session/session.h"

#include "vm/composition/session/lifecycle.h"

#include "vm/profile/default_profile/firmware/bios.h"

C_INT main(C_VOID)
{
    vm_session *session;
    const vm_session *machine;

    session = vm_session_fixture_allocate();
    if (session == STD_NULL) return 1;
    vm_session_initialize(session);
    machine = session;
    if (machine == STD_NULL) {
        vm_session_finalize(session);
        vm_session_fixture_free(session);
        return 1;
    }
    vm_session_finalize(session);
    vm_session_fixture_free(session);
    puts("M5:T38:S1:BIOS-AUTHORITY:OK");
    return 0;
}
