#include "type.h"
#include "tests/support/vm_session_fixture.h"




#include "vm/composition/session/session_interface.h"

#include "vm/composition/session/lifecycle.h"

#include "vm/machine/fdc.h"

C_INT main(C_VOID)
{
    vm_session *session;
    const vm_session *machine;

    session = vm_session_fixture_allocate();
    if (session == STD_NULL) return 1;
    vm_session_initialize(session);
    machine = session;
    if (machine == STD_NULL ||
        vm_session_fixture_fdc(machine).connect.fdd != vm_session_fixture_fdd(machine) ||
        vm_session_fixture_fdc(machine).connect.dma_request.core_owner == STD_NULL ||
        vm_session_fixture_fdc(machine).connect.dma_request.channel != 2u ||
        vm_session_fixture_fdc(machine).connect.irq_source.master == STD_NULL ||
        vm_session_fixture_fdc(machine).connect.irq_source.slave == STD_NULL ||
        vm_session_fixture_fdc(machine).connect.port == STD_NULL) {
        vm_session_finalize(session);
        vm_session_fixture_free(session);
        return 1;
    }
    vm_session_finalize(session);
    vm_session_fixture_free(session);
    puts("M5:T230:S3:FDC-DMA-BINDING:OK");
    return 0;
}
