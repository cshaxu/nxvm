#ifndef NTVDM64_CORE_MACHINE_H
#define NTVDM64_CORE_MACHINE_H

#include <stddef.h>
#include <stdatomic.h>
#include <stdint.h>

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"

#define CORE_MACHINE_TRACE_CAPACITY 32u

typedef struct core_machine_cpu {
    core_machine_cpu_state state;
} core_machine_cpu;

typedef struct core_machine_memory {
    uint8_t *bytes;
    size_t size;
    int a20_enabled;
} core_machine_memory;

typedef struct core_machine_port_slot {
    core_machine_port_provider provider;
    void *owner;
} core_machine_port_slot;

typedef struct core_machine_port_table {
    core_machine_port_slot *slots;
} core_machine_port_table;

typedef struct core_machine_trace_state {
    core_machine_trace_provider provider;
    core_machine_trace_event events[CORE_MACHINE_TRACE_CAPACITY];
    uint64_t next_sequence;
    size_t count;
    int flushing;
} core_machine_trace_state;

struct core_machine {
    core_machine_config config;
    core_machine_lifecycle lifecycle;
    atomic_bool stop_requested;
    uint32_t fault_detail;
    core_machine_cpu cpu;
    core_machine_memory memory;
    core_machine_port_table ports;
    core_machine_trace_state trace;
};

nxvm_core_status core_machine_cpu_reset(core_machine *machine);
nxvm_core_status core_machine_instance_memory_initialize(core_machine *machine);
void core_machine_instance_memory_finalize(core_machine *machine);
nxvm_core_status core_machine_instance_memory_reset(core_machine *machine);
nxvm_core_status core_machine_port_initialize(core_machine *machine);
void core_machine_port_finalize(core_machine *machine);
void core_machine_trace_initialize(core_machine *machine);
void core_machine_trace_finalize(core_machine *machine);
void core_machine_trace_record(
    core_machine *machine,
    core_machine_trace_event_type type,
    uint32_t address,
    uint32_t value,
    uint32_t detail);

#endif
