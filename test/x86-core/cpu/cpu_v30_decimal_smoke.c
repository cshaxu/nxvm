#include "support/cpu_v30_memory_fixture.h"
#include "lib/types/file.h"

/* NEC scalar decimal adjustment and documented radix-ten conversion. */
typedef struct v30_decimal_adjustment {
    lib_u8 byte;
    lib_bool auxiliary, carry;
} v30_decimal_adjustment;

static lib_u16 v30_decimal_result(lib_u8 operation, lib_u16 input,
    lib_u32 *flags, lib_u32 *defined, const v30_decimal_adjustment *packed)
{
    const lib_u8 low = (lib_u8)input, high = (lib_u8)(input >> 8u);
    if (operation < 2u) {
        const lib_bool adjust = low % 16u > 9u || (*flags & VCPU_EFLAGS_AF);
        const lib_i32 direction = operation == 0u ? 1 : -1;
        const lib_u8 upper = (lib_u8)(high + (adjust ? direction : 0));
        const lib_u8 lower = (lib_u8)(low + (adjust ? direction * 6 : 0)) % 16u;
        *defined = ~(VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF);
        *flags = (*flags & ~(VCPU_EFLAGS_CF | VCPU_EFLAGS_AF)) |
            (adjust ? VCPU_EFLAGS_CF | VCPU_EFLAGS_AF : 0u);
        return (lib_u16)(256u * upper + lower);
    }
    const lib_u16 result = operation < 4u ? (operation == 2u ?
        (lib_u16)(256u * (low / 10u) + low % 10u) : (lib_u8)(10u * high + low)) :
        (lib_u16)(256u * high + packed->byte);
    const lib_u8 byte = (lib_u8)result;
    lib_u8 ones = 0u;
    for (lib_u8 bit = 0u; bit < 8u; ++bit) ones += (byte >> bit) & 1u;
    *defined = operation < 4u ? ~(VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_CF) :
        ~VCPU_EFLAGS_OF;
    if (operation >= 4u)
        *flags = (*flags & ~(VCPU_EFLAGS_CF | VCPU_EFLAGS_AF)) |
            (packed->carry ? VCPU_EFLAGS_CF : 0u) |
            (packed->auxiliary ? VCPU_EFLAGS_AF : 0u);
    *flags = (*flags & ~(VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF)) |
        (byte >= 128u ? VCPU_EFLAGS_SF : 0u) | (byte == 0u ? VCPU_EFLAGS_ZF : 0u) |
        (ones % 2u == 0u ? VCPU_EFLAGS_PF : 0u);
    return result;
}

static lib_u32 v30_decimal_flags(lib_u8 seed)
{
    static const lib_u32 bits[] = {VCPU_EFLAGS_CF, VCPU_EFLAGS_PF, VCPU_EFLAGS_AF,
        VCPU_EFLAGS_ZF, VCPU_EFLAGS_SF, VCPU_EFLAGS_OF};
    lib_u32 flags = VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
    for (lib_u8 bit = 0u; bit < sizeof(bits)/sizeof(bits[0]); ++bit)
        if (seed & (1u << bit)) flags |= bits[bit];
    return flags;
}

static void v30_decimal_prepare(v30_memory_fixture *fixture, lib_u8 operation,
    lib_u16 input, lib_u8 seed, lib_u8 prefix, lib_bool wrap)
{
    static const lib_u8 opcodes[] = {0x37u,0x3fu,0xd4u,0xd5u,0x27u,0x2fu};
    static const lib_u8 prefixes[] = {0u,0x26u,0x2eu,0x36u,0x3eu};
    v30_memory_restart(fixture);
    fixture->bus.cpu.data.eax = 0x12340000u | input;
    fixture->bus.cpu.data.eflags |= v30_decimal_flags(seed);
    fixture->bus.cpu.data.eip = wrap ? 0xffffu : 0x100u;
    lib_u16 offset = (lib_u16)fixture->bus.cpu.data.eip;
    if (prefix) fixture->memory[0x80000u + offset++] = prefixes[prefix];
    fixture->memory[0x80000u + offset++] = opcodes[operation];
    if (operation == 2u || operation == 3u) fixture->memory[0x80000u + offset] = 10u;
}

