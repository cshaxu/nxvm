#include "type.h"



#include "vm/composition/session/session.h"

#include "vm/composition/session/lifecycle.h"

#include "vm/machine/fdd.h"

C_INT main(C_VOID)
{
    vm_session session = {0};
    const vm_session *machine;

    vm_session_initialize(&session);
    machine = (&session);
    if (machine == STD_NULL) {
        vm_session_finalize(&session);
        return 1;
    }
    vm_session_finalize(&session);
    puts("M5:T70:S1:P7:FDD-AUTHORITY:OK");
    return 0;
}
