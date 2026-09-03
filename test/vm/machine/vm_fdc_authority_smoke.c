#include "type.h"
#include "core/machine/machine.h"
#include "vm/composition/session/session_private.h"




#include "vm/composition/session/session_interface.h"

#include "vm/composition/session/media.h"

#include "vm/composition/session/lifecycle.h"

#include "core/machine/fdc.h"
#include "../support/rom/session_assets.h"

C_INT main(C_VOID)
{
    vm_session *session;
    const vm_session *machine;

    if (vm_test_default_pc_at_session_create(STD_NULL, &session) != TYPE_STATUS_OK ||
        session == STD_NULL) return 1;
    machine = session;
    if (machine == STD_NULL ||
        machine->media_registry == STD_NULL ||
        machine->core_machine->fdc.connect.drives.media_id[0] !=
            VM_SESSION_MEDIA_FDD_ID ||
        machine->core_machine->fdc.connect.drives.media_id[1] !=
            CORE_MACHINE_MEDIA_ID_INVALID ||
        machine->core_machine->fdc.connect.dma_request.core_token == 0u ||
        machine->core_machine->fdc.connect.dma_request.channel != 2u ||
        machine->core_machine->fdc.connect.irq_source.master == STD_NULL ||
        machine->core_machine->fdc.connect.irq_source.slave == STD_NULL ||
        machine->core_machine->fdc.connect.port == STD_NULL) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_destroy(session);
    puts("M5:T230:S3:FDC-DMA-BINDING:OK");
    puts("M5:T290:S2:FDC-TOPOLOGY:VM:OK");
    return 0;
}
