#include "type.h"




#include "vm/composition/session/session.h"

#include "vm/composition/session/lifecycle.h"

#include "vm/machine/fdc.h"

C_INT main(C_VOID)
{
    vm_session *session;
    const vm_session *machine;

    session = (vm_session *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return 1;
    vm_session_initialize(session);
    machine = session;
    if (machine == STD_NULL ||
        machine->fdc.connect.fdd != &machine->fdd ||
        machine->fdc.connect.dma_latch == STD_NULL ||
        machine->fdc.connect.dma_primary == STD_NULL ||
        machine->fdc.connect.dma_secondary == STD_NULL ||
        machine->fdc.connect.pic_master == STD_NULL ||
        machine->fdc.connect.pic_slave == STD_NULL ||
        machine->fdc.connect.port == STD_NULL) {
        vm_session_finalize(session);
        STD_FREE(session);
        return 1;
    }
    vm_session_finalize(session);
    STD_FREE(session);
    puts("M5:T70:S1:P5:FDC-CONNECT:OK");
    return 0;
}
