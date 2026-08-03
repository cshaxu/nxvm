#include "type.h"



#include "vm/composition/session.h"

#include "vm/composition/session_lifecycle.h"

#include "vm/machine/cmos.h"

C_INT main(C_VOID)
{
    vm_session session = {0};
    const vm_session *machine;

    vm_session_initialize(&session);
    machine = (&session);
    if (machine == STD_NULL || machine->cmos != &session.cmos_storage) {
        vm_session_finalize(&session);
        return 1;
    }
    vm_session_finalize(&session);
    puts("M5:T70:S1:CMOS-AUTHORITY:OK");
    return 0;
}
