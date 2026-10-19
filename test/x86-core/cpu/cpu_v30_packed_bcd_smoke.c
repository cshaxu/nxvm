#include "support/cpu_bus_fixture.h"
#include "lib/types/file.h"

static lib_u32 v30_bcd_source_base(const cpu_bus_fixture *fixture, lib_u8 prefix)
{
    switch (prefix) {
    case 0x26u: return fixture->cpu.data.es.base;
    case 0x2eu: return fixture->cpu.data.cs.base;
    case 0x36u: return fixture->cpu.data.ss.base;
    default: return fixture->cpu.data.ds.base;
    }
}

static lib_size v30_bcd_address(lib_u32 base, lib_u16 offset)
{
    return (base + offset) % sizeof(((cpu_bus_fixture *)LIB_NULL)->memory);
}

static void v30_bcd_fill(cpu_bus_fixture *fixture, lib_u8 prefix,
    lib_u8 source, lib_u8 destination)
{
    const lib_u16 bytes = ((lib_u16)fixture->cpu.data.cl + 1u) / 2u;
    for (lib_u16 index = 0u; index < bytes; ++index)
        fixture->memory[v30_bcd_address(v30_bcd_source_base(fixture, prefix),
            (lib_u16)(fixture->cpu.data.si + index))] = source;
    for (lib_u16 index = 0u; index < bytes; ++index)
        fixture->memory[v30_bcd_address(fixture->cpu.data.es.base,
            (lib_u16)(fixture->cpu.data.di + index))] = destination;
}

static void v30_bcd_prepare(cpu_bus_fixture *fixture, lib_u8 digits,
    lib_bool direction)
{
    cpu_bus_prepare(fixture, X86_CORE_CPU_PROFILE_V30);
    fixture->cpu.data.cx = 0xa500u | digits;
    fixture->cpu.data.si = 0x500u;
    fixture->cpu.data.di = 0x100u;
    fixture->cpu.data.ds.base = 0x100u;
    fixture->cpu.data.es.base = 0x200u;
    fixture->cpu.data.ss.base = 0x300u;
    fixture->cpu.data.eflags = X86_CPU_NEC_PSW_MD | 2u | VCPU_EFLAGS_CF |
        VCPU_EFLAGS_OF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF |
        (direction ? VCPU_EFLAGS_DF : 0u);
}

