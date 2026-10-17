#include "support/cpu_v30_memory_fixture.h"
#include "lib/types/file.h"

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

static void v30_memory_fetch_code(v30_memory_fixture *fixture, lib_u32 base,
    lib_u16 start, const lib_u8 *code, lib_u8 bytes)
{
    v30_memory_prepare(fixture);
    fixture->bus.cpu.data.cs.base = base;
    fixture->bus.cpu.data.cs.selector = (lib_u16)(base / 16u);
    fixture->bus.cpu.data.eip = start;
    fixture->bus.cpu.data.ax = 0x1234u;
    fixture->bus.cpu.data.cx = 0x0101u;
    fixture->bus.cpu.data.dx = 0x0202u;
    fixture->bus.cpu.data.bp = 0x600u;
    fixture->bus.cpu.data.si = 0x400u;
    fixture->bus.cpu.data.di = 0x500u;
    lib_memory_copy(fixture->memory + 0x10400u,
        (const lib_u8[]){0x78u,0x56u,0x34u,0x12u}, 4u);
    const lib_u32 segments[] = {base,0x10000u,0x20000u,0x30000u};
    for (lib_size segment = 0u; segment < sizeof(segments) / sizeof(segments[0]); ++segment)
        lib_memory_copy(fixture->memory + (segments[segment] + 0x600u) % 0x100000u,
            (const lib_u8[]){0u,0u,0xffu,0x7fu}, 4u);
    fixture->memory[(base + 0x400u) % 0x100000u] = 0x34u;
    fixture->memory[(base + 0x401u) % 0x100000u] = 0x12u;
    for (lib_u8 item = 0u; item < bytes + 15u; ++item)
        fixture->memory[(base + (lib_u16)(start + item)) % 0x100000u] =
            item < bytes ? code[item] : 0x90u;
}

/* An actual bus record must fit the program segment, not its unwrapped alias. */
static lib_i32 v30_memory_fetch_records(const v30_memory_fixture *fixture,
    lib_u32 base)
{
    for (lib_u32 item = 0u; item < fixture->record_count; ++item) {
        const v30_memory_record *record = &fixture->records[item];
        if (record->provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA) continue;
        const lib_u32 offset = (record->address - base) & 0xfffffu;
        if (record->write || offset + record->bytes > 0x10000u) return 1;
    }
    return fixture->invalid;
}

static lib_i32 v30_memory_fetch_run(v30_memory_fixture *fixture, lib_u32 base,
    lib_u16 start, const lib_u8 *code, lib_u8 bytes)
{
    x86_core_cpu_instruction_lexeme lexeme;
    x86_core_cpu_instruction_observation observation;
    v30_memory_fetch_code(fixture, base, start, code, bytes);
    const t_cpu before = fixture->bus.cpu;
    if (!x86_core_cpu_execution_preview_lexeme(&fixture->bus.execution, &lexeme) ||
        lexeme.byte_count != bytes || fixture->phase_count || fixture->reads || fixture->writes ||
        fixture->fetches || !fixture->observations || fixture->bus.execution.prefetch_valid ||
        lib_memory_compare(&fixture->bus.cpu, &before, sizeof(before)) != 0 ||
        v30_memory_fetch_records(fixture, base)) return 1;
    fixture->record_count = fixture->observations = 0u;
    x86_core_cpu_execution_refresh(&fixture->bus.execution);
    x86_core_cpu_execution_copy_observation(&fixture->bus.execution, &observation);
    if (fixture->bus.execution.stop_requested || fixture->bus.faults ||
        fixture->bus.delivered_exceptions || fixture->bus.cpu.data.eip != (lib_u16)(start + bytes) ||
        observation.point.byte_count < bytes ||
        lib_memory_compare(observation.point.bytes, code, bytes) != 0 ||
        v30_memory_fetch_records(fixture, base)) return 1;
    return 0;
}

