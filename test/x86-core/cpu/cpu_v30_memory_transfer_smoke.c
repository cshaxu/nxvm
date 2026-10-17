#include "support/cpu_bus_fixture.h"
#include "lib/types/file.h"

typedef struct v30_memory_record {
    lib_u32 address;
    lib_u8 bytes;
    lib_bool write, observe, accepted;
    x86_core_cpu_memory_access_provenance provenance;
} v30_memory_record;

typedef struct v30_memory_fixture {
    cpu_bus_fixture bus;
    lib_u8 memory[0x100000u];
    v30_memory_record records[64];
    x86_core_cpu_external_cycle_phase phases[128];
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
    if (fixture->record_count >= 64u || bytes == 0u || bytes > 2u ||
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
        if (fixture->phase_count >= 128u) fixture->invalid = LIB_TRUE;
        else fixture->phases[fixture->phase_count++] = phase;
    }
    return 0u;
}

static void v30_memory_prepare(v30_memory_fixture *fixture)
{
    static const x86_core_cpu_bus_provider provider = {
        .read_memory = v30_memory_read, .write_memory = v30_memory_write,
        .interrupt_pending = cpu_bus_pending
    };
    lib_memory_set(fixture, 0, sizeof(*fixture));
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

static lib_i32 v30_memory_data_records(const v30_memory_fixture *fixture,
    lib_u32 start, lib_u8 bytes, lib_bool write, lib_u32 failed_part)
{
    lib_u8 done = 0u;
    lib_u32 part = 0u;
    for (lib_u32 item = 0u; item < fixture->record_count; ++item) {
        const v30_memory_record *record = &fixture->records[item];
        if (record->observe || record->provenance != X86_CORE_CPU_MEMORY_ACCESS_DATA) continue;
        const lib_u32 address = (start + done) % 0x100000u;
        const lib_u8 width = done + 2u <= bytes && (address % 2u) == 0u ? 2u : 1u;
        ++part;
        if (done >= bytes || record->address != address || record->bytes != width ||
            record->write != write || record->accepted != (part != failed_part) ||
            fixture->phases[(part - 1u) * 2u] != X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN ||
            fixture->phases[(part - 1u) * 2u + 1u] != (part == failed_part ?
                X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL : X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT)) return 1;
        if (part == failed_part) break;
        done += width;
    }
    return fixture->invalid || fixture->phase_count != part * 2u ||
        fixture->reads + fixture->writes != part ||
        (failed_part ? part != failed_part : done != bytes);
}

static lib_i32 v30_memory_scalars(void)
{
    static v30_memory_fixture fixture;
    static const lib_u8 prefixes[] = {0u,0x26u,0x2eu,0x36u,0x3eu};
    static const lib_u16 offsets[] = {0x400u,0x401u,0xfffeu,0xffffu};
    for (lib_size prefix = 0u; prefix < sizeof(prefixes); ++prefix)
        for (lib_size item = 0u; item < sizeof(offsets)/sizeof(offsets[0]); ++item)
            for (lib_u8 width = 1u; width <= 2u; ++width)
                for (lib_u8 write = 0u; write < 2u; ++write) {
                    lib_u8 code[4], size = 0u;
                    v30_memory_prepare(&fixture);
                    const lib_u32 base = prefix == 1u ? 0x30000u : prefix == 2u ?
                        0x80000u : prefix == 3u ? 0x20000u : 0x10000u;
                    const lib_u32 address = base + offsets[item];
                    fixture.memory[address] = 0x6du;
                    fixture.memory[address + 1u] = 0xc2u;
                    fixture.memory[base] = 0xa5u; /* Distinguishes segment wrap from linear carry. */
                    if (prefixes[prefix]) code[size++] = prefixes[prefix];
                    code[size++] = (lib_u8)(0xa0u + (write ? 2u : 0u) + width - 1u);
                    code[size++] = (lib_u8)offsets[item];
                    code[size++] = (lib_u8)(offsets[item] >> 8u);
                    lib_memory_copy(fixture.memory + 0x80100u, code, size);
                    x86_core_cpu_execution_refresh(&fixture.bus.execution);
                    if (fixture.bus.execution.stop_requested || fixture.bus.delivered_exceptions ||
                        fixture.bus.cpu.data.eip != 0x100u + size ||
                        fixture.bus.cpu.data.ax != (write ? 0x9a78u : width == 1u ? 0x9a6du : 0xc26du) ||
                        fixture.memory[address] != (write ? 0x78u : 0x6du) ||
                        fixture.memory[address + 1u] != (write && width == 2u ? 0x9au : 0xc2u) ||
                        fixture.memory[base] != 0xa5u ||
                        v30_memory_data_records(&fixture, address, width, write != 0u, 0u)) return 1;
                }
    return 0;
}

static lib_i32 v30_memory_linear(void)
{
    static v30_memory_fixture fixture;
    static const lib_u32 starts[] = {0x400u,0x401u,0xfffffu,0x1fffffu};
    for (lib_size item = 0u; item < sizeof(starts)/sizeof(starts[0]); ++item)
        for (lib_u8 bytes = 1u; bytes <= 6u; ++bytes)
            for (lib_u8 write = 0u; write < 2u; ++write) {
                lib_u8 data[6] = {0x12u,0x34u,0x56u,0x78u,0x9au,0xbcu};
                v30_memory_prepare(&fixture);
                for (lib_u8 index = 0u; index < bytes; ++index)
                    fixture.memory[(starts[item] + index) % 0x100000u] = (lib_u8)(0xe0u + index);
                const lib_i32 status = write ? x86_core_cpu_write_linear(&fixture.bus.execution,
                    starts[item], data, bytes) : x86_core_cpu_read_linear(&fixture.bus.execution,
                    starts[item], data, bytes);
                if (status || v30_memory_data_records(&fixture, starts[item], bytes, write != 0u, 0u)) return 1;
                for (lib_u8 index = 0u; index < bytes; ++index)
                    if ((write ? fixture.memory[(starts[item] + index) % 0x100000u] :
                        data[index]) != (write ? data[index] : 0xe0u + index)) return 1;
            }
    return 0;
}

static lib_i32 v30_memory_pair_and_stack(void)
{
    static v30_memory_fixture fixture;
    static const lib_u8 load_pair[] = {0xc5u,0x06u,0xffu,0xffu};
    v30_memory_prepare(&fixture);
    lib_memory_copy(fixture.memory + 0x1ffffu, (const lib_u8[]){0x78u,0x56u,0x34u,0x12u}, 4u);
    lib_memory_copy(fixture.memory + 0x80100u, load_pair, sizeof(load_pair));
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    if (fixture.bus.execution.stop_requested || fixture.bus.cpu.data.ax != 0x5678u ||
        fixture.bus.cpu.data.ds.selector != 0x1234u || fixture.bus.cpu.data.ds.base != 0x12340u ||
        fixture.reads != 4u || fixture.writes || fixture.invalid) return 1;
    for (lib_u32 item = 0u, data = 0u; item < fixture.record_count; ++item) {
        const v30_memory_record *record = &fixture.records[item];
        if (record->provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA) {
            if (record->address != 0x1ffffu + data || record->bytes != 1u) return 1;
            ++data;
        }
    }
    v30_memory_prepare(&fixture);
    fixture.bus.cpu.data.sp = 1u;
    fixture.memory[0x80100u] = 0x50u; /* PUSH AX */
    fixture.memory[0x80101u] = 0x5bu; /* POP BX */
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    if (fixture.bus.execution.stop_requested || fixture.bus.cpu.data.sp != 0xffffu ||
        fixture.memory[0x2ffffu] != 0x78u || fixture.memory[0x30000u] != 0x9au ||
        v30_memory_data_records(&fixture, 0x2ffffu, 2u, LIB_TRUE, 0u)) return 1;
    fixture.record_count = fixture.phase_count = fixture.reads = fixture.writes = 0u;
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    return fixture.bus.execution.stop_requested || fixture.bus.cpu.data.sp != 1u ||
        fixture.bus.cpu.data.bx != 0x9a78u ||
        v30_memory_data_records(&fixture, 0x2ffffu, 2u, LIB_FALSE, 0u);
}

static lib_i32 v30_memory_fetch(void)
{
    static v30_memory_fixture fixture;
    x86_core_cpu_instruction_lexeme lexeme;
    v30_memory_prepare(&fixture);
    fixture.bus.cpu.data.eip = 0xfffeu;
    fixture.memory[0x8fffeu] = 0xb8u;
    fixture.memory[0x8ffffu] = 0x34u;
    fixture.memory[0x80000u] = 0x12u;
    fixture.memory[0x90000u] = 0xa5u;
    const t_cpu before = fixture.bus.cpu;
    if (!x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution, &lexeme) ||
        lexeme.byte_count != 3u || fixture.phase_count || fixture.reads || fixture.writes ||
        lib_memory_compare(&fixture.bus.cpu, &before, sizeof(before)) != 0) return 1;
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    if (fixture.bus.execution.stop_requested || fixture.bus.cpu.data.ax != 0x1234u ||
        fixture.bus.cpu.data.eip != 1u || fixture.invalid || fixture.reads || fixture.writes) return 1;
    v30_memory_prepare(&fixture);
    fixture.bus.cpu.data.eip = 0x101u;
    fixture.memory[0x80101u] = 0x90u;
    if (!x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution, &lexeme) ||
        fixture.record_count != 3u || fixture.observations != 3u || fixture.phase_count ||
        fixture.records[0].bytes != 1u || fixture.records[1].bytes != 2u ||
        fixture.records[2].bytes != 2u || fixture.bus.execution.prefetch_valid) return 1;
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    return fixture.bus.execution.stop_requested || fixture.bus.cpu.data.eip != 0x102u ||
        fixture.bus.execution.prefetch_count != 5u || fixture.fetches != 3u || fixture.invalid;
}

