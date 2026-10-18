#include "support/cpu_v30_memory_fixture.h"
#include "lib/types/file.h"

static lib_bool v30_repeat_condition(lib_u8 prefix, lib_u16 left, lib_u16 right)
{
    if (prefix == 0x65u) return left < right;
    if (prefix == 0x64u) return left >= right;
    if (prefix == 0xf3u) return left == right;
    return left != right;
}

static void v30_repeat_prepare(cpu_bus_fixture *fixture)
{
    cpu_bus_prepare(fixture, X86_CORE_CPU_PROFILE_V30);
    fixture->cpu.data.ds.base = 0x100u;
    fixture->cpu.data.es.base = 0x200u;
    fixture->cpu.data.si = 0x300u;
    fixture->cpu.data.di = 0x400u;
    fixture->cpu.data.ax = 2u;
}

typedef struct v30_compare_case {
    lib_u8 opcode, repeat, source, terminal;
    lib_u16 count, si, di, pc, left, right, flags;
    lib_bool reverse, pattern;
} v30_compare_case;

static t_cpu_data_sreg *v30_compare_source(t_cpu *cpu, lib_u8 prefix)
{
    switch (prefix) {
    case 0x26u: return &cpu->data.es;
    case 0x2eu: return &cpu->data.cs;
    case 0x36u: return &cpu->data.ss;
    default: return &cpu->data.ds;
    }
}

static lib_u16 v30_compare_read(const v30_memory_fixture *fixture,
    lib_u32 address, lib_u8 width)
{
    return fixture->memory[address] |
        (width == 2u ? (lib_u32)fixture->memory[address+1u] << 8u : 0u);
}

static void v30_compare_store(v30_memory_fixture *fixture, lib_u32 address,
    lib_u16 value, lib_u8 width)
{
    fixture->memory[address]=(lib_u8)value;
    if (width == 2u) fixture->memory[address+1u]=(lib_u8)(value >> 8u);
}

static void v30_compare_clear(v30_memory_fixture *fixture)
{
    lib_memory_set(&fixture->record_count,0,
        sizeof(*fixture)-lib_offsetof(v30_memory_fixture,record_count));
    fixture->bus.faults=fixture->bus.delivered_exceptions=fixture->bus.instruction_count=0u;
}

static lib_u8 v30_compare_prepare(v30_memory_fixture *fixture, const v30_compare_case *item)
{
    v30_memory_restart(fixture);
    const lib_u8 width=(item->opcode & 1u)+1u;
    const lib_bool scan=(item->opcode & 0xfeu) == 0xaeu;
    t_cpu *cpu=&fixture->bus.cpu;
    cpu->data.eax=0x12340000u | (scan ? (width == 1u ? 0x9a00u | item->left : item->left) : 0x9a78u);
    cpu->data.ebx=0x23456789u;
    cpu->data.ecx=0x34560000u | item->count;
    cpu->data.edx=0x456789abu;
    cpu->data.esi=0x56780000u | item->si;
    cpu->data.edi=0x67890000u | item->di;
    cpu->data.ebp=0x789abcdeu;
    cpu->data.eip=item->pc;
    cpu->data.eflags=0x8002u | item->flags | (item->reverse ? VCPU_EFLAGS_DF : 0u);
    const lib_u32 source=v30_compare_source(cpu,item->source)->base;
    const lib_u16 elements=item->repeat ? (item->count < 4u ? item->count : 4u) : 1u;
    for (lib_u16 index=0u; index < elements; ++index) {
        const lib_u16 delta=(lib_u16)(item->reverse ? 0u-index*width : index*width);
        lib_u16 left=item->left, right=item->right;
        if (item->pattern) {
            const lib_bool continuing=item->terminal == 0u || index+1u < item->terminal;
            left=2u;
            if (item->repeat == 0x65u) right=continuing ? 3u : 2u;
            else if (item->repeat == 0x64u) right=continuing ? 2u : 3u;
            else if (item->repeat == 0xf3u) right=continuing ? 2u : 1u;
            else right=continuing ? 1u : 2u;
        }
        if (!scan) v30_compare_store(fixture,source+(lib_u16)(item->si+delta),left,width);
        v30_compare_store(fixture,cpu->data.es.base+(lib_u16)(item->di+delta),right,width);
    }
    lib_u8 code[3], length=0u;
    if (item->repeat) code[length++]=item->repeat;
    if (item->source) code[length++]=item->source;
    code[length++]=item->opcode;
    for (lib_u8 byte=0u; byte < length; ++byte)
        fixture->memory[cpu->data.cs.base+(lib_u16)(item->pc+byte)]=code[byte];
    return length;
}

