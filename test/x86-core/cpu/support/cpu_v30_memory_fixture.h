#ifndef TEST_CPU_V30_MEMORY_FIXTURE_H
#define TEST_CPU_V30_MEMORY_FIXTURE_H

#include "cpu_bus_fixture.h"
#include "cpu_v30_timing.h"
#include "lib/types/file.h"

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

#define CPU_V30_MEMORY_PLAN_CAPACITY 14u

typedef struct v30_memory_expected_transfer {
    lib_u32 address;
    lib_u16 value, initial;
    lib_u8 bytes;
    lib_bool write;
} v30_memory_expected_transfer;

typedef struct v30_memory_plan {
    v30_memory_expected_transfer transfers[CPU_V30_MEMORY_PLAN_CAPACITY];
    lib_u8 count;
} v30_memory_plan;

static inline void v30_memory_plan_transfer(v30_memory_plan *plan, const v30_memory_fixture *fixture,
    lib_u32 address, lib_u8 bytes, lib_bool write, lib_u16 value)
{
    for (lib_u8 offset = 0u; offset < bytes;) {
        const lib_u8 part = bytes-offset >= 2 && !((address+offset) & 1u) ? 2u : 1u;
        const lib_u16 initial = fixture->memory[address+offset] |
            (part == 2u ? 256u*fixture->memory[address+offset+1u] : 0u);
        plan->transfers[plan->count++] = (v30_memory_expected_transfer){address+offset,
            (lib_u16)(value >> (8u*offset)),initial,part,write};
        offset += part;
    }
}

static inline lib_i32 v30_memory_check_plan(const v30_memory_fixture *fixture,
    const v30_memory_plan *plan, lib_bool failed)
{
    lib_u32 reads = 0u, writes = 0u;
    lib_u8 used = 0u;
    lib_bool refused = LIB_FALSE;
    for (lib_u32 index = 0u; index < fixture->record_count; ++index) {
        const v30_memory_record *record = &fixture->records[index];
        if (record->observe || record->provenance != X86_CORE_CPU_MEMORY_ACCESS_DATA) continue;
        if (refused || used >= plan->count) return 1;
        const v30_memory_expected_transfer *transfer = &plan->transfers[used];
        const lib_u32 ordinal = transfer->write ? ++writes : ++reads;
        const lib_bool accepted = ordinal != (transfer->write ? fixture->fail_write : fixture->fail_read);
        if (record->address != transfer->address || record->bytes != transfer->bytes ||
            record->write != transfer->write || record->accepted != accepted ||
            fixture->phases[2u*used] != X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN ||
            fixture->phases[2u*used+1u] != (accepted ? X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT :
                X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL)) return 1;
        refused = !accepted;
        ++used;
    }
    if (fixture->invalid || fixture->reads != reads || fixture->writes != writes ||
        fixture->phase_count != 2u*used || refused != failed || (!failed && used != plan->count)) return 1;
    for (lib_u8 index = 0u; index < plan->count; ++index) {
        const v30_memory_expected_transfer *transfer = &plan->transfers[index];
        for (lib_u8 byte = 0u; byte < transfer->bytes; ++byte) {
            const lib_u32 address = transfer->address+byte;
            lib_u8 value = (lib_u8)(transfer->initial >> (8u*byte));
            for (lib_u8 write = 0u; write < used; ++write) {
                const v30_memory_expected_transfer *prior = &plan->transfers[write];
                if (prior->write && !(refused && write+1u == used) &&
                    address >= prior->address && address-prior->address < prior->bytes)
                    value = (lib_u8)(prior->value >> (8u*(address-prior->address)));
            }
            if (fixture->memory[address] != value) return 1;
        }
    }
    return 0;
}

/* Completion and event-only delivery are separate from clock allocation.
 * The public event query consumes its result, never the saved observation. */
static inline lib_i32 v30_memory_check_completion(v30_memory_fixture *fixture,
    lib_bool completed, lib_bool event_only)
{
    x86_core_cpu_execution_context *execution = &fixture->bus.execution;
    const lib_bool delivered = x86_core_cpu_execution_consume_instruction_fault_delivery(execution);
    const lib_bool consumed = x86_core_cpu_execution_consume_instruction_fault_delivery(execution);
    if (execution->completed_observation_valid != completed || delivered != event_only || consumed ||
        (!completed && x86_core_cpu_capture_timing(execution).ticks != 0u)) {
        lib_c_printf("V30:COMPLETION:FAIL completed=%u/%u event=%u/%u consumed=%u pc=%x halt=%u stop=%u\n",
            execution->completed_observation_valid,completed,delivered,event_only,consumed,
            fixture->bus.cpu.data.eip,fixture->bus.cpu.data.flagHalt,execution->stop_requested);
        return 1;
    }
    return 0;
}

/* Preserve the independent nominal oracle at the actual execution seam. */
static inline lib_i32 v30_memory_check_nominal(const v30_memory_fixture *fixture,
    lib_u64 ticks, lib_u32 key, lib_u32 inputs, x86_core_cpu_retirement_timing_origin origin)
{
    const x86_core_cpu_timing_result actual = x86_core_cpu_capture_timing(&fixture->bus.execution);
    if (v30_timing_check(&fixture->bus.execution,(x86_core_cpu_timing_result){
        .ticks=ticks,.key_id=key,.form_id=key,.formula_inputs=inputs,
        .retirement_origin=origin},
        fixture->bus.instructions.data.timing_fetch_bus_ticks) || fixture->observations) {
        lib_c_printf("V30:NOMINAL-CONTROL:FAIL clocks=%llu/%llu key=%x/%x inputs=%x/%x origin=%u/%u\n",
            actual.ticks,ticks,actual.key_id,key,actual.formula_inputs,inputs,actual.retirement_origin,origin);
        lib_c_printf("V30:NOMINAL-CONTROL:DECODE bytes=%u opcode=%02x/%02x/%02x oldflags=%x\n",
            fixture->bus.instructions.data.oplen,fixture->bus.instructions.data.opcodes[0],
            fixture->bus.instructions.data.opcodes[1],fixture->bus.instructions.data.opcodes[2],
            fixture->bus.instructions.data.oldcpu.data.eflags);
        return 1;
    }
    return 0;
}

#endif
