#include "support/cpu_bus_fixture.h"
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

static lib_i32 v30_repeat_case(lib_u8 prefix, lib_u8 opcode, lib_u16 count,
    lib_u16 terminal, lib_bool direction, lib_u32 initial_flags)
{
    cpu_bus_fixture fixture;
    const lib_bool scan = (opcode & 0xfeu) == 0xaeu;
    const lib_u8 width = (opcode & 1u) + 1u;
    lib_u16 consumed = 0u;
    lib_u16 left = 0u, right = 0u;
    const lib_u8 code[] = {prefix,opcode};
    x86_core_cpu_instruction_lexeme lexeme;

    v30_repeat_prepare(&fixture);
    fixture.cpu.data.ecx = 0xa55a0000u | count;
    fixture.cpu.data.eflags |= initial_flags | (direction ? VCPU_EFLAGS_DF : 0u);
    lib_memory_copy(&fixture.memory[0x100u], code, sizeof(code));
    for (lib_u16 item = 0u; item < count; ++item) {
        const lib_bool continuing = terminal == 0u || item + 1u < terminal;
        const lib_u16 delta = direction ? (lib_u16)(0u - item * width) : item * width;
        if (prefix == 0x65u || prefix == 0x64u) {
            const lib_bool carry = prefix == 0x65u ? continuing : !continuing;
            left = scan ? 2u : carry ? 1u : 2u;
            right = scan ? (carry ? 3u : 2u) : 2u;
        } else {
            const lib_bool equal = prefix == 0xf3u ? continuing : !continuing;
            left = 2u;
            right = equal ? 2u : 1u;
        }
        lib_memory_copy(&fixture.memory[0x100u + (lib_u16)(0x300u + delta)], &left, width);
        lib_memory_copy(&fixture.memory[0x200u + (lib_u16)(0x400u + delta)], &right, width);
    }
    const lib_u32 flags = fixture.cpu.data.eflags;
    if (!x86_core_cpu_instruction_lexeme_scan(code, sizeof(code), X86_CORE_CPU_PROFILE_V30,
        LIB_FALSE, &lexeme) || lexeme.byte_count != sizeof(code) ||
        !x86_core_cpu_execution_preview_lexeme(&fixture.execution, &lexeme)) return 1;
    do {
        const lib_u16 delta = direction ? (lib_u16)(0u - consumed * width) : consumed * width;
        if (count) {
            left = scan ? 2u : fixture.memory[0x100u + (lib_u16)(0x300u + delta)];
            right = fixture.memory[0x200u + (lib_u16)(0x400u + delta)];
            ++consumed;
        }
        x86_core_cpu_execution_refresh(&fixture.execution);
        const lib_bool continuing = consumed < count && v30_repeat_condition(prefix, left, right);
        if (fixture.execution.stop_requested || fixture.faults || fixture.delivered_exceptions ||
            fixture.cpu.data.ecx != (0xa55a0000u | (count - consumed)) || fixture.writes ||
            fixture.cpu.data.si != (lib_u16)(0x300u + (scan ? 0u :
                direction ? 0u - consumed * width : consumed * width)) ||
            fixture.cpu.data.di != (lib_u16)(0x400u +
                (direction ? 0u - consumed * width : consumed * width)) ||
            fixture.cpu.data.eip != (continuing ? 0x100u : 0x102u) ||
            fixture.instructions.data.msize != (lib_u32)(count ? (scan ? 1u : 2u) : 0u) ||
            (count ? ((fixture.cpu.data.eflags & VCPU_EFLAGS_CF) != (lib_u32)(left < right) ||
                !!(fixture.cpu.data.eflags & VCPU_EFLAGS_ZF) != (left == right)) :
                fixture.cpu.data.eflags != flags)) return 1;
        if (!continuing) break;
    } while (consumed < count);
    return consumed != (terminal && terminal < count ? terminal : count);
}

