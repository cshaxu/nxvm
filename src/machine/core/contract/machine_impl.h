#ifndef NXVM_CORE_MACHINE_IMPL_H
#define NXVM_CORE_MACHINE_IMPL_H

#include <stddef.h>
#include <stdatomic.h>
#include <stdint.h>

#include "machine/core/contract/cpu.h"
#include "machine/core/contract/lifecycle.h"
#include "machine/core/contract/port.h"
#include "machine/core/contract/profile.h"
#include "core/trace.h"

#define NXVM_CORE_TRACE_CAPACITY 32u

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

typedef struct nxvm_core_trace_state {
    nxvm_core_trace_sink sink;
    nxvm_core_trace_event events[NXVM_CORE_TRACE_CAPACITY];
    uint64_t next_sequence;
    size_t count;
    int flushing;
} nxvm_core_trace_state;

struct nxvm_core_machine {
    nxvm_core_machine_config config;
    nxvm_core_machine_lifecycle lifecycle;
    atomic_bool stop_requested;
    uint32_t fault_detail;
    nxvm_core_cpu cpu;
    nxvm_core_memory memory;
    nxvm_core_port_table ports;
    nxvm_core_trace_state trace;
};

nxvm_core_status nxvm_core_cpu_reset(nxvm_core_machine *machine);
nxvm_core_status nxvm_core_memory_initialize(nxvm_core_machine *machine);
void nxvm_core_memory_finalize(nxvm_core_machine *machine);
nxvm_core_status nxvm_core_memory_reset(nxvm_core_machine *machine);
nxvm_core_status nxvm_core_port_initialize(nxvm_core_machine *machine);
void nxvm_core_port_finalize(nxvm_core_machine *machine);
void nxvm_core_trace_initialize(nxvm_core_machine *machine);
void nxvm_core_trace_finalize(nxvm_core_machine *machine);
void nxvm_core_trace_record(
    nxvm_core_machine *machine,
    nxvm_core_trace_event_type type,
    uint32_t address,
    uint32_t value,
    uint32_t detail);

#endif
