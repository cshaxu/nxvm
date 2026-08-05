#include "type.h"
#include "tests/support/vm_session_fixture.h"




#include "core/machine/machine.h"

#include "vm/composition/session/session_interface.h"

C_INT main(C_VOID)
{
    vm_session *machine = vm_session_fixture_allocate();

    if (machine == STD_NULL) return 1;
    vm_session_fixture_storage_initialize(machine);
    if (vm_session_fixture_machine(machine) == STD_NULL ||
        core_machine_configuration_cpu_borrow(vm_session_fixture_machine(machine)) != core_machine_configuration_cpu_borrow(vm_session_fixture_machine(machine)) ||
        core_machine_configuration_cpu_instructions_borrow(vm_session_fixture_machine(machine)) != core_machine_configuration_cpu_instructions_borrow(
            vm_session_fixture_machine(machine)) ||
        core_machine_configuration_cpu_execution_borrow(vm_session_fixture_machine(machine)) != core_machine_configuration_cpu_execution_borrow(
            vm_session_fixture_machine(machine)) ||
        core_machine_configuration_memory_borrow(vm_session_fixture_machine(machine)) != core_machine_configuration_memory_borrow(vm_session_fixture_machine(machine)) ||
        core_machine_configuration_port_borrow(vm_session_fixture_machine(machine)) != core_machine_configuration_port_borrow(vm_session_fixture_machine(machine)) ||
        core_machine_configuration_shared_pic_master_borrow(vm_session_fixture_machine(machine)) != core_machine_configuration_shared_pic_master_borrow(
            vm_session_fixture_machine(machine)) ||
        core_machine_configuration_shared_pic_slave_borrow(vm_session_fixture_machine(machine)) != core_machine_configuration_shared_pic_slave_borrow(
            vm_session_fixture_machine(machine)) ||
        core_machine_configuration_shared_pit_borrow(vm_session_fixture_machine(machine)) != core_machine_configuration_shared_pit_borrow(vm_session_fixture_machine(machine)) ||
        core_machine_configuration_shared_dma_latch_borrow(vm_session_fixture_machine(machine)) != core_machine_configuration_shared_dma_latch_borrow(
            vm_session_fixture_machine(machine)) ||
        core_machine_configuration_shared_dma_primary_borrow(vm_session_fixture_machine(machine)) != core_machine_configuration_shared_dma_primary_borrow(
            vm_session_fixture_machine(machine)) ||
        core_machine_configuration_shared_dma_secondary_borrow(vm_session_fixture_machine(machine)) != core_machine_configuration_shared_dma_secondary_borrow(
            vm_session_fixture_machine(machine)) ||
        core_machine_configuration_shared_kbc_borrow(vm_session_fixture_machine(machine)) != core_machine_configuration_shared_kbc_borrow(vm_session_fixture_machine(machine))) {
        vm_session_fixture_storage_finalize(machine);
        vm_session_fixture_free(machine);
        return 1;
    }
    vm_session_fixture_storage_finalize(machine);
    vm_session_fixture_free(machine);
    puts("M5:T83:S2:CORE-EXECUTOR-STORAGE:OK");
    return 0;
}
