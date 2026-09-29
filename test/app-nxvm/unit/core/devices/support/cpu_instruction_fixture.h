#ifndef TEST_CPU_INSTRUCTION_FIXTURE_H
#define TEST_CPU_INSTRUCTION_FIXTURE_H

#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/cpu_instructions.h"

/* CPU-owned instruction tests: real storage, no PC board or address aliases. */
typedef struct cpu_instruction_fixture {
    t_cpu cpu;
    t_cpuins instructions;
    core_machine_cpu_execution_context execution;
    core_machine_cpu_fault_snapshot fault;
    lib_u8 memory[65536];
} cpu_instruction_fixture;

static lib_status cpu_instruction_read(void *opaque, lib_u32 address,
    void *destination, lib_u8 bytes,
    core_machine_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch)
{
    cpu_instruction_fixture *fixture = (cpu_instruction_fixture *)opaque;

    (void)provenance;
    (void)observe_only;
    (void)reset_fetch;
    if (address > sizeof(fixture->memory) ||
        bytes > sizeof(fixture->memory) - address) return LIB_STATUS_IO_ERROR;
    lib_memory_copy(destination, fixture->memory + address, bytes);
    return LIB_STATUS_OK;
}

static lib_status cpu_instruction_write(void *opaque, lib_u32 address,
    const void *source, lib_u8 bytes,
    core_machine_cpu_memory_access_provenance provenance)
{
    cpu_instruction_fixture *fixture = (cpu_instruction_fixture *)opaque;

    (void)provenance;
    if (address > sizeof(fixture->memory) ||
        bytes > sizeof(fixture->memory) - address) return LIB_STATUS_IO_ERROR;
    lib_memory_copy(fixture->memory + address, source, bytes);
    return LIB_STATUS_OK;
}

static lib_bool cpu_instruction_interrupt_pending(void *opaque)
{
    (void)opaque;
    return LIB_FALSE;
}

static const core_machine_cpu_bus_provider cpu_instruction_bus = {
    .read_memory = cpu_instruction_read,
    .write_memory = cpu_instruction_write,
    .interrupt_pending = cpu_instruction_interrupt_pending
};

static void cpu_instruction_fault(void *opaque,
    const core_machine_cpu_fault_snapshot *fault)
{
    ((cpu_instruction_fixture *)opaque)->fault = *fault;
}

static const core_machine_cpu_execution_diagnostic_provider cpu_instruction_diagnostics = {
    .record_fault = cpu_instruction_fault
};

static void cpu_instruction_prepare(cpu_instruction_fixture *fixture,
    core_machine_cpu_profile profile)
{
    const core_machine_instruction_timing timing = { .base_ticks = 1u };

    lib_memory_set(fixture, 0, sizeof(*fixture));
    core_machine_cpu_execution_context_initialize(&fixture->execution,
        &fixture->cpu, &fixture->instructions, &cpu_instruction_bus, fixture);
    core_machine_cpu_execution_context_bind_profiles(&fixture->execution,
        profile, X86_FPU_PROFILE_NONE, LIB_FALSE, &timing);
    core_machine_cpu_state_initialize(&fixture->execution);
    core_machine_cpu_state_reset(&fixture->execution);
    core_machine_cpu_execution_context_bind_diagnostic_provider(
        &fixture->execution, &cpu_instruction_diagnostics, fixture);
    core_machine_cpu_execution_load_segment(&fixture->execution,
        &fixture->cpu.data.cs, 0u);
    core_machine_cpu_execution_load_segment(&fixture->execution,
        &fixture->cpu.data.ds, 0u);
    core_machine_cpu_execution_load_segment(&fixture->execution,
        &fixture->cpu.data.es, 0u);
    core_machine_cpu_execution_load_segment(&fixture->execution,
        &fixture->cpu.data.ss, 0u);
    fixture->cpu.data.eip = 0u;
}

#endif
