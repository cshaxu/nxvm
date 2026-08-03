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

t_pic *vm_composition_machine_access_pic_master(
    const vm_composition_machine_access *access)
{ return access == STD_NULL ? STD_NULL : core_machine_shared_pic_master_borrow(access->machine); }
t_pic *vm_composition_machine_access_pic_slave(
    const vm_composition_machine_access *access)
{ return access == STD_NULL ? STD_NULL : core_machine_shared_pic_slave_borrow(access->machine); }
t_pit *vm_composition_machine_access_pit(
    const vm_composition_machine_access *access)
{ return access == STD_NULL ? STD_NULL : core_machine_shared_pit_borrow(access->machine); }
t_latch *vm_composition_machine_access_dma_latch(
    const vm_composition_machine_access *access)
{ return access == STD_NULL ? STD_NULL : core_machine_shared_dma_latch_borrow(access->machine); }
t_dma *vm_composition_machine_access_dma_primary(
    const vm_composition_machine_access *access)
{ return access == STD_NULL ? STD_NULL : core_machine_shared_dma_primary_borrow(access->machine); }
t_dma *vm_composition_machine_access_dma_secondary(
    const vm_composition_machine_access *access)
{ return access == STD_NULL ? STD_NULL : core_machine_shared_dma_secondary_borrow(access->machine); }
t_kbc *vm_composition_machine_access_kbc(
    const vm_composition_machine_access *access)
{ return access == STD_NULL ? STD_NULL : core_machine_shared_kbc_borrow(access->machine); }
t_vadp *vm_composition_machine_access_vadp(
    const vm_composition_machine_access *access)
{ return access == STD_NULL ? STD_NULL : core_machine_shared_vadp_borrow(access->machine); }
