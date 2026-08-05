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
        machine->fdc.connect.dma_request.core_owner == STD_NULL ||
        machine->fdc.connect.dma_request.channel != 2u ||
        machine->fdc.connect.irq_source.master == STD_NULL ||
        machine->fdc.connect.irq_source.slave == STD_NULL ||
        machine->fdc.connect.port == STD_NULL) {
        vm_session_finalize(session);
        STD_FREE(session);
        return 1;
    }
    vm_session_finalize(session);
    STD_FREE(session);
    puts("M5:T230:S3:FDC-DMA-BINDING:OK");
    return 0;
}
