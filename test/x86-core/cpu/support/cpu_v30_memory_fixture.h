#ifndef TEST_CPU_V30_MEMORY_FIXTURE_H
#define TEST_CPU_V30_MEMORY_FIXTURE_H

#include "cpu_bus_fixture.h"

#define CPU_V30_MEMORY_RECORD_CAPACITY 2048u

typedef struct v30_memory_record {
    lib_u32 address;
    lib_u8 bytes;
    lib_bool write, observe, accepted;
    x86_core_cpu_memory_access_provenance provenance;
} v30_memory_record;

typedef struct v30_memory_fixture {
    cpu_bus_fixture bus;
    lib_u8 memory[0x100000u];
    v30_memory_record records[CPU_V30_MEMORY_RECORD_CAPACITY];
    x86_core_cpu_external_cycle_phase phases[2u * CPU_V30_MEMORY_RECORD_CAPACITY];
    lib_u32 record_count, phase_count, reads, writes, fetches, observations;
    lib_u32 fail_read, fail_write, fail_fetch, fail_observe;
    lib_bool invalid;
} v30_memory_fixture;

static lib_status v30_memory_transfer(v30_memory_fixture *fixture, lib_u32 address,
    void *data, lib_u8 bytes, lib_bool write, lib_bool observe,
    x86_core_cpu_memory_access_provenance provenance)
{
    const lib_bool refused = observe ? ++fixture->observations == fixture->fail_observe :
        write ? ++fixture->writes == fixture->fail_write :
        provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA ? ++fixture->reads == fixture->fail_read :
        ++fixture->fetches == fixture->fail_fetch;
    if (fixture->record_count >= CPU_V30_MEMORY_RECORD_CAPACITY || bytes == 0u || bytes > 2u ||
        (bytes == 2u && (address & 1u)) || address + bytes > sizeof(fixture->memory)) {
        fixture->invalid = LIB_TRUE;
        return LIB_STATUS_IO_ERROR;
    }
    fixture->records[fixture->record_count++] = (v30_memory_record){
        address, bytes, write, observe, !refused, provenance};
    if (refused) return LIB_STATUS_IO_ERROR;
    if (write) lib_memory_copy(fixture->memory + address, data, bytes);
    else lib_memory_copy(data, fixture->memory + address, bytes);
    return LIB_STATUS_OK;
}

static lib_status v30_memory_read(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    (void)reset_fetch;
    (void)cache_control;
    return v30_memory_transfer(opaque, address, destination, bytes,
        LIB_FALSE, observe_only, provenance);
}

static lib_status v30_memory_write(void *opaque, lib_u32 address, const void *source,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance, lib_u8 cache_control)
{
    (void)cache_control;
    return v30_memory_transfer(opaque, address, (void *)source, bytes,
        LIB_TRUE, LIB_FALSE, provenance);
}

static lib_u64 v30_memory_cycle(void *opaque, x86_core_cpu_external_cycle_phase phase,
    x86_core_cpu_external_cycle_space space, lib_u32 address, lib_u8 bytes,
    lib_u8 write, x86_core_cpu_memory_access_provenance provenance)
{
    v30_memory_fixture *fixture = opaque;
    (void)address;
    (void)bytes;
    (void)write;
    if (space != X86_CORE_CPU_EXTERNAL_CYCLE_SPACE_MEMORY) fixture->invalid = LIB_TRUE;
    if (provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA) {
        if (fixture->phase_count >= 2u * CPU_V30_MEMORY_RECORD_CAPACITY) fixture->invalid = LIB_TRUE;
        else fixture->phases[fixture->phase_count++] = phase;
    }
    return 0u;
}

/* Restart CPU/probes without clearing RAM. Reusing callers must initialize
 * every code/operand byte; unexpected transfers remain visible in records. */
static void v30_memory_restart(v30_memory_fixture *fixture)
{
    static const x86_core_cpu_bus_provider provider = {
        .read_memory = v30_memory_read, .write_memory = v30_memory_write,
        .interrupt_pending = cpu_bus_pending
    };
    lib_memory_set(&fixture->record_count, 0,
        sizeof(*fixture) - lib_offsetof(v30_memory_fixture, record_count));
    cpu_bus_prepare(&fixture->bus, X86_CORE_CPU_PROFILE_V30);
    fixture->bus.execution.bus = &provider;
    fixture->bus.execution.bus_context = fixture;
    fixture->bus.cpu.data.cs.base = 0x80000u;
    fixture->bus.cpu.data.cs.selector = 0x8000u;
    fixture->bus.cpu.data.ds.base = 0x10000u;
    fixture->bus.cpu.data.ds.selector = 0x1000u;
    fixture->bus.cpu.data.ss.base = 0x20000u;
    fixture->bus.cpu.data.ss.selector = 0x2000u;
    fixture->bus.cpu.data.es.base = 0x30000u;
    fixture->bus.cpu.data.es.selector = 0x3000u;
    fixture->bus.cpu.data.ax = 0x9a78u;
    x86_core_cpu_execution_context_bind_external_cycle_provider(&fixture->bus.execution,
        v30_memory_cycle, fixture);
}

static inline void v30_memory_prepare(v30_memory_fixture *fixture)
{
    lib_memory_set(fixture, 0, sizeof(*fixture));
    v30_memory_restart(fixture);
}

#endif
