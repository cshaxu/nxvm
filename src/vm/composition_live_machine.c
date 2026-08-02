#include "vm/composition_live_machine.h"

static vm_composition_live_machine vmCompositionLiveMachine;
static int vmCompositionLiveMachineBound;

void vm_composition_live_machine_bind(void)
{
    vmCompositionLiveMachine.cpu = &vmCompositionLiveMachine.cpu_storage;
    vmCompositionLiveMachine.cpuins =
        &vmCompositionLiveMachine.cpuins_storage;
    vmCompositionLiveMachine.ram = &vmCompositionLiveMachine.ram_storage;
    vmCompositionLiveMachine.port = &vmCompositionLiveMachine.port_storage;
    core_machine_cpu_bind_live(vmCompositionLiveMachine.cpu);
    core_machine_cpu_instructions_bind_live(vmCompositionLiveMachine.cpuins);
    core_machine_memory_bind_live(vmCompositionLiveMachine.ram);
    core_machine_port_bind_live(vmCompositionLiveMachine.port);
    vmCompositionLiveMachineBound = 1;
}

void vm_composition_live_machine_clear(void)
{
    core_machine_memory_unbind_live();
    core_machine_port_unbind_live();
    core_machine_cpu_instructions_unbind_live();
    core_machine_cpu_unbind_live();
    vmCompositionLiveMachine.cpu = NULL;
    vmCompositionLiveMachine.cpuins = NULL;
    vmCompositionLiveMachine.ram = NULL;
    vmCompositionLiveMachine.port = NULL;
    vmCompositionLiveMachineBound = 0;
}

const vm_composition_live_machine *vm_composition_live_machine_current(void)
{
    return vmCompositionLiveMachineBound ? &vmCompositionLiveMachine : NULL;
}
