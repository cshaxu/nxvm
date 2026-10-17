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
    lib_u64 cycle_wait_ticks;
    lib_u8 cycle_begins;
    lib_u8 cycle_commits;
    lib_u8 cycle_cancels;
    lib_bool cycle_order_error;
} cpu_port_instruction_fixture;

static inline lib_u64 cpu_port_cycle_wait(void *opaque,
    x86_core_cpu_external_cycle_phase phase,
    x86_core_cpu_external_cycle_space space, lib_u32 address,
    lib_u8 bytes, lib_u8 write,
    x86_core_cpu_memory_access_provenance provenance)
{
    cpu_port_instruction_fixture *fixture = opaque;

    (void)address;
    (void)bytes;
    (void)write;
    (void)provenance;
    if (space == X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_PORT) {
        if (phase == X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN) {
            ++fixture->cycle_begins;
            fixture->cycle_order_error |= fixture->transfer_count != fixture->complete_count;
        } else if (phase == X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT) {
            ++fixture->cycle_commits;
            fixture->cycle_order_error |= fixture->complete_count != fixture->cycle_commits;
            return fixture->cycle_wait_ticks;
        } else if (phase == X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL) ++fixture->cycle_cancels;
    }
    return phase == X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT ? 0u : LIB_UINT64_MAX;
}

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

static const x86_core_cpu_bus_provider cpu_port_bus = {
    .read_memory = cpu_instruction_read,
    .write_memory = cpu_instruction_write,
    .transfer_port = cpu_port_transfer,
    .complete_port = cpu_port_complete,
    .interrupt_pending = cpu_instruction_interrupt_pending,
    .special_cycle = cpu_instruction_special_cycle,
    .cancel_special_cycle = cpu_instruction_cancel_special_cycle
};

/* Cached operand/permission tests leave low instruction bytes uncached. */
static inline lib_status cpu_port_cache_query(void *opaque, lib_u32 address,
    x86_core_cpu_cache_query query, x86_core_cpu_cache_memory *out_memory)
{
    (void)opaque;
    *out_memory = (x86_core_cpu_cache_memory){.address = address,
        .fill_enabled = query == X86_CORE_CPU_CACHE_FILL_PROBE && address >= 0x600u};
    return LIB_STATUS_OK;
}

static void cpu_port_prepare(cpu_port_instruction_fixture *fixture,
    x86_core_cpu_profile profile)
{
    lib_memory_set(fixture, 0, sizeof(*fixture));
    cpu_instruction_prepare_with_bus(&fixture->instruction, profile,
        &cpu_port_bus, fixture);
}

#endif
