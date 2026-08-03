#ifndef NTVDM64_CORE_MACHINE_H
#define NTVDM64_CORE_MACHINE_H

#include "type.h"





#include "core/machine/cpu.h"

#include "core/machine/cpu_instructions.h"

#include "core/machine/execution_provider.h"

#include "core/machine/machine_interface.h"

#include "core/machine/memory.h"

#include "core/machine/port.h"

#include "core/machine/pic.h"

#include "core/machine/pit.h"

#include "core/machine/dma.h"

#include "core/machine/kbc.h"

#include "core/machine/vadp.h"

#define CORE_MACHINE_TRACE_CAPACITY 32u

typedef struct core_machine_cpu {
    core_machine_cpu_state state;
} core_machine_cpu;

typedef struct core_machine_memory {
    uint8_t *bytes;
    STD_SIZE_T size;
    C_INT a20_enabled;
} core_machine_memory;

typedef struct core_machine_port_slot {
    core_machine_port_provider provider;
    C_VOID *owner;
} core_machine_port_slot;

typedef struct core_machine_port_table {
    core_machine_port_slot *slots;
} core_machine_port_table;

typedef struct core_machine_trace_state {
    core_machine_trace_provider provider;
    core_machine_trace_event events[CORE_MACHINE_TRACE_CAPACITY];
    uint64_t next_sequence;
    STD_SIZE_T count;
    C_INT flushing;
} core_machine_trace_state;

struct core_machine {
    core_machine_config config;
    core_machine_lifecycle lifecycle;
    STD_ATOMIC_BOOL stop_requested;
    uint32_t fault_detail;
    core_machine_cpu cpu;
    core_machine_memory memory;
    core_machine_port_table ports;
    core_machine_trace_state trace;
    t_cpu executor_cpu;
    t_cpuins executor_cpu_instructions;
    core_machine_cpu_execution_context executor_cpu_execution;
    t_ram executor_memory;
    t_port executor_port;
    C_INT executor_enabled;
    t_pic shared_pic_master;
    t_pic shared_pic_slave;
    t_pit shared_pit;
    t_latch shared_dma_latch;
    t_dma shared_dma_primary;
    t_dma shared_dma_secondary;
    t_kbc shared_kbc;
    t_vadp shared_vadp;
    C_INT shared_devices_enabled;
    const core_machine_execution_provider *execution_provider;
    C_VOID *execution_provider_context;
    C_INT execution_provider_frozen;
};

ntvdm64_status core_machine_cpu_reset(core_machine *machine);
ntvdm64_status core_machine_instance_memory_initialize(core_machine *machine);
C_VOID core_machine_instance_memory_finalize(core_machine *machine);
ntvdm64_status core_machine_instance_memory_reset(core_machine *machine);
ntvdm64_status core_machine_bus_initialize(core_machine *machine);
C_VOID core_machine_bus_finalize(core_machine *machine);
C_VOID core_machine_trace_initialize(core_machine *machine);
C_VOID core_machine_trace_finalize(core_machine *machine);
C_VOID core_machine_trace_record(
    core_machine *machine,
    core_machine_trace_event_type type,
    uint32_t address,
    uint32_t value,
    uint32_t detail);
ntvdm64_status core_machine_enable_executor(core_machine *machine);
t_cpu *core_machine_executor_cpu_borrow(core_machine *machine);
t_cpuins *core_machine_executor_cpu_instructions_borrow(core_machine *machine);
core_machine_cpu_execution_context *core_machine_executor_cpu_execution_borrow(
    core_machine *machine);
t_ram *core_machine_executor_memory_borrow(core_machine *machine);
t_port *core_machine_executor_port_borrow(core_machine *machine);
ntvdm64_status core_machine_enable_shared_devices(core_machine *machine);
t_pic *core_machine_shared_pic_master_borrow(core_machine *machine);
t_pic *core_machine_shared_pic_slave_borrow(core_machine *machine);
t_pit *core_machine_shared_pit_borrow(core_machine *machine);
t_latch *core_machine_shared_dma_latch_borrow(core_machine *machine);
t_dma *core_machine_shared_dma_primary_borrow(core_machine *machine);
t_dma *core_machine_shared_dma_secondary_borrow(core_machine *machine);
t_kbc *core_machine_shared_kbc_borrow(core_machine *machine);
t_vadp *core_machine_shared_vadp_borrow(core_machine *machine);
ntvdm64_status core_machine_bind_execution_provider(core_machine *machine,
    const core_machine_execution_provider *provider, C_VOID *context);
ntvdm64_status core_machine_freeze_execution_providers(core_machine *machine);

#endif
