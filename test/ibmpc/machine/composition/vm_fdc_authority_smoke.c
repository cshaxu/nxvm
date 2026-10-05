#include "ibmpc/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "../../board-common/controller_fixture.h"
#include <stdio.h>
#include "ibmpc/machine/machine_private.h"




#include "ibmpc/machine/machine_interface.h"

#include "ibmpc/machine/media/media_interface.h"

#include "ibmpc/machine/lifecycle.h"

#include "../support/rom/session_assets.h"

lib_i32 main(void)
{
    vm_machine *session;
    const vm_machine *machine;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK ||
        session == LIB_NULL) return 1;
    machine = session;
    if (machine == LIB_NULL ||
        machine->media_registry == LIB_NULL ||
        !test_board_fdc_binding_matches(machine->board, machine->core_machine,
            VM_MACHINE_MEDIA_FDD_ID, 2u)) {
        vm_machine_destroy(session);
        return 1;
    }
    vm_machine_destroy(session);
    puts("M5:T230:S3:FDC-DMA-BINDING:OK");
    puts("M5:T290:S2:FDC-TOPOLOGY:VM:OK");
    return 0;
}
