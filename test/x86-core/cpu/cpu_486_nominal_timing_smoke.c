#include "support/cpu_port_instruction_fixture.h"
#include "x86-core/cpu/cpu_timing.h"
#include "lib/types/file.h"

typedef struct nominal_case {
    const char *name;
    lib_u8 code[8];
    lib_u8 bytes;
    lib_u64 ticks;
} nominal_case;

/* Independent expected cache-hit columns from Intel 1992 PRM E-3--E-13.
 * Actual execution supplies ModR/M, mode and operand state to the calculator.
 * These are nominal-only checks, not dynamic-penalty qualification. */
static const nominal_case nominal_cases[] = {
    {"ADD register", {0x00u, 0xc1u}, 2u, 1u},
    {"ADD memory destination", {0x00u, 0x06u, 0u, 0x20u}, 4u, 3u},
    {"ADD memory source", {0x02u, 0x06u, 0u, 0x20u}, 4u, 2u},
    {"ADC accumulator", {0x14u, 1u}, 2u, 1u},
    {"SBB register", {0x18u, 0xc1u}, 2u, 1u},
    {"AND memory", {0x20u, 0x06u, 0u, 0x20u}, 4u, 3u},
    {"XOR memory", {0x32u, 0x06u, 0u, 0x20u}, 4u, 2u},
    {"CMP memory destination", {0x38u, 0x06u, 0u, 0x20u}, 4u, 2u},
    {"CMP memory source", {0x3au, 0x06u, 0u, 0x20u}, 4u, 2u},
    {"DAA", {0x27u}, 1u, 2u},
    {"DAS", {0x2fu}, 1u, 2u},
    {"AAA", {0x37u}, 1u, 3u},
    {"AAS", {0x3fu}, 1u, 3u},
    {"INC register", {0x40u}, 1u, 1u},
    {"DEC register", {0x48u}, 1u, 1u},
    {"PUSH short register", {0x50u}, 1u, 1u},
    {"POP short register", {0x58u}, 1u, 1u},
    {"PUSH segment", {0x1eu}, 1u, 3u},
    {"POP segment", {0x1fu}, 1u, 3u},
    {"PUSHA", {0x60u}, 1u, 11u},
    {"POPA", {0x61u}, 1u, 9u},
    {"PUSH immediate", {0x68u, 1u, 0u}, 3u, 1u},
    {"PUSH signed immediate", {0x6au, 1u}, 2u, 1u},
    {"JZ not taken", {0x74u, 0u}, 2u, 1u},
    {"JNZ taken", {0x75u, 0u}, 2u, 3u},
    {"ALU immediate register", {0x80u, 0xc1u, 1u}, 3u, 1u},
    {"ALU immediate memory", {0x80u, 0x06u, 0u, 0x20u, 1u}, 5u, 3u},
    {"CMP immediate memory", {0x80u, 0x3eu, 0u, 0x20u, 1u}, 5u, 2u},
    {"TEST register", {0x84u, 0xc1u}, 2u, 1u},
    {"TEST memory", {0x84u, 0x06u, 0u, 0x20u}, 4u, 2u},
    {"XCHG register", {0x86u, 0xc1u}, 2u, 3u},
    {"XCHG accumulator", {0x91u}, 1u, 3u},
    {"MOV register", {0x88u, 0xc1u}, 2u, 1u},
    {"MOV memory source", {0x8au, 0x06u, 0u, 0x20u}, 4u, 1u},
    {"MOV memory destination", {0x88u, 0x06u, 0u, 0x20u}, 4u, 1u},
    {"MOV moffs read", {0xa0u, 0u, 0x20u}, 3u, 1u},
    {"MOV moffs write", {0xa2u, 0u, 0x20u}, 3u, 1u},
    {"MOV immediate register", {0xb0u, 1u}, 2u, 1u},
    {"MOV immediate memory", {0xc6u, 0x06u, 0u, 0x20u, 1u}, 5u, 1u},
    {"MOV segment load", {0x8eu, 0xd8u}, 2u, 3u},
    {"MOV segment store", {0x8cu, 0xd8u}, 2u, 3u},
    {"LEA base", {0x8du, 0x04u}, 2u, 1u},
    {"LEA base index", {0x8du, 0x00u}, 2u, 2u},
    {"LEA displacement", {0x8du, 0x06u, 0u, 0x20u}, 4u, 1u},
    {"LEA SIB base index", {0x67u, 0x8du, 0x04u, 0x31u}, 4u, 2u},
    {"LEA SIB displacement", {0x67u, 0x8du, 0x04u, 0x25u, 0u, 0x20u, 0u, 0u}, 8u, 1u},
    {"POP long register", {0x8fu, 0xc0u}, 2u, 4u},
    {"POP memory", {0x8fu, 0x06u, 0u, 0x20u}, 4u, 5u},
    {"NOP", {0x90u}, 1u, 1u},
    {"CBW", {0x98u}, 1u, 3u},
    {"CWD", {0x99u}, 1u, 3u},
    {"PUSHF real", {0x9cu}, 1u, 4u},
    {"POPF real", {0x9du}, 1u, 9u},
    {"SAHF", {0x9eu}, 1u, 2u},
    {"LAHF", {0x9fu}, 1u, 3u},
    {"TEST accumulator", {0xa8u, 1u}, 2u, 1u},
    {"MOVS primitive", {0xa4u}, 1u, 7u},
    {"CMPS primitive", {0xa6u}, 1u, 8u},
    {"STOS primitive", {0xaau}, 1u, 5u},
    {"LODS primitive", {0xacu}, 1u, 5u},
    {"SCAS primitive", {0xaeu}, 1u, 6u},
    {"ROL by one", {0xd0u, 0xc0u}, 2u, 3u},
    {"RCL by one", {0xd0u, 0xd0u}, 2u, 3u},
    {"SHL by CL", {0xd2u, 0xe0u}, 2u, 3u},
    {"SHL immediate", {0xc0u, 0xe0u, 2u}, 3u, 2u},
    {"SHL memory immediate", {0xc0u, 0x26u, 0u, 0x20u, 2u}, 5u, 4u},
    {"ENTER zero", {0xc8u, 0u, 0u, 0u}, 4u, 14u},
    {"ENTER one", {0xc8u, 0u, 0u, 1u}, 4u, 17u},
    {"ENTER nested", {0xc8u, 0u, 0u, 3u}, 4u, 26u},
    {"LEAVE", {0xc9u}, 1u, 5u},
    {"RET near", {0xc3u}, 1u, 5u},
    {"RET near immediate", {0xc2u, 2u, 0u}, 3u, 5u},
    {"RET far real", {0xcbu}, 1u, 13u},
    {"RET far real immediate", {0xcau, 2u, 0u}, 3u, 14u},
    {"INT3 real", {0xccu}, 1u, 26u},
    {"INT real", {0xcdu, 0x20u}, 2u, 30u},
    {"INTO not taken", {0xceu}, 1u, 3u},
    {"IRET real", {0xcfu}, 1u, 15u},
    {"AAM", {0xd4u, 10u}, 2u, 15u},
    {"AAD", {0xd5u, 10u}, 2u, 14u},
    {"XLAT", {0xd7u}, 1u, 4u},
    {"LOOPNE taken", {0xe0u, 0u}, 2u, 9u},
    {"LOOPE not taken", {0xe1u, 0u}, 2u, 6u},
    {"LOOP taken", {0xe2u, 0u}, 2u, 7u},
    {"JCXZ not taken", {0xe3u, 0u}, 2u, 5u},
    {"CALL near", {0xe8u, 0u, 0u}, 3u, 3u},
    {"JMP near", {0xe9u, 0u, 0u}, 3u, 3u},
    {"JMP short", {0xebu, 0u}, 2u, 3u},
    {"HLT", {0xf4u}, 1u, 4u},
    {"CMC", {0xf5u}, 1u, 2u},
    {"TEST group register", {0xf6u, 0xc0u, 1u}, 3u, 1u},
    {"TEST group memory", {0xf6u, 0x06u, 0u, 0x20u, 1u}, 5u, 2u},
    {"NOT register", {0xf6u, 0xd0u}, 2u, 1u},
    {"NEG memory", {0xf6u, 0x1eu, 0u, 0x20u}, 4u, 3u},
    {"DIV byte", {0xf6u, 0xf1u}, 2u, 16u},
    {"DIV word", {0xf7u, 0xf1u}, 2u, 24u},
    {"DIV dword", {0x66u, 0xf7u, 0xf1u}, 3u, 40u},
    {"IDIV byte", {0xf6u, 0xf9u}, 2u, 19u},
    {"IDIV word", {0xf7u, 0xf9u}, 2u, 27u},
    {"IDIV dword", {0x66u, 0xf7u, 0xf9u}, 3u, 43u},
    {"CLC", {0xf8u}, 1u, 2u},
    {"STC", {0xf9u}, 1u, 2u},
    {"CLI", {0xfau}, 1u, 5u},
    {"STI", {0xfbu}, 1u, 5u},
    {"CLD", {0xfcu}, 1u, 2u},
    {"STD", {0xfdu}, 1u, 2u},
    {"INC group register", {0xfeu, 0xc0u}, 2u, 1u},
    {"DEC group memory", {0xfeu, 0x0eu, 0u, 0x20u}, 4u, 3u},
    {"PUSH long register", {0xffu, 0xf0u}, 2u, 4u},
    {"PUSH memory", {0xffu, 0x36u, 0u, 0x20u}, 4u, 4u},
    {"CALL register", {0xffu, 0xd0u}, 2u, 5u},
    {"JMP register", {0xffu, 0xe0u}, 2u, 5u},
    {"SGDT", {0x0fu, 0x01u, 0x06u, 0u, 0x20u}, 5u, 10u},
    {"SIDT", {0x0fu, 0x01u, 0x0eu, 0u, 0x20u}, 5u, 10u},
    {"SMSW register", {0x0fu, 0x01u, 0xe0u}, 3u, 2u},
    {"SMSW memory", {0x0fu, 0x01u, 0x26u, 0u, 0x20u}, 5u, 3u},
    {"MOV from CR0", {0x0fu, 0x20u, 0xc0u}, 3u, 4u},
    {"MOV to CR0", {0x0fu, 0x22u, 0xc0u}, 3u, 17u},
    {"MOV from DR0", {0x0fu, 0x21u, 0xc0u}, 3u, 9u},
    {"MOV to DR0", {0x0fu, 0x23u, 0xc0u}, 3u, 10u},
    {"MOV from TR3", {0x0fu, 0x24u, 0xd8u}, 3u, 3u},
    {"MOV to TR3", {0x0fu, 0x26u, 0xd8u}, 3u, 4u},
    {"JZ near not taken", {0x0fu, 0x84u, 0u, 0u}, 4u, 1u},
    {"JNZ near taken", {0x0fu, 0x85u, 0u, 0u}, 4u, 3u},
    {"SETZ false", {0x0fu, 0x94u, 0xc0u}, 3u, 3u},
    {"SETNZ true", {0x0fu, 0x95u, 0xc0u}, 3u, 4u},
    {"SETZ memory false", {0x0fu, 0x94u, 0x06u, 0u, 0x20u}, 5u, 4u},
    {"SETNZ memory true", {0x0fu, 0x95u, 0x06u, 0u, 0x20u}, 5u, 3u},
    {"PUSH FS", {0x0fu, 0xa0u}, 2u, 3u},
    {"POP FS", {0x0fu, 0xa1u}, 2u, 3u},
    {"MOVZX", {0x0fu, 0xb6u, 0xc0u}, 3u, 3u},
    {"MOVSX", {0x0fu, 0xbeu, 0xc0u}, 3u, 3u},
    {"BT register", {0x0fu, 0xa3u, 0xc0u}, 3u, 3u},
    {"BTS register", {0x0fu, 0xabu, 0xc0u}, 3u, 6u},
    {"BT immediate", {0x0fu, 0xbau, 0xe0u, 1u}, 4u, 3u},
    {"BTC immediate", {0x0fu, 0xbau, 0xf8u, 1u}, 4u, 6u},
    {"SHLD immediate", {0x0fu, 0xa4u, 0xc1u, 1u}, 4u, 2u},
    {"SHRD CL", {0x0fu, 0xadu, 0xc0u}, 3u, 3u},
    {"CMPXCHG register", {0x0fu, 0xb0u, 0xc1u}, 3u, 6u},
    {"BSF zero", {0x0fu, 0xbcu, 0xc0u}, 3u, 6u},
    {"BSF bit one", {0x0fu, 0xbcu, 0xc1u}, 3u, 21u},
    {"BSR zero", {0x0fu, 0xbdu, 0xc0u}, 3u, 6u},
    {"BSR bit one", {0x0fu, 0xbdu, 0xc1u}, 3u, 100u},
    {"XADD register", {0x0fu, 0xc0u, 0xc1u}, 3u, 3u},
    {"BSWAP", {0x66u, 0x0fu, 0xc8u}, 3u, 1u}
};

