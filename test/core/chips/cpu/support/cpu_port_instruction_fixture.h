#ifndef TEST_CPU_PORT_INSTRUCTION_FIXTURE_H
#define TEST_CPU_PORT_INSTRUCTION_FIXTURE_H

#include "cpu_instruction_fixture.h"

typedef struct cpu_port_transfer_record {
    lib_u16 port;
    lib_u8 bytes;
    lib_bool write;
    lib_u32 value;
} cpu_port_transfer_record;

typedef struct cpu_port_instruction_fixture {
    cpu_instruction_fixture instruction;
    cpu_port_transfer_record transfers[3];
    lib_u32 input_value;
    lib_u8 transfer_count;
    lib_u8 complete_count;
    lib_status fail_status;
} cpu_port_instruction_fixture;

static lib_status cpu_port_transfer(void *opaque, lib_u16 port, lib_u8 bytes,
    lib_bool write, lib_u32 *value)
{
    cpu_port_instruction_fixture *fixture =
        (cpu_port_instruction_fixture *)opaque;
    cpu_port_transfer_record *record;

    if (fixture->fail_status != LIB_STATUS_OK) return fixture->fail_status;
    if (fixture->transfer_count == 3u) return LIB_STATUS_LIMIT_EXCEEDED;
    if (!write) *value = fixture->input_value;
    record = &fixture->transfers[fixture->transfer_count++];
    record->port = port;
    record->bytes = bytes;
    record->write = write;
    record->value = *value;
    return LIB_STATUS_OK;
}

static void cpu_port_complete(void *opaque, lib_u16 port, lib_u8 bytes,
    lib_bool write)
{
    cpu_port_instruction_fixture *fixture =
        (cpu_port_instruction_fixture *)opaque;

    (void)port;
    (void)bytes;
    (void)write;
    ++fixture->complete_count;
}

static const core_machine_cpu_bus_provider cpu_port_bus = {
    .read_memory = cpu_instruction_read,
    .write_memory = cpu_instruction_write,
    .transfer_port = cpu_port_transfer,
    .complete_port = cpu_port_complete,
    .interrupt_pending = cpu_instruction_interrupt_pending
};

static void cpu_port_prepare(cpu_port_instruction_fixture *fixture,
    core_machine_cpu_profile profile)
{
    lib_memory_set(fixture, 0, sizeof(*fixture));
    cpu_instruction_prepare_with_bus(&fixture->instruction, profile,
        &cpu_port_bus, fixture);
}

#endif
