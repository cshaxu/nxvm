#ifndef CORE_MACHINE_H
#define CORE_MACHINE_H

#include "type.h"





#include "core/machine/cpu.h"

#include "core/machine/cpu_instructions.h"
#include "core/machine/fpu.h"

#include "core/machine/execution_provider.h"

#include "core/machine/machine_interface.h"

#include "core/machine/clock.h"

#include "core/machine/memory.h"

#include "core/machine/port.h"

#include "core/machine/pic.h"

#include "core/machine/pit.h"

#include "core/machine/dma.h"

#include "core/machine/kbc.h"

#include "core/machine/vadp.h"

#define CORE_MACHINE_TRACE_CAPACITY 32u
#define CORE_MACHINE_UNDEFINED_INSTRUCTION_TRANSITION_CAPACITY 8u
#define CORE_MACHINE_IMMUTABLE_ROM_MAPPING_CAPACITY 4u

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

typedef struct core_machine_cpu_diagnostic_state {
    core_machine_cpu_diagnostic snapshot;
    STD_SIZE_T next_index;
} core_machine_cpu_diagnostic_state;

typedef struct core_machine_undefined_instruction_transition {
    uint8_t pattern[CORE_MACHINE_UNDEFINED_INSTRUCTION_MAX_BYTES];
    uint8_t length;
    core_machine_undefined_instruction_consumer consumer;
    C_VOID *owner;
} core_machine_undefined_instruction_transition;

typedef struct core_machine_undefined_instruction_registry {
    core_machine_undefined_instruction_transition
        entries[CORE_MACHINE_UNDEFINED_INSTRUCTION_TRANSITION_CAPACITY];
    STD_SIZE_T count;
    type_bool frozen;
} core_machine_undefined_instruction_registry;

typedef struct core_machine_immutable_rom_mapping {
    uint32_t physical_start;
    STD_SIZE_T bytes;
    uint8_t *image;
} core_machine_immutable_rom_mapping;

struct core_machine {
    core_machine_lifecycle lifecycle;
    STD_ATOMIC_BOOL stop_requested;
    uint32_t fault_detail;
    uint64_t elapsed_ticks;
    core_machine_instruction_timing instruction_timing;
    uint64_t maximum_instruction_ticks;
    core_machine_clock_domain dma_clock;
    core_machine_clock_domain pit_clock;
    core_machine_clock_domain vadp_clock;
    core_machine_clock_domain kbc_clock;
    core_machine_clock_domain provider_clock;
    uint32_t kbc_typematic_initial_ticks;
    uint32_t kbc_typematic_repeat_ticks;
    uint32_t kbc_command_response_ticks;
    core_machine_port_table port_providers;
    core_machine_trace_state trace;
    core_machine_cpu_diagnostic_state cpu_diagnostic;
    core_machine_undefined_instruction_registry undefined_instruction_registry;
    core_machine_immutable_rom_mapping
        immutable_rom_mappings[CORE_MACHINE_IMMUTABLE_ROM_MAPPING_CAPACITY];
    STD_SIZE_T immutable_rom_mapping_count;
    type_bool entry_plan_applied;
    core_machine_cpu_profile cpu_profile;
    core_machine_fpu fpu;
    t_cpu executor_cpu;
    t_cpuins executor_cpu_instructions;
    core_machine_cpu_execution_context executor_cpu_execution;
    t_ram executor_memory;
    t_port executor_port;
    t_pic shared_pic_master;
    t_pic shared_pic_slave;
    core_machine_pic_irq_source shared_pit_irq0_source;
    t_pit shared_pit;
    t_latch shared_dma_latch;
    t_dma shared_dma_primary;
    t_dma shared_dma_secondary;
    t_kbc shared_kbc;
    t_vadp shared_vadp;
    const core_machine_execution_provider *execution_provider;
    C_VOID *execution_provider_context;
    C_INT execution_provider_frozen;
};

type_status core_machine_bus_initialize(core_machine *machine);
C_VOID core_machine_bus_finalize(core_machine *machine);
C_VOID core_machine_trace_initialize(core_machine *machine);
C_VOID core_machine_trace_finalize(core_machine *machine);
C_VOID core_machine_trace_record(
    core_machine *machine,
    core_machine_trace_event_type type,
    uint32_t address,
    uint32_t value,
    uint32_t detail);
C_VOID core_machine_cpu_diagnostic_initialize(core_machine *machine);
C_VOID core_machine_cpu_diagnostic_reset(core_machine *machine);
C_INT core_machine_configuration_is_open(const core_machine *machine);
#endif
