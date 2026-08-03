#include "vm/composition/machine_access.h"

#include "core/machine/machine.h"

C_VOID vm_composition_machine_access_initialize(
    vm_composition_machine_access *access, core_machine *machine)
{
    if (access != STD_NULL) access->machine = machine;
}

C_VOID vm_composition_machine_access_finalize(
    vm_composition_machine_access *access)
{
    if (access != STD_NULL) access->machine = STD_NULL;
}

t_cpu *vm_composition_machine_access_cpu(
    const vm_composition_machine_access *access)
{ return access == STD_NULL ? STD_NULL : core_machine_executor_cpu_borrow(access->machine); }

t_cpuins *vm_composition_machine_access_instructions(
    const vm_composition_machine_access *access)
{ return access == STD_NULL ? STD_NULL :
    core_machine_executor_cpu_instructions_borrow(access->machine); }

core_machine_cpu_execution_context *vm_composition_machine_access_execution(
    const vm_composition_machine_access *access)
{ return access == STD_NULL ? STD_NULL :
    core_machine_executor_cpu_execution_borrow(access->machine); }

t_ram *vm_composition_machine_access_memory(
    const vm_composition_machine_access *access)
{ return access == STD_NULL ? STD_NULL : core_machine_executor_memory_borrow(access->machine); }

t_port *vm_composition_machine_access_port(
    const vm_composition_machine_access *access)
{ return access == STD_NULL ? STD_NULL : core_machine_executor_port_borrow(access->machine); }
