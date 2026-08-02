#ifndef NXVM_COMPOSITION_LIVE_MACHINE_H
#define NXVM_COMPOSITION_LIVE_MACHINE_H

#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"
#include "core/machine/port.h"
#include "core/machine/memory.h"
#include "core/machine/pic.h"

typedef struct vm_composition_live_machine {
    t_cpu cpu_storage;
    t_cpuins cpuins_storage;
    t_ram ram_storage;
    t_port port_storage;
    t_pic pic_master_storage;
    t_pic pic_slave_storage;
    t_cpu *cpu;
    t_cpuins *cpuins;
    t_ram *ram;
    t_port *port;
    t_pic *pic_master;
    t_pic *pic_slave;
} vm_composition_live_machine;

void vm_composition_live_machine_bind(void);
void vm_composition_live_machine_clear(void);
const vm_composition_live_machine *vm_composition_live_machine_current(void);

#endif
