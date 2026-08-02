#ifndef NXVM_COMPOSITION_LIVE_MACHINE_H
#define NXVM_COMPOSITION_LIVE_MACHINE_H

#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"
#include "core/machine/port.h"
#include "core/machine/memory.h"
#include "core/machine/pic.h"
#include "core/machine/pit.h"
#include "core/machine/dma.h"
#include "core/machine/keyboard_controller.h"
#include "core/machine/video_adapter.h"
#include "vm/machine/vcmos.h"

typedef struct vm_composition_live_machine {
    t_cpu cpu_storage;
    t_cpuins cpuins_storage;
    t_ram ram_storage;
    t_port port_storage;
    t_pic pic_master_storage;
    t_pic pic_slave_storage;
    t_pit pit_storage;
    t_latch dma_latch_storage;
    t_dma dma_primary_storage;
    t_dma dma_secondary_storage;
    t_kbc keyboard_controller_storage;
    t_vadp video_adapter_storage;
    t_cmos cmos_storage;
    t_cpu *cpu;
    t_cpuins *cpuins;
    t_ram *ram;
    t_port *port;
    t_pic *pic_master;
    t_pic *pic_slave;
    t_pit *pit;
    t_latch *dma_latch;
    t_dma *dma_primary;
    t_dma *dma_secondary;
    t_kbc *keyboard_controller;
    t_vadp *video_adapter;
    t_cmos *cmos;
} vm_composition_live_machine;

void vm_composition_live_machine_bind(void);
void vm_composition_live_machine_clear(void);
const vm_composition_live_machine *vm_composition_live_machine_current(void);

#endif
