#include "support/cpu_bus_fixture.h"
#include "lib/types/file.h"
#include "support/cpu_v30_string_timing.h"

typedef struct v30_block_record {
    lib_u32 address;
    lib_u16 value;
    lib_u8 bytes;
    lib_bool write, accepted;
} v30_block_record;

typedef struct v30_block_fixture {
    cpu_bus_fixture bus;
    lib_u8 memory[0x100000u], expected[0x100000u];
    v30_block_record records[32];
    lib_u32 count, reads, writes, fail_read, fail_write;
    lib_bool invalid;
} v30_block_fixture;

static lib_status v30_block_transfer(v30_block_fixture *fixture, lib_u32 address,
    void *data, lib_u8 bytes, lib_bool write, lib_bool observed)
{
    if (address >= sizeof(fixture->memory) || bytes > sizeof(fixture->memory) - address ||
        bytes == 0u || bytes > 2u || (bytes == 2u && (address & 1u))) {
        fixture->invalid = LIB_TRUE;
        return LIB_STATUS_IO_ERROR;
    }
    lib_bool accepted = LIB_TRUE;
    if (observed) {
        if (fixture->count >= 32u) { fixture->invalid = LIB_TRUE; return LIB_STATUS_IO_ERROR; }
        accepted = write ? ++fixture->writes != fixture->fail_write : ++fixture->reads != fixture->fail_read;
        lib_u16 value = 0u;
        lib_memory_copy(&value, write ? data : fixture->memory + address, bytes);
        fixture->records[fixture->count++] = (v30_block_record){address,value,bytes,write,accepted};
    }
    if (!accepted) return LIB_STATUS_IO_ERROR;
    if (write) lib_memory_copy(fixture->memory + address, data, bytes);
    else lib_memory_copy(data, fixture->memory + address, bytes);
    return LIB_STATUS_OK;
}

static lib_status v30_block_read(void *opaque, lib_u32 address, void *data,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe, lib_bool reset_fetch, lib_u8 cache_control)
{
    (void)reset_fetch;
    (void)cache_control;
    if (observe) ++((v30_block_fixture *)opaque)->bus.observations;
    return v30_block_transfer(opaque, address, data, bytes, LIB_FALSE,
        !observe && provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA);
}

static lib_status v30_block_write(void *opaque, lib_u32 address, const void *data,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance, lib_u8 cache_control)
{
    (void)provenance;
    (void)cache_control;
    return v30_block_transfer(opaque, address, (void *)data, bytes, LIB_TRUE, LIB_TRUE);
}

static void v30_block_prepare(v30_block_fixture *fixture)
{
    static const x86_core_cpu_bus_provider provider = {
        .read_memory = v30_block_read, .write_memory = v30_block_write,
        .interrupt_pending = cpu_bus_pending, .acknowledge_interrupt = cpu_bus_acknowledge
    };
    lib_memory_set(fixture, 0, sizeof(*fixture));
    cpu_bus_prepare(&fixture->bus, X86_CORE_CPU_PROFILE_V30);
    fixture->bus.execution.bus = &provider;
    fixture->bus.execution.bus_context = fixture;
    fixture->bus.cpu.data.cs.base = 0x80000u;
    fixture->bus.cpu.data.cs.selector = 0x8000u;
    fixture->bus.cpu.data.ds.base = 0x10000u;
    fixture->bus.cpu.data.ss.base = 0x20000u;
    fixture->bus.cpu.data.es.base = 0x30000u;
    fixture->bus.cpu.data.esi = 0xa55a0400u;
    fixture->bus.cpu.data.edi = 0x5aa00600u;
    fixture->bus.cpu.data.ecx = 0x76540003u;
    fixture->bus.cpu.data.eax = 0x12349876u;
    fixture->bus.cpu.data.eflags |= VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF |
        VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF;
    for (lib_u32 item = 0u; item < sizeof(fixture->memory); ++item)
        fixture->memory[item] = (lib_u8)(item ^ (item >> 8u) ^ (item >> 16u));
}