/* Direct unsigned/nibble subtraction and sign relations, independent of the
 * native ALU engine. This is native auxiliary borrow, not 8080 nibble carry. */
static lib_u32 v30_compare_flags(lib_u16 left, lib_u16 right, lib_u8 width)
{
    const lib_u32 mask=width == 1u ? 255u : 65535u;
    const lib_u32 sign=width == 1u ? 128u : 32768u;
    const lib_u32 result=(left-right) & mask;
    lib_u8 ones=0u;
    for (lib_u8 bit=0u; bit < 8u; ++bit) ones += (result >> bit) & 1u;
    return (left < right ? VCPU_EFLAGS_CF : 0u) |
        ((left & 15u) < (right & 15u) ? VCPU_EFLAGS_AF : 0u) |
        (!(ones & 1u) ? VCPU_EFLAGS_PF : 0u) |
        (result == 0u ? VCPU_EFLAGS_ZF : 0u) |
        (result & sign ? VCPU_EFLAGS_SF : 0u) |
        ((left ^ right) & (left ^ result) & sign ? VCPU_EFLAGS_OF : 0u);
}

static lib_bool v30_compare_expected(v30_memory_fixture *fixture,
    const v30_compare_case *item, lib_u8 length, t_cpu *expected, v30_memory_plan *plan)
{
    const lib_u8 width=(item->opcode & 1u)+1u;
    const lib_bool scan=(item->opcode & 0xfeu) == 0xaeu;
    expected->data.eip=(lib_u16)(item->pc+length);
    if (item->repeat && expected->data.cx == 0u) return LIB_FALSE;
    const lib_u32 source=v30_compare_source(expected,item->source)->base+expected->data.si;
    const lib_u32 destination=expected->data.es.base+expected->data.di;
    const lib_u16 left=scan ? (width == 1u ? expected->data.al : expected->data.ax) :
        v30_compare_read(fixture,source,width);
    const lib_u16 right=v30_compare_read(fixture,destination,width);
    if (!scan) v30_memory_plan_transfer(plan,fixture,source,width,LIB_FALSE,left);
    v30_memory_plan_transfer(plan,fixture,destination,width,LIB_FALSE,right);
    expected->data.eflags=(expected->data.eflags & ~0x8d5u) | v30_compare_flags(left,right,width);
    const lib_u16 delta=(lib_u16)(item->reverse ? 0u-width : width);
    if (!scan) expected->data.si+=delta;
    expected->data.di+=delta;
    if (!item->repeat) return LIB_FALSE;
    --expected->data.cx;
    const lib_bool continuing=expected->data.cx && v30_repeat_condition(item->repeat,left,right);
    if (continuing) expected->data.eip=item->pc;
    return continuing;
}