static lib_i32 v30_bcd_run(cpu_bus_fixture *fixture, lib_u8 opcode, lib_u8 prefix)
{
    lib_u8 expected[sizeof(fixture->memory)];
    const lib_u8 code[] = {prefix ? prefix : 0x0fu, prefix ? 0x0fu : opcode, opcode};
    const lib_u8 size = prefix ? 3u : 2u;
    const lib_u16 bytes = ((lib_u16)fixture->cpu.data.cl + 1u) / 2u;
    const lib_bool odd = (fixture->cpu.data.cl & 1u) != 0u;
    const t_cpu before = fixture->cpu;
    lib_bool carry = LIB_FALSE;
    lib_bool zero = LIB_TRUE;
    x86_core_cpu_instruction_lexeme lexeme;

    lib_memory_copy(&fixture->memory[0x100u], code, size);
    lib_memory_copy(expected, fixture->memory, sizeof(expected));
    for (lib_u16 index = 0u; index < bytes; ++index) {
        const lib_size source_address = v30_bcd_address(v30_bcd_source_base(fixture, prefix),
            (lib_u16)(before.data.si + index));
        const lib_size destination_address = v30_bcd_address(before.data.es.base,
            (lib_u16)(before.data.di + index));
        const lib_u8 source = expected[source_address];
        const lib_u8 destination = expected[destination_address];
        lib_u8 result = 0u;
        /* Independent digit-at-a-time oracle, not the production base-100 algorithm. */
        for (lib_u8 nibble = 0u; nibble < 2u; ++nibble) {
            const lib_i32 a = (destination >> (nibble * 4u)) & 15u;
            const lib_i32 b = (source >> (nibble * 4u)) & 15u;
            lib_i32 value = opcode == 0x20u ? a + b + (carry ? 1 : 0) :
                a - b - (carry ? 1 : 0);
            carry = opcode == 0x20u ? value >= 10 : value < 0;
            if (carry) value += opcode == 0x20u ? -10 : 10;
            result |= (lib_u8)((lib_u32)value << (nibble * 4u));
        }
        zero = zero && result == 0u;
        if (opcode != 0x26u) expected[destination_address] = result;
    }
    if (!x86_core_cpu_instruction_lexeme_scan(code, size, X86_CORE_CPU_PROFILE_V30,
        LIB_FALSE, &lexeme) || lexeme.byte_count != size) return 1;
    for (lib_u8 available = 0u; available < size; ++available)
        if (x86_core_cpu_instruction_lexeme_scan(code, available,
            X86_CORE_CPU_PROFILE_V30, LIB_FALSE, &lexeme)) return 1;
    if (!x86_core_cpu_execution_preview_lexeme(&fixture->execution, &lexeme) ||
        lexeme.byte_count != size || fixture->writes != 0u) return 1;
    fixture->observations = 0u;
    x86_core_cpu_execution_refresh(&fixture->execution);
    t_cpu actual_cpu = fixture->cpu, expected_cpu = before;
    expected_cpu.data.eip += size;
    expected_cpu.data.eflags = (expected_cpu.data.eflags & ~(VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF)) |
        (carry ? VCPU_EFLAGS_CF : 0u) | (zero ? VCPU_EFLAGS_ZF : 0u);
    const lib_u32 defined = ~(VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
    actual_cpu.data.eflags &= defined;
    expected_cpu.data.eflags &= defined;
    const lib_u64 clocks = 7u + 19u * bytes + (prefix ? 2u : 0u);
    const lib_u32 key = clocks ? 0x0fu | ((lib_u32)opcode << 16u) :
        X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED;
    const x86_core_cpu_timing_result timing = x86_core_cpu_capture_timing(&fixture->execution);
    if (timing.ticks != clocks || !timing.source_timing_unallocated || timing.event_ticks ||
        timing.form_id != key || timing.key_id != key || fixture->observations ||
        timing.formula_inputs != (clocks ? X86_CORE_CPU_TIMING_INPUT_MEMORY | X86_CORE_CPU_TIMING_INPUT_SIZE |
            (prefix ? X86_CORE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE : 0u) : 0u) ||
        timing.retirement_origin != (odd ? X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_L2_DYNAMIC_ARITHMETIC :
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE)) return 1;
    if (fixture->execution.stop_requested || fixture->faults || fixture->delivered_exceptions ||
        lib_memory_compare(&actual_cpu,&expected_cpu,sizeof(expected_cpu)) ||
        fixture->writes != (opcode == 0x26u ? 0u : bytes) ||
        fixture->instructions.data.msize != bytes * (opcode == 0x26u ? 2u : 3u)) return 1;
    if (lib_memory_compare(expected, fixture->memory, sizeof(expected)) != 0) return 1;
    return (fixture->instructions.data.udf & (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_PF | VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF)) !=
        (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF) ||
        (fixture->cpu.data.eflags & (X86_CPU_NEC_PSW_MD | VCPU_EFLAGS_DF | 2u)) !=
        (before.data.eflags & (X86_CPU_NEC_PSW_MD | VCPU_EFLAGS_DF | 2u));
}

static lib_i32 v30_bcd_operands(void)
{
    static const lib_u8 opcodes[] = {0x20u,0x22u,0x26u};
    cpu_bus_fixture fixture;
    lib_u32 odd_cases = 0u;
    for (lib_size op = 0u; op < sizeof(opcodes); ++op)
        for (lib_u16 source = 0u; source < 100u; ++source)
            for (lib_u16 destination = 0u; destination < 100u; ++destination)
            for (lib_u8 digits = 1u; digits <= 2u; ++digits) {
                v30_bcd_prepare(&fixture, digits, LIB_FALSE);
                v30_bcd_fill(&fixture, 0u, (lib_u8)(((source / 10u) << 4u) | (source % 10u)),
                    (lib_u8)(((destination / 10u) << 4u) | (destination % 10u)));
                if (v30_bcd_run(&fixture, opcodes[op], 0u)) return 1;
                if (digits == 1u) ++odd_cases;
            }
    for (lib_size op = 0u; op < sizeof(opcodes); ++op)
        for (lib_u16 digits = 1u; digits <= 254u; ++digits)
            for (lib_u8 direction = 0u; direction < 2u; ++direction) {
                v30_bcd_prepare(&fixture, (lib_u8)digits, direction != 0u);
                v30_bcd_fill(&fixture, 0u, direction ? 0x01u : 0x99u,
                    direction ? 0x99u : 0u);
                if (v30_bcd_run(&fixture, opcodes[op], 0u)) return 1;
            }
    lib_c_printf("V30:BCD-ODD-BYTES:OK cases=%u lengths=127\n",odd_cases);
    return 0;
}

static lib_i32 v30_bcd_padding(void)
{
    static const struct { lib_u8 opcode, source, destination, result, flags; } cases[] = {
        {0x20u,0x01u,0x09u,0x10u,0u}, {0x20u,0u,0u,0u,VCPU_EFLAGS_ZF},
        {0x22u,0x01u,0u,0x99u,VCPU_EFLAGS_CF}, {0x26u,0x01u,0u,0u,VCPU_EFLAGS_CF},
        {0x20u,0x90u,0x10u,0u,VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF},
        {0x26u,0x10u,0x20u,0x20u,0u}, {0x22u,0x20u,0x10u,0x90u,VCPU_EFLAGS_CF},
        {0x26u,0x90u,0x90u,0x90u,VCPU_EFLAGS_ZF}
    };
    static const lib_u8 prefixes[] = {0u,0x26u,0x2eu,0x36u,0x3eu};
    cpu_bus_fixture fixture;
    lib_u32 count = 0u;
    for (lib_size item = 0u; item < sizeof(cases)/sizeof(cases[0]); ++item)
        for (lib_size prefix = 0u; prefix < sizeof(prefixes); ++prefix)
            for (lib_u8 direction = 0u; direction < 2u; ++direction) {
                v30_bcd_prepare(&fixture,1u,direction != 0u);
                v30_bcd_fill(&fixture,prefixes[prefix],cases[item].source,cases[item].destination);
                if (v30_bcd_run(&fixture,cases[item].opcode,prefixes[prefix]) ||
                    fixture.memory[0x300u] != cases[item].result ||
                    (fixture.cpu.data.eflags & (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF)) != cases[item].flags)
                    return 1;
                ++count;
            }
    lib_c_printf("V30:BCD-PADDING:OK cases=%u\n",count);
    return 0;
}

static lib_i32 v30_bcd_addressing(void)
{
    static const lib_u8 opcodes[] = {0x20u,0x22u,0x26u};
    static const lib_u8 prefixes[] = {0u,0x26u,0x2eu,0x36u,0x3eu};
    static const lib_u8 lengths[] = {2u,3u,8u,17u,254u};
    cpu_bus_fixture fixture;
    for (lib_size op = 0u; op < sizeof(opcodes); ++op)
        for (lib_size prefix = 0u; prefix < sizeof(prefixes); ++prefix)
            for (lib_size length = 0u; length < sizeof(lengths); ++length) {
                v30_bcd_prepare(&fixture, lengths[length], LIB_FALSE);
                v30_bcd_fill(&fixture, prefixes[prefix], 0x09u, 0x91u);
                if (lengths[length] & 1u) {
                    const lib_u16 last = lengths[length] / 2u;
                    fixture.memory[v30_bcd_address(v30_bcd_source_base(&fixture, prefixes[prefix]),
                        (lib_u16)(fixture.cpu.data.si + last))] = 0x99u;
                    fixture.memory[v30_bcd_address(fixture.cpu.data.es.base,
                        (lib_u16)(fixture.cpu.data.di + last))] = 0x91u;
                }
                if (v30_bcd_run(&fixture, opcodes[op], prefixes[prefix])) return 1;
            }
    for (lib_size op = 0u; op < sizeof(opcodes); ++op)
        for (lib_u8 variant = 0u; variant < 3u; ++variant) {
            v30_bcd_prepare(&fixture, 8u, LIB_TRUE);
            fixture.cpu.data.ds.base = fixture.cpu.data.es.base = 0u;
            fixture.cpu.data.si = variant == 0u ? 0xffffu : 0x500u;
            fixture.cpu.data.di = variant == 1u ? 0xffffu :
                variant == 2u ? 0x501u : 0x600u;
            v30_bcd_fill(&fixture, 0u, 0x12u, 0x93u);
            if (v30_bcd_run(&fixture, opcodes[op], 0u)) return 1;
        }
    return 0;
}

static lib_i32 v30_bcd_example(void)
{
    static const lib_u8 code[] = {0x0fu,0x26u,0x0fu,0x20u,0x0fu,0x26u,
        0x0fu,0x22u,0x0fu,0x22u,0x0fu,0x22u};
    static const lib_u8 results[][4] = {
        {0x21u,0x43u,0x65u,0x07u},{0x42u,0x86u,0x30u,0x15u},
        {0x42u,0x86u,0x30u,0x15u},{0x21u,0x43u,0x65u,0x07u},
        {0u,0u,0u,0u},{0x79u,0x56u,0x34u,0x92u}
    };
    static const lib_u32 flags[] = {VCPU_EFLAGS_ZF,0u,0u,0u,VCPU_EFLAGS_ZF,VCPU_EFLAGS_CF};
    cpu_bus_fixture fixture;
    v30_bcd_prepare(&fixture, 8u, LIB_FALSE);
    lib_memory_copy(&fixture.memory[0x100u], code, sizeof(code));
    lib_memory_copy(&fixture.memory[0x600u], results[0], 4u);
    lib_memory_copy(&fixture.memory[0x300u], results[0], 4u);
    for (lib_size step = 0u; step < sizeof(flags) / sizeof(flags[0]); ++step) {
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (fixture.execution.stop_requested || fixture.faults ||
            fixture.cpu.data.eip != 0x102u + step * 2u || fixture.cpu.data.si != 0x500u ||
            fixture.cpu.data.di != 0x100u || fixture.cpu.data.cx != 0xa508u ||
            lib_memory_compare(&fixture.memory[0x300u], results[step], 4u) != 0 ||
            (fixture.cpu.data.eflags & (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF)) != flags[step]) return 1;
    }
    return 0;
}

typedef struct v30_bcd_failure_fixture {
    cpu_bus_fixture bus;
    lib_u32 reads;
    lib_u32 writes;
    lib_u32 fail_read;
    lib_u32 fail_write;
} v30_bcd_failure_fixture;

static lib_status v30_bcd_read(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch, lib_u8 cache_control)
{
    v30_bcd_failure_fixture *fixture = opaque;
    if (provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA && !observe_only &&
        ++fixture->reads == fixture->fail_read) return LIB_STATUS_IO_ERROR;
    return cpu_bus_read(&fixture->bus, address, destination, bytes, provenance,
        observe_only, reset_fetch, cache_control);
}

static lib_status v30_bcd_write(void *opaque, lib_u32 address, const void *source,
    lib_u8 bytes, x86_core_cpu_memory_access_provenance provenance, lib_u8 cache_control)
{
    v30_bcd_failure_fixture *fixture = opaque;
    if (++fixture->writes == fixture->fail_write) return LIB_STATUS_IO_ERROR;
    return cpu_bus_write(&fixture->bus, address, source, bytes, provenance, cache_control);
}

static lib_i32 v30_bcd_failures(void)
{
    static const lib_u8 opcodes[] = {0x20u,0x22u,0x26u};
    for (lib_size op = 0u; op < sizeof(opcodes); ++op)
        for (lib_u8 odd = 0u; odd < 2u; ++odd)
        for (lib_u32 failure = 1u; failure <= (opcodes[op] == 0x26u ? 6u : 9u); ++failure) {
            v30_bcd_failure_fixture fixture = {0};
            x86_core_cpu_bus_provider provider = cpu_bus_provider;
            const lib_u8 code[] = {0x0fu,opcodes[op]};
            const lib_u32 accepted = opcodes[op] == 0x26u ? 0u :
                failure <= 6u ? (failure - 1u) / 2u : failure - 7u;
            lib_u32 flags;

            v30_bcd_prepare(&fixture.bus, 6u-odd, LIB_FALSE);
            v30_bcd_fill(&fixture.bus, 0u, 0x01u, opcodes[op] == 0x20u ? 0x99u : 0u);
            fixture.fail_read = failure <= 6u ? failure : 0u;
            fixture.fail_write = failure > 6u ? failure - 6u : 0u;
            provider.read_memory = v30_bcd_read;
            provider.write_memory = v30_bcd_write;
            fixture.bus.execution.bus = &provider;
            fixture.bus.execution.bus_context = &fixture;
            flags = fixture.bus.cpu.data.eflags;
            const t_cpu before = fixture.bus.cpu;
            lib_memory_copy(&fixture.bus.memory[0x100u], code, sizeof(code));
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (!fixture.bus.execution.stop_requested || fixture.bus.faults != 1u ||
                fixture.bus.execution.completed_observation_valid || fixture.bus.execution.timing_result.ticks ||
                fixture.bus.execution.timing_result.retirement_origin != X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED ||
                fixture.bus.delivered_exceptions || fixture.bus.writes != accepted ||
                lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before)) ||
                fixture.bus.cpu.data.eip != 0x100u || fixture.bus.cpu.data.cx != (0xa506u-odd) ||
                fixture.bus.cpu.data.si != 0x500u || fixture.bus.cpu.data.di != 0x100u ||
                fixture.bus.cpu.data.eflags != flags ||
                (fixture.fail_read && fixture.reads != fixture.fail_read) ||
                (fixture.fail_write && fixture.writes != fixture.fail_write)) return 1;
            for (lib_u32 index = 0u; index < 3u; ++index) {
                const lib_u8 expected = index < accepted ?
                    (opcodes[op] == 0x20u ? (index ? 0x01u : 0u) : (index ? 0x98u : 0x99u)) :
                    (opcodes[op] == 0x20u ? 0x99u : 0u);
                if (fixture.bus.memory[0x300u + index] != expected) return 1;
            }
            /* The owning executor consumes the request; refresh is not a stop latch. */
            if (!x86_core_cpu_execution_consume_stop_request(&fixture.bus.execution) ||
                x86_core_cpu_execution_consume_stop_request(&fixture.bus.execution)) return 1;
        }
    for (lib_size op = 0u; op < sizeof(opcodes); ++op)
        for (lib_u8 variant = 0u; variant < 5u; ++variant) {
            v30_bcd_failure_fixture fixture = {0};
            x86_core_cpu_bus_provider provider = cpu_bus_provider;
            const lib_u8 code[] = {0x0fu,opcodes[op]};
            v30_bcd_prepare(&fixture.bus, variant == 0u ? 0u : variant == 1u ? 255u : variant == 2u ? 2u : 1u,
                LIB_FALSE);
            provider.read_memory = v30_bcd_read;
            fixture.bus.execution.bus = &provider;
            fixture.bus.execution.bus_context = &fixture;
            fixture.bus.memory[variant == 4u ? 0x300u : 0x600u] =
                variant < 3u ? 0xfau : variant == 3u ? 0xf9u : 0xe1u;
            lib_memory_copy(&fixture.bus.memory[0x100u], code, sizeof(code));
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (!fixture.bus.execution.stop_requested || fixture.bus.writes ||
                fixture.bus.execution.completed_observation_valid || fixture.bus.execution.timing_result.ticks ||
                fixture.bus.execution.timing_result.retirement_origin != X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED ||
                fixture.bus.cpu.data.eip != 0x100u || fixture.bus.delivered_exceptions ||
                fixture.reads != (variant >= 2u ? 2u : 0u)) return 1;
        }
    return 0;
}