static void v30_block_expected(v30_block_fixture *fixture, t_cpu *cpu, lib_u8 opcode,
    lib_u32 source_base)
{
    const lib_u8 width = (opcode & 1u) + 1u;
    const lib_bool store = (opcode & 0xfeu) == 0xaau;
    const lib_bool load = (opcode & 0xfeu) == 0xacu;
    lib_u16 value = 0u;
    for (lib_u8 byte = 0u; byte < width; ++byte)
        value |= (lib_u16)(store ? (lib_u8)(cpu->data.ax >> (byte * 8u)) :
            fixture->expected[(source_base + cpu->data.si + byte) % 0x100000u]) << (byte * 8u);
    if (load) {
        if (width == 1u) cpu->data.al = (lib_u8)value;
        else cpu->data.ax = value;
    } else for (lib_u8 byte = 0u; byte < width; ++byte)
        fixture->expected[(cpu->data.es.base + cpu->data.di + byte) % 0x100000u] =
            (lib_u8)(value >> (byte * 8u));
    const lib_u16 delta = (cpu->data.eflags & VCPU_EFLAGS_DF) ? (lib_u16)(0u - width) : width;
    if (!store) cpu->data.si += delta;
    if (!load) cpu->data.di += delta;
}

static lib_i32 v30_block_addresses(const v30_block_fixture *fixture, lib_u32 first,
    lib_u32 source, lib_u32 destination, lib_u8 opcode)
{
    const lib_u8 width = (opcode & 1u) + 1u;
    const lib_bool store = (opcode & 0xfeu) == 0xaau;
    const lib_bool load = (opcode & 0xfeu) == 0xacu;
    lib_u32 cursor = first;
    for (lib_u8 side = 0u; side < 2u; ++side) {
        if ((side == 0u && store) || (side == 1u && load)) continue;
        for (lib_u8 byte = 0u; byte < width;) {
            const lib_u32 address = ((side ? destination : source) + byte) % 0x100000u;
            const lib_u8 part = (address % 2u) == 0u && byte + 1u < width ? 2u : 1u;
            if (cursor >= fixture->count) return 1;
            const v30_block_record *record = &fixture->records[cursor++];
            if (record->address != address || record->bytes != part ||
                record->write != (side != 0u) || !record->accepted) return 1;
            byte += part;
        }
    }
    return fixture->invalid || cursor != fixture->count;
}