static lib_i32 v30_compare_refusals(const v30_compare_case *item, lib_u8 iteration,
    const t_cpu *before, const v30_memory_plan *plan, lib_u32 reads,
    lib_u32 fetches, lib_u32 observations)
{
    static v30_memory_fixture fixture;
    for (lib_u8 kind=0u; kind < 3u; ++kind) {
        const lib_u32 count=kind == 0u ? reads : kind == 1u ? fetches : observations;
        for (lib_u32 ordinal=1u; ordinal <= count; ++ordinal) {
            v30_compare_prepare(&fixture,item);
            for (lib_u8 prior=0u; prior < iteration; ++prior)
                x86_core_cpu_execution_refresh(&fixture.bus.execution);
            v30_compare_clear(&fixture);
            if (kind == 2u) {
                fixture.fail_observe=ordinal;
                x86_core_cpu_instruction_lexeme lexeme;
                if (x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme) ||
                    fixture.bus.execution.stop_requested || fixture.reads || fixture.writes ||
                    fixture.fetches || fixture.phase_count || fixture.bus.faults ||
                    lib_memory_compare(&fixture.bus.cpu,before,sizeof(*before))) return 1;
                continue;
            }
            if (kind == 0u) fixture.fail_read=ordinal;
            else fixture.fail_fetch=ordinal;
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (!fixture.bus.execution.stop_requested || fixture.bus.faults != 1u ||
                fixture.bus.delivered_exceptions || fixture.writes || fixture.invalid ||
                lib_memory_compare(&fixture.bus.cpu,before,sizeof(*before)) ||
                (kind == 0u && v30_memory_check_plan(&fixture,plan,LIB_TRUE))) return 1;
        }
    }
    return 0;
}

static lib_i32 v30_compare_check(const v30_compare_case *item, lib_bool deep)
{
    static v30_memory_fixture fixture;
    const lib_u8 length=v30_compare_prepare(&fixture,item);
    for (lib_u8 iteration=0u; iteration < 4u; ++iteration) {
        const t_cpu before=fixture.bus.cpu;
        t_cpu expected=before;
        v30_memory_plan plan={0};
        const lib_bool continuing=v30_compare_expected(&fixture,item,length,&expected,&plan);
        v30_compare_clear(&fixture);
        lib_u32 observations=0u;
        if (deep) {
            x86_core_cpu_instruction_lexeme lexeme;
            if (!x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme) ||
                lexeme.byte_count != length || lexeme.component_count != length ||
                !lexeme.available || fixture.reads || fixture.writes || fixture.fetches ||
                fixture.phase_count || fixture.bus.faults ||
                lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before))) return 1;
            observations=fixture.observations;
            v30_compare_clear(&fixture);
        }
        x86_core_cpu_execution_refresh(&fixture.bus.execution);
        if (fixture.bus.execution.stop_requested || fixture.bus.faults || fixture.bus.delivered_exceptions ||
            fixture.bus.execution.nec_md_write_enabled || fixture.writes ||
            lib_memory_compare(&fixture.bus.cpu,&expected,sizeof(expected)) ||
            v30_memory_check_plan(&fixture,&plan,LIB_FALSE)) {
            lib_c_printf("comparison op=%02x repeat=%02x src=%02x count=%u step=%u flags=%04x/%04x\n",
                item->opcode,item->repeat,item->source,item->count,iteration,
                fixture.bus.cpu.data.flags,expected.data.flags);
            return 1;
        }
        if (deep && v30_compare_refusals(item,iteration,&before,&plan,fixture.reads,
                fixture.fetches,observations)) return 1;
        if (!continuing) return 0;
    }
    return 1; /* A budget alone is never successful repetition coverage. */
}

