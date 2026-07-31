#ifndef NXVM_CORE_MACHINE_IMPL_H
#define NXVM_CORE_MACHINE_IMPL_H

#include <stddef.h>
#include <stdint.h>

#include "core/cpu.h"
#include "core/port.h"
#include "core/profile.h"

typedef enum nxvm_core_machine_state {
    NXVM_CORE_MACHINE_NEW = 0,
    NXVM_CORE_MACHINE_RESET,
    NXVM_CORE_MACHINE_STOP_REQUESTED
} nxvm_core_machine_state;

typedef struct nxvm_core_cpu {
    nxvm_core_cpu_state state;
} nxvm_core_cpu;

typedef struct nxvm_core_memory {
    uint8_t *bytes;
    size_t size;
    int a20_enabled;
} nxvm_core_memory;

typedef struct nxvm_core_port_slot {
    nxvm_core_port_ops ops;
    void *owner;
} nxvm_core_port_slot;

typedef struct nxvm_core_port_table {
    nxvm_core_port_slot *slots;
} nxvm_core_port_table;

struct nxvm_core_machine {
    nxvm_core_machine_config config;
    nxvm_core_machine_state state;
    nxvm_core_cpu cpu;
    nxvm_core_memory memory;
    nxvm_core_port_table ports;
};

nxvm_core_status nxvm_core_cpu_reset(nxvm_core_machine *machine);
nxvm_core_status nxvm_core_memory_initialize(nxvm_core_machine *machine);
void nxvm_core_memory_finalize(nxvm_core_machine *machine);
nxvm_core_status nxvm_core_memory_reset(nxvm_core_machine *machine);
nxvm_core_status nxvm_core_port_initialize(nxvm_core_machine *machine);
void nxvm_core_port_finalize(nxvm_core_machine *machine);

#endif
