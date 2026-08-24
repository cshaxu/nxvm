#include "type.h"



#include "vm/composition/session/session_interface.h"
#include "vm/composition/session/session_private.h"

#include "vm/composition/session/lifecycle.h"

#include "core/machine/rtc.h"

C_INT main(C_VOID)
{
    vm_session *session = STD_NULL;
    const vm_session *machine;

    if (vm_session_create(STD_NULL, &session) != TYPE_STATUS_OK) return 1;
    machine = (session);
    if (machine == STD_NULL) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_destroy(session);
    puts("M5:T70:S1:CMOS-AUTHORITY:OK");
    return 0;
}
