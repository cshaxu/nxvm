#include "type.h"
#include "tests/support/vm_session_fixture.h"




#include "core/machine/memory.h"

#include "vm/composition/session/session_interface.h"

C_INT main(C_VOID)
{
    vm_session *first;
    vm_session *second;
    C_UCHAR first_value = 0x11u;
    C_UCHAR second_value = 0x22u;
    C_UCHAR observed = 0u;
    C_INT failed = 0;

    first = vm_session_fixture_allocate();
    second = vm_session_fixture_allocate();
    if (first == STD_NULL || second == STD_NULL) {
        vm_session_fixture_free(second);
        vm_session_fixture_free(first);
        return 1;
    }

    vm_session_fixture_storage_initialize(first);
    vm_session_fixture_storage_initialize(second);

    failed |= core_machine_configuration_cpu_borrow(vm_session_fixture_machine(first)) == core_machine_configuration_cpu_borrow(vm_session_fixture_machine(second));
    failed |= core_machine_configuration_cpu_instructions_borrow(vm_session_fixture_machine(first)) == core_machine_configuration_cpu_instructions_borrow(vm_session_fixture_machine(second));
    failed |= core_machine_configuration_cpu_execution_borrow(vm_session_fixture_machine(first)) == core_machine_configuration_cpu_execution_borrow(vm_session_fixture_machine(second));
    failed |= core_machine_configuration_memory_borrow(vm_session_fixture_machine(first)) == core_machine_configuration_memory_borrow(vm_session_fixture_machine(second));
    failed |= core_machine_configuration_port_borrow(vm_session_fixture_machine(first)) == core_machine_configuration_port_borrow(vm_session_fixture_machine(second));
    failed |= core_machine_configuration_cpu_execution_borrow(vm_session_fixture_machine(first))->cpu != core_machine_configuration_cpu_borrow(vm_session_fixture_machine(first));
    failed |= core_machine_configuration_cpu_execution_borrow(vm_session_fixture_machine(second))->cpu != core_machine_configuration_cpu_borrow(vm_session_fixture_machine(second));
    failed |= core_machine_configuration_cpu_execution_borrow(vm_session_fixture_machine(first))->instructions != core_machine_configuration_cpu_instructions_borrow(vm_session_fixture_machine(first));
    failed |= core_machine_configuration_cpu_execution_borrow(vm_session_fixture_machine(second))->instructions != core_machine_configuration_cpu_instructions_borrow(vm_session_fixture_machine(second));

    core_machine_memory_write_physical(core_machine_configuration_memory_borrow(vm_session_fixture_machine(first)), 0u,
        (type_virtual_address)&first_value, 1u);
    core_machine_memory_write_physical(core_machine_configuration_memory_borrow(vm_session_fixture_machine(second)), 0u,
        (type_virtual_address)&second_value, 1u);
    core_machine_memory_read_physical(core_machine_configuration_memory_borrow(vm_session_fixture_machine(first)), 0u,
        (type_virtual_address)&observed, 1u);
    failed |= observed != first_value;
    core_machine_memory_read_physical(core_machine_configuration_memory_borrow(vm_session_fixture_machine(second)), 0u,
        (type_virtual_address)&observed, 1u);
    failed |= observed != second_value;

    core_machine_configuration_cpu_borrow(vm_session_fixture_machine(first))->data.eax = 0x11111111u;
    core_machine_configuration_cpu_borrow(vm_session_fixture_machine(second))->data.eax = 0x22222222u;
    core_machine_configuration_cpu_instructions_borrow(vm_session_fixture_machine(first))->data.flagWR = TYPE_TRUE;
    failed |= core_machine_configuration_cpu_borrow(vm_session_fixture_machine(second))->data.eax != 0x22222222u;
    failed |= core_machine_configuration_cpu_instructions_borrow(vm_session_fixture_machine(second))->data.flagWR != TYPE_FALSE;

    vm_session_fixture_storage_finalize(second);
    vm_session_fixture_storage_finalize(first);
    vm_session_fixture_free(second);
    vm_session_fixture_free(first);

    if (failed) return 1;
    puts("M5:T73:S1:TWO-SESSION-ISOLATION:OK");
    return 0;
}