static lib_i32 v30_repeat_operands(void)
{
    static const lib_u8 opcodes[]={0xa6u,0xa7u,0xaeu,0xafu};
    static const lib_u8 repeats[]={0u,0x64u,0x65u,0xf2u,0xf3u};
    static const lib_u8 sources[]={0u,0x26u,0x2eu,0x36u,0x3eu};
    static const lib_u16 edges[]={0u,1u,0xfffeu,0xffffu};
    static const lib_u16 boundaries[]={0u,1u,0x7fffu,0x8000u,0xffffu};
    lib_u32 cases=0u;
    for (lib_u8 operation=0u; operation < sizeof(opcodes); ++operation)
        for (lib_u16 left=0u; left < 256u; ++left)
            for (lib_u16 right=0u; right < 256u; ++right) {
                const v30_compare_case item={.opcode=opcodes[operation],.count=3u,
                    .si=0x501u,.di=0x602u,.pc=0x100u,.left=left,.right=right,
                    .flags=(left & 1u) ? 0x8d5u : 0u};
                if (v30_compare_check(&item,LIB_FALSE)) return 1;
                ++cases;
            }
    for (lib_u8 operation=1u; operation < sizeof(opcodes); operation+=2u)
        for (lib_u32 value=0u; value < 65536u; ++value)
            for (lib_u8 boundary=0u; boundary < sizeof(boundaries)/sizeof(boundaries[0]); ++boundary) {
                const v30_compare_case item={.opcode=opcodes[operation],.count=3u,
                    .si=0x500u,.di=0x601u,.pc=0x100u,.left=(lib_u16)value,
                    .right=boundaries[boundary],.flags=(value & 1u) ? 0x8d5u : 0u};
                if (v30_compare_check(&item,LIB_FALSE)) return 1;
                ++cases;
            }
    for (lib_u8 operation=0u; operation < sizeof(opcodes); ++operation)
        for (lib_u8 repeat=0u; repeat < sizeof(repeats); ++repeat)
            for (lib_u8 source=0u; source < sizeof(sources); ++source)
                for (lib_u8 direction=0u; direction < 2u; ++direction)
                    for (lib_u16 count=0u; count <= 4u; ++count)
                        for (lib_u8 terminal=0u; terminal <= count; ++terminal)
                            for (lib_u8 edge=0u; edge < sizeof(edges)/sizeof(edges[0]); ++edge)
                                for (lib_u8 flags=0u; flags < 2u; ++flags) {
                                    const v30_compare_case item={.opcode=opcodes[operation],
                                        .repeat=repeats[repeat],.source=sources[source],.terminal=terminal,
                                        .count=count,.si=edges[edge],.di=edges[3u-edge],.pc=edges[edge],
                                        .left=2u,.right=2u,.flags=flags ? 0x8d5u : 0u,
                                        .reverse=direction != 0u,.pattern=LIB_TRUE};
                                    if (v30_compare_check(&item,LIB_TRUE)) return 1;
                                    ++cases;
                                }
    for (lib_u8 operation=0u; operation < sizeof(opcodes); ++operation)
        for (lib_u8 repeat=0u; repeat < sizeof(repeats); ++repeat)
            for (lib_u8 overlap=0u; overlap < 3u; ++overlap)
                for (lib_u8 direction=0u; direction < 2u; ++direction) {
                    const v30_compare_case item={.opcode=opcodes[operation],.repeat=repeats[repeat],
                        .source=0x26u,.count=4u,.si=0x501u,.di=(lib_u16)(0x500u+overlap),
                        .pc=0x100u,.left=2u,.right=2u,.flags=0x8d5u,
                        .reverse=direction != 0u,.pattern=LIB_TRUE};
                    if (v30_compare_check(&item,LIB_TRUE)) return 1;
                    ++cases;
                }
    for (lib_u8 operation=0u; operation < sizeof(opcodes); ++operation)
        for (lib_u8 repeat=1u; repeat < sizeof(repeats); ++repeat) {
            const v30_compare_case item={.opcode=opcodes[operation],.repeat=repeats[repeat],
                .count=0xffffu,.terminal=1u,.si=0xffffu,.di=0x601u,.pc=0x100u,
                .left=2u,.right=2u,.flags=0x8d5u,.pattern=LIB_TRUE};
            if (v30_compare_check(&item,LIB_TRUE)) return 1;
            ++cases;
        }
    for (lib_u8 operation=0u; operation < sizeof(opcodes); ++operation)
        for (lib_u8 repeat=0u; repeat < sizeof(repeats); ++repeat)
            for (lib_u8 flags=0u; flags < 64u; ++flags) {
                static const lib_u16 masks[]={1u,4u,16u,64u,128u,2048u};
                lib_u16 seed=0u;
                for (lib_u8 bit=0u; bit < sizeof(masks)/sizeof(masks[0]); ++bit)
                    if (flags & (1u << bit)) seed |= masks[bit];
                const v30_compare_case item={.opcode=opcodes[operation],.repeat=repeats[repeat],
                    .count=(lib_u16)(flags % 3u),.si=0x500u,.di=0x600u,.pc=0x100u,
                    .left=0x80u,.right=1u,.flags=seed};
                if (v30_compare_check(&item,LIB_FALSE)) return 1;
                ++cases;
            }
    lib_c_printf("V30:COMPARISON:OK cases=%u\n",cases);
    return 0;
}