static lib_bool v30_decimal_no_data(const v30_memory_fixture *fixture, lib_bool preview)
{
    if (fixture->invalid || fixture->reads || fixture->writes || fixture->phase_count) return LIB_FALSE;
    for (lib_u32 index = 0u; index < fixture->record_count; ++index) {
        const v30_memory_record *record = &fixture->records[index];
        if (record->write || record->provenance == X86_CORE_CPU_MEMORY_ACCESS_DATA ||
            (preview && !record->observe)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_i32 v30_decimal_check(lib_u8 operation, lib_u16 input, lib_u8 seed,
    lib_u8 prefix, lib_bool wrap, lib_bool deep, const v30_decimal_adjustment *packed)
{
    static v30_memory_fixture fixture;
    const lib_u8 length = 1u + (operation == 2u || operation == 3u) + (prefix != 0u);
    v30_decimal_prepare(&fixture,operation,input,seed,prefix,wrap);
    const t_cpu before = fixture.bus.cpu;
    t_cpu expected = before;
    lib_u32 defined;
    expected.data.ax = v30_decimal_result(operation,input,&expected.data.eflags,&defined,
        packed);
    expected.data.eip = (lib_u16)(expected.data.eip + length);
    lib_u32 observations = 0u;
    if (deep) {
        x86_core_cpu_instruction_lexeme lexeme;
        if (!x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme) ||
            lexeme.byte_count != length || !v30_decimal_no_data(&fixture,LIB_TRUE) ||
            fixture.fetches || lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before))) {
            lib_c_printf("V30:DECIMAL:PREVIEW op=%u prefix=%u wrap=%u bytes=%u/%u\n",
                operation,prefix,wrap,lexeme.byte_count,length);
            return 1;
        }
        observations = fixture.observations;
        fixture.record_count = fixture.observations = 0u;
    }
    x86_core_cpu_execution_refresh(&fixture.bus.execution);
    t_cpu actual = fixture.bus.cpu;
    actual.data.eflags &= defined;
    expected.data.eflags &= defined;
    static const lib_u8 nominal[] = {3u,7u,15u,7u,3u,7u};
    const lib_u64 clocks = nominal[operation] ? nominal[operation] + (prefix ? 2u : 0u) : 0u;
    const x86_core_cpu_timing_result timing = x86_core_cpu_capture_timing(&fixture.bus.execution);
    const lib_u32 form = clocks ? fixture.memory[0x80000u+(lib_u16)(before.data.eip+(prefix != 0u))] :
        X86_CORE_CPU_RETIREMENT_SOURCE_FORM_UNATTRIBUTED;
    if (timing.ticks != clocks || !timing.source_timing_unallocated || timing.event_ticks ||
        timing.form_id != form || timing.key_id != form || timing.formula_inputs !=
            (clocks && prefix ? X86_CORE_CPU_TIMING_INPUT_SEGMENT_OVERRIDE : 0u) ||
        timing.retirement_origin != (clocks ? X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_V30_NOMINAL_SOURCE :
            X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED) || fixture.observations ||
        x86_core_cpu_timing_maximum_ticks(X86_CORE_CPU_PROFILE_V30,LIB_NULL)) return 1;
    if (fixture.bus.execution.stop_requested || fixture.bus.faults || fixture.bus.delivered_exceptions ||
        fixture.bus.instruction_count != 1u ||
        fixture.bus.execution.interrupt_shadow != CPU_INTERRUPT_SHADOW_NONE ||
        !v30_decimal_no_data(&fixture,LIB_FALSE) || lib_memory_compare(&actual,&expected,sizeof(actual))) {
        lib_c_printf("V30:DECIMAL:FAIL op=%u ax=%04x seed=%u prefix=%u wrap=%u result=%08x/%08x flags=%08x/%08x\n",
            operation,input,seed,prefix,wrap,actual.data.eax,expected.data.eax,
            actual.data.eflags,expected.data.eflags);
        return 1;
    }
    const lib_u32 fetches = fixture.fetches;
    if (deep) for (lib_u8 preview = 0u; preview < 2u; ++preview)
        for (lib_u32 part = 1u; part <= (preview ? observations : fetches); ++part) {
            v30_decimal_prepare(&fixture,operation,input,seed,prefix,wrap);
            if (preview) {
                x86_core_cpu_instruction_lexeme lexeme;
                fixture.fail_observe = part;
                if (x86_core_cpu_execution_preview_lexeme(&fixture.bus.execution,&lexeme) ||
                    fixture.bus.execution.stop_requested || fixture.bus.faults) {
                    lib_c_printf("V30:DECIMAL:OBSERVE op=%u prefix=%u wrap=%u part=%u\n",
                        operation,prefix,wrap,part);
                    return 1;
                }
            } else {
                fixture.fail_fetch = part;
                x86_core_cpu_execution_refresh(&fixture.bus.execution);
                /* Entry diagnostics also observe a failed fetch; they are not retirement. */
                if (!fixture.bus.execution.stop_requested || !fixture.bus.faults ||
                    fixture.bus.execution.completed_observation_valid || fixture.bus.execution.timing_result.ticks ||
                    fixture.bus.execution.timing_result.retirement_origin != X86_CORE_CPU_RETIREMENT_TIMING_ORIGIN_UNATTRIBUTED ||
                    fixture.bus.instruction_count != 1u) {
                    lib_c_printf("V30:DECIMAL:FETCH op=%u prefix=%u wrap=%u part=%u observations=%u\n",
                        operation,prefix,wrap,part,fixture.bus.instruction_count);
                    return 1;
                }
            }
            if (!v30_decimal_no_data(&fixture,preview != 0u) || fixture.bus.delivered_exceptions ||
                fixture.bus.execution.interrupt_shadow != CPU_INTERRUPT_SHADOW_NONE ||
                lib_memory_compare(&fixture.bus.cpu,&before,sizeof(before))) {
                lib_c_printf("V30:DECIMAL:ROLLBACK op=%u prefix=%u wrap=%u preview=%u part=%u ip=%x/%x\n",
                    operation,prefix,wrap,preview,part,fixture.bus.cpu.data.eip,before.data.eip);
                return 1;
            }
        }
    return 0;
}