static lib_i32 v30_block_cases(void)
{
    static const lib_u8 opcodes[] = {0xa4u,0xa5u,0xaau,0xabu,0xacu,0xadu};
    static const struct { lib_u8 code[2], bytes, segment; lib_bool lock; } prefixes[] = {
        {{0u},0u,0u,LIB_FALSE},
        {{0x26u},1u,1u,LIB_FALSE}, {{0x2eu},1u,2u,LIB_FALSE},
        {{0x36u},1u,3u,LIB_FALSE}, {{0x3eu},1u,4u,LIB_FALSE},
        {{0xf0u},1u,0u,LIB_TRUE},
        {{0xf0u,0x26u},2u,1u,LIB_TRUE}, {{0x26u,0xf0u},2u,1u,LIB_TRUE},
        {{0x26u,0x36u},2u,3u,LIB_FALSE}, {{0x36u,0x26u},2u,1u,LIB_FALSE},
        {{0xf0u,0xf0u},2u,0u,LIB_TRUE}, {{0x26u,0x26u},2u,1u,LIB_FALSE}
    };
    static const lib_u32 bases[] = {0x10000u,0x30000u,0x80000u,0x20000u,0x10000u};
    static const lib_u16 indices[] = {0u,1u,0xfffeu,0xffffu};
    static v30_block_fixture fixture;
    lib_u32 cases=0u;
    for (lib_size form = 0u; form < sizeof(opcodes); ++form)
        for (lib_size pattern=0u; pattern<sizeof(prefixes)/sizeof(prefixes[0]); ++pattern)
            for (lib_u8 mode = 0u; mode < 16u; ++mode)
                for (lib_u8 count = 0u; count < 7u; ++count)
                    for (lib_u8 position=0u; position<(count ? prefixes[pattern].bytes+1u : 1u); ++position) {
                        const lib_u8 segment=prefixes[pattern].segment;
                        const lib_u32 inputs=(segment ? X86_CORE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE : 0u) |
                            (prefixes[pattern].lock ? X86_CORE_CPU_TIMING_INPUT_LOCK : 0u);
                        const lib_bool repeat = count != 0u;
                        const lib_u8 repeat_opcode=count>=4u ? 0xf2u : 0xf3u;
                        const lib_u16 iterations = count == 0u ? 1u : count == 1u || count == 4u ? 0u :
                            count == 2u || count == 5u ? 1u : 3u;
                        lib_u8 code[4], size = 0u;
                        for (lib_u8 index=0u; index<=prefixes[pattern].bytes; ++index) {
                            if (repeat && index==position) code[size++]=repeat_opcode;
                            if (index<prefixes[pattern].bytes) code[size++]=prefixes[pattern].code[index];
                        }
                        code[size++] = opcodes[form];
                        v30_block_prepare(&fixture);
                        fixture.bus.cpu.data.si = indices[mode % 4u];
                        fixture.bus.cpu.data.di = indices[(mode + ((mode & 8u) ? 0u : 1u)) % 4u];
                        if (mode & 4u) fixture.bus.cpu.data.eflags |= VCPU_EFLAGS_DF;
                        if (mode & 2u) fixture.bus.cpu.data.eflags &=
                            ~(VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_OF);
                        if (repeat) fixture.bus.cpu.data.cx = iterations;
                        lib_memory_copy(fixture.memory + 0x80100u, code, size);
                        lib_memory_copy(fixture.expected, fixture.memory, sizeof(fixture.memory));
                        t_cpu expected = fixture.bus.cpu;
                        x86_core_cpu_instruction_lexeme lexeme;
                        const lib_bool preview=x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme);
                        /* LOCK preview is deliberately unavailable for every profile. */
                        if (preview != !prefixes[pattern].lock ||
                            lexeme.byte_count != (preview ? size : 0u) || fixture.count ||
                            lib_memory_compare(&fixture.bus.cpu, &expected, sizeof(expected)) != 0) {
                            lib_c_printf("block preview op=%02x pattern=%u mode=%u count=%u position=%u bytes=%u/%u\n",
                                opcodes[form],(lib_u32)pattern,mode,count,position,lexeme.byte_count,size);
                            return 1;
                        }
                        fixture.bus.observations = 0u;
                        for (lib_u16 item = 0u; item < (iterations ? iterations : 1u); ++item) {
                            const t_cpu before = fixture.bus.cpu;
                            const lib_u32 source = bases[segment] + expected.data.si;
                            const lib_u32 destination = expected.data.es.base + expected.data.di;
                            const lib_u32 first = fixture.count;
                            if (iterations) v30_block_expected(&fixture, &expected, opcodes[form], bases[segment]);
                            if (repeat && iterations) --expected.data.cx;
                            expected.data.eip = repeat && expected.data.cx ? 0x100u : 0x100u + size;
                            x86_core_cpu_execution_refresh(&fixture.bus.execution);
                            if (v30_string_timing(&fixture.bus.execution,&before,opcodes[form],
                                repeat ? repeat_opcode : 0u,(lib_u8)item,prefixes[pattern].bytes,inputs) || fixture.bus.observations)
                                return 1;
                            if (fixture.bus.execution.stop_requested || fixture.bus.delivered_exceptions ||
                                lib_memory_compare(&fixture.bus.cpu, &expected, sizeof(expected)) != 0 ||
                                lib_memory_compare(fixture.memory, fixture.expected, sizeof(fixture.memory)) != 0 ||
                                (iterations ? v30_block_addresses(&fixture, first, source, destination, opcodes[form]) :
                                    fixture.count != 0u)) {
                                lib_c_printf("block op=%02x pattern=%u mode=%u count=%u position=%u step=%u\n",
                                    opcodes[form],(lib_u32)pattern,mode,count,position,item);
                                return 1;
                            }
                        }
                        ++cases;
                    }
    lib_c_printf("V30:PREFIX-STRING:BLOCK:OK cases=%u\n",cases);
    return 0;
}

