#include "type.h"




#include "core/machine/memory.h"

#include "vm/composition/session/session.h"

C_INT main(C_VOID)
{
    vm_session *first;
    vm_session *second;
    C_UCHAR first_value = 0x11u;
    C_UCHAR second_value = 0x22u;
    C_UCHAR observed = 0u;
    C_INT failed = 0;

    first = (vm_session *)STD_CALLOC(1u, sizeof(*first));
    second = (vm_session *)STD_CALLOC(1u, sizeof(*second));
    if (first == STD_NULL || second == STD_NULL) {
        STD_FREE(second);
        STD_FREE(first);
        return 1;
    }

    vm_session_storage_initialize(first);
    vm_session_storage_initialize(second);

    failed |= core_machine_configuration_cpu_borrow(first->core_machine) == core_machine_configuration_cpu_borrow(second->core_machine);
    failed |= core_machine_configuration_cpu_instructions_borrow(first->core_machine) == core_machine_configuration_cpu_instructions_borrow(second->core_machine);
    failed |= core_machine_configuration_cpu_execution_borrow(first->core_machine) == core_machine_configuration_cpu_execution_borrow(second->core_machine);
    failed |= core_machine_configuration_memory_borrow(first->core_machine) == core_machine_configuration_memory_borrow(second->core_machine);
    failed |= core_machine_configuration_port_borrow(first->core_machine) == core_machine_configuration_port_borrow(second->core_machine);
    failed |= core_machine_configuration_cpu_execution_borrow(first->core_machine)->cpu != core_machine_configuration_cpu_borrow(first->core_machine);
    failed |= core_machine_configuration_cpu_execution_borrow(second->core_machine)->cpu != core_machine_configuration_cpu_borrow(second->core_machine);
    failed |= core_machine_configuration_cpu_execution_borrow(first->core_machine)->instructions != core_machine_configuration_cpu_instructions_borrow(first->core_machine);
    failed |= core_machine_configuration_cpu_execution_borrow(second->core_machine)->instructions != core_machine_configuration_cpu_instructions_borrow(second->core_machine);

    core_machine_memory_write_physical(core_machine_configuration_memory_borrow(first->core_machine), 0u,
        (ntvdm64_type_virtual_address)&first_value, 1u);
    core_machine_memory_write_physical(core_machine_configuration_memory_borrow(second->core_machine), 0u,
        (ntvdm64_type_virtual_address)&second_value, 1u);
    core_machine_memory_read_physical(core_machine_configuration_memory_borrow(first->core_machine), 0u,
        (ntvdm64_type_virtual_address)&observed, 1u);
    failed |= observed != first_value;
    core_machine_memory_read_physical(core_machine_configuration_memory_borrow(second->core_machine), 0u,
        (ntvdm64_type_virtual_address)&observed, 1u);
    failed |= observed != second_value;

    core_machine_configuration_cpu_borrow(first->core_machine)->data.eax = 0x11111111u;
    core_machine_configuration_cpu_borrow(second->core_machine)->data.eax = 0x22222222u;
    core_machine_configuration_cpu_instructions_borrow(first->core_machine)->data.flagWR = NTVDM64_TYPE_TRUE;
    failed |= core_machine_configuration_cpu_borrow(second->core_machine)->data.eax != 0x22222222u;
    failed |= core_machine_configuration_cpu_instructions_borrow(second->core_machine)->data.flagWR != NTVDM64_TYPE_FALSE;

    vm_session_storage_finalize(second);
    vm_session_storage_finalize(first);
    STD_FREE(second);
    STD_FREE(first);

    if (failed) return 1;
    puts("M5:T73:S1:TWO-SESSION-ISOLATION:OK");
    return 0;
}