static lib_bool nominal_execution(void)
{
    for (lib_size index = 0u; index < sizeof(nominal_cases) / sizeof(nominal_cases[0]); ++index) {
        const nominal_case *test = &nominal_cases[index];
        cpu_instruction_fixture fixture;
        x86_core_cpu_timing_result before, after;
        t_cpu cpu_after;
        lib_u64 ticks = LIB_UINT64_MAX;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        fixture.cpu.data.esp = 0x6000u;
        fixture.cpu.data.ebp = 0x5000u;
        fixture.cpu.data.ebx = 0x2000u;
        fixture.cpu.data.ecx = 2u;
        fixture.cpu.data.edx = 0u;
        fixture.cpu.data.esi = 0x2000u;
        fixture.cpu.data.edi = 0x3000u;
        if (cpu_instruction_run(&fixture, test->code, test->bytes, &cpu_after) != LIB_STATUS_OK ||
            fixture.fault.valid || fixture.delivered_exception.valid) {
            lib_c_fprintf(lib_c_stderr, "Nominal execution: %s\n", test->name);
            return LIB_FALSE;
        }
        before = x86_core_cpu_capture_timing(&fixture.execution);
        if (!x86_core_cpu_80486_nominal_instruction_cost(&fixture.execution, &ticks) ||
            ticks != test->ticks) {
            lib_c_fprintf(lib_c_stderr, "Nominal cost: %s expected=%llu actual=%llu\n",
                test->name, test->ticks, ticks);
            return LIB_FALSE;
        }
        if (fixture.instructions.data.timing_data_misses == 0u &&
            fixture.instructions.data.timing_locked_reads == 0u) {
            lib_bool modeled = LIB_TRUE;

            if (!x86_core_cpu_80486_memory_penalty(&fixture.execution, &ticks, &modeled) ||
                ticks != 3u * (lib_u64)fixture.instructions.data.timing_unaligned_accesses || modeled) {
                lib_c_fprintf(lib_c_stderr, "Zero data surcharge: %s\n", test->name);
                return LIB_FALSE;
            }
        }
        after = x86_core_cpu_capture_timing(&fixture.execution);
        if (before.ticks != after.ticks || before.key_id != after.key_id ||
            before.formula_inputs != after.formula_inputs ||
            before.retirement_origin != after.retirement_origin ||
            before.source_timing_unallocated != after.source_timing_unallocated ||
            before.form_id != after.form_id || before.repeat_phase != after.repeat_phase ||
            before.event_ticks != after.event_ticks)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

/* E-4 gives the same row to all eight ALU operations except CMP's memory
 * destination. Exercise each encoding with real decode, including the SX 82
 * alias; operand/address overrides must not change the nominal subtotal. */
static lib_bool nominal_alu_forms(void)
{
    for (lib_u8 operation = 0u; operation < 8u; ++operation)
    for (lib_u8 size = 0u; size < 3u; ++size)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 address32 = 0u; address32 < 2u; ++address32)
    for (lib_u8 memory = 0u; memory < 2u; ++memory)
    for (lib_u8 form = 0u; form < 6u; ++form) {
        cpu_instruction_fixture fixture;
        lib_u8 code[15], bytes = 0u;
        lib_u64 ticks = LIB_UINT64_MAX;
        t_cpu after;
        const lib_u8 operand_bytes = (lib_u8)(1u << size);
        const lib_bool wide = size != 0u;
        const lib_bool accumulator = form == 2u;
        const lib_bool immediate = form >= 2u;
        const lib_bool short_immediate = form == 4u || form == 5u;
        const lib_u64 expected = !memory || accumulator ? 1u :
            form == 1u || operation == 7u ? 2u : 3u;

        if ((accumulator && memory) || (form == 4u && !wide) ||
            (form == 5u && wide)) continue;
        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        fixture.cpu.data.eax = 0x81u;
        fixture.cpu.data.ecx = 2u;
        if (wide && (size == 2u) != code32) code[bytes++] = 0x66u;
        if (memory && address32 != code32) code[bytes++] = 0x67u;
        if (form < 3u)
            code[bytes++] = (lib_u8)((operation << 3u) | (form << 1u) | wide);
        else
            code[bytes++] = form == 3u ? (lib_u8)(0x80u | wide) :
                form == 4u ? 0x83u : 0x82u;
        if (!accumulator) {
            const lib_u8 reg = form < 3u ? 1u : operation;

            code[bytes++] = (lib_u8)((reg << 3u) |
                (memory ? (address32 ? 5u : 6u) : 0xc0u));
            if (memory) {
                code[bytes++] = 0u;
                code[bytes++] = 0x20u;
                if (address32) { code[bytes++] = 0u; code[bytes++] = 0u; }
            }
        }
        if (immediate) {
            code[bytes++] = 3u;
            if (!short_immediate)
                for (lib_u8 byte = 1u; byte < operand_bytes; ++byte)
                    code[bytes++] = 0u;
        }
        if (cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
            fixture.fault.valid || fixture.delivered_exception.valid ||
            !x86_core_cpu_80486_nominal_instruction_cost(&fixture.execution, &ticks) ||
            ticks != expected) {
            lib_c_fprintf(lib_c_stderr,
                "ALU nominal operation=%u size=%u code32=%u address32=%u memory=%u form=%u ticks=%llu\n",
                (lib_u32)operation, (lib_u32)size, (lib_u32)code32,
                (lib_u32)address32, (lib_u32)memory, (lib_u32)form, ticks);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

/* E-7 distinguishes taken/not-taken Jcc and true/false SETcc, with the
 * opposite SETcc clock ordering for register and memory destinations. */
static lib_bool nominal_condition_forms(void)
{
    for (lib_u8 flags = 0u; flags < 32u; ++flags)
    for (lib_u8 condition = 0u; condition < 16u; ++condition)
    for (lib_u8 form = 0u; form < 4u; ++form)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32) {
        cpu_instruction_fixture fixture;
        lib_u8 code[8], bytes = 0u;
        lib_u64 ticks = LIB_UINT64_MAX;
        t_cpu after;
        const lib_bool carry = (flags & 1u) != 0u;
        const lib_bool zero = (flags & 2u) != 0u;
        const lib_bool sign = (flags & 4u) != 0u;
        const lib_bool overflow = (flags & 8u) != 0u;
        const lib_bool parity = (flags & 16u) != 0u;
        const lib_bool conditions[] = {
            overflow, !overflow, carry, !carry, zero, !zero,
            carry || zero, !carry && !zero, sign, !sign, parity, !parity,
            sign != overflow, sign == overflow,
            zero || sign != overflow, !zero && sign == overflow
        };
        const lib_bool taken = conditions[condition];
        const lib_u64 expected = form < 2u ? (taken ? 3u : 1u) :
            form == 2u ? (taken ? 4u : 3u) : (taken ? 3u : 4u);

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        fixture.cpu.data.eflags |= (carry ? VCPU_EFLAGS_CF : 0u) |
            (zero ? VCPU_EFLAGS_ZF : 0u) | (sign ? VCPU_EFLAGS_SF : 0u) |
            (overflow ? VCPU_EFLAGS_OF : 0u) | (parity ? VCPU_EFLAGS_PF : 0u);
        if (form == 3u && code32) code[bytes++] = 0x67u;
        if (form != 0u) code[bytes++] = 0x0fu;
        code[bytes++] = (lib_u8)((form == 0u ? 0x70u :
            form == 1u ? 0x80u : 0x90u) | condition);
        if (form < 2u) {
            code[bytes++] = 1u;
            if (form == 1u) {
                code[bytes++] = 0u;
                if (code32) { code[bytes++] = 0u; code[bytes++] = 0u; }
            }
        } else {
            code[bytes++] = form == 2u ? 0xc0u : 0x06u;
            if (form == 3u) { code[bytes++] = 0u; code[bytes++] = 0x20u; }
        }
        if (cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
            fixture.fault.valid || fixture.delivered_exception.valid ||
            !x86_core_cpu_80486_nominal_instruction_cost(&fixture.execution, &ticks) ||
            ticks != expected || (form < 2u && after.data.eip != bytes + taken)) {
            lib_c_fprintf(lib_c_stderr,
                "Condition nominal flags=%u condition=%u form=%u code32=%u ticks=%llu\n",
                (lib_u32)flags, (lib_u32)condition, (lib_u32)form,
                (lib_u32)code32, ticks);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

/* E-12 query clocks do not depend on whether the selector is accepted.
 * Use real protected descriptors and both ModR/M storage forms. */
static lib_bool nominal_protection_queries(void)
{
    static const lib_u8 opcodes[] = {0x63u, 0x02u, 0x03u, 0x00u, 0x00u};
    static const lib_u8 fields[] = {1u, 1u, 1u, 4u, 5u};
    static const lib_u8 expected[] = {9u, 11u, 10u, 11u, 11u};

    for (lib_u8 cpl = 0u; cpl < 4u; ++cpl)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 memory = 0u; memory < 2u; ++memory)
    for (lib_u8 valid = 0u; valid < 2u; ++valid)
    for (lib_u8 form = 0u; form < sizeof(opcodes); ++form) {
        cpu_instruction_fixture fixture;
        lib_u8 code[8], bytes = 0u;
        const lib_u16 selector = valid ? (lib_u16)(0x10u | cpl) : 0u;
        lib_u64 ticks = LIB_UINT64_MAX;
        t_cpu after;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        cpu_instruction_prepare_protected_exception(&fixture, cpl);
        fixture.cpu.data.ds = fixture.cpu.data.ss;
        fixture.cpu.data.ds.sregtype = SREG_DATA;
        fixture.memory[0x215u] |= (lib_u8)(cpl << 5u);
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        fixture.cpu.data.eax = selector;
        fixture.cpu.data.ecx = 3u;
        lib_memory_copy(fixture.memory + 0x2000u, &selector, sizeof(selector));
        if (memory && code32) code[bytes++] = 0x67u;
        if (form != 0u) code[bytes++] = 0x0fu;
        code[bytes++] = opcodes[form];
        code[bytes++] = (lib_u8)((fields[form] << 3u) | (memory ? 6u : 0xc0u));
        if (memory) { code[bytes++] = 0u; code[bytes++] = 0x20u; }
        if (cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
            fixture.fault.valid || fixture.delivered_exception.valid ||
            !x86_core_cpu_80486_nominal_instruction_cost(&fixture.execution, &ticks) ||
            ticks != expected[form] || (form != 0u &&
                ((after.data.eflags & VCPU_EFLAGS_ZF) != 0u) != (valid != 0u))) {
            lib_c_fprintf(lib_c_stderr,
                "Protection nominal cpl=%u code32=%u memory=%u valid=%u form=%u ticks=%llu\n",
                (lib_u32)cpl, (lib_u32)code32, (lib_u32)memory,
                (lib_u32)valid, (lib_u32)form, ticks);
            return LIB_FALSE;
        }
        {
            static const lib_u8 operand_penalty[] = {0u, 2u, 3u, 4u, 4u};
            const lib_u64 expected_penalty = (memory ? operand_penalty[form] : 0u) +
                (form != 0u && valid ? 3u : 0u);
            lib_bool modeled = LIB_TRUE;

            if (fixture.instructions.data.timing_missed_descriptors != (lib_u32)(form != 0u && valid) ||
                !x86_core_cpu_80486_memory_penalty(&fixture.execution, &ticks, &modeled) ||
                ticks != expected_penalty || modeled) return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

/* E-6 distinguishes CMPXCHG's memory result paths; a register-only witness
 * cannot qualify either of these two rows. */
static lib_bool nominal_exchange_memory(void)
{
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 size = 0u; size < 3u; ++size)
    for (lib_u8 equal = 0u; equal < 2u; ++equal) {
        cpu_instruction_fixture fixture;
        lib_u8 code[8], bytes = 0u;
        lib_u64 ticks = LIB_UINT64_MAX;
        t_cpu after;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        fixture.cpu.data.eax = equal ? 7u : 8u;
        fixture.cpu.data.ecx = 9u;
        fixture.memory[0x2000u] = 7u;
        if (code32) code[bytes++] = 0x67u;
        if (size != 0u && (size == 2u) != code32) code[bytes++] = 0x66u;
        code[bytes++] = 0x0fu;
        code[bytes++] = size == 0u ? 0xb0u : 0xb1u;
        code[bytes++] = 0x0eu;
        code[bytes++] = 0u;
        code[bytes++] = 0x20u;
        if (cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
            fixture.fault.valid || fixture.delivered_exception.valid ||
            !x86_core_cpu_80486_nominal_instruction_cost(&fixture.execution, &ticks) ||
            ticks != (equal ? 7u : 10u) ||
            ((after.data.eflags & VCPU_EFLAGS_ZF) != 0u) != (equal != 0u) ||
            fixture.memory[0x2000u] != (equal ? 9u : 7u)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

/* E-12 system rows, including successful null LLDT and available-TSS LTR.
 * The instruction itself performs descriptor validation and state updates. */
static lib_bool nominal_system_forms(void)
{
    static const nominal_case cases[] = {
        {"SLDT register", {0x0fu, 0x00u, 0xc0u}, 3u, 2u},
        {"SLDT memory", {0x0fu, 0x00u, 0x06u, 0u, 0x20u}, 5u, 3u},
        {"STR register", {0x0fu, 0x00u, 0xc8u}, 3u, 2u},
        {"STR memory", {0x0fu, 0x00u, 0x0eu, 0u, 0x20u}, 5u, 3u},
        {"LLDT register", {0x0fu, 0x00u, 0xd0u}, 3u, 11u},
        {"LLDT memory", {0x0fu, 0x00u, 0x16u, 0u, 0x20u}, 5u, 11u},
        {"LTR register", {0x0fu, 0x00u, 0xd8u}, 3u, 20u},
        {"LTR memory", {0x0fu, 0x00u, 0x1eu, 0u, 0x20u}, 5u, 20u},
        {"LGDT", {0x0fu, 0x01u, 0x16u, 0u, 0x20u}, 5u, 12u},
        {"LIDT", {0x0fu, 0x01u, 0x1eu, 0u, 0x20u}, 5u, 12u},
        {"LMSW register", {0x0fu, 0x01u, 0xf0u}, 3u, 13u},
        {"LMSW memory", {0x0fu, 0x01u, 0x36u, 0u, 0x20u}, 5u, 13u},
        {"CLTS", {0x0fu, 0x06u}, 2u, 7u}
    };

    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_size row = 0u; row < sizeof(cases) / sizeof(cases[0]); ++row) {
        cpu_instruction_fixture fixture;
        const nominal_case *test = &cases[row];
        const lib_u8 tss[] = {0x67u, 0u, 0u, 6u, 0u, 0x89u, 0u, 0u};
        const lib_bool task = test->code[1] == 0u &&
            ((test->code[2] >> 3u) & 7u) == 3u;
        lib_u8 code[8], bytes = 0u;
        lib_u64 ticks = LIB_UINT64_MAX;
        t_cpu after;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        cpu_instruction_prepare_protected_exception(&fixture, 0u);
        fixture.cpu.data.ds = fixture.cpu.data.ss;
        fixture.cpu.data.ds.sregtype = SREG_DATA;
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        fixture.cpu.data.gdtr.limit = 31u;
        lib_memory_copy(fixture.memory + 0x218u, tss, sizeof(tss));
        fixture.cpu.data.eax = task ? 0x18u : 0u;
        fixture.memory[0x2000u] = task ? 0x18u : 0u;
        if (code32 && test->bytes == 5u) code[bytes++] = 0x67u;
        lib_memory_copy(code + bytes, test->code, test->bytes);
        bytes += test->bytes;
        if (cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
            fixture.fault.valid || fixture.delivered_exception.valid ||
            !x86_core_cpu_80486_nominal_instruction_cost(&fixture.execution, &ticks) ||
            ticks != test->ticks) {
            lib_c_fprintf(lib_c_stderr, "System nominal %s code32=%u ticks=%llu\n",
                test->name, (lib_u32)code32, ticks);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool timing_prefix_penalties(void)
{
    static const lib_u8 prefixes[] = {0x26u, 0x2eu, 0x36u, 0x3eu,
        0x64u, 0x65u, 0x66u, 0x67u};

    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u16 mask = 0u; mask < 256u; ++mask) {
        cpu_instruction_fixture fixture;
        lib_u8 code[9], bytes = 0u;
        lib_u64 ticks = LIB_UINT64_MAX;
        t_cpu after;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        for (lib_u8 index = 0u; index < sizeof(prefixes); ++index)
            if ((mask & (1u << index)) != 0u) code[bytes++] = prefixes[index];
        const lib_u64 expected = bytes;
        code[bytes++] = 0x90u;
        if (cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
            fixture.fault.valid || fixture.delivered_exception.valid ||
            !x86_core_cpu_80486_decode_penalty(&fixture.execution, &ticks) || ticks != expected) {
            lib_c_fprintf(lib_c_stderr, "Prefix penalty code32=%u mask=%x ticks=%llu\n",
                code32, mask, ticks);
            return LIB_FALSE;
        }
    }
    for (lib_u8 count = 0u; count < 15u; ++count) {
        cpu_instruction_fixture fixture;
        lib_u8 code[15];
        lib_u64 ticks = LIB_UINT64_MAX;
        t_cpu after;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        for (lib_u8 index = 0u; index < count; ++index) code[index] = 0x2eu;
        code[count] = 0x90u;
        if (cpu_instruction_run(&fixture, code, count + 1u, &after) != LIB_STATUS_OK ||
            fixture.fault.valid || fixture.delivered_exception.valid ||
            !x86_core_cpu_80486_decode_penalty(&fixture.execution, &ticks) || ticks != count)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool timing_decode_penalties(void)
{
    static const nominal_case cases[] = {
        {"base only immediate", {0xc6u, 0x04u, 1u}, 3u, 0u},
        {"displacement immediate", {0xc6u, 0x06u, 0u, 0x20u, 1u}, 5u, 1u},
        {"short displacement immediate", {0xc6u, 0x44u, 1u, 1u}, 4u, 1u},
        {"index immediate", {0xc6u, 0x00u, 1u}, 3u, 1u},
        {"index displacement immediate", {0xc6u, 0x40u, 1u, 1u}, 4u, 2u},
        {"LEA index already nominal", {0x8du, 0x00u}, 2u, 0u},
        {"LEA index with address prefix", {0x67u, 0x8du, 0x04u, 0x31u}, 4u, 1u},
        {"SIB displacement immediate", {0x67u, 0xc6u, 0x44u, 0x31u, 1u, 1u}, 6u, 3u},
        {"double size prefix immediate", {0x66u, 0x66u, 0xc7u, 0xc0u, 1u, 0u, 0u, 0u}, 8u, 2u},
        {"far transfer includes note 22", {0x9au, 0x10u, 0u, 0u, 0u}, 5u, 0u},
        {"REP setup owns repeat prefix", {0xf3u, 0xa4u}, 2u, 0u},
        {"REP with ordinary prefix", {0x66u, 0xf3u, 0xa5u}, 3u, 1u}
    };

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        const nominal_case *test = &cases[index];
        cpu_instruction_fixture fixture;
        lib_u64 ticks = LIB_UINT64_MAX;
        t_cpu after;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        fixture.cpu.data.esp = 0x6000u;
        fixture.cpu.data.ebx = fixture.cpu.data.esi = 0x1000u;
        if (cpu_instruction_run(&fixture, test->code, test->bytes, &after) != LIB_STATUS_OK ||
            fixture.fault.valid || fixture.delivered_exception.valid ||
            !x86_core_cpu_80486_decode_penalty(&fixture.execution, &ticks) || ticks != test->ticks) {
            lib_c_fprintf(lib_c_stderr, "Decode penalty %s: ticks=%llu\n", test->name, ticks);
            return LIB_FALSE;
        }
        const t_cpuins before_instructions = fixture.instructions;
        const x86_core_cpu_execution_context before_execution = fixture.execution;
        const t_cpu before_cpu = fixture.cpu;
        if (!x86_core_cpu_80486_decode_penalty(&fixture.execution, &ticks) ||
            lib_memory_compare(&before_instructions, &fixture.instructions, sizeof(before_instructions)) ||
            lib_memory_compare(&before_execution, &fixture.execution, sizeof(before_execution)) ||
            lib_memory_compare(&before_cpu, &fixture.cpu, sizeof(before_cpu)))
            return LIB_FALSE;
    }
    for (lib_u8 form = 0u; form < 2u; ++form) {
        cpu_instruction_fixture fixture;
        const lib_u8 code[] = {0xf3u, form ? 0xf3u : 0x90u, 0xa4u};
        lib_u64 ticks = LIB_UINT64_MAX;
        t_cpu after;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        if (cpu_instruction_run(&fixture, code, sizeof(code), &after) != LIB_STATUS_OK ||
            fixture.fault.valid || fixture.delivered_exception.valid ||
            x86_core_cpu_80486_decode_penalty(&fixture.execution, &ticks) ||
            ticks != LIB_UINT64_MAX)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool nominal_rejects_unowned(void)
{
    const lib_u8 code[] = {0x90u};
    cpu_instruction_fixture fixture;
    t_cpu after;
    lib_u64 ticks = LIB_UINT64_MAX;

    cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80386);
    if (cpu_instruction_run(&fixture, code, sizeof(code), &after) != LIB_STATUS_OK ||
        x86_core_cpu_80486_nominal_instruction_cost(&fixture.execution, &ticks) ||
        x86_core_cpu_80486_decode_penalty(&fixture.execution, &ticks) ||
        ticks != LIB_UINT64_MAX) return LIB_FALSE;
    cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
    return !x86_core_cpu_80486_nominal_instruction_cost(&fixture.execution, &ticks) &&
        !x86_core_cpu_80486_decode_penalty(&fixture.execution, &ticks) &&
        ticks == LIB_UINT64_MAX &&
        !x86_core_cpu_80486_nominal_instruction_cost(LIB_NULL, &ticks) &&
        !x86_core_cpu_80486_nominal_instruction_cost(&fixture.execution, LIB_NULL) &&
        !x86_core_cpu_80486_decode_penalty(LIB_NULL, &ticks) &&
        !x86_core_cpu_80486_decode_penalty(&fixture.execution, LIB_NULL);
}

static lib_bool nominal_modes(void)
{
    static const nominal_case cases[] = {
        {"ADD", {0x01u, 0xc1u}, 2u, 1u},
        {"PUSH", {0x50u}, 1u, 1u},
        {"POP", {0x58u}, 1u, 1u},
        {"CBW/CWDE", {0x98u}, 1u, 3u},
        {"CWD/CDQ", {0x99u}, 1u, 3u},
        {"MOVZX", {0x0fu, 0xb6u, 0xc0u}, 3u, 3u},
        {"XADD", {0x0fu, 0xc1u, 0xc1u}, 3u, 3u},
        {"PUSHF", {0x9cu}, 1u, 4u},
        {"POPF", {0x9du}, 1u, 9u}
    };

    for (lib_u8 mode = 0u; mode != 6u; ++mode) {
        for (lib_u8 prefixes = 0u; prefixes != 8u; ++prefixes) {
            for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
                cpu_instruction_fixture fixture;
                const nominal_case *test = &cases[index];
                lib_u8 code[8], bytes = 0u;
                const lib_bool protected_mode = mode != 0u && mode != 5u;
                lib_u64 ticks = LIB_UINT64_MAX;
                const lib_u64 expected = test->code[0] == 0x9cu ?
                    (protected_mode ? 3u : 4u) : test->code[0] == 0x9du ?
                    (protected_mode ? 6u : 9u) : test->ticks;
                t_cpu after;

                cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
                fixture.cpu.data.cs.seg.exec.defsize = (prefixes & 4u) != 0u;
                fixture.cpu.data.esp = 0x6000u;
                fixture.cpu.data.eflags = 2u | VCPU_EFLAGS_IOPL;
                if (mode != 0u) {
                    fixture.cpu.data.cr0 |= VCPU_CR0_PE;
                    fixture.cpu.data.cs.dpl = mode == 5u ? 3u : mode - 1u;
                    fixture.cpu.data.ss.dpl = fixture.cpu.data.cs.dpl;
                    if (mode == 5u) fixture.cpu.data.eflags |= VCPU_EFLAGS_VM;
                }
                /* POPF receives a valid, non-VM image for either stack width;
                 * its nominal mode is the entry mode, not the result flags. */
                lib_memory_copy(fixture.memory + 0x6000u,
                    &(lib_u32){2u | VCPU_EFLAGS_IOPL}, sizeof(lib_u32));
                if (prefixes & 1u) code[bytes++] = 0x66u;
                if (prefixes & 2u) code[bytes++] = 0x67u;
                lib_memory_copy(code + bytes, test->code, test->bytes);
                bytes += test->bytes;
                if (cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
                    fixture.fault.valid || fixture.delivered_exception.valid ||
                    !x86_core_cpu_80486_nominal_instruction_cost(&fixture.execution, &ticks) ||
                    ticks != expected) {
                    lib_c_fprintf(lib_c_stderr,
                        "Nominal mode: %s mode=%u prefixes=%u expected=%llu actual=%llu\n",
                        test->name, mode, prefixes, expected, ticks);
                    return LIB_FALSE;
                }
            }
        }
    }
    return LIB_TRUE;
}

static lib_bool nominal_cpuid(void)
{
    static const lib_u32 leaves[] = {0u, 1u, 2u, 0x80000000u, LIB_UINT32_MAX};
    static const lib_u8 code[] = {0x0fu, 0xa2u};

    for (lib_size index = 0u; index < sizeof(leaves) / sizeof(leaves[0]); ++index) {
        cpu_instruction_fixture fixture;
        lib_u64 ticks = LIB_UINT64_MAX;
        t_cpu after;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        fixture.cpu.data.eax = leaves[index];
        if (cpu_instruction_run(&fixture, code, sizeof(code), &after) != LIB_STATUS_OK ||
            fixture.fault.valid || fixture.delivered_exception.valid ||
            !x86_core_cpu_80486_nominal_instruction_cost(&fixture.execution, &ticks) ||
            ticks != (leaves[index] == 1u ? 14u : 9u)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool nominal_bit_scan(void)
{
    /* E-14 notes 12--15, evaluated independently for each set-bit position. */
    static const lib_u8 forward_ticks[32] = {
        18u, 21u, 24u, 27u, 21u, 24u, 27u, 30u,
        22u, 25u, 28u, 31u, 25u, 28u, 31u, 34u,
        26u, 29u, 32u, 35u, 29u, 32u, 35u, 38u,
        30u, 33u, 36u, 39u, 33u, 36u, 39u, 42u
    };
    static const lib_u8 reverse_ticks[32] = {
        103u, 100u, 97u, 94u, 91u, 88u, 85u, 82u,
        79u, 76u, 73u, 70u, 67u, 64u, 61u, 58u,
        55u, 52u, 49u, 46u, 43u, 40u, 37u, 34u,
        31u, 28u, 25u, 22u, 19u, 16u, 13u, 10u
    };

    for (lib_u8 dword = 0u; dword != 2u; ++dword) {
        for (lib_u8 memory = 0u; memory != 2u; ++memory) {
            for (lib_u8 reverse = 0u; reverse != 2u; ++reverse) {
                const lib_u8 bits = dword ? 32u : 16u;

                for (lib_u8 bit = 0u; bit <= bits; ++bit) {
                    cpu_instruction_fixture fixture;
                    lib_u8 code[6], bytes = 0u;
                    const lib_u32 value = bit == bits ? 0u : (lib_u32)1u << bit;
                    const lib_u64 expected = memory + (bit == bits ? 6u :
                        reverse ? reverse_ticks[bit] : forward_ticks[bit]);
                    lib_u64 ticks = LIB_UINT64_MAX;
                    t_cpu after;

                    cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
                    fixture.cpu.data.ecx = value;
                    lib_memory_copy(fixture.memory + 0x2000u, &value, sizeof(value));
                    if (dword) code[bytes++] = 0x66u;
                    code[bytes++] = 0x0fu;
                    code[bytes++] = reverse ? 0xbdu : 0xbcu;
                    code[bytes++] = memory ? 0x06u : 0xc1u;
                    if (memory) {
                        code[bytes++] = 0u;
                        code[bytes++] = 0x20u;
                    }
                    if (cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
                        fixture.fault.valid || fixture.delivered_exception.valid ||
                        !x86_core_cpu_80486_nominal_instruction_cost(&fixture.execution, &ticks) ||
                        ticks != expected) {
                        lib_c_fprintf(lib_c_stderr,
                            "Nominal scan: dword=%u memory=%u reverse=%u bit=%u expected=%llu actual=%llu\n",
                            dword, memory, reverse, bit, expected, ticks);
                        return LIB_FALSE;
                    }
                }
            }
        }
    }
    return LIB_TRUE;
}

static lib_bool nominal_unsigned_multiply(void)
{
    static const struct {
        lib_u32 multiplier;
        lib_u8 ticks;
    } cases[] = {
        {0u, 13u}, {1u, 13u}, {2u, 13u}, {3u, 13u}, {7u, 13u}, {8u, 13u},
        {9u, 14u}, {127u, 17u}, {128u, 17u}, {129u, 18u}, {255u, 18u},
        {256u, 18u}, {257u, 19u}, {32767u, 25u}, {32768u, 25u},
        {32769u, 26u}, {65535u, 26u}, {65536u, 26u}, {65537u, 27u},
        {0x7fffffffu, 41u}, {0x80000000u, 41u}, {0x80000001u, 42u},
        {LIB_UINT32_MAX, 42u}
    };

    for (lib_u8 bytes = 1u; bytes <= 4u; bytes *= 2u) {
        for (lib_u8 code32 = 0u; code32 != 2u; ++code32) {
            for (lib_u8 memory = 0u; memory != 2u; ++memory) {
                for (lib_u8 address_prefix = 0u; address_prefix != 2u; ++address_prefix) {
                    const lib_bool address32 = code32 != address_prefix;
                    const lib_u32 mask = bytes == 1u ? LIB_UINT8_MAX :
                        bytes == 2u ? LIB_UINT16_MAX : LIB_UINT32_MAX;

                    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
                        cpu_instruction_fixture fixture;
                        lib_u8 code[10], length = 0u;
                        lib_u64 ticks = LIB_UINT64_MAX;
                        t_cpu after;

                        if (cases[index].multiplier > mask) continue;
                        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
                        fixture.cpu.data.cs.seg.exec.defsize = code32;
                        fixture.cpu.data.eax = 3u;
                        fixture.cpu.data.ecx = cases[index].multiplier;
                        fixture.cpu.data.edx = 0u;
                        lib_memory_copy(fixture.memory + 0x2000u,
                            &cases[index].multiplier, bytes);
                        if (bytes != 1u && (code32 != 0u) != (bytes == 4u))
                            code[length++] = 0x66u;
                        if (address_prefix) code[length++] = 0x67u;
                        code[length++] = bytes == 1u ? 0xf6u : 0xf7u;
                        code[length++] = memory ? (address32 ? 0x25u : 0x26u) : 0xe1u;
                        if (memory) {
                            code[length++] = 0u;
                            code[length++] = 0x20u;
                            if (address32) {
                                code[length++] = 0u;
                                code[length++] = 0u;
                            }
                        }
                        if (cpu_instruction_run(&fixture, code, length, &after) != LIB_STATUS_OK ||
                            fixture.fault.valid || fixture.delivered_exception.valid ||
                            !x86_core_cpu_80486_nominal_instruction_cost(&fixture.execution, &ticks) ||
                            ticks != cases[index].ticks) {
                            lib_c_fprintf(lib_c_stderr,
                                "Nominal MUL: bytes=%u code32=%u memory=%u address_prefix=%u m=%x ticks=%llu\n",
                                bytes, code32, memory, address_prefix, cases[index].multiplier, ticks);
                            return LIB_FALSE;
                        }
                    }
                }
            }
        }
    }
    return LIB_TRUE;
}

static lib_bool nominal_ports(void)
{
    static const struct {
        lib_u8 opcode;
        lib_u8 ticks[4];
    } cases[] = {
        {0xe4u, {14u, 9u, 29u, 27u}}, {0xe5u, {14u, 9u, 29u, 27u}},
        {0xe6u, {16u, 11u, 31u, 29u}}, {0xe7u, {16u, 11u, 31u, 29u}},
        {0xecu, {14u, 8u, 28u, 27u}}, {0xedu, {14u, 8u, 28u, 27u}},
        {0xeeu, {16u, 10u, 30u, 29u}}, {0xefu, {16u, 10u, 30u, 29u}},
        {0x6cu, {17u, 10u, 32u, 30u}}, {0x6du, {17u, 10u, 32u, 30u}},
        {0x6eu, {17u, 10u, 32u, 30u}}, {0x6fu, {17u, 10u, 32u, 30u}}
    };
    static const lib_u8 input_setup[] = {16u, 10u, 30u, 29u};
    static const lib_u8 output_setup[] = {17u, 11u, 31u, 30u};

    for (lib_u8 mode = 0u; mode != 4u; ++mode) {
        for (lib_u8 dword = 0u; dword != 2u; ++dword) {
            for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
                const lib_bool string = cases[index].opcode < 0xe4u;

                for (lib_u8 form = 0u; form != (string ? 4u : 1u); ++form) {
                    cpu_port_instruction_fixture fixture;
                    lib_u8 code[5], length = 0u;
                    const lib_u8 opcode = cases[index].opcode;
                    const lib_bool repeat = form != 0u;
                    const lib_u32 count = repeat ? form - 1u : 1u;
                    lib_u64 expected = cases[index].ticks[mode];
                    lib_u64 ticks = LIB_UINT64_MAX;
                    t_cpu after;

                    cpu_port_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
                    fixture.instruction.cpu.data.esp = 0x8000u;
                    fixture.instruction.cpu.data.edx = 0x20u;
                    fixture.instruction.cpu.data.esi = 0x2000u;
                    fixture.instruction.cpu.data.edi = 0x3000u;
                    fixture.instruction.cpu.data.ecx = count;
                    if (mode != 0u) {
                        fixture.instruction.cpu.data.cr0 |= VCPU_CR0_PE;
                        fixture.instruction.cpu.data.cs.dpl = mode == 1u ? 0u : 3u;
                        fixture.instruction.cpu.data.ss.dpl = fixture.instruction.cpu.data.cs.dpl;
                        fixture.instruction.cpu.data.ds.selector = fixture.instruction.cpu.data.es.selector =
                            (lib_u16)(0x10u | fixture.instruction.cpu.data.cs.dpl);
                        fixture.instruction.cpu.data.ds.dpl = fixture.instruction.cpu.data.es.dpl =
                            fixture.instruction.cpu.data.cs.dpl;
                        if (mode == 3u)
                            fixture.instruction.cpu.data.eflags |= VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL;
                        fixture.instruction.cpu.data.tr = (t_cpu_data_sreg){
                            .flagValid = LIB_TRUE, .selector = 0x28u, .base = 0x600u,
                            .limit = 0xffu, .sregtype = SREG_TR,
                            .sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY};
                        lib_memory_copy(fixture.instruction.memory + 0x666u,
                            &(lib_u16){0x80u}, sizeof(lib_u16));
                    }
                    if (dword) code[length++] = 0x66u;
                    if (repeat) code[length++] = 0xf3u;
                    code[length++] = opcode;
                    if (opcode >= 0xe4u && opcode <= 0xe7u) code[length++] = 0x20u;
                    if (repeat) expected = opcode < 0x6eu ?
                        input_setup[mode] + (count ? 8u : 0u) :
                        output_setup[mode] + (count ? 5u : 0u);
                    if (cpu_instruction_run(&fixture.instruction, code, length, &after) != LIB_STATUS_OK ||
                        fixture.instruction.fault.valid || fixture.instruction.delivered_exception.valid ||
                        fixture.transfer_count != (count ? 1u : 0u) ||
                        fixture.complete_count != fixture.transfer_count ||
                        !x86_core_cpu_80486_nominal_instruction_cost(&fixture.instruction.execution, &ticks) ||
                        ticks != expected) {
                        lib_c_fprintf(lib_c_stderr,
                            "Nominal port: mode=%u dword=%u opcode=%x form=%u expected=%llu actual=%llu exception=%x transfers=%u\n",
                            mode, dword, opcode, form, expected, ticks,
                            fixture.instruction.delivered_exception.exception_mask, fixture.transfer_count);
                        return LIB_FALSE;
                    }
                    if (!repeat) {
                        const lib_u8 width = (opcode & 1u) == 0u ? 1u : dword ? 4u : 2u;
                        const lib_u32 misses = (mode >= 2u ? 1u + width : 0u) +
                            (lib_u32)(opcode == 0x6eu || opcode == 0x6fu);
                        lib_bool modeled = LIB_TRUE;

                        if (fixture.instruction.instructions.data.timing_data_misses != misses ||
                            !x86_core_cpu_80486_memory_penalty(&fixture.instruction.execution, &ticks, &modeled) ||
                            ticks != 2u * (lib_u64)misses || modeled != (mode >= 2u))
                            return LIB_FALSE;
                    }
                }
            }
        }
    }
    return LIB_TRUE;
}

static lib_status port_timing_query(void *opaque, lib_u32 address,
    x86_core_cpu_cache_query query, x86_core_cpu_cache_memory *out_memory)
{
    (void)opaque;
    *out_memory = (x86_core_cpu_cache_memory){.address = address,
        .fill_enabled = query == X86_CORE_CPU_CACHE_FILL_PROBE && address >= 0x600u};
    return LIB_STATUS_OK;
}

static lib_bool port_memory_timing(void)
{
    static const lib_u8 opcodes[] = {0xe4u, 0xe5u, 0xe6u, 0xe7u, 0xecu, 0xedu,
        0xeeu, 0xefu, 0x6cu, 0x6du, 0x6eu, 0x6fu};

    for (lib_u8 mode = 0u; mode < 4u; ++mode)
    for (lib_size row = 0u; row < sizeof(opcodes); ++row)
    for (lib_u8 dword = 0u; dword < 2u; ++dword)
    for (lib_u8 warm = 0u; warm < 8u; ++warm)
    for (lib_u8 failure = 0u; failure < (warm == 0u ? 2u : 1u); ++failure) {
        cpu_port_instruction_fixture fixture;
        const lib_u8 opcode = opcodes[row];
        const lib_u8 width = (opcode & 1u) == 0u ? 1u : dword ? 4u : 2u;
        const lib_u16 addresses[] = {0x666u, 0x68fu, 0x200fu};
        const lib_bool permission = mode >= 2u, output = opcode == 0x6eu || opcode == 0x6fu;
        const lib_u32 permission_misses = permission ? ((warm & 1u) ? 0u : 1u) +
            ((warm & 2u) ? 0u : width == 1u ? 1u : 2u) : 0u;
        const lib_u32 data_misses = output && (warm & 4u) == 0u ? (width == 1u ? 1u : 2u) : 0u;
        const lib_u32 misses = permission_misses + data_misses;
        const lib_u32 reads = permission_misses + (lib_u32)(data_misses != 0u);
        lib_u8 code[3], bytes = 0u;
        lib_u64 ticks = 999u;
        lib_bool modeled = LIB_TRUE;
        t_cpu after;
        x86_core_cpu_bus_provider bus = cpu_port_bus;

        cpu_port_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        bus.query_cache_memory = port_timing_query;
        fixture.instruction.execution.bus = &bus;
        fixture.instruction.cpu.data.cr0 &= ~(VCPU_CR0_CD | VCPU_CR0_NW);
        lib_memory_copy(fixture.instruction.memory + 0x666u, &(lib_u16){0x80u}, sizeof(lib_u16));
        for (lib_u8 part = 0u; part < 3u; ++part) {
            const lib_u8 load[] = {0x66u, 0xa1u, (lib_u8)addresses[part], (lib_u8)(addresses[part] >> 8u)};

            if ((warm & (1u << part)) == 0u) continue;
            fixture.instruction.cpu.data.eip = 0u;
            x86_core_cpu_execution_invalidate_prefetch(&fixture.instruction.execution);
            if (cpu_instruction_run(&fixture.instruction, load, sizeof(load), &after) != LIB_STATUS_OK ||
                fixture.instruction.fault.valid || fixture.instruction.delivered_exception.valid)
                return LIB_FALSE;
        }
        fixture.instruction.cpu.data.esp = 0x8000u;
        fixture.instruction.cpu.data.edx = 0x7fu;
        fixture.instruction.cpu.data.esi = 0x200fu;
        fixture.instruction.cpu.data.edi = 0x3000u;
        if (mode) {
            fixture.instruction.cpu.data.cr0 |= VCPU_CR0_PE;
            fixture.instruction.cpu.data.cs.dpl = mode == 1u ? 0u : 3u;
            fixture.instruction.cpu.data.ss.dpl = fixture.instruction.cpu.data.cs.dpl;
            fixture.instruction.cpu.data.ds.selector = fixture.instruction.cpu.data.es.selector =
                (lib_u16)(0x10u | fixture.instruction.cpu.data.cs.dpl);
            fixture.instruction.cpu.data.ds.dpl = fixture.instruction.cpu.data.es.dpl =
                fixture.instruction.cpu.data.cs.dpl;
            if (mode == 3u) fixture.instruction.cpu.data.eflags |= VCPU_EFLAGS_VM | VCPU_EFLAGS_IOPL;
            fixture.instruction.cpu.data.tr = (t_cpu_data_sreg){.flagValid = LIB_TRUE,
                .selector = 0x28u, .base = 0x600u, .limit = 0xffu, .sregtype = SREG_TR,
                .sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY};
        }
        if (dword) code[bytes++] = 0x66u;
        code[bytes++] = opcode;
        if (opcode >= 0xe4u && opcode <= 0xe7u) code[bytes++] = 0x7fu;
        fixture.instruction.cpu.data.eip = 0u;
        x86_core_cpu_execution_invalidate_prefetch(&fixture.instruction.execution);
        fixture.fail_status = failure ? LIB_STATUS_IO_ERROR : LIB_STATUS_OK;
        const lib_u64 expected = 2u * (lib_u64)reads + 5u * (lib_u64)(misses - reads) +
            (output && width != 1u ? 3u : 0u);
        const lib_status status = cpu_instruction_run(&fixture.instruction, code, bytes, &after);
        if (failure) {
            if (!fixture.instruction.fault.valid || fixture.transfer_count != 0u || fixture.complete_count != 0u ||
                fixture.instruction.instructions.data.timing_data_misses != misses ||
                x86_core_cpu_80486_memory_penalty(&fixture.instruction.execution, &ticks, &modeled) ||
                ticks != 999u || !modeled) return LIB_FALSE;
            continue;
        }
        if (status != LIB_STATUS_OK ||
            fixture.instruction.fault.valid || fixture.instruction.delivered_exception.valid ||
            fixture.transfer_count != 1u || fixture.complete_count != 1u ||
            fixture.instruction.instructions.data.timing_data_misses != misses ||
            fixture.instruction.instructions.data.timing_missed_reads != reads ||
            !x86_core_cpu_80486_memory_penalty(&fixture.instruction.execution, &ticks, &modeled) ||
            ticks != expected || modeled != (misses != 0u && (permission || data_misses > 1u))) {
            lib_c_printf("Port memory mode=%u opcode=%x dword=%u warm=%u: misses=%u/%u reads=%u/%u ticks=%llu/%llu\n",
                (lib_u32)mode, (lib_u32)opcode, (lib_u32)dword, (lib_u32)warm,
                fixture.instruction.instructions.data.timing_data_misses, misses,
                fixture.instruction.instructions.data.timing_missed_reads, reads, ticks, expected);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool nominal_repeat_first(void)
{
    static const struct {
        lib_u8 opcode;
        lib_u8 first;
    } cases[] = {
        {0xa4u, 15u}, {0xa5u, 15u}, {0xa6u, 14u}, {0xa7u, 14u},
        {0xaau, 11u}, {0xabu, 11u}, {0xacu, 11u}, {0xadu, 11u},
        {0xaeu, 12u}, {0xafu, 12u}
    };

    for (lib_u8 dword = 0u; dword != 2u; ++dword) {
        for (lib_u8 count = 0u; count != 4u; ++count) {
            for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
                cpu_instruction_fixture fixture;
                lib_u8 code[3], length = 0u;
                const lib_u64 expected = count == 0u ? 5u :
                    cases[index].opcode <= 0xa5u && count == 1u ? 13u : cases[index].first;
                lib_u64 ticks = LIB_UINT64_MAX;
                t_cpu after;

                cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
                fixture.cpu.data.esi = 0x2000u;
                fixture.cpu.data.edi = 0x3000u;
                fixture.cpu.data.ecx = count;
                if (dword) code[length++] = 0x66u;
                code[length++] = 0xf3u;
                code[length++] = cases[index].opcode;
                if (cpu_instruction_run(&fixture, code, length, &after) != LIB_STATUS_OK ||
                    fixture.fault.valid || fixture.delivered_exception.valid ||
                    !x86_core_cpu_80486_nominal_instruction_cost(&fixture.execution, &ticks) ||
                    ticks != expected) return LIB_FALSE;
            }
        }
    }
    return LIB_TRUE;
}

static lib_bool timing_decode_facts(void)
{
    static const lib_u8 base16[] = {3u, 3u, 5u, 5u, 6u, 7u, 5u, 3u};

    for (lib_u8 code32 = 0u; code32 != 2u; ++code32) {
        for (lib_u8 address_prefix = 0u; address_prefix != 2u; ++address_prefix) {
            const lib_bool address32 = code32 != address_prefix;

            for (lib_u8 mod = 0u; mod != 4u; ++mod) {
                for (lib_u8 rm = 0u; rm != 8u; ++rm) {
                    const lib_bool has_sib = address32 && mod != 3u && rm == 4u;

                    for (lib_u16 sib = 0u; sib < (has_sib ? 256u : 1u); ++sib) {
                        cpu_instruction_fixture fixture;
                        lib_u8 code[8], length = 0u;
                        const lib_bool memory = mod != 3u;
                        const lib_u8 base = address32 ? (has_sib ? sib & 7u : rm) : base16[rm];
                        const lib_bool has_base = memory &&
                            (mod != 0u || (address32 ? base != 5u : rm != 6u));
                        const lib_bool indexed = memory && (address32 ?
                            has_sib && ((sib >> 3u) & 7u) != 4u : rm < 4u);
                        const lib_bool displacement = memory && (mod != 0u || !has_base);
                        const lib_u8 displacement_bytes = !displacement ? 0u :
                            mod == 1u ? 1u : address32 ? 4u : 2u;
                        const lib_u8 base_mask = has_base ? (lib_u8)(1u << base) : 0u;
                        lib_u64 penalty = LIB_UINT64_MAX;
                        t_cpu after;

                        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
                        fixture.cpu.data.cs.seg.exec.defsize = code32;
                        fixture.cpu.data.eax = fixture.cpu.data.ecx = 0x1000u;
                        fixture.cpu.data.edx = fixture.cpu.data.ebx = 0x1000u;
                        fixture.cpu.data.esp = fixture.cpu.data.ebp = 0x1000u;
                        fixture.cpu.data.esi = fixture.cpu.data.edi = 0x1000u;
                        if (address_prefix) code[length++] = 0x67u;
                        code[length++] = 0x8au;
                        code[length++] = (lib_u8)((mod << 6u) | rm);
                        if (has_sib) code[length++] = (lib_u8)sib;
                        for (lib_u8 byte = 0u; byte < displacement_bytes; ++byte)
                            code[length++] = byte == 0u ? 0x10u : 0u;
                        if (cpu_instruction_run(&fixture, code, length, &after) != LIB_STATUS_OK ||
                            fixture.fault.valid || fixture.delivered_exception.valid ||
                            fixture.instructions.data.timing_ea_base_mask != base_mask ||
                            fixture.instructions.data.timing_ea_indexed != indexed ||
                            fixture.instructions.data.timing_ea_displacement != displacement ||
                            fixture.instructions.data.timing_immediate ||
                            !x86_core_cpu_80486_decode_penalty(&fixture.execution, &penalty) ||
                            penalty != (lib_u64)address_prefix + indexed) {
                            lib_c_fprintf(lib_c_stderr,
                                "Timing decode: code32=%u address_prefix=%u mod=%u rm=%u sib=%x\n",
                                code32, address_prefix, mod, rm, sib);
                            return LIB_FALSE;
                        }
                    }
                }
            }
        }
    }
    cpu_instruction_fixture fixture;
    const lib_u8 immediate_then_nop[] = {0xc6u, 0x06u, 0u, 0x20u, 1u, 0x90u};
    t_cpu after;

    cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
    if (cpu_instruction_run(&fixture, immediate_then_nop, sizeof(immediate_then_nop), &after) !=
            LIB_STATUS_OK || fixture.fault.valid || fixture.delivered_exception.valid ||
        !fixture.instructions.data.timing_ea_displacement ||
        !fixture.instructions.data.timing_immediate) return LIB_FALSE;
    lib_u64 penalty = LIB_UINT64_MAX;
    if (!x86_core_cpu_80486_decode_penalty(&fixture.execution, &penalty) || penalty != 1u)
        return LIB_FALSE;
    x86_core_cpu_execution_refresh(&fixture.execution);
    return !fixture.fault.valid && !fixture.delivered_exception.valid &&
        fixture.cpu.data.eip == sizeof(immediate_then_nop) &&
        fixture.instructions.data.timing_ea_base_mask == 0u &&
        !fixture.instructions.data.timing_ea_indexed &&
        !fixture.instructions.data.timing_ea_displacement &&
        !fixture.instructions.data.timing_immediate &&
        x86_core_cpu_80486_decode_penalty(&fixture.execution, &penalty) && penalty == 0u;
}

static lib_bool nominal_protected_transfers(void)
{
    static const lib_u8 expected[] = {20u, 19u, 20u, 18u, 17u, 18u, 20u, 44u, 44u};

    for (lib_u8 cpl = 0u; cpl != 4u; ++cpl) {
        for (lib_u8 dword = 0u; dword != 2u; ++dword) {
            for (lib_u8 accessed = 0u; accessed != 2u; ++accessed) {
                for (lib_u8 form = 0u; form < sizeof(expected); ++form) {
                    cpu_instruction_fixture fixture;
                    lib_u8 code[8], length = 0u;
                    const lib_u32 target = 0x300u;
                    const lib_u16 selector = 8u | cpl;
                    const lib_u16 frame16[] = {0x300u, selector, 2u};
                    const lib_u32 frame32[] = {0x300u, selector, 2u};
                    const lib_u8 gate[] = {0u, 3u, (lib_u8)selector, 0u, 0u, 0xeeu, 0u, 0u};
                    const lib_u8 width = dword ? 4u : 2u;
                    lib_u64 ticks = LIB_UINT64_MAX;
                    t_cpu after;

                    cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
                    cpu_instruction_prepare_protected_exception(&fixture, cpl);
                    fixture.cpu.data.ds.selector = (lib_u16)(0x10u | cpl);
                    fixture.cpu.data.ds.dpl = cpl;
                    fixture.memory[0x20du] = (lib_u8)((fixture.memory[0x20du] & ~1u) | accessed);
                    fixture.cpu.data.idtr.limit = 0x107u;
                    lib_memory_copy(fixture.memory + 0x418u, gate, sizeof(gate));
                    lib_memory_copy(fixture.memory + 0x500u, gate, sizeof(gate));
                    lib_memory_copy(fixture.memory + 0x2000u, &target, width);
                    lib_memory_copy(fixture.memory + 0x2000u + width, &selector, sizeof(selector));
                    if (dword)
                        lib_memory_copy(fixture.memory + 0x800u, frame32, sizeof(frame32));
                    else
                        lib_memory_copy(fixture.memory + 0x800u, frame16, sizeof(frame16));
                    if (dword) code[length++] = 0x66u;
                    if (form < 2u) {
                        code[length++] = form == 0u ? 0x9au : 0xeau;
                        lib_memory_copy(code + length, &target, width);
                        length += width;
                        lib_memory_copy(code + length, &selector, sizeof(selector));
                        length += sizeof(selector);
                    } else if (form < 4u) {
                        code[length++] = 0xffu;
                        code[length++] = form == 2u ? 0x1eu : 0x2eu;
                        code[length++] = 0u;
                        code[length++] = 0x20u;
                    } else {
                        code[length++] = form == 4u ? 0xcbu : form == 5u ? 0xcau :
                            form == 6u ? 0xcfu : form == 7u ? 0xccu : 0xcdu;
                        if (form == 5u) { code[length++] = 0u; code[length++] = 0u; }
                        if (form == 8u) code[length++] = 0x20u;
                    }
                    if (cpu_instruction_run(&fixture, code, length, &after) != LIB_STATUS_OK ||
                        fixture.fault.valid || fixture.delivered_exception.valid ||
                        !x86_core_cpu_80486_nominal_instruction_cost(&fixture.execution, &ticks) ||
                        ticks != expected[form] ||
                        fixture.instructions.data.timing_unaccessed_descriptors != (accessed ? 0u : 1u) ||
                        fixture.instructions.data.timing_transfer_type != 0u ||
                        fixture.instructions.data.timing_transfer_parameters != 0u) {
                        lib_c_fprintf(lib_c_stderr,
                            "Nominal transfer: cpl=%u dword=%u accessed=%u form=%u expected=%u actual=%llu updates=%u exception=%x\n",
                            cpl, dword, accessed, form, expected[form], ticks,
                            fixture.instructions.data.timing_unaccessed_descriptors,
                            fixture.delivered_exception.exception_mask);
                        return LIB_FALSE;
                    }
                }
            }
        }
    }
    return LIB_TRUE;
}

typedef struct dependency_case {
    const char *name;
    lib_u8 code[8];
    lib_u8 bytes;
    lib_u8 writes;
} dependency_case;

static lib_bool timing_dependency_writer(const dependency_case *test)
{
    const lib_bool cpuid = test->code[0] == 0x0fu && test->code[1] == 0xa2u;

    for (lib_u8 base = 0u; base < (cpuid ? 1u : 8u); ++base) {
        cpu_instruction_fixture fixture;
        lib_u8 code[15], bytes = test->bytes;
        lib_u64 ticks = LIB_UINT64_MAX;
        t_cpu after;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        lib_u32 *registers[] = {&fixture.cpu.data.eax, &fixture.cpu.data.ecx,
            &fixture.cpu.data.edx, &fixture.cpu.data.ebx, &fixture.cpu.data.esp,
            &fixture.cpu.data.ebp, &fixture.cpu.data.esi, &fixture.cpu.data.edi};
        for (lib_u8 reg = 0u; reg < 8u; ++reg) *registers[reg] = 0x2000u;
        fixture.cpu.data.cr0 = VCPU_CR0_ET;
        fixture.cpu.data.esp = 0x6000u;
        fixture.cpu.data.ebp = 0x5000u;
        for (lib_u8 slot = 0u; slot < 16u; slot += 2u) fixture.memory[0x6000u + slot] = 2u;
        if (test->code[0] == 0xf2u || test->code[0] == 0xf3u) fixture.cpu.data.ecx = 1u;
        if (test->code[0] == 0xf6u || test->code[0] == 0xf7u ||
            (test->code[0] == 0x66u && test->code[1] == 0xf7u)) {
            fixture.cpu.data.eax = 6u;
            fixture.cpu.data.ecx = 2u;
            fixture.cpu.data.edx = 0u;
        }
        if (test->code[0] == 0x66u && test->code[1] == 0x0fu && test->code[2] >= 0xc8u)
            *registers[test->code[2] & 7u] = 0u;
        if (test->code[0] == 0xc3u || test->code[0] == 0xcbu || test->code[0] == 0xcfu) {
            fixture.memory[0x6000u] = test->bytes;
            fixture.memory[0x6002u] = 0u;
        }
        lib_memory_copy(code, test->code, bytes);
        code[bytes++] = 0x67u;
        code[bytes++] = 0x8au;
        code[bytes++] = (lib_u8)(0x40u | base);
        if (base == 4u) code[bytes++] = 0x24u;
        code[bytes++] = 0u;
        if (cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
            fixture.fault.valid || fixture.delivered_exception.valid ||
            fixture.cpu.data.eip != test->bytes ||
            fixture.instructions.data.timing_gpr_writes != test->writes) {
            lib_c_fprintf(lib_c_stderr, "GPR writer %s: base=%u writes=%x expected=%x eip=%x fault=%x\n",
                test->name, base, fixture.instructions.data.timing_gpr_writes,
                test->writes, fixture.cpu.data.eip, fixture.fault.exception_mask);
            return LIB_FALSE;
        }
        if (cpuid) continue;
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (fixture.fault.valid || fixture.delivered_exception.valid ||
            fixture.cpu.data.eip != bytes ||
            fixture.instructions.data.timing_previous_gpr_writes != test->writes ||
            !x86_core_cpu_80486_decode_penalty(&fixture.execution, &ticks) ||
            ticks != 1u + ((test->writes & (1u << base)) != 0u)) {
            lib_c_fprintf(lib_c_stderr, "GPR dependency %s: base=%u previous=%x ticks=%llu fault=%x\n",
                test->name, base, fixture.instructions.data.timing_previous_gpr_writes,
                ticks, fixture.fault.exception_mask);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool timing_dependency_writers(void)
{
    static const dependency_case cases[] = {
        {"NOP", {0x90u}, 1u, 0u}, {"CMP", {0x39u, 0xc8u}, 2u, 0u},
        {"TEST", {0x85u, 0xc0u}, 2u, 0u}, {"LAHF", {0x9fu}, 1u, 1u},
        {"ADD accumulator", {0x04u, 0u}, 2u, 1u},
        {"OR accumulator", {0x0du, 0u, 0u}, 3u, 1u},
        {"ADC accumulator", {0x15u, 0u, 0u}, 3u, 1u},
        {"SBB accumulator", {0x1du, 0u, 0u}, 3u, 1u},
        {"AND accumulator", {0x25u, 0xffu, 0xffu}, 3u, 1u},
        {"SUB accumulator", {0x2du, 0u, 0u}, 3u, 1u},
        {"XOR accumulator", {0x35u, 0u, 0u}, 3u, 1u},
        {"SAHF", {0x9eu}, 1u, 0u}, {"AAA", {0x37u}, 1u, 1u},
        {"AAS", {0x3fu}, 1u, 1u}, {"DAA", {0x27u}, 1u, 1u},
        {"DAS", {0x2fu}, 1u, 1u}, {"AAM", {0xd4u, 10u}, 2u, 1u},
        {"AAD", {0xd5u, 10u}, 2u, 1u}, {"SALC", {0xd6u}, 1u, 1u},
        {"CBW", {0x98u}, 1u, 1u}, {"CWD", {0x99u}, 1u, 4u},
        {"MUL byte", {0xf6u, 0xe1u}, 2u, 1u},
        {"IMUL byte", {0xf6u, 0xe9u}, 2u, 1u},
        {"DIV byte", {0xf6u, 0xf1u}, 2u, 1u},
        {"IDIV byte", {0xf6u, 0xf9u}, 2u, 1u},
        {"MUL word", {0xf7u, 0xe1u}, 2u, 5u},
        {"IMUL word", {0xf7u, 0xe9u}, 2u, 5u},
        {"DIV word", {0xf7u, 0xf1u}, 2u, 5u},
        {"IDIV word", {0xf7u, 0xf9u}, 2u, 5u},
        {"MUL dword", {0x66u, 0xf7u, 0xe1u}, 3u, 5u},
        {"DIV dword", {0x66u, 0xf7u, 0xf1u}, 3u, 5u},
        {"MOV memory", {0xa1u, 0u, 0x20u}, 3u, 1u},
        {"LODS", {0xacu}, 1u, 0x41u}, {"STOS", {0xaau}, 1u, 0x80u},
        {"MOVS", {0xa4u}, 1u, 0xc0u}, {"CMPS", {0xa6u}, 1u, 0xc0u},
        {"SCAS", {0xaeu}, 1u, 0x80u}, {"REP MOVS", {0xf3u, 0xa4u}, 2u, 0xc2u},
        {"REP LODS", {0xf3u, 0xacu}, 2u, 0x43u},
        {"REP STOS", {0xf3u, 0xaau}, 2u, 0x82u},
        {"REP SCAS", {0xf3u, 0xaeu}, 2u, 0x82u},
        {"XLAT", {0xd7u}, 1u, 1u}, {"LOOP", {0xe2u, 0u}, 2u, 2u},
        {"PUSHA", {0x60u}, 1u, 0x10u}, {"POPA", {0x61u}, 1u, 0xffu},
        {"ENTER", {0xc8u, 0u, 0u, 0u}, 4u, 0x30u},
        {"LEAVE", {0xc9u}, 1u, 0x30u}, {"CALL", {0xe8u, 0u, 0u}, 3u, 0x10u},
        {"RET", {0xc3u}, 1u, 0x10u}, {"RETF", {0xcbu}, 1u, 0x10u},
        {"IRET", {0xcfu}, 1u, 0x10u}, {"PUSHF", {0x9cu}, 1u, 0x10u},
        {"POPF", {0x9du}, 1u, 0x10u},
        {"MOV control", {0x0fu, 0x20u, 0xc1u}, 3u, 2u},
        {"MOV debug", {0x0fu, 0x21u, 0xc1u}, 3u, 2u},
        {"MOV cache test", {0x0fu, 0x24u, 0xd9u}, 3u, 2u},
        {"XADD", {0x0fu, 0xc1u, 0xc8u}, 3u, 3u},
        {"CMPXCHG equal", {0x0fu, 0xb1u, 0xd1u}, 3u, 2u},
        {"CMPXCHG unequal", {0x0fu, 0xb1u, 0xc4u}, 3u, 0x11u},
        {"CMPXCHG high byte", {0x0fu, 0xb0u, 0xc4u}, 3u, 1u},
        {"CPUID", {0x0fu, 0xa2u}, 2u, 0x0fu}
    };

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index)
        if (!timing_dependency_writer(&cases[index])) return LIB_FALSE;
    for (lib_u8 size = 0u; size < 3u; ++size)
    for (lib_u8 form = 0u; form < 4u; ++form)
    for (lib_u8 reg = 0u; reg < 8u; ++reg) {
        dependency_case test = {.name = "register matrix",
            .writes = (lib_u8)(1u << (size == 0u ? reg & 3u : reg))};

        if (size == 2u) test.code[test.bytes++] = 0x66u;
        if (form == 0u) {
            test.code[test.bytes++] = size ? 0x89u : 0x88u;
            test.code[test.bytes++] = (lib_u8)(0xc0u | (reg << 3u) | reg);
        } else if (form == 1u) {
            test.code[test.bytes++] = (lib_u8)((size ? 0xb8u : 0xb0u) | reg);
            for (lib_u8 byte = 0u; byte < (1u << size); ++byte)
                test.code[test.bytes++] = byte == 1u ? 0x20u : 0u;
        } else if (size == 0u) {
            test.code[test.bytes++] = 0xfeu;
            test.code[test.bytes++] = (lib_u8)(0xc0u | (form == 3u ? 8u : 0u) | reg);
        } else test.code[test.bytes++] = (lib_u8)((form == 2u ? 0x40u : 0x48u) | reg);
        if (!timing_dependency_writer(&test)) return LIB_FALSE;
    }
    for (lib_u8 reg = 0u; reg < 8u; ++reg) {
        const dependency_case test = {"BSWAP", {0x66u, 0x0fu, (lib_u8)(0xc8u | reg)},
            3u, (lib_u8)(1u << reg)};
        if (!timing_dependency_writer(&test)) return LIB_FALSE;
    }
    for (lib_u8 size = 0u; size < 2u; ++size)
    for (lib_u8 reg = 1u; reg < 8u; ++reg) {
        dependency_case test = {.name = "XCHG accumulator", .writes = (lib_u8)(1u | (1u << reg))};

        if (size) test.code[test.bytes++] = 0x66u;
        test.code[test.bytes++] = (lib_u8)(0x90u | reg);
        if (!timing_dependency_writer(&test)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool timing_dependency_adjacency(void)
{
    static const struct {
        lib_u8 code[9];
        lib_u8 bytes, first, middle;
        lib_u64 ticks;
    } cases[] = {
        {{0x89u, 0xe4u, 0x50u}, 3u, 2u, 0u, 1u},
        {{0x50u, 0x58u}, 2u, 1u, 0u, 0u},
        {{0x58u, 0x50u}, 2u, 1u, 0u, 0u},
        {{0x58u, 0x67u, 0xffu, 0x30u}, 4u, 1u, 0u, 2u},
        {{0x89u, 0xc0u, 0x90u, 0x67u, 0x8au, 0x40u, 0u}, 7u, 2u, 1u, 1u},
        {{0xc1u, 0xe0u, 0u, 0x67u, 0x8au, 0x40u, 0u}, 7u, 3u, 0u, 1u},
        {{0x89u, 0xdbu, 0x8au, 0u}, 4u, 2u, 0u, 2u},
        {{0x89u, 0xf6u, 0x8au, 0u}, 4u, 2u, 0u, 1u},
        {{0xbeu, 0u, 0x20u, 0xacu}, 4u, 3u, 0u, 1u}
    };

    for (lib_size index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        cpu_instruction_fixture fixture;
        t_cpu after;
        lib_u64 ticks = LIB_UINT64_MAX;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        fixture.cpu.data.eax = fixture.cpu.data.ebx = fixture.cpu.data.esi = 0x2000u;
        fixture.cpu.data.esp = 0x6000u;
        if (cpu_instruction_run(&fixture, cases[index].code, cases[index].bytes, &after) !=
            LIB_STATUS_OK || fixture.cpu.data.eip != cases[index].first) return LIB_FALSE;
        if (cases[index].middle) x86_core_cpu_execution_refresh(&fixture.execution);
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (fixture.fault.valid || fixture.delivered_exception.valid ||
            fixture.cpu.data.eip != cases[index].bytes ||
            !x86_core_cpu_80486_decode_penalty(&fixture.execution, &ticks) || ticks != cases[index].ticks) {
            lib_c_fprintf(lib_c_stderr, "Dependency adjacency %u: previous=%x ticks=%llu\n",
                (lib_u32)index, fixture.instructions.data.timing_previous_gpr_writes, ticks);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool timing_dependency_boundaries(void)
{
    for (lib_u8 nmi = 0u; nmi < 2u; ++nmi) {
        cpu_instruction_fixture fixture;
        const lib_u8 code[] = {0x89u, 0xc0u, 0x0fu, 0x0bu};
        const lib_u8 handler[] = {0x67u, 0x8au, 0x40u, 0u};
        const lib_u16 vector[] = {0x100u, 0u};
        lib_u64 ticks = LIB_UINT64_MAX;
        t_cpu after;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        fixture.cpu.data.eax = 0x2000u;
        fixture.cpu.data.esp = 0x6000u;
        lib_memory_copy(fixture.memory + (nmi ? 2u : 6u) * 4u, vector, sizeof(vector));
        lib_memory_copy(fixture.memory + 0x100u, handler, sizeof(handler));
        if (nmi && x86_core_cpu_set_nmi(&fixture.execution, LIB_TRUE) != LIB_STATUS_OK)
            return LIB_FALSE;
        if (cpu_instruction_run(&fixture, code, sizeof(code), &after) != LIB_STATUS_OK)
            return LIB_FALSE;
        if (!nmi) x86_core_cpu_execution_refresh(&fixture.execution);
        if (fixture.fault.valid || fixture.cpu.data.eip != 0x100u ||
            fixture.instructions.data.timing_gpr_writes != 0u) return LIB_FALSE;
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (fixture.fault.valid || fixture.execution.instruction_fault_delivered ||
            fixture.cpu.data.eip != 0x100u + sizeof(handler) ||
            !x86_core_cpu_80486_decode_penalty(&fixture.execution, &ticks) || ticks != 1u)
            return LIB_FALSE;
    }
    {
        cpu_instruction_fixture fixture;
        const lib_u8 code[] = {0x89u, 0xc0u, 0x67u, 0x8au, 0x40u, 0u};
        const x86_core_cpu_debug_register_patch patch = {
            .mask = X86_CORE_CPU_DEBUG_REGISTER_MASK(X86_CORE_CPU_DEBUG_EAX),
            .values = {[X86_CORE_CPU_DEBUG_EAX] = 0x2000u}};
        lib_u64 ticks = LIB_UINT64_MAX;
        t_cpu after;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        fixture.cpu.data.eax = 0x2000u;
        if (cpu_instruction_run(&fixture, code, sizeof(code), &after) != LIB_STATUS_OK ||
            fixture.instructions.data.timing_gpr_writes != 1u)
            return LIB_FALSE;
        const x86_core_cpu_debug_register_patch invalid = {0};
        if (x86_core_cpu_debug_patch_registers(&fixture.execution, &invalid) !=
                LIB_STATUS_INVALID_ARGUMENT || fixture.instructions.data.timing_gpr_writes != 1u ||
            x86_core_cpu_debug_patch_registers(&fixture.execution, &patch) != LIB_STATUS_OK)
            return LIB_FALSE;
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (!x86_core_cpu_80486_decode_penalty(&fixture.execution, &ticks) || ticks != 1u)
            return LIB_FALSE;
        x86_core_cpu_state_reset(&fixture.execution);
        if (fixture.instructions.data.timing_gpr_writes != 0u ||
            fixture.instructions.data.timing_previous_gpr_writes != 0u) return LIB_FALSE;
    }
    for (lib_u8 commit = 0u; commit < 2u; ++commit) {
        cpu_instruction_fixture fixture;
        const lib_u8 code[] = {0x89u, 0xc0u, 0x67u, 0x8au, 0x40u, 0u};
        const x86_core_cpu_entry_plan_state state = {.ip = 2u, .sp = 0x6000u, .eax = 0x2000u};
        x86_core_cpu_prepared_entry *entry = LIB_NULL;
        lib_u64 ticks = LIB_UINT64_MAX;
        t_cpu after;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        fixture.cpu.data.eax = 0x2000u;
        if (cpu_instruction_run(&fixture, code, sizeof(code), &after) != LIB_STATUS_OK ||
            fixture.instructions.data.timing_gpr_writes != 1u ||
            x86_core_cpu_prepare_entry(&fixture.execution, &state, &entry) != LIB_STATUS_OK)
            return LIB_FALSE;
        x86_core_cpu_finish_entry(entry, commit);
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (!x86_core_cpu_80486_decode_penalty(&fixture.execution, &ticks) ||
            ticks != (commit ? 1u : 2u)) return LIB_FALSE;
    }
    {
        cpu_instruction_fixture fixture;
        const lib_u8 code[] = {0x66u, 0xc8u, 0u, 0u, 2u};
        t_cpu after;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        fixture.cpu.data.ss.limit = LIB_UINT32_MAX;
        fixture.cpu.data.ss.seg.data.big = LIB_TRUE;
        fixture.cpu.data.esp = sizeof(fixture.memory);
        fixture.cpu.data.ebp = sizeof(fixture.memory) + 8u;
        if (cpu_instruction_run(&fixture, code, sizeof(code), &after) == LIB_STATUS_OK ||
            !fixture.fault.valid || fixture.writes == 0u ||
            fixture.instructions.data.timing_gpr_writes != 0u) return LIB_FALSE;
    }
    return LIB_TRUE;
}

/* These numerical checks qualify the frozen L2 equations and operand choice,
 * not the contradictory source tables as measurements of actual silicon. */
static lib_bool instruction_model_expect(cpu_instruction_fixture *fixture, lib_u64 expected)
{
    const x86_core_cpu_execution_context before = fixture->execution;
    const t_cpuins instructions = fixture->instructions;
    const t_cpu cpu = fixture->cpu;
    lib_u64 ticks = 999u, nominal = 999u;

    if (fixture->fault.valid || fixture->delivered_exception.valid ||
        !x86_core_cpu_80486_l2_instruction_cost(&fixture->execution, &ticks) || ticks != expected ||
        x86_core_cpu_80486_nominal_instruction_cost(&fixture->execution, &nominal) || nominal != 999u ||
        lib_memory_compare(&before, &fixture->execution, sizeof(before)) ||
        lib_memory_compare(&instructions, &fixture->instructions, sizeof(instructions)) ||
        lib_memory_compare(&cpu, &fixture->cpu, sizeof(cpu))) {
        lib_c_fprintf(lib_c_stderr, "L2 instruction expected=%llu actual=%llu nominal=%llu\n",
            expected, ticks, nominal);
        return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool arithmetic_model_multiply(void)
{
    static const struct {
        lib_u32 value;
        lib_u8 ticks[3];
    } cases[] = {
        {0u, {13u, 13u, 13u}}, {1u, {13u, 13u, 13u}},
        {LIB_UINT32_MAX, {15u, 15u, 15u}}, {2u, {13u, 13u, 13u}},
        {0xfffffffeu, {15u, 15u, 15u}}, {7u, {13u, 13u, 13u}},
        {0xfffffff9u, {15u, 15u, 15u}}, {8u, {13u, 13u, 13u}},
        {0xfffffff8u, {15u, 15u, 15u}}, {9u, {14u, 14u, 14u}},
        {0xfffffff7u, {15u, 15u, 15u}}, {32u, {15u, 15u, 15u}},
        {0xffffffe0u, {15u, 15u, 15u}}, {33u, {16u, 16u, 16u}},
        {0xffffffdfu, {16u, 16u, 16u}}, {0x7fu, {17u, 17u, 17u}},
        {0x80u, {17u, 17u, 17u}}, {0xffu, {15u, 18u, 18u}},
        {0x100u, {13u, 18u, 18u}}, {0x7fffu, {15u, 25u, 25u}},
        {0x8000u, {13u, 25u, 25u}}, {0xffffu, {15u, 15u, 26u}},
        {0x10000u, {13u, 13u, 26u}}, {0x7fffffffu, {15u, 15u, 41u}},
        {0x80000000u, {13u, 13u, 41u}}, {0x40000000u, {13u, 13u, 40u}},
        {0x40000001u, {13u, 13u, 41u}}
    };

    for (lib_u8 form = 0u; form < 5u; ++form)
    for (lib_u8 size = 0u; size < 3u; ++size)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 memory = 0u; memory < 2u; ++memory)
    for (lib_u8 other = 0u; other < 2u; ++other)
    for (lib_size row = 0u; row < sizeof(cases) / sizeof(cases[0]); ++row) {
        cpu_instruction_fixture fixture;
        lib_u8 code[12], bytes = 0u;
        const lib_u32 multiplicand = other ? 0x80000001u : 2u;
        const lib_u32 operand = form >= 3u ? multiplicand : cases[row].value;
        t_cpu after;

        if ((form == 0u) != (size == 0u)) continue;
        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        fixture.cpu.data.eax = multiplicand;
        fixture.cpu.data.ebx = operand;
        lib_memory_copy(fixture.memory + 0x2000u, &operand, sizeof(operand));
        if (code32) code[bytes++] = 0x67u; /* Direct memory remains 16-bit. */
        if (size != 0u && (size == 2u) != code32) code[bytes++] = 0x66u;
        if (form == 2u) code[bytes++] = 0x0fu;
        code[bytes++] = form == 0u ? 0xf6u : form == 1u ? 0xf7u :
            form == 2u ? 0xafu : form == 3u ? 0x69u : 0x6bu;
        code[bytes++] = (memory ? 0x06u : 0xc3u) | (form < 2u ? 5u << 3u : 0u);
        if (memory) { code[bytes++] = 0u; code[bytes++] = 0x20u; }
        if (form >= 3u) {
            const lib_u8 immediate_bytes = form == 4u ? 1u : (1u << size);

            for (lib_u8 index = 0u; index < immediate_bytes; ++index)
                code[bytes++] = (lib_u8)(cases[row].value >> (index * 8u));
        }
        if (cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
            !instruction_model_expect(&fixture, cases[row].ticks[form == 4u ? 0u : size])) {
            lib_c_fprintf(lib_c_stderr, "L2 IMUL form=%u size=%u code32=%u memory=%u other=%u value=%x\n",
                (lib_u32)form, (lib_u32)size, (lib_u32)code32,
                (lib_u32)memory, (lib_u32)other, cases[row].value);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool arithmetic_model_rotate(void)
{
    static const struct {
        lib_u8 count, reg[3], memory[3];
    } cases[] = {
        {0u, {8u, 8u, 8u}, {9u, 9u, 9u}}, {1u, {8u, 8u, 8u}, {9u, 9u, 9u}},
        {7u, {8u, 8u, 8u}, {9u, 9u, 9u}}, {8u, {8u, 8u, 8u}, {9u, 9u, 9u}},
        {9u, {16u, 8u, 8u}, {16u, 9u, 9u}}, {15u, {16u, 8u, 8u}, {16u, 9u, 9u}},
        {16u, {23u, 8u, 8u}, {23u, 9u, 9u}}, {17u, {23u, 16u, 8u}, {23u, 16u, 9u}},
        {23u, {23u, 16u, 8u}, {23u, 16u, 9u}}, {24u, {30u, 16u, 8u}, {30u, 16u, 9u}},
        {31u, {30u, 16u, 8u}, {30u, 16u, 9u}}, {32u, {8u, 8u, 8u}, {9u, 9u, 9u}},
        {33u, {8u, 8u, 8u}, {9u, 9u, 9u}}, {255u, {30u, 16u, 8u}, {30u, 16u, 9u}}
    };

    for (lib_u8 size = 0u; size < 3u; ++size)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 immediate = 0u; immediate < 2u; ++immediate)
    for (lib_u8 right = 0u; right < 2u; ++right)
    for (lib_u8 memory = 0u; memory < 2u; ++memory)
    for (lib_u8 carry = 0u; carry < 2u; ++carry)
    for (lib_size row = 0u; row < sizeof(cases) / sizeof(cases[0]); ++row) {
        cpu_instruction_fixture fixture;
        lib_u8 code[8], bytes = 0u;
        t_cpu after;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        fixture.cpu.data.ebx = 0x80008081u;
        fixture.cpu.data.ecx = cases[row].count;
        fixture.cpu.data.eflags = 2u | carry;
        lib_memory_copy(fixture.memory + 0x2000u, &fixture.cpu.data.ebx, sizeof(lib_u32));
        if (code32) code[bytes++] = 0x67u;
        if (size != 0u && (size == 2u) != code32) code[bytes++] = 0x66u;
        code[bytes++] = (immediate ? 0xc0u : 0xd2u) + (size != 0u);
        code[bytes++] = (memory ? 0x06u : 0xc3u) | ((2u + right) << 3u);
        if (memory) { code[bytes++] = 0u; code[bytes++] = 0x20u; }
        if (immediate) code[bytes++] = cases[row].count;
        if (cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
            !instruction_model_expect(&fixture,
                memory ? cases[row].memory[size] : cases[row].reg[size])) {
            lib_c_fprintf(lib_c_stderr, "L2 carry-rotate size=%u code32=%u immediate=%u right=%u memory=%u carry=%u count=%u\n",
                (lib_u32)size, (lib_u32)code32, (lib_u32)immediate, (lib_u32)right,
                (lib_u32)memory, (lib_u32)carry, (lib_u32)cases[row].count);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool special_instruction_models(void)
{
    for (lib_u8 mode = 0u; mode < 6u; ++mode)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes)
    for (lib_u8 carry = 0u; carry < 2u; ++carry)
    for (lib_u8 form = 0u; form < 10u; ++form) {
        cpu_instruction_fixture fixture;
        lib_u8 code[10], bytes = 0u;
        t_cpu after;
        const lib_bool memory = form >= 6u;
        const lib_bool address32 = code32 != ((prefixes & 2u) != 0u);

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        if (mode != 0u) cpu_instruction_prepare_protected_exception(&fixture, mode - 1u);
        fixture.cpu.data.ds = fixture.cpu.data.ss;
        fixture.cpu.data.ds.sregtype = SREG_DATA;
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        fixture.cpu.data.eax = 0x12345678u;
        fixture.cpu.data.ecx = 0x87654321u;
        fixture.cpu.data.eflags |= carry ? VCPU_EFLAGS_CF : 0u;
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        if (form < 2u) code[bytes++] = form == 0u ? 0x9bu : 0xd6u;
        else {
            code[bytes++] = 0x0fu;
            code[bytes++] = (lib_u8)(0x10u + ((form - 2u) & 3u));
            code[bytes++] = (lib_u8)(0x08u | (memory ? (address32 ? 5u : 6u) : 0xc0u));
            if (memory) {
                code[bytes++] = 0u;
                code[bytes++] = 0x20u;
                if (address32) { code[bytes++] = 0u; code[bytes++] = 0u; }
            }
        }
        if (cpu_instruction_run(&fixture, code, bytes, &after) != LIB_STATUS_OK ||
            !instruction_model_expect(&fixture, form == 0u ? 3u : 1u)) {
            lib_c_fprintf(lib_c_stderr,
                "Special model mode=%u code32=%u prefixes=%u carry=%u form=%u\n",
                (lib_u32)mode, (lib_u32)code32, (lib_u32)prefixes,
                (lib_u32)carry, (lib_u32)form);
            return LIB_FALSE;
        }
    }
    /* A fault is not a successful WAIT/alias subtotal. Preserve caller output
     * on real NM, invalid LOCK and provider failure, not only fabricated flags. */
    for (lib_u8 form = 0u; form < 4u; ++form) {
        cpu_instruction_fixture fixture;
        const lib_u8 wait[] = {0x9bu};
        const lib_u8 locked[] = {0xf0u, form == 1u ? 0x9bu : 0xd6u};
        const lib_u8 bad_read[] = {0x67u, 0x0fu, 0x12u, 0x05u, 0u, 0u, 8u, 0u};
        lib_u64 ticks = 999u;
        t_cpu after;

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        if (form == 0u) {
            fixture.cpu.data.cr0 |= VCPU_CR0_TS | VCPU_CR0_MP;
            if (!cpu_instruction_expect_real_fault(&fixture, wait, sizeof(wait), 7u))
                return LIB_FALSE;
        } else if (form < 3u) {
            if (!cpu_instruction_expect_real_fault(&fixture, locked, sizeof(locked), 6u))
                return LIB_FALSE;
        } else {
            fixture.cpu.data.ds.limit = LIB_UINT32_MAX;
            if (cpu_instruction_run(&fixture, bad_read, sizeof(bad_read), &after) == LIB_STATUS_OK ||
                !fixture.fault.valid) return LIB_FALSE;
        }
        if (x86_core_cpu_80486_l2_instruction_cost(&fixture.execution, &ticks) || ticks != 999u)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

/* A decoder census complements source-derived numerical vectors: every
 * successfully executed encoding must have exactly one nominal owner.
 * Fault paths and SMM-only RSM have separate event/system proof owners. */
static lib_bool nominal_encoding_allocation(void)
{
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 escaped = 0u; escaped < 2u; ++escaped)
    for (lib_u16 opcode = 0u; opcode < 256u; ++opcode)
    for (lib_u16 suffix = 0u; suffix < 256u; ++suffix) {
        cpu_port_instruction_fixture ports;
        cpu_instruction_fixture *fixture = &ports.instruction;
        lib_u8 code[15] = {0};
        lib_u64 nominal = 999u, model = 999u;
        t_cpu after;

        cpu_port_prepare(&ports, X86_CORE_CPU_PROFILE_80486SX);
        fixture->cpu.data.cs.seg.exec.defsize = code32;
        fixture->cpu.data.esp = 0x6000u;
        fixture->cpu.data.ebp = 0x5000u;
        fixture->cpu.data.ebx = fixture->cpu.data.esi = 0x2000u;
        fixture->cpu.data.edi = 0x3000u;
        fixture->cpu.data.ecx = 2u;
        if (escaped) code[0] = 0x0fu;
        code[escaped] = (lib_u8)opcode;
        code[escaped + 1u] = (lib_u8)suffix;
        if (cpu_instruction_run(fixture, code, sizeof(code), &after) != LIB_STATUS_OK ||
            fixture->fault.valid || fixture->delivered_exception.valid ||
            fixture->instructions.data.except != 0u) continue;
        const lib_bool original =
            x86_core_cpu_80486_nominal_instruction_cost(&fixture->execution, &nominal);
        const lib_bool modeled =
            x86_core_cpu_80486_l2_instruction_cost(&fixture->execution, &model);

        if (original == modeled) {
            lib_c_fprintf(lib_c_stderr,
                "Nominal allocation code32=%u escaped=%u opcode=%02x suffix=%02x original=%u modeled=%u\n",
                (lib_u32)code32, (lib_u32)escaped, (lib_u32)opcode,
                (lib_u32)suffix, (lib_u32)original, (lib_u32)modeled);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_bool arithmetic_model_rejects_unowned(void)
{
    cpu_instruction_fixture fixture;
    const lib_u8 cases[][2] = {{0x90u, 0x90u}, {0xf7u, 0xe3u}, {0xd0u, 0xd3u}};
    lib_u64 ticks = 999u;
    t_cpu after;

    cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
    if (x86_core_cpu_80486_l2_instruction_cost(LIB_NULL, &ticks) ||
        x86_core_cpu_80486_l2_instruction_cost(&fixture.execution, LIB_NULL) ||
        x86_core_cpu_80486_l2_instruction_cost(&fixture.execution, &ticks) || ticks != 999u)
        return LIB_FALSE;
    for (lib_size row = 0u; row < sizeof(cases) / sizeof(cases[0]); ++row) {
        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        if (cpu_instruction_run(&fixture, cases[row], row ? 2u : 1u, &after) != LIB_STATUS_OK ||
            x86_core_cpu_80486_l2_instruction_cost(&fixture.execution, &ticks) || ticks != 999u)
            return LIB_FALSE;
    }
    cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80386);
    if (cpu_instruction_run(&fixture, (const lib_u8[]){0xf7u, 0xebu}, 2u, &after) != LIB_STATUS_OK ||
        x86_core_cpu_80486_l2_instruction_cost(&fixture.execution, &ticks) || ticks != 999u)
        return LIB_FALSE;
    for (lib_u8 rotate = 0u; rotate < 2u; ++rotate) {
        const lib_u8 code[] = {0x67u, rotate ? 0xd3u : 0xf7u,
            rotate ? 0x15u : 0x2du, 0u, 0u, 8u, 0u};

        cpu_instruction_prepare(&fixture, X86_CORE_CPU_PROFILE_80486SX);
        fixture.cpu.data.ds.limit = LIB_UINT32_MAX;
        fixture.cpu.data.ecx = 1u;
        if (cpu_instruction_run(&fixture, code, sizeof(code), &after) == LIB_STATUS_OK ||
            !fixture.fault.valid || fixture.fault.exception_mask != VCPUINS_EXCEPT_CE ||
            x86_core_cpu_80486_l2_instruction_cost(&fixture.execution, &ticks) || ticks != 999u)
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

int main(void)
{
    if (!nominal_execution() || !nominal_alu_forms() || !nominal_condition_forms() ||
        !nominal_protection_queries() || !nominal_exchange_memory() || !nominal_system_forms() ||
        !nominal_rejects_unowned() || !nominal_modes() ||
        !nominal_cpuid() || !nominal_bit_scan() || !nominal_unsigned_multiply() ||
        !nominal_ports() || !port_memory_timing() || !nominal_repeat_first() || !timing_decode_facts() ||
        !nominal_protected_transfers() || !timing_prefix_penalties() ||
        !timing_decode_penalties() || !timing_dependency_writers() ||
        !timing_dependency_adjacency() || !arithmetic_model_multiply() ||
        !arithmetic_model_rotate() || !special_instruction_models() || !nominal_encoding_allocation() ||
        !arithmetic_model_rejects_unowned()) return 1;
    if (!timing_dependency_boundaries()) {
        lib_c_fprintf(lib_c_stderr, "Dependency boundary failed\n");
        return 1;
    }
    lib_c_printf("80486:NOMINAL-TIMING:OK\n");
    return 0;
}
