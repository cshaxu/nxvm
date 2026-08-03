#include "type.h"




#include "vm/composition/session.h"

#include "vm/composition/session_lifecycle.h"

#include "vm/machine/fdc.h"

C_INT main(C_VOID)
{
    vm_session *session;
    const vm_session *machine;

    session = (vm_session *)STD_CALLOC(1u, sizeof(*session));
    if (session == STD_NULL) return 1;
    vm_session_initialize(session);
    machine = session;
    if (machine == STD_NULL || machine->fdc == STD_NULL ||
        machine->fdc->connect.fdd != machine->fdd ||
        machine->fdc->connect.dma_latch !=
            vm_composition_machine_access_dma_latch(machine->core_access) ||
        machine->fdc->connect.dma_primary !=
            vm_composition_machine_access_dma_primary(machine->core_access) ||
        machine->fdc->connect.dma_secondary !=
            vm_composition_machine_access_dma_secondary(machine->core_access) ||
        machine->fdc->connect.pic_master !=
            vm_composition_machine_access_pic_master(machine->core_access) ||
        machine->fdc->connect.pic_slave !=
            vm_composition_machine_access_pic_slave(machine->core_access) ||
        machine->fdc->connect.port !=
            vm_composition_machine_access_port(machine->core_access)) {
        vm_session_finalize(session);
        STD_FREE(session);
        return 1;
    }
    vm_session_finalize(session);
    STD_FREE(session);
    puts("M5:T70:S1:P5:FDC-CONNECT:OK");
    return 0;
}