static lib_i32 v30_block_overlap(void)
{
    static v30_block_fixture fixture;
    for (lib_u8 word = 0u; word < 2u; ++word)
        for (lib_u8 direction = 0u; direction < 2u; ++direction)
            for (lib_u8 delta = 0u; delta < 4u; ++delta) {
                const lib_u8 code[] = {0xf3u,word ? 0xa5u : 0xa4u};
                v30_block_prepare(&fixture);
                fixture.bus.cpu.data.es.base = fixture.bus.cpu.data.ds.base;
                fixture.bus.cpu.data.si = 0x400u;
                fixture.bus.cpu.data.di = 0x400u + (direction ? -(lib_i32)delta : delta);
                if (direction) fixture.bus.cpu.data.eflags |= VCPU_EFLAGS_DF;
                lib_memory_copy(fixture.memory + 0x80100u, code, sizeof(code));
                lib_memory_copy(fixture.expected, fixture.memory, sizeof(fixture.memory));
                t_cpu expected = fixture.bus.cpu;
                for (lib_u8 item = 0u; item < 3u; ++item) {
                    v30_block_expected(&fixture, &expected, code[1], 0x10000u);
                    --expected.data.cx;
                    expected.data.eip = item == 2u ? 0x102u : 0x100u;
                    x86_core_cpu_execution_refresh(&fixture.bus.execution);
                    if (fixture.bus.execution.stop_requested || fixture.invalid ||
                        lib_memory_compare(&fixture.bus.cpu, &expected, sizeof(expected)) != 0 ||
                        lib_memory_compare(fixture.memory, fixture.expected, sizeof(fixture.memory)) != 0) return 1;
                }
            }
    return 0;
}

static lib_i32 v30_block_failures(void)
{
    static const lib_u8 opcodes[] = {0xa4u,0xa5u,0xaau,0xabu,0xacu,0xadu};
    static v30_block_fixture fixture;
    for (lib_size form = 0u; form < sizeof(opcodes); ++form)
        for (lib_u8 phase = 0u; phase < 4u; ++phase)
            for (lib_u8 write = 0u; write < 2u; ++write)
                for (lib_u8 stage = 1u; stage <= (lib_u8)((opcodes[form] & 1u) + 1u); ++stage) {
                    if ((write && (opcodes[form] & 0xfeu) == 0xacu) ||
                        (!write && (opcodes[form] & 0xfeu) == 0xaau)) continue;
                    const lib_bool prior=(phase & 1u)!=0u, additional=(phase & 2u)!=0u;
                    const lib_u8 code[] = {0xf0u,0x3eu,0xf3u,opcodes[form]};
                    v30_block_prepare(&fixture);
                    fixture.bus.cpu.data.si = fixture.bus.cpu.data.di = prior ? 0xfffdu : 0xffffu;
                    lib_memory_copy(fixture.memory + 0x80100u, code+(additional ? 0u : 2u),additional ? 4u : 2u);
                    if (prior) x86_core_cpu_execution_refresh(&fixture.bus.execution);
                    const t_cpu before = fixture.bus.cpu;
                    const lib_u32 first = fixture.count;
                    const lib_u32 transfers = write ? fixture.writes : fixture.reads;
                    lib_memory_copy(fixture.expected, fixture.memory, sizeof(fixture.memory));
                    if (write) fixture.fail_write = transfers + stage;
                    else fixture.fail_read = transfers + stage;
                    x86_core_cpu_execution_refresh(&fixture.bus.execution);
                    if (!fixture.bus.execution.stop_requested || fixture.bus.delivered_exceptions || fixture.invalid ||
                        fixture.bus.execution.completed_observation_valid || fixture.bus.execution.timing_result.ticks ||
                        fixture.bus.execution.timing_result.retirement_origin != X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED ||
                        lib_memory_compare(&fixture.bus.cpu, &before, sizeof(before)) != 0 ||
                        (write ? fixture.writes : fixture.reads) != transfers + stage) return 1;
                    for (lib_u32 item = first; item < fixture.count; ++item) {
                        const v30_block_record *record = &fixture.records[item];
                        if (record->write && record->accepted)
                            lib_memory_copy(fixture.expected + record->address, &record->value, record->bytes);
                        if (record->accepted != (item + 1u != fixture.count)) return 1;
                    }
                    if (lib_memory_compare(fixture.memory, fixture.expected, sizeof(fixture.memory)) != 0) return 1;
                }
    return 0;
}