lib_i32 main(void)
{
    static const lib_u16 boundaries[] = {0u,5u,6u,9u,10u,0x99u,0x9au,0xffu,
        0x0100u,0x09ffu,0x7fffu,0xffffu};
    lib_u32 cases = 0u;
    for (lib_u8 operation = 0u; operation < 4u; ++operation)
        for (lib_u32 input = 0u; input <= 65535u; ++input)
            for (lib_u8 seed = 0u; seed < (operation < 2u ? 4u : 2u); ++seed) {
                const lib_u8 flags = operation < 2u ?
                    (lib_u8)(((seed & 1u) ? 4u : 0u) | ((seed & 2u) ? 1u : 0u)) : seed ? 63u : 0u;
                if (v30_decimal_check(operation,(lib_u16)input,flags,0u,LIB_FALSE,LIB_FALSE,
                    LIB_NULL)) return 1;
                ++cases;
            }
    for (lib_u8 operation = 0u; operation < 4u; ++operation)
        for (lib_u8 prefix = 0u; prefix < 5u; ++prefix)
            for (lib_u8 seed = 0u; seed < 64u; ++seed)
                for (lib_u8 value = 0u; value < sizeof(boundaries)/sizeof(boundaries[0]); ++value)
                    for (lib_u8 wrap = 0u; wrap < 2u; ++wrap) {
                        if (v30_decimal_check(operation,boundaries[value],seed,prefix,wrap != 0u,LIB_TRUE,
                            LIB_NULL)) return 1;
                        ++cases;
                    }
    lib_c_printf("V30:DECIMAL:OK cases=%u\n",cases);
    lib_c_printf("V30:NOMINAL-DECIMAL:OK cases=%u\n",cases);
    lib_u32 packed_cases = 0u;
    for (lib_u8 operation = 4u; operation < 6u; ++operation)
        for (lib_i32 left = 0; left < 100; ++left)
            for (lib_i32 right = 0; right < 100; ++right)
                for (lib_u8 carry = 0u; carry < 2u; ++carry) {
                    const lib_i32 direction = operation == 4u ? 1 : -1;
                    const lib_i32 binary = 16 * (left / 10) + left % 10 + direction *
                        (16 * (right / 10) + right % 10 + carry);
                    const lib_i32 low = left % 10 + direction * (right % 10 + carry);
                    const lib_i32 decimal = left + direction * (right + carry);
                    const lib_bool auxiliary = low < 0 || low >= 10;
                    const lib_u8 digits = (lib_u8)((decimal + 100) % 100);
                    const v30_decimal_adjustment packed = {
                        .byte=(lib_u8)(16u*(digits/10u)+digits%10u),
                        .auxiliary=auxiliary,.carry=decimal<0 || decimal>=100
                    };
                    const lib_u8 seed = (lib_u8)(58u |
                        (binary < 0 || binary > 255 ? 1u : 0u) |
                        (low < 0 || low > 15 ? 4u : 0u));
                    const lib_u16 input = (lib_u16)(256u * (lib_u8)(left * right + 0xa5) +
                        (lib_u8)binary);
                    if (v30_decimal_check(operation,input,seed,0u,LIB_FALSE,LIB_FALSE,
                        &packed)) return 1;
                    ++packed_cases;
                    /* Carry, low correction, zero and wrap witnesses use every segment/fetch seam. */
                    if ((left == 0 || left == 9 || left == 99) &&
                        (right == 0 || right == 9 || right == 99))
                        for (lib_u8 prefix = 0u; prefix < 5u; ++prefix)
                            for (lib_u8 wrap = 0u; wrap < 2u; ++wrap) {
                                if (v30_decimal_check(operation,input,seed,prefix,wrap != 0u,
                                    LIB_TRUE,&packed)) return 1;
                                ++packed_cases;
                            }
                }
    lib_c_printf("V30:PACKED-ADJUST:OK cases=%u\n",packed_cases);
    lib_u32 raw_cases=0u;
    for (lib_u8 operation=4u; operation<6u; ++operation)
        for (lib_u32 input=0u; input<=65535u; ++input)
            for (lib_u8 flags=0u; flags<4u; ++flags) {
                const lib_u8 seed=(lib_u8)((flags & 1u) | ((flags & 2u) ? 4u : 0u));
                const lib_u8 low=(lib_u8)input;
                const lib_bool auxiliary=low%16u>9u || (flags & 2u);
                const lib_i32 direction=operation==4u ? 1 : -1;
                const lib_i32 corrected=(lib_i32)low+direction*(auxiliary ? 6 : 0);
                const lib_bool carry=(flags & 1u) || corrected<0 || corrected>255 ||
                    (lib_u8)corrected>0x9fu;
                const v30_decimal_adjustment packed={
                    .byte=(lib_u8)(corrected+direction*(carry ? 0x60 : 0)),
                    .auxiliary=auxiliary,.carry=carry
                };
                if (v30_decimal_check(operation,(lib_u16)input,seed,0u,LIB_FALSE,
                        LIB_FALSE,&packed)) return 1;
                ++raw_cases;
                /* Raw-input model boundaries also use the original failure checker. */
                if ((input==0u || input==0xffffu || input==0x009au || input==0xfffau))
                    for (lib_u8 prefix=0u; prefix<5u; ++prefix)
                        for (lib_u8 wrap=0u; wrap<2u; ++wrap) {
                            if (v30_decimal_check(operation,(lib_u16)input,seed,prefix,
                                    wrap!=0u,LIB_TRUE,&packed)) return 1;
                            ++raw_cases;
                        }
            }
    lib_c_printf("V30:PACKED-RAW-L2:OK cases=%u\n",raw_cases);
    return 0;
}