static lib_i32 v30_compare_interrupts(void)
{
    static const x86_core_cpu_bus_provider provider={
        .read_memory=v30_memory_read,.write_memory=v30_memory_write,
        .interrupt_pending=cpu_bus_pending,.acknowledge_interrupt=cpu_bus_acknowledge
    };
    static const lib_u8 opcodes[]={0xa6u,0xa7u,0xaeu,0xafu};
    static const lib_u8 repeats[]={0u,0x64u,0x65u,0xf2u,0xf3u};
    static const lib_u8 sources[]={0u,0x26u,0x2eu,0x36u,0x3eu};
    static v30_memory_fixture fixture;
    for (lib_u8 operation=0u; operation < sizeof(opcodes); ++operation)
        for (lib_u8 repeat=0u; repeat < sizeof(repeats); ++repeat)
            for (lib_u8 source=0u; source < sizeof(sources); ++source)
                for (lib_u8 direction=0u; direction < 2u; ++direction)
                    for (lib_u8 odd=0u; odd < 2u; ++odd)
                        for (lib_u8 nmi=0u; nmi < 2u; ++nmi) {
                            const v30_compare_case item={.opcode=opcodes[operation],
                                .repeat=repeats[repeat],.source=sources[source],.terminal=2u,
                                .count=3u,.si=direction ? 0u : 0xffffu,.di=0x601u,
                                .pc=odd ? 0x101u : 0x100u,.left=2u,.right=2u,
                                .flags=0x200u | 0x8d5u,.reverse=direction != 0u,.pattern=LIB_TRUE};
                            const lib_u8 length=v30_compare_prepare(&fixture,&item);
                            fixture.bus.cpu.data.sp=(lib_u16)(0x700u+odd);
                            fixture.bus.execution.bus=&provider;
                            fixture.bus.execution.bus_context=&fixture;
                            const lib_u8 vector=nmi ? 2u : 0x30u;
                            v30_compare_store(&fixture,4u*vector,0x200u,2u);
                            v30_compare_store(&fixture,4u*vector+2u,0x9000u,2u);
                            fixture.memory[0x90200u]=0xcfu;
                            for (lib_u8 byte=0u; byte < 8u; ++byte)
                                fixture.memory[fixture.bus.cpu.data.ss.base+
                                    (lib_u16)(fixture.bus.cpu.data.sp-6u+byte)]=0xa5u;
                            t_cpu expected=fixture.bus.cpu;
                            v30_memory_plan plan={0};
                            if (v30_compare_expected(&fixture,&item,length,&expected,&plan) != !!item.repeat) {
                                lib_c_printf("compare irq preparation op=%02x rep=%02x src=%02x\n",
                                    item.opcode,item.repeat,item.source); return 1;
                            }
                            const lib_u16 words[]={(lib_u16)(expected.data.flags | 0x7000u),
                                expected.data.cs.selector,(lib_u16)expected.data.eip};
                            for (lib_u8 word=0u; word < 3u; ++word)
                                v30_memory_plan_transfer(&plan,&fixture,expected.data.ss.base+
                                    (lib_u16)(expected.data.sp-2u*(word+1u)),2u,LIB_TRUE,words[word]);
                            v30_memory_plan_transfer(&plan,&fixture,4u*vector,2u,LIB_FALSE,0x200u);
                            v30_memory_plan_transfer(&plan,&fixture,4u*vector+2u,2u,LIB_FALSE,0x9000u);
                            if (nmi) {
                                if (!x86_core_cpu_request_nmi(&fixture.bus.execution)) return 1;
                            } else fixture.bus.interrupt=LIB_TRUE;
                            x86_core_cpu_execution_refresh(&fixture.bus.execution);
                            if (fixture.bus.execution.stop_requested || fixture.bus.faults ||
                                fixture.bus.cpu.data.cs.base != 0x90000u || fixture.bus.cpu.data.eip != 0x200u ||
                                fixture.bus.acknowledgements != (lib_u32)!nmi ||
                                v30_memory_check_plan(&fixture,&plan,LIB_FALSE)) {
                                lib_c_printf("compare irq entry op=%02x rep=%02x src=%02x dir=%u odd=%u nmi=%u pc=%x flags=%x reads=%u writes=%u plan=%u\n",
                                    item.opcode,item.repeat,item.source,direction,odd,nmi,
                                    fixture.bus.cpu.data.eip,fixture.bus.cpu.data.flags,
                                    fixture.reads,fixture.writes,plan.count); return 1;
                            }
                            x86_core_cpu_execution_refresh(&fixture.bus.execution);
                            if (fixture.bus.execution.stop_requested || fixture.bus.faults ||
                                lib_memory_compare(&fixture.bus.cpu,&expected,sizeof(expected))) {
                                lib_c_printf("compare irq return op=%02x rep=%02x src=%02x dir=%u odd=%u nmi=%u pc=%x/%x flags=%x/%x sp=%x/%x\n",
                                    item.opcode,item.repeat,item.source,direction,odd,nmi,
                                    fixture.bus.cpu.data.eip,expected.data.eip,fixture.bus.cpu.data.flags,
                                    expected.data.flags,fixture.bus.cpu.data.sp,expected.data.sp); return 1;
                            }
                            if (!item.repeat) continue;
                            v30_compare_clear(&fixture);
                            plan=(v30_memory_plan){0};
                            if (v30_compare_expected(&fixture,&item,length,&expected,&plan) ||
                                expected.data.cx != 1u) {
                                lib_c_printf("compare irq terminal op=%02x rep=%02x src=%02x dir=%u odd=%u nmi=%u si=%x di=%x flags=%x\n",
                                    item.opcode,item.repeat,item.source,direction,odd,nmi,
                                    fixture.bus.cpu.data.si,fixture.bus.cpu.data.di,expected.data.flags);
                                return 1;
                            }
                            x86_core_cpu_execution_refresh(&fixture.bus.execution);
                            if (fixture.bus.execution.stop_requested || fixture.bus.faults ||
                                lib_memory_compare(&fixture.bus.cpu,&expected,sizeof(expected)) ||
                                v30_memory_check_plan(&fixture,&plan,LIB_FALSE)) return 1;
                        }
    return 0;
}

