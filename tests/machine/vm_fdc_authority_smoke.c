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
    if (machine == STD_NULL || machine->fdc == STD_NULL ||
        machine->fdc->connect.fdd != machine->fdd ||
        machine->fdc->connect.dma_latch !=
            core_machine_shared_dma_latch_borrow(machine->core_machine) ||
        machine->fdc->connect.dma_primary !=
            core_machine_shared_dma_primary_borrow(machine->core_machine) ||
        machine->fdc->connect.dma_secondary !=
            core_machine_shared_dma_secondary_borrow(machine->core_machine) ||
        machine->fdc->connect.pic_master !=
            core_machine_shared_pic_master_borrow(machine->core_machine) ||
        machine->fdc->connect.pic_slave !=
            core_machine_shared_pic_slave_borrow(machine->core_machine) ||
        machine->fdc->connect.port !=
            core_machine_executor_port_borrow(machine->core_machine)) {
        vm_session_finalize(session);
        STD_FREE(session);
        return 1;
    }
    vm_session_finalize(session);
    STD_FREE(session);
    puts("M5:T70:S1:P5:FDC-CONNECT:OK");
    return 0;
}
