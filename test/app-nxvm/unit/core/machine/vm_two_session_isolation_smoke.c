#include "lib/types/types_interface.h"
#include "type.h"
#include "app-nxvm/machine/machine_private.h"




#include "../devices/support/core_machine_cpu_fixture.h"

#include "app-nxvm/machine/machine_interface.h"
#include "support/rom/session_assets.h"

C_INT main(C_VOID)
{
    vm_machine *first = LIB_NULL;
    vm_machine *second = LIB_NULL;
    C_INT failed = 0;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &first) != TYPE_STATUS_OK ||
        vm_test_default_pc_at_session_create(LIB_NULL, &second) != TYPE_STATUS_OK) failed = 1;

    if (!failed) {
        failed |= first->fdc_dma_request.core_token == second->fdc_dma_request.core_token;
        failed |= !test_core_machine_fixture_sessions_are_isolated(
            first->core_machine, second->core_machine);
    }

    vm_machine_destroy(second);
    vm_machine_destroy(first);

    if (failed) return 1;
    puts("M5:T73:S1:TWO-SESSION-ISOLATION:OK");
    return 0;
}
