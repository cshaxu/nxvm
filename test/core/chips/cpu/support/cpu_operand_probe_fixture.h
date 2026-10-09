#ifndef TEST_CPU_OPERAND_PROBE_FIXTURE_H
#define TEST_CPU_OPERAND_PROBE_FIXTURE_H

#include "cpu_instruction_fixture.h"

typedef struct cpu_operand_probe_fixture {
    cpu_instruction_fixture instruction;
    lib_u32 operand_reads;
    lib_u32 operand_writes;
} cpu_operand_probe_fixture;

static lib_status cpu_operand_probe_read(void *opaque, lib_u32 address,
    void *destination, lib_u8 bytes,
    core_machine_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch)
{
    cpu_operand_probe_fixture *fixture = (cpu_operand_probe_fixture *)opaque;

    if (address == 0x5000u && !observe_only) ++fixture->operand_reads;
    return cpu_instruction_read(&fixture->instruction, address, destination,
        bytes, provenance, observe_only, reset_fetch);
}

static lib_status cpu_operand_probe_write(void *opaque, lib_u32 address,
    const void *source, lib_u8 bytes,
    core_machine_cpu_memory_access_provenance provenance)
{
    cpu_operand_probe_fixture *fixture = (cpu_operand_probe_fixture *)opaque;

    if (address == 0x5000u) ++fixture->operand_writes;
    return cpu_instruction_write(&fixture->instruction, address, source,
        bytes, provenance);
}

static const core_machine_cpu_bus_provider cpu_operand_probe_bus = {
    .read_memory = cpu_operand_probe_read,
    .write_memory = cpu_operand_probe_write,
    .interrupt_pending = cpu_instruction_interrupt_pending
};

static void cpu_operand_probe_prepare(cpu_operand_probe_fixture *fixture,
    core_machine_cpu_profile profile)
{
    cpu_instruction_prepare(&fixture->instruction, profile);
    fixture->operand_reads = 0u;
    fixture->operand_writes = 0u;
    fixture->instruction.execution.bus = &cpu_operand_probe_bus;
    fixture->instruction.execution.bus_context = fixture;
}

#endif