static lib_i32 v30_bcd_invalid_digits(void)
{
    static const lib_u8 opcodes[]={0x20u,0x22u,0x26u};
    static const lib_u8 prefixes[]={0u,0x26u,0x2eu,0x36u,0x3eu};
    lib_u32 cases=0u;
    for (lib_size op=0u; op<sizeof(opcodes); ++op)
      for (lib_size prefix=0u; prefix<sizeof(prefixes); ++prefix)
       for (lib_u8 odd=0u; odd<2u; ++odd)
        for (lib_u8 side=0u; side<2u; ++side)
         for (lib_u8 position=0u; position<3u; ++position)
          for (lib_u16 value=0u; value<256u; ++value) {
            if ((value & 15u)<10u && (value >> 4u)<10u) continue;
            v30_bcd_failure_fixture fixture={0};
            x86_core_cpu_bus_provider provider=cpu_bus_provider;
            lib_u8 expected[sizeof(fixture.bus.memory)];
            v30_bcd_prepare(&fixture.bus,6u-odd,LIB_FALSE);
            v30_bcd_fill(&fixture.bus,prefixes[prefix],0x01u,0x19u);
            const lib_u32 base=side ? fixture.bus.cpu.data.es.base :
                v30_bcd_source_base(&fixture.bus,prefixes[prefix]);
            const lib_u16 offset=side ? fixture.bus.cpu.data.di : fixture.bus.cpu.data.si;
            fixture.bus.memory[v30_bcd_address(base,(lib_u16)(offset+position))]=(lib_u8)value;
            const lib_u8 code[]={prefixes[prefix] ? prefixes[prefix] : 0x0fu,
                prefixes[prefix] ? 0x0fu : opcodes[op],opcodes[op]};
            lib_memory_copy(fixture.bus.memory+0x100u,code,prefixes[prefix] ? 3u : 2u);
            lib_memory_copy(expected,fixture.bus.memory,sizeof(expected));
            if (opcodes[op]!=0x26u)
                for (lib_u8 byte=0u; byte<position; ++byte)
                    expected[v30_bcd_address(fixture.bus.cpu.data.es.base,
                        (lib_u16)(fixture.bus.cpu.data.di+byte))]=opcodes[op]==0x20u ? 0x20u : 0x18u;
            provider.read_memory=v30_bcd_read;
            provider.write_memory=v30_bcd_write;
            fixture.bus.execution.bus=&provider;
            fixture.bus.execution.bus_context=&fixture;
            const t_cpu before=fixture.bus.cpu;
            x86_core_cpu_execution_refresh(&fixture.bus.execution);
            if (!fixture.bus.execution.stop_requested || fixture.bus.faults!=1u ||
                fixture.bus.delivered_exceptions || fixture.bus.execution.completed_observation_valid ||
                fixture.bus.execution.timing_result.ticks ||
                fixture.bus.execution.timing_result.retirement_origin!=
                    X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED ||
                fixture.reads!=2u*(position+1u) ||
                fixture.writes!=(opcodes[op]==0x26u ? 0u : position) ||
                lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before)) ||
                lib_memory_compare(fixture.bus.memory,expected,sizeof(expected))) {
                lib_c_printf("V30:BCD-INVALID:FAIL op=%02x prefix=%02x odd=%u side=%u position=%u byte=%02x\n",
                    opcodes[op],prefixes[prefix],odd,side,position,value);
                return 1;
            }
            ++cases;
          }
    lib_c_printf("V30:BCD-INVALID:OK cases=%u\n",cases);
    return 0;
}

lib_i32 main(void)
{
    if (v30_bcd_operands()) { lib_c_printf("V30:BCD-OPERANDS:FAIL\n"); return 1; }
    if (v30_bcd_padding()) { lib_c_printf("V30:BCD-PADDING:FAIL\n"); return 1; }
    if (v30_bcd_addressing()) { lib_c_printf("V30:BCD-ADDRESSING:FAIL\n"); return 1; }
    if (v30_bcd_example()) { lib_c_printf("V30:BCD-EXAMPLE:FAIL\n"); return 1; }
    if (v30_bcd_failures()) { lib_c_printf("V30:BCD-FAILURE:FAIL\n"); return 1; }
    if (v30_bcd_invalid_digits()) return 1;
    lib_c_printf("V30:PACKED-BCD:OK\n");
    lib_c_printf("V30:NOMINAL-BCD:OK\n");
    return 0;
}