static lib_i32 v30_block_interrupts(void)
{
    static const lib_u8 opcodes[] = {0xa4u,0xa5u,0xaau,0xabu,0xacu,0xadu};
    static v30_block_fixture fixture;
    for (lib_size form = 0u; form < sizeof(opcodes); ++form)
        for (lib_u8 entry = 0u; entry < 8u; ++entry) {
            const lib_bool nmi = (entry & 1u) != 0u;
            const lib_bool same = (entry & 2u) != 0u;
            const lib_bool additional=(entry & 4u)!=0u;
            const lib_u8 bytes=additional ? 4u : 2u;
            const lib_u32 inputs=additional ? X86_CORE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
                X86_CORE_CPU_TIMING_INPUT_LOCK : 0u;
            const lib_u8 code[] = {0xf0u,0x3eu,0xf3u,opcodes[form]};
            v30_block_prepare(&fixture);
            fixture.bus.cpu.data.cx = 2u;
            fixture.bus.cpu.data.eflags |= VCPU_EFLAGS_IF;
            lib_memory_copy(fixture.memory + (nmi ? 8u : 0xc0u),
                (const lib_u8[]){0u,same ? 1u : 2u,0u,0x80u}, 4u);
            fixture.memory[0x80200u] = 0xcfu;
            lib_memory_copy(fixture.memory + 0x80100u,code+(additional ? 0u : 2u),bytes);
            lib_memory_copy(fixture.expected, fixture.memory, sizeof(fixture.memory));
            t_cpu expected = fixture.bus.cpu;
            v30_block_expected(&fixture, &expected, opcodes[form], 0x10000u);
            v30_block_expected(&fixture, &expected, opcodes[form], 0x10000u);
            if (nmi) { if (!x86_core_cpu_request_nmi(&fixture.bus.execution)) return 1; }
            else fixture.bus.interrupt = LIB_TRUE;
            const t_cpu before = fixture.bus.cpu;
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (v30_string_timing(&fixture.bus.execution,&before,opcodes[form],0xf3u,0u,additional ? 2u : 0u,inputs)) return 1;
            lib_u16 saved_ip = 0u;
            lib_memory_copy(&saved_ip, fixture.memory + 0x206fau, 2u);
            if (fixture.bus.execution.stop_requested || saved_ip != 0x100u || fixture.bus.cpu.data.cx != 1u ||
                fixture.bus.cpu.data.eip != (same ? 0x100u : 0x200u) ||
                fixture.bus.acknowledgements != (lib_u32)!nmi) return 1;
            if (same) {
                /* Entry restarts setup even when the handler has the same code identity. */
                const t_cpu handler = fixture.bus.cpu;
                x86_core_cpu_execution_refresh(&fixture.bus.execution);
                if (v30_string_timing(&fixture.bus.execution,&handler,opcodes[form],0xf3u,0u,additional ? 2u : 0u,inputs) ||
                    fixture.bus.execution.stop_requested || fixture.bus.cpu.data.cx ||
                    fixture.bus.cpu.data.eip != 0x100u+bytes || fixture.bus.cpu.data.sp != 0x6fau) return 1;
                continue;
            }
            const lib_u32 first = fixture.count;
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (fixture.bus.execution.stop_requested || fixture.bus.cpu.data.eip != 0x100u ||
                fixture.bus.cpu.data.sp != 0x700u || !(fixture.bus.cpu.data.eflags & X86_CPU_NEC_PSW_MD)) return 1;
            const lib_u32 returning = fixture.count;
            if (returning != first + 3u) return 1;
            const t_cpu resumed = fixture.bus.cpu;
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (v30_string_timing(&fixture.bus.execution,&resumed,opcodes[form],0xf3u,0u,additional ? 2u : 0u,inputs)) return 1;
            const lib_u8 width = (opcodes[form] & 1u) + 1u;
            if (fixture.bus.execution.stop_requested || fixture.invalid || fixture.bus.cpu.data.cx ||
                fixture.bus.cpu.data.eip != 0x100u+bytes ||
                fixture.bus.cpu.data.eax != expected.data.eax ||
                lib_memory_compare(fixture.memory + 0x30600u, fixture.expected + 0x30600u, 4u) != 0 ||
                fixture.bus.cpu.data.si != ((opcodes[form] & 0xfeu) == 0xaau ? 0x400u : 0x400u + width * 2u) ||
                fixture.bus.cpu.data.di != ((opcodes[form] & 0xfeu) == 0xacu ? 0x600u : 0x600u + width * 2u)) return 1;
        }
    return 0;
}

