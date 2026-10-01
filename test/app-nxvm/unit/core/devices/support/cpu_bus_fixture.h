#ifndef TEST_CPU_BUS_FIXTURE_H
#define TEST_CPU_BUS_FIXTURE_H

#include "lib/types/types_interface.h"
#include "x86/devices/cpu/cpu.h"
#include "x86/devices/cpu/cpu_instructions.h"

typedef struct cpu_bus_fixture {
    t_cpu cpu;
    t_cpuins instructions;
    core_machine_cpu_execution_context execution;
    lib_u8 memory[2048];
    lib_u32 last_address;
    lib_u32 port_value;
    lib_u32 observations;
    lib_u32 reset_fetches;
    lib_u32 completions;
    lib_u32 acknowledgements;
    core_machine_cpu_state write_cpu;
    lib_u32 writes;
    core_machine_cpu_fault_snapshot fault;
    lib_u32 faults;
    core_machine_cpu_instruction_observation instruction;
    lib_u32 instruction_count;
    lib_bool port_active;
    lib_bool fail_transfer;
    lib_bool fail_port;
    lib_bool fail_acknowledge;
    lib_bool interrupt;
    lib_bool failed;
} cpu_bus_fixture;

static lib_status cpu_bus_read(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, core_machine_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch)
{
    cpu_bus_fixture *fixture = (cpu_bus_fixture *)opaque;
    lib_u8 *output = (lib_u8 *)destination;

    (void)provenance;
    fixture->last_address = address;
    if (observe_only) ++fixture->observations;
    if (reset_fetch) ++fixture->reset_fetches;
    if (fixture->fail_transfer) return LIB_STATUS_IO_ERROR;
    for (lib_u8 index = 0u; index < bytes; ++index)
        output[index] = fixture->memory[(address + index) % sizeof(fixture->memory)];
    return LIB_STATUS_OK;
}

static lib_status cpu_bus_write(void *opaque, lib_u32 address, const void *source,
    lib_u8 bytes, core_machine_cpu_memory_access_provenance provenance)
{
    cpu_bus_fixture *fixture = (cpu_bus_fixture *)opaque;
    const lib_u8 *input = (const lib_u8 *)source;

    (void)provenance;
    fixture->last_address = address;
    if (fixture->fail_transfer) return LIB_STATUS_IO_ERROR;
    for (lib_u8 index = 0u; index < bytes; ++index)
        fixture->memory[(address + index) % sizeof(fixture->memory)] = input[index];
    core_machine_cpu_capture_state(&fixture->execution, &fixture->write_cpu);
    ++fixture->writes;
    return LIB_STATUS_OK;
}

static lib_status cpu_bus_port(void *opaque, lib_u16 port, lib_u8 bytes,
    lib_bool write, lib_u32 *value)
{
    cpu_bus_fixture *fixture = (cpu_bus_fixture *)opaque;

    fixture->failed |= port != 0x1234u || fixture->port_active ||
        (bytes != 1u && bytes != 2u && bytes != 4u);
    if (fixture->fail_port) return LIB_STATUS_IO_ERROR;
    fixture->port_active = LIB_TRUE;
    if (write) fixture->port_value = *value;
    else *value = fixture->port_value;
    return LIB_STATUS_OK;
}

static void cpu_bus_complete(void *opaque, lib_u16 port, lib_u8 bytes,
    lib_bool write)
{
    cpu_bus_fixture *fixture = (cpu_bus_fixture *)opaque;
    lib_u32 mask = bytes == 1u ? 0xffu : bytes == 2u ? 0xffffu : LIB_UINT32_MAX;

    fixture->failed |= port != 0x1234u || !fixture->port_active ||
        !fixture->instructions.data.flagIgnore;
    if (!write)
        fixture->failed |= (fixture->cpu.data.eax & mask) != (fixture->port_value & mask);
    fixture->port_active = LIB_FALSE;
    ++fixture->completions;
}

static lib_bool cpu_bus_pending(void *opaque)
{
    return ((cpu_bus_fixture *)opaque)->interrupt;
}

static lib_status cpu_bus_acknowledge(void *opaque, lib_u8 *vector)
{
    cpu_bus_fixture *fixture = (cpu_bus_fixture *)opaque;

    if (fixture->fail_acknowledge) return LIB_STATUS_IO_ERROR;
    *vector = 0x30u;
    fixture->interrupt = LIB_FALSE;
    ++fixture->acknowledgements;
    return LIB_STATUS_OK;
}

static void cpu_bus_extension(void *opaque, lib_u8 opcode, lib_u8 modrm)
{
    cpu_bus_fixture *fixture = (cpu_bus_fixture *)opaque;

    (void)opcode;
    (void)modrm;
    fixture->failed = LIB_TRUE;
}

static const core_machine_cpu_bus_provider cpu_bus_provider = {
    .read_memory = cpu_bus_read,
    .write_memory = cpu_bus_write,
    .transfer_port = cpu_bus_port,
    .complete_port = cpu_bus_complete,
    .interrupt_pending = cpu_bus_pending,
    .acknowledge_interrupt = cpu_bus_acknowledge,
    .extension_command = cpu_bus_extension
};

static void cpu_bus_fault(void *opaque,
    const core_machine_cpu_fault_snapshot *snapshot)
{
    cpu_bus_fixture *fixture = (cpu_bus_fixture *)opaque;

    fixture->fault = *snapshot;
    ++fixture->faults;
}

static void cpu_bus_instruction(void *opaque,
    const core_machine_cpu_instruction_observation *observation)
{
    cpu_bus_fixture *fixture = (cpu_bus_fixture *)opaque;

    fixture->instruction = *observation;
    ++fixture->instruction_count;
}

static const core_machine_cpu_execution_diagnostic_provider cpu_bus_diagnostics = {
    .record_instruction = cpu_bus_instruction,
    .record_fault = cpu_bus_fault
};

static void cpu_bus_prepare(cpu_bus_fixture *fixture,
    core_machine_cpu_profile profile)
{
    const core_machine_instruction_timing timing = { .base_ticks = 1u };

    lib_memory_set(fixture, 0, sizeof(*fixture));
    core_machine_cpu_execution_context_initialize(&fixture->execution,
        &fixture->cpu, &fixture->instructions, &cpu_bus_provider, fixture);
    core_machine_cpu_execution_context_bind_profiles(&fixture->execution,
        profile, X86_FPU_PROFILE_NONE, LIB_FALSE, &timing);
    core_machine_cpu_state_initialize(&fixture->execution);
    core_machine_cpu_state_reset(&fixture->execution);
    core_machine_cpu_execution_context_bind_diagnostic_provider(
        &fixture->execution, &cpu_bus_diagnostics, fixture);
    fixture->cpu.data.cs.selector = 0u;
    fixture->cpu.data.cs.base = 0u;
    fixture->cpu.data.eip = 0x100u;
    fixture->cpu.data.sp = 0x700u;
    fixture->cpu.data.dx = 0x1234u;
    fixture->cpu.data.eax = 0x76543210u;
    fixture->port_value = 0x12345678u;
}

#endif
