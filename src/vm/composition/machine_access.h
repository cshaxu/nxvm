#ifndef NTVDM64_VM_COMPOSITION_MACHINE_ACCESS_H
#define NTVDM64_VM_COMPOSITION_MACHINE_ACCESS_H

#include "core/machine/machine_interface.h"
#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"
#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "core/machine/pic.h"
#include "core/machine/pit.h"
#include "core/machine/dma.h"
#include "core/machine/kbc.h"
#include "core/machine/vadp.h"

typedef struct vm_composition_machine_access {
    core_machine *machine;
} vm_composition_machine_access;

C_VOID vm_composition_machine_access_initialize(
    vm_composition_machine_access *access, core_machine *machine);
C_VOID vm_composition_machine_access_finalize(
    vm_composition_machine_access *access);
t_cpu *vm_composition_machine_access_cpu(
    const vm_composition_machine_access *access);
t_cpuins *vm_composition_machine_access_instructions(
    const vm_composition_machine_access *access);
core_machine_cpu_execution_context *vm_composition_machine_access_execution(
    const vm_composition_machine_access *access);
t_ram *vm_composition_machine_access_memory(
    const vm_composition_machine_access *access);
t_port *vm_composition_machine_access_port(
    const vm_composition_machine_access *access);
t_pic *vm_composition_machine_access_pic_master(
    const vm_composition_machine_access *access);
t_pic *vm_composition_machine_access_pic_slave(
    const vm_composition_machine_access *access);
t_pit *vm_composition_machine_access_pit(
    const vm_composition_machine_access *access);
t_latch *vm_composition_machine_access_dma_latch(
    const vm_composition_machine_access *access);
t_dma *vm_composition_machine_access_dma_primary(
    const vm_composition_machine_access *access);
t_dma *vm_composition_machine_access_dma_secondary(
    const vm_composition_machine_access *access);
t_kbc *vm_composition_machine_access_kbc(
    const vm_composition_machine_access *access);
t_vadp *vm_composition_machine_access_vadp(
    const vm_composition_machine_access *access);

#endif