static lib_i32 v30_repeat_prefixes(void)
{
    static const lib_u8 cases[][5] = {
        {0x65u,0x36u,0xa6u}, {0x36u,0x65u,0xa6u},
        {0xf3u,0x65u,0xa6u}, {0x65u,0xf3u,0xa6u},
        {0x65u,0x64u,0xa6u}, {0x64u,0x65u,0xa6u},
        {0x26u,0x65u,0x36u,0xa6u}, {0x65u,0x36u,0x26u,0xa6u}
    };
    static const lib_u8 sizes[] = {3u,3u,3u,3u,3u,3u,4u,4u};
    for (lib_size item = 0u; item < sizeof(sizes); ++item) {
        cpu_bus_fixture fixture;
        x86_core_cpu_instruction_lexeme lexeme;
        v30_repeat_prepare(&fixture);
        fixture.cpu.data.cx = 2u;
        fixture.memory[0x300u] = 1u; /* SS source */
        fixture.memory[0x400u] = 3u; /* DS source */
        fixture.memory[0x500u] = 1u; /* ES source */
        fixture.memory[0x600u] = 2u;
        lib_memory_copy(&fixture.memory[0x100u], cases[item], sizes[item]);
        if (!x86_core_cpu_instruction_lexeme_scan(cases[item], sizes[item],
            X86_CORE_CPU_PROFILE_V30, LIB_FALSE, &lexeme) || lexeme.byte_count != sizes[item] ||
            !x86_core_cpu_execution_preview_lexeme(&fixture.execution, &lexeme)) return 1;
        for (lib_u8 available = 0u; available < sizes[item]; ++available)
            if (x86_core_cpu_instruction_lexeme_scan(cases[item], available,
                X86_CORE_CPU_PROFILE_V30, LIB_FALSE, &lexeme)) return 1;
        x86_core_cpu_execution_refresh(&fixture.execution);
        const lib_bool continuing = item < 2u || item == 4u || item >= 6u;
        if (fixture.execution.stop_requested || fixture.cpu.data.cx != 1u ||
            fixture.cpu.data.si != 0x301u || fixture.cpu.data.di != 0x401u ||
            fixture.cpu.data.eip != (continuing ? 0x100u : 0x100u + sizes[item])) return 1;
    }
    static const lib_u8 unsupported[] = {0xa4u,0xa5u,0xaau,0xabu,0xacu,0xadu,0x90u,0x0fu};
    for (lib_size item = 0u; item < sizeof(unsupported); ++item) {
        cpu_bus_fixture fixture;
        x86_core_cpu_instruction_lexeme lexeme;
        const lib_u8 code[] = {0x65u,unsupported[item],0x20u};
        v30_repeat_prepare(&fixture);
        fixture.cpu.data.cx = 2u;
        lib_memory_copy(&fixture.memory[0x100u], code, sizeof(code));
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (!fixture.execution.stop_requested || fixture.writes || fixture.cpu.data.cx != 2u ||
            fixture.cpu.data.eip != 0x100u || fixture.delivered_exceptions ||
            x86_core_cpu_instruction_lexeme_scan(code, sizeof(code),
                X86_CORE_CPU_PROFILE_V30, LIB_FALSE, &lexeme)) return 1;
    }
    return 0;
}

