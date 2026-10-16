#include "x86-nxvm/machine/machine_interface.h"
#include "lib/types/types_interface.h"
#include "../../x86-nxvm/board-base/support/controller_fixture.h"
#include <stdio.h>
#include "x86-nxvm/machine/machine_private.h"




#include "x86-nxvm/machine/machine_interface.h"

#include "x86-nxvm/machine/media/media_interface.h"

#include "x86-nxvm/machine/lifecycle.h"

#include "../support/rom/session_assets.h"

lib_i32 main(void)
{
    x86_nxvm_machine *session;
    const x86_nxvm_machine *machine;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &session) != LIB_STATUS_OK ||
        session == LIB_NULL) return 1;
    machine = session;
    if (machine == LIB_NULL ||
        machine->media_registry == LIB_NULL ||
        !test_board_fdc_binding_matches(machine->board, machine->x86_core_machine,
            X86_NXVM_MACHINE_MEDIA_FDD_ID, 2u)) {
        x86_nxvm_machine_destroy(session);
        return 1;
    }
    x86_nxvm_machine_destroy(session);
    puts("FDC-DMA-BINDING:OK");
    puts("FDC-TOPOLOGY:VM:OK");
    return 0;
}