static lib_i32 v30_block_restart(void)
{
    static v30_block_fixture fixture;
    static const lib_u8 code[][4]={
        {0xf0u,0xf3u,0x3eu,0xa4u}, {0x3eu,0xf0u,0xf3u,0xa4u},
        {0x26u,0xf0u,0xf3u,0xa4u}, {0x26u,0xf0u,0xf3u,0xaau},
        {0xf3u,0xaau}
    };
    v30_block_prepare(&fixture);
    fixture.bus.cpu.data.cx=7u;
    for (lib_u8 step=0u; step<7u; ++step) {
        const lib_u8 row=step<4u ? step : step==4u ? 3u : 4u;
        const lib_u8 additional=step<5u ? 2u : 0u, bytes=additional+2u;
        const lib_u8 opcode=step<3u ? 0xa4u : 0xaau;
        const lib_u32 inputs=additional ? X86_CORE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE |
            X86_CORE_CPU_TIMING_INPUT_LOCK : 0u;
        lib_memory_copy(fixture.memory+0x80100u,code[row],bytes);
        /* A changed lookahead byte is not part of the repeated instruction. */
        if (step==6u) fixture.memory[0x80102u]^=0xffu;
        lib_memory_copy(fixture.expected,fixture.memory,sizeof(fixture.memory));
        const t_cpu before=fixture.bus.cpu;
        t_cpu expected=before;
        v30_block_expected(&fixture,&expected,opcode,step<2u ? 0x10000u : 0x30000u);
        --expected.data.cx;
        expected.data.eip=expected.data.cx ? 0x100u : 0x100u+bytes;
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        /* A prefix permutation with identical effective facts continues the L2 block. */
        if (v30_string_timing(&fixture.bus.execution,&before,opcode,0xf3u,
            step==1u || step==4u || step==6u ? 1u : 0u,additional,inputs) ||
            fixture.bus.execution.stop_requested || fixture.invalid ||
            lib_memory_compare(&fixture.bus.cpu,&expected,sizeof(expected)) ||
            lib_memory_compare(fixture.memory,fixture.expected,sizeof(fixture.memory))) return 1;
    }
    lib_c_printf("V30:PREFIX-STRING:RESTART:OK cases=7\n");
    return 0;
}

static lib_i32 v30_block_bus_tail_and_counter(void)
{
    static const lib_u8 opcodes[] = {0xa4u,0xa5u,0xaau,0xabu,0xacu,0xadu};
    static v30_block_fixture fixture;
    for (lib_size form = 0u; form < sizeof(opcodes); ++form) {
        const lib_u8 code[] = {0xf3u,opcodes[form]};
        v30_block_prepare(&fixture);
        fixture.bus.cpu.data.ds.base = fixture.bus.cpu.data.es.base = 0xffff0u;
        fixture.bus.cpu.data.si = fixture.bus.cpu.data.di = 15u;
        fixture.bus.cpu.data.cx = 0xffffu;
        lib_memory_copy(fixture.memory + 0x80100u, code, sizeof(code));
        lib_memory_copy(fixture.expected, fixture.memory, sizeof(fixture.memory));
        t_cpu expected = fixture.bus.cpu;
        v30_block_expected(&fixture, &expected, opcodes[form], 0xffff0u);
        --expected.data.cx;
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (fixture.bus.execution.stop_requested || fixture.bus.delivered_exceptions ||
            lib_memory_compare(&fixture.bus.cpu, &expected, sizeof(expected)) != 0 ||
            lib_memory_compare(fixture.memory, fixture.expected, sizeof(fixture.memory)) != 0 ||
            v30_block_addresses(&fixture, 0u, 0xfffffu, 0xfffffu, opcodes[form])) return 1;
    }
    return 0;
}

lib_i32 main(void)
{
    if (v30_block_cases()) { lib_c_printf("V30:BLOCK-CASES:FAIL\n"); return 1; }
    if (v30_block_overlap()) { lib_c_printf("V30:BLOCK-OVERLAP:FAIL\n"); return 1; }
    if (v30_block_failures()) { lib_c_printf("V30:BLOCK-FAILURES:FAIL\n"); return 1; }
    if (v30_block_interrupts()) { lib_c_printf("V30:BLOCK-INTERRUPTS:FAIL\n"); return 1; }
    if (v30_block_restart()) { lib_c_printf("V30:BLOCK-RESTART:FAIL\n"); return 1; }
    if (v30_block_bus_tail_and_counter()) { lib_c_printf("V30:BLOCK-BUS-COUNTER:FAIL\n"); return 1; }
    lib_c_printf("V30:BLOCK-TRANSFER:OK\n");
    lib_c_printf("V30:NOMINAL-STRING:OK\n");
    return 0;
}