static lib_i32 v30_memory_fetch_forms(void)
{
    static const struct { lib_u8 code[6], bytes; } forms[] = {
        {{0x0fu,0x31u,0xc9u},3u}, {{0x0fu,0x33u,0xc9u},3u},
        {{0x0fu,0x39u,0xc1u,3u},4u}, {{0x0fu,0x3bu,0xc1u,3u},4u},
        {{0x0fu,0x10u,0xc0u},3u}, {{0x0fu,0x11u,0xc0u},3u},
        {{0x0fu,0x18u,0xc0u,3u},4u}, {{0x0fu,0x19u,0xc0u,3u},4u},
        {{0x0fu,0x28u,6u,0u,4u},5u}, {{0x0fu,0x2au,6u,0u,4u},5u},
        {{0xb8u,0x34u,0x12u},3u}, {{5u,0x34u,0x12u},3u},
        {{0x8bu,0x86u,0u,4u},4u}, {{0x8bu,0x46u,7u},3u},
        {{0xc7u,6u,0u,4u,0x34u,0x12u},6u}, {{0xc6u,0x86u,0u,4u,0x12u},5u},
        {{0xc1u,0x26u,0u,4u,3u},5u}, {{0x69u,0xc0u,3u,0u},4u},
        {{0x68u,0x34u,0x12u},3u}, {{0xc8u,4u,0u,0u},4u},
        {{0x8du,0x86u,0x34u,0x12u},4u}, {{0xa1u,0u,4u},3u},
        {{0xd1u,0x26u,0u,4u},4u}, {{0xb0u,3u},2u}, {{0x90u},1u},
        {{0x0fu,0x20u},2u}, {{0x0fu,0x22u},2u}, {{0x0fu,0x26u},2u},
        {{0x0fu,0x16u,6u,0u,4u},5u}, {{0x0fu,0x1eu,6u,0u,4u,3u},6u},
        {{0xebu,0u},2u}, {{0xe9u,0u,0u},3u}, {{0x74u,0u},2u}, {{0x75u,0u},2u},
        {{0x62u,6u,0u,6u},4u}, {{0x8fu,6u,0u,4u},4u},
        {{0xffu,6u,0u,4u},4u}, {{0xffu,0x36u,0u,4u},4u}
    };
    static const lib_u32 bases[] = {0x80000u,0u,0xffff0u};
    static const lib_u16 starts[] = {0xfffeu,0xffffu,0xfffdu,0xfffcu,0xfffbu,0xfffau,0xfff9u,0xfff8u,0x101u};
    static const lib_u8 prefixes[] = {0u,0x26u,0x2eu,0x36u,0x3eu};
    static v30_memory_fixture reference, fixture;
    for (lib_size base = 0u; base < sizeof(bases) / sizeof(bases[0]); ++base)
        for (lib_size form = 0u; form < sizeof(forms) / sizeof(forms[0]); ++form)
            for (lib_u8 prefix = 0u; prefix < sizeof(prefixes); ++prefix) {
                lib_u8 code[7];
                const lib_u8 extra = prefix != 0u;
                if (extra) code[0] = prefixes[prefix];
                lib_memory_copy(code + extra, forms[form].code, forms[form].bytes);
                const lib_u8 bytes = forms[form].bytes + extra;
                if (v30_memory_fetch_run(&reference, bases[base], 0x100u, code, bytes)) {
                    lib_c_printf("V30:FETCH-REFERENCE:%u:%u:%u\n",(lib_u32)base,(lib_u32)form,prefix);
                    return 1;
                }
                for (lib_size start = 0u; start < sizeof(starts) / sizeof(starts[0]); ++start) {
                    if (v30_memory_fetch_run(&fixture, bases[base], starts[start], code, bytes)) {
                        lib_c_printf("V30:FETCH-FORM:%u:%u:%u:%04x\n",(lib_u32)base,
                            (lib_u32)form,prefix,starts[start]);
                        return 1;
                    }
                    t_cpu expected = reference.bus.cpu;
                    expected.data.eip = (lib_u16)(starts[start] + bytes);
                    if (lib_memory_compare(&fixture.bus.cpu, &expected, sizeof(expected)) != 0 ||
                        lib_memory_compare(fixture.memory + 0x10000u, reference.memory + 0x10000u, 0x30000u) != 0 ||
                        fixture.memory[(bases[base] + 0x400u) % 0x100000u] !=
                            reference.memory[(bases[base] + 0x400u) % 0x100000u] ||
                        fixture.memory[(bases[base] + 0x401u) % 0x100000u] !=
                            reference.memory[(bases[base] + 0x401u) % 0x100000u]) return 1;
                }
            }
    return 0;
}