static lib_i32 v30_repeat_operands(void)
{
    static const lib_u8 prefixes[] = {0x64u,0x65u,0xf2u,0xf3u};
    static const lib_u8 opcodes[] = {0xa6u,0xa7u,0xaeu,0xafu};
    for (lib_size prefix = 0u; prefix < sizeof(prefixes); ++prefix)
        for (lib_size opcode = 0u; opcode < sizeof(opcodes); ++opcode)
            for (lib_u16 count = 0u; count <= 4u; ++count)
                for (lib_u16 terminal = 0u; terminal <= count; ++terminal)
                    for (lib_u8 direction = 0u; direction < 2u; ++direction)
                        for (lib_u8 flags = 0u; flags < 4u; ++flags)
                            if (v30_repeat_case(prefixes[prefix], opcodes[opcode], count,
                                terminal, direction != 0u, (flags & 1u ? VCPU_EFLAGS_CF : 0u) |
                                (flags & 2u ? VCPU_EFLAGS_ZF : 0u))) {
                                lib_c_printf("repeat prefix=%02X opcode=%02X count=%u terminal=%u\n",
                                    prefixes[prefix], opcodes[opcode], count, terminal);
                                return 1;
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

static lib_i32 v30_repeat_counter_and_intel(void)
{
    for (lib_u8 opcode = 0xa6u; opcode <= 0xa7u; ++opcode) {
        cpu_bus_fixture fixture;
        const lib_u8 code[] = {0x64u,opcode};
        v30_repeat_prepare(&fixture);
        fixture.cpu.data.ecx = 0xa55affffu;
        fixture.memory[0x400u] = 1u;
        fixture.memory[0x600u] = 2u; /* First borrow terminates REPNC. */
        lib_memory_copy(&fixture.memory[0x100u], code, sizeof(code));
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (fixture.execution.stop_requested || fixture.cpu.data.ecx != 0xa55afffeu ||
            fixture.cpu.data.eip != 0x102u || !(fixture.cpu.data.eflags & VCPU_EFLAGS_CF)) return 1;
    }
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
            if (nmi) {
                if (!x86_core_cpu_request_nmi(&fixture.execution)) return 1;
            } else fixture.interrupt = LIB_TRUE;
            x86_core_cpu_execution_refresh(&fixture.execution);
            lib_memory_copy(&return_ip, fixture.memory + 0x6fau, sizeof(return_ip));
            if (fixture.execution.stop_requested || fixture.cpu.data.eip != 0x200u ||
                fixture.cpu.data.cx != 1u || fixture.cpu.data.si != 0x301u ||
                fixture.cpu.data.di != 0x401u || return_ip != (item == 3u ? 0x101u : 0x100u) ||
                fixture.acknowledgements != (lib_u32)!nmi) return 1;
            x86_core_cpu_execution_refresh(&fixture.execution);
            if (fixture.execution.stop_requested || fixture.cpu.data.eip != return_ip ||
                fixture.cpu.data.sp != 0x700u || !(fixture.cpu.data.eflags & X86_CPU_NEC_PSW_MD)) return 1;
            x86_core_cpu_execution_refresh(&fixture.execution);
            if (fixture.execution.stop_requested || fixture.cpu.data.cx != 0u ||
                fixture.cpu.data.si != 0x302u || fixture.cpu.data.di != 0x402u ||
                fixture.cpu.data.eip != 0x102u + item) return 1;
        }
    return 0;
}

typedef struct v30_repeat_failure_fixture {
    cpu_bus_fixture bus;
    lib_u32 reads;
    lib_u32 fail_read;
} v30_repeat_failure_fixture;

static lib_status v30_repeat_read(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    v30_repeat_failure_fixture *fixture = opaque;
    if (provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA && !observe_only &&
        ++fixture->reads == fixture->fail_read) return LIB_STATUS_IO_ERROR;
    return cpu_bus_read(&fixture->bus, address, destination, bytes, provenance,
        observe_only, reset_fetch, cache_control);
}

static lib_i32 v30_repeat_failures(void)
{
    static const lib_u8 opcodes[] = {0xa6u,0xa7u,0xaeu,0xafu};
    for (lib_size op = 0u; op < sizeof(opcodes); ++op)
        for (lib_u32 stage = 1u; stage <= (op < 2u ? 2u : 1u); ++stage) {
            v30_repeat_failure_fixture fixture = {0};
            x86_core_cpu_bus_provider provider = cpu_bus_provider;
            const lib_u8 code[] = {0x65u,opcodes[op]};
            v30_repeat_prepare(&fixture.bus);
            fixture.bus.cpu.data.cx = 3u;
            fixture.bus.memory[0x400u] = fixture.bus.memory[0x402u] = 1u;
            fixture.bus.memory[0x600u] = fixture.bus.memory[0x602u] = 3u;
            provider.read_memory = v30_repeat_read;
            fixture.bus.execution.bus = &provider;
            fixture.bus.execution.bus_context = &fixture;
            lib_memory_copy(&fixture.bus.memory[0x100u], code, sizeof(code));
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            const t_cpu before = fixture.bus.cpu;
            const lib_u32 reads = fixture.reads;
            if (fixture.bus.execution.stop_requested || before.data.cx != 2u ||
                before.data.eip != 0x100u) return 1;
            fixture.fail_read = reads + stage;
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (!fixture.bus.execution.stop_requested || fixture.bus.delivered_exceptions ||
                fixture.bus.cpu.data.eip != before.data.eip ||
                fixture.bus.cpu.data.cx != before.data.cx || fixture.bus.cpu.data.si != before.data.si ||
                fixture.bus.cpu.data.di != before.data.di || fixture.bus.cpu.data.eflags != before.data.eflags ||
                fixture.reads != reads + stage || fixture.bus.writes) return 1;
        }
    return 0;
}

lib_i32 main(void)
{
    if (v30_repeat_operands()) { lib_c_printf("V30:REPEAT-OPERANDS:FAIL\n"); return 1; }
    if (v30_repeat_prefixes()) { lib_c_printf("V30:REPEAT-PREFIXES:FAIL\n"); return 1; }
    if (v30_repeat_counter_and_intel()) { lib_c_printf("V30:REPEAT-COUNTER-INTEL:FAIL\n"); return 1; }
    if (v30_repeat_interrupts()) { lib_c_printf("V30:REPEAT-INTERRUPTS:FAIL\n"); return 1; }
    if (v30_repeat_failures()) { lib_c_printf("V30:REPEAT-FAILURES:FAIL\n"); return 1; }
    lib_c_printf("V30:CONDITIONAL-REPEAT:OK\n");
    return 0;
}
