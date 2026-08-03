#include "type.h"




#include "core/machine/machine.h"

#include "vm/composition/session.h"

C_INT main(C_VOID)
{
    vm_session *machine = STD_CALLOC(1u, sizeof(*machine));

    if (machine == STD_NULL) return 1;
    vm_session_storage_initialize(machine);
    if (machine->core_machine == STD_NULL ||
        vm_composition_machine_access_cpu(machine->core_access) != core_machine_executor_cpu_borrow(machine->core_machine) ||
        vm_composition_machine_access_instructions(machine->core_access) != core_machine_executor_cpu_instructions_borrow(
            machine->core_machine) ||
        vm_composition_machine_access_execution(machine->core_access) != core_machine_executor_cpu_execution_borrow(
            machine->core_machine) ||
        vm_composition_machine_access_memory(machine->core_access) != core_machine_executor_memory_borrow(machine->core_machine) ||
        vm_composition_machine_access_port(machine->core_access) != core_machine_executor_port_borrow(machine->core_machine) ||
        machine->pic_master != core_machine_shared_pic_master_borrow(
            machine->core_machine) ||
        machine->pic_slave != core_machine_shared_pic_slave_borrow(
            machine->core_machine) ||
        machine->pit != core_machine_shared_pit_borrow(machine->core_machine) ||
        machine->dma_latch != core_machine_shared_dma_latch_borrow(
            machine->core_machine) ||
        machine->dma_primary != core_machine_shared_dma_primary_borrow(
            machine->core_machine) ||
        machine->dma_secondary != core_machine_shared_dma_secondary_borrow(
            machine->core_machine) ||
        machine->kbc != core_machine_shared_kbc_borrow(machine->core_machine) ||
        machine->vadp != core_machine_shared_vadp_borrow(machine->core_machine)) {
        vm_session_storage_finalize(machine);
        STD_FREE(machine);
        return 1;
    }
    vm_session_storage_finalize(machine);
    STD_FREE(machine);
    puts("M5:T83:S2:CORE-EXECUTOR-STORAGE:OK");
    return 0;
}
