#include "vm/composition_live_machine.h"

static vm_composition_live_machine vmCompositionLiveMachine;
static int vmCompositionLiveMachineBound;

void vm_composition_live_machine_bind(t_cpu *cpu, t_cpuins *cpuins,
    t_ram *ram, t_port *port)
{
    vmCompositionLiveMachine.cpu = cpu;
    vmCompositionLiveMachine.cpuins = cpuins;
    vmCompositionLiveMachine.ram = ram;
    vmCompositionLiveMachine.port = port;
    vmCompositionLiveMachineBound = cpu != NULL && cpuins != NULL &&
        ram != NULL && port != NULL;
}

void vm_composition_live_machine_clear(void)
{
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
