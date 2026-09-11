#include "type.h"
#include "vm/machine/runtime/machine_private.h"




#include "../../core/support/core_machine_cpu_fixture.h"

#include "vm/machine/runtime/machine_interface.h"

C_INT main(C_VOID)
{
    vm_machine *machine = ((vm_machine *)STD_CALLOC(1u, sizeof(vm_machine)));

    if (machine == STD_NULL) return 1;
    vm_machine_storage_initialize(machine);
    if (machine->core_machine == STD_NULL ||
        !test_core_machine_fixture_executor_storage_is_coherent(
            machine->core_machine)) {
        vm_machine_storage_finalize(machine);
        STD_FREE(machine);
        return 1;
    }
    vm_machine_storage_finalize(machine);
    STD_FREE(machine);
    puts("M5:T83:S2:CORE-EXECUTOR-STORAGE:OK");
    return 0;
}