static lib_i32 v30_memory_fetch_cache_and_failures(void)
{
    static const lib_u32 bases[] = {0x80000u,0u,0xffff0u};
    static const lib_u8 code[] = {0x0fu,0x18u,0xc0u,3u};
    static v30_memory_fixture fixture;
    for (lib_size base = 0u; base < sizeof(bases) / sizeof(bases[0]); ++base) {
        v30_memory_fetch_code(&fixture, bases[base], 0xfffbu,
            (const lib_u8[]){0xb8u,0x34u,0x12u,0x0fu,0x18u,0xc0u,3u}, 7u);
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (fixture.bus.execution.stop_requested || fixture.bus.cpu.data.ax != 0x1234u ||
            fixture.bus.cpu.data.eip != 0xfffeu || !fixture.bus.execution.prefetch_valid ||
            fixture.bus.execution.prefetch_count != 5u) return 1;
        fixture.record_count = fixture.fetches = 0u;
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (fixture.bus.execution.stop_requested || fixture.bus.cpu.data.eip != 2u ||
            fixture.bus.cpu.data.ax != 0x1234u || !(fixture.bus.cpu.data.eflags & VCPU_EFLAGS_ZF) ||
            !fixture.fetches || v30_memory_fetch_records(&fixture, bases[base])) return 1;
        for (lib_u32 item = 0u; item < fixture.record_count; ++item)
            if (fixture.records[item].provenance == X86_CORE_CPU_MEMORY_ACCESS_INSTRUCTION_PREFETCH) return 1;
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (fixture.bus.execution.stop_requested || fixture.bus.cpu.data.eip != 3u ||
            v30_memory_fetch_records(&fixture, bases[base])) return 1;

        v30_memory_fetch_code(&fixture, bases[base], 0xfffeu, code, sizeof(code));
        x86_core_cpu_instruction_lexeme lexeme;
        if (!x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution, &lexeme)) return 1;
        const lib_u32 observation_parts = fixture.observations;
        if (observation_parts == 0u) return 1;
        fixture.record_count = fixture.observations = 0u;
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (fixture.bus.execution.stop_requested) return 1;
        const lib_u32 fetch_parts = fixture.fetches;
        if (fetch_parts == 0u) return 1;
        for (lib_u8 observe = 0u; observe < 2u; ++observe)
            for (lib_u32 part = 1u; part <= (observe ? observation_parts : fetch_parts); ++part) {
                v30_memory_fetch_code(&fixture, bases[base], 0xfffeu, code, sizeof(code));
                const t_cpu before = fixture.bus.cpu;
                if (observe) {
                    fixture.fail_observe = part;
                    if (x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution, &lexeme) ||
                        fixture.observations != part || fixture.bus.execution.stop_requested ||
                        fixture.bus.execution.prefetch_valid || fixture.phase_count || fixture.fetches ||
                        fixture.bus.faults) return 1;
                } else {
                    fixture.fail_fetch = part;
                    x86_core_cpu_execution_refresh(&fixture.bus.execution);
                    if (!fixture.bus.execution.stop_requested || !fixture.bus.faults ||
                        fixture.fetches != part) return 1;
                }
                if (fixture.reads || fixture.writes || fixture.bus.delivered_exceptions ||
                    fixture.bus.execution.memory_access_provenance != X86_CORE_CPU_MEMORY_ACCESS_DATA ||
                    lib_memory_compare(&fixture.bus.cpu, &before, sizeof(before)) != 0 ||
                    v30_memory_fetch_records(&fixture, bases[base])) return 1;
                for (lib_u32 item = 0u; item < fixture.record_count; ++item)
                    if (fixture.records[item].accepted != (item + 1u != part)) return 1;
            }
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
    if (v30_memory_fetch_forms()) { lib_c_printf("V30:FETCH-FORMS:FAIL\n"); return 1; }
    if (v30_memory_fetch_cache_and_failures()) { lib_c_printf("V30:FETCH-CACHE-FAILURES:FAIL\n"); return 1; }
    lib_c_printf("V30:MEMORY-TRANSFER:OK\n");
    return 0;
}