static lib_i32 v30_memory_failures(void)
{
    static v30_memory_fixture fixture;
    for (lib_u8 write = 0u; write < 2u; ++write)
        for (lib_u32 stage = 1u; stage <= 4u; ++stage) {
            lib_u8 data[6] = {0x12u,0x34u,0x56u,0x78u,0x9au,0xbcu};
            v30_memory_prepare(&fixture);
            fixture.fail_read = write ? 0u : stage;
            fixture.fail_write = write ? stage : 0u;
            lib_memory_set(fixture.memory + 0x401u, 0xe0u, 6u);
            if (!(write ? x86_core_cpu_write_linear(&fixture.bus.execution, 0x401u, data, 6u) :
                x86_core_cpu_read_linear(&fixture.bus.execution, 0x401u, data, 6u)) ||
                v30_memory_data_records(&fixture, 0x401u, 6u, write != 0u, stage)) return 1;
            const lib_u8 accepted = stage == 1u ? 0u : (lib_u8)(stage * 2u - 3u);
            for (lib_u8 index = 0u; index < 6u; ++index)
                if (write ? fixture.memory[0x401u + index] != (index < accepted ? data[index] : 0xe0u) :
                    data[index] != (index < accepted ? 0xe0u : 0x12u + index * 0x22u)) return 1;
        }
    for (lib_u8 kind = 0u; kind < 3u; ++kind)
        for (lib_u32 stage = 1u; stage <= 2u; ++stage) {
            const lib_bool write = kind != 0u;
            const lib_u32 address = kind == 2u ? 0x2ffffu : 0x1ffffu;
            const lib_u8 code[] = {kind == 2u ? 0x50u : write ? 0xa3u : 0xa1u,0xffu,0xffu};
            v30_memory_prepare(&fixture);
            if (kind == 2u) fixture.bus.cpu.data.sp = 1u;
            fixture.fail_read = write ? 0u : stage;
            fixture.fail_write = write ? stage : 0u;
            fixture.memory[address] = fixture.memory[address + 1u] = 0xe0u;
            lib_memory_copy(fixture.memory + 0x80100u, code, sizeof(code));
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (!fixture.bus.execution.stop_requested || fixture.bus.delivered_exceptions ||
                fixture.bus.cpu.data.eip != 0x100u || fixture.bus.cpu.data.ax != 0x9a78u ||
                fixture.bus.cpu.data.sp != (kind == 2u ? 1u : 0x700u) ||
                fixture.memory[address] != (write && stage == 2u ? 0x78u : 0xe0u) ||
                fixture.memory[address + 1u] != 0xe0u ||
                v30_memory_data_records(&fixture, address, 2u, write, stage)) return 1;
        }
    for (lib_u32 stage = 1u; stage <= 4u; ++stage) {
        const lib_u8 code[] = {0xc5u,0x06u,0xffu,0xffu};
        v30_memory_prepare(&fixture);
        fixture.fail_read = stage;
        lib_memory_copy(fixture.memory + 0x1ffffu, (const lib_u8[]){0x78u,0x56u,0x34u,0x12u}, 4u);
        lib_memory_copy(fixture.memory + 0x80100u, code, sizeof(code));
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (!fixture.bus.execution.stop_requested || fixture.bus.delivered_exceptions ||
            fixture.bus.cpu.data.eip != 0x100u || fixture.bus.cpu.data.ax != 0x9a78u ||
            fixture.bus.cpu.data.ds.selector != 0x1000u || fixture.bus.cpu.data.ds.base != 0x10000u ||
            fixture.reads != stage || fixture.writes || fixture.phase_count != stage * 2u ||
            fixture.invalid) return 1;
        for (lib_u32 item = 0u; item < stage; ++item)
            if (fixture.phases[item * 2u] != X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_BEGIN ||
                fixture.phases[item * 2u + 1u] != (item + 1u == stage ?
                    X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_CANCEL : X86_CORE_CPU_EXTERNAL_CYCLE_PHASE_COMMIT)) return 1;
    }
    for (lib_u32 stage = 1u; stage <= 3u; ++stage) {
        v30_memory_prepare(&fixture);
        fixture.bus.cpu.data.eip = 0x101u;
        fixture.memory[0x80101u] = 0x90u;
        fixture.fail_fetch = stage;
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (!fixture.bus.execution.stop_requested || fixture.bus.cpu.data.eip != 0x101u ||
            fixture.bus.cpu.data.ax != 0x9a78u || fixture.bus.execution.prefetch_valid ||
            fixture.fetches != stage || fixture.invalid) return 1;
        v30_memory_prepare(&fixture);
        fixture.bus.cpu.data.eip = 0x101u;
        fixture.memory[0x80101u] = 0x90u;
        fixture.fail_observe = stage;
        const t_cpu before = fixture.bus.cpu;
        x86_core_cpu_instruction_lexeme lexeme;
        if (x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution, &lexeme) ||
            fixture.observations != stage || fixture.phase_count || fixture.bus.execution.stop_requested ||
            lib_memory_compare(&fixture.bus.cpu, &before, sizeof(before)) != 0) return 1;
    }
    return 0;
}

lib_i32 main(void)
{
    if (v30_memory_scalars()) { lib_c_printf("V30:MEMORY-SCALAR:FAIL\n"); return 1; }
    if (v30_memory_linear()) { lib_c_printf("V30:MEMORY-LINEAR:FAIL\n"); return 1; }
    if (v30_memory_pair_and_stack()) { lib_c_printf("V30:MEMORY-PAIR-STACK:FAIL\n"); return 1; }
    if (v30_memory_fetch()) { lib_c_printf("V30:MEMORY-FETCH:FAIL\n"); return 1; }
    if (v30_memory_failures()) { lib_c_printf("V30:MEMORY-FAILURES:FAIL\n"); return 1; }
    lib_c_printf("V30:MEMORY-TRANSFER:OK\n");
    return 0;
}
