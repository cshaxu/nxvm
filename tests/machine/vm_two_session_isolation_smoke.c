#include "type.h"




#include "core/machine/memory.h"

#include "vm/composition/session.h"

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

    failed |= vm_composition_machine_access_cpu(first->core_access) == vm_composition_machine_access_cpu(second->core_access);
    failed |= vm_composition_machine_access_instructions(first->core_access) == vm_composition_machine_access_instructions(second->core_access);
    failed |= vm_composition_machine_access_execution(first->core_access) == vm_composition_machine_access_execution(second->core_access);
    failed |= vm_composition_machine_access_memory(first->core_access) == vm_composition_machine_access_memory(second->core_access);
    failed |= vm_composition_machine_access_port(first->core_access) == vm_composition_machine_access_port(second->core_access);
    failed |= vm_composition_machine_access_execution(first->core_access)->cpu != vm_composition_machine_access_cpu(first->core_access);
    failed |= vm_composition_machine_access_execution(second->core_access)->cpu != vm_composition_machine_access_cpu(second->core_access);
    failed |= vm_composition_machine_access_execution(first->core_access)->instructions != vm_composition_machine_access_instructions(first->core_access);
    failed |= vm_composition_machine_access_execution(second->core_access)->instructions != vm_composition_machine_access_instructions(second->core_access);

    core_machine_memory_write_physical(vm_composition_machine_access_memory(first->core_access), 0u,
        (ntvdm64_type_virtual_address)&first_value, 1u);
    core_machine_memory_write_physical(vm_composition_machine_access_memory(second->core_access), 0u,
        (ntvdm64_type_virtual_address)&second_value, 1u);
    core_machine_memory_read_physical(vm_composition_machine_access_memory(first->core_access), 0u,
        (ntvdm64_type_virtual_address)&observed, 1u);
    failed |= observed != first_value;
    core_machine_memory_read_physical(vm_composition_machine_access_memory(second->core_access), 0u,
        (ntvdm64_type_virtual_address)&observed, 1u);
    failed |= observed != second_value;

    vm_composition_machine_access_cpu(first->core_access)->data.eax = 0x11111111u;
    vm_composition_machine_access_cpu(second->core_access)->data.eax = 0x22222222u;
    vm_composition_machine_access_instructions(first->core_access)->data.flagWR = NTVDM64_TYPE_TRUE;
    failed |= vm_composition_machine_access_cpu(second->core_access)->data.eax != 0x22222222u;
    failed |= vm_composition_machine_access_instructions(second->core_access)->data.flagWR != NTVDM64_TYPE_FALSE;

    vm_session_storage_finalize(second);
    vm_session_storage_finalize(first);
    STD_FREE(second);
    STD_FREE(first);

    if (failed) return 1;
    puts("M5:T73:S1:TWO-SESSION-ISOLATION:OK");
    return 0;
}
