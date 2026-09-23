#include "lib/types/types_interface.h"
#include "type.h"
#include "app-nxvm/machine/machine_private.h"




#include "../devices/support/core_machine_cpu_fixture.h"

#include "app-nxvm/machine/machine_interface.h"
#include "support/rom/session_assets.h"

C_INT main(C_VOID)
{
    vm_machine *machine = LIB_NULL;

    if (vm_test_default_pc_at_session_create(LIB_NULL, &machine) != TYPE_STATUS_OK ||
        machine == LIB_NULL || machine->core_machine == LIB_NULL ||
        !test_core_machine_fixture_executor_storage_is_coherent(
            machine->core_machine)) {
        vm_machine_destroy(machine);
        return 1;
    }
    vm_machine_destroy(machine);
    puts("M5:T83:S2:CORE-EXECUTOR-STORAGE:OK");
    return 0;
}
