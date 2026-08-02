#ifndef NXVM_COMPOSITION_LIVE_MACHINE_H
#define NXVM_COMPOSITION_LIVE_MACHINE_H

#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"
#include "core/machine/vport.h"
#include "core/machine/vram.h"

typedef struct vm_composition_live_machine {
    t_cpu cpu_storage;
    t_cpuins cpuins_storage;
    t_cpu *cpu;
    t_cpuins *cpuins;
    t_ram *ram;
    t_port *port;
} vm_composition_live_machine;

void vm_composition_live_machine_bind(t_ram *ram, t_port *port);
void vm_composition_live_machine_clear(void);
const vm_composition_live_machine *vm_composition_live_machine_current(void);

#endif