static lib_i32 v30_repeat_intel_isolation(void)
{
    for (lib_u8 prefix = 0x64u; prefix <= 0x65u; ++prefix) {
        cpu_bus_fixture fixture;
        const lib_u8 code[] = {prefix,0xa6u};
        cpu_bus_prepare(&fixture, X86_CORE_CPU_PROFILE_80386);
        fixture.cpu.data.fs.base = 0x100u;
        fixture.cpu.data.gs.base = 0x200u;
        fixture.cpu.data.si = 0x300u;
        fixture.cpu.data.di = 0x600u;
        fixture.cpu.data.cx = 2u;
        fixture.memory[prefix == 0x64u ? 0x400u : 0x500u] = 1u;
        fixture.memory[0x600u] = 2u;
        lib_memory_copy(&fixture.memory[0x100u], code, sizeof(code));
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (fixture.execution.stop_requested || fixture.cpu.data.cx != 2u ||
            fixture.cpu.data.eip != 0x102u || fixture.cpu.data.si != 0x301u ||
            fixture.cpu.data.di != 0x601u || !(fixture.cpu.data.eflags & VCPU_EFLAGS_CF)) return 1;
    }
    return 0;
}

static lib_i32 v30_repeat_interrupts(void)
{
    static const lib_u8 codes[][5] = {
        {0x65u,0xa6u}, {0x65u,0x36u,0xa6u},
        {0x26u,0x65u,0x36u,0xa6u}, {0x26u,0xf3u,0x65u,0x36u,0xa6u}
    };
    for (lib_u8 item = 0u; item < 4u; ++item)
        for (lib_u8 nmi = 0u; nmi < 2u; ++nmi) {
            cpu_bus_fixture fixture;
            lib_u16 return_ip = 0u;
            v30_repeat_prepare(&fixture);
            fixture.cpu.data.cx = 2u;
            fixture.cpu.data.eflags |= VCPU_EFLAGS_IF;
            fixture.memory[item == 0u ? 0x400u : 0x300u] = 1u;
            fixture.memory[item == 0u ? 0x401u : 0x301u] = 1u;
            fixture.memory[0x600u] = fixture.memory[0x601u] = 2u;
            fixture.memory[nmi ? 0x09u : 0xc1u] = 2u;
            fixture.memory[0x200u] = 0xcfu; /* Actual IRET, not direct PC injection. */
            lib_memory_copy(&fixture.memory[0x100u], codes[item], item + 2u);
            const lib_u16 compared_flags=(lib_u16)((fixture.cpu.data.flags & ~0x8d5u) |
                v30_compare_flags(1u,2u,1u));
            if (nmi) {
                if (!x86_core_cpu_request_nmi(&fixture.execution)) return 1;
            } else fixture.interrupt = LIB_TRUE;
            x86_core_cpu_execution_refresh(&fixture.execution);
            lib_memory_copy(&return_ip, fixture.memory + 0x6fau, sizeof(return_ip));
            lib_u16 saved_flags;
            lib_memory_copy(&saved_flags,fixture.memory+0x6feu,sizeof(saved_flags));
            if (fixture.execution.stop_requested || fixture.cpu.data.eip != 0x200u ||
                fixture.cpu.data.cx != 1u || fixture.cpu.data.si != 0x301u ||
                fixture.cpu.data.di != 0x401u || return_ip != (item == 3u ? 0x101u : 0x100u) ||
                fixture.acknowledgements != (lib_u32)!nmi ||
                saved_flags != (lib_u16)(compared_flags | 0x7000u)) return 1;
            x86_core_cpu_execution_refresh(&fixture.execution);
            if (fixture.execution.stop_requested || fixture.cpu.data.eip != return_ip ||
                fixture.cpu.data.sp != 0x700u || fixture.cpu.data.flags != compared_flags) return 1;
            x86_core_cpu_execution_refresh(&fixture.execution);
            if (fixture.execution.stop_requested || fixture.cpu.data.cx != 0u ||
                fixture.cpu.data.si != 0x302u || fixture.cpu.data.di != 0x402u ||
                fixture.cpu.data.eip != 0x102u + item) return 1;
        }
    return 0;
}


lib_i32 main(void)
{
    if (v30_repeat_operands()) { lib_c_printf("V30:REPEAT-OPERANDS:FAIL\n"); return 1; }
    if (v30_compare_interrupts()) { lib_c_printf("V30:COMPARISON-INTERRUPTS:FAIL\n"); return 1; }
    if (v30_repeat_prefixes()) { lib_c_printf("V30:REPEAT-PREFIXES:FAIL\n"); return 1; }
    if (v30_repeat_intel_isolation()) { lib_c_printf("V30:REPEAT-INTEL:FAIL\n"); return 1; }
    if (v30_repeat_interrupts()) { lib_c_printf("V30:REPEAT-INTERRUPTS:FAIL\n"); return 1; }
    lib_c_printf("V30:CONDITIONAL-REPEAT:OK\n");
    return 0;
}
