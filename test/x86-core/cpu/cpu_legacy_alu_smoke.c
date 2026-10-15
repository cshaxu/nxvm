#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "support/cpu_instruction_fixture.h"
/* REAL_UD_TERMINAL_CPU_OWNER: legacy invalid opcodes are CPU-owned. */

#define LEGACY_ALU_MEMORY 0x5000u
#define LEGACY_ALU_FLAGS (VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF | \
    VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF)

typedef cpu_instruction_fixture legacy_alu_machine;

typedef enum legacy_alu_operation {
    LEGACY_ALU_ADD,
    LEGACY_ALU_OR,
    LEGACY_ALU_ADC,
    LEGACY_ALU_SBB,
    LEGACY_ALU_AND,
    LEGACY_ALU_SUB,
    LEGACY_ALU_XOR,
    LEGACY_ALU_CMP
} legacy_alu_operation;

static lib_u32 *legacy_alu_register(t_cpu *cpu, lib_u8 index)
{
    switch (index) {
    case 0u: return &cpu->data.eax;
    case 1u: return &cpu->data.ecx;
    case 2u: return &cpu->data.edx;
    case 3u: return &cpu->data.ebx;
    case 4u: return &cpu->data.esp;
    case 5u: return &cpu->data.ebp;
    case 6u: return &cpu->data.esi;
    case 7u: return &cpu->data.edi;
    default: return LIB_NULL;
    }
}

static lib_i32 legacy_alu_run(legacy_alu_machine *state,
    const lib_u8 *code, lib_size bytes, lib_i32 fault, t_cpu *after)
{
    lib_status status;

    if (fault) state->cpu.data.idtr.limit = 0x17u;
    status = cpu_instruction_run(state, code, (lib_u8)bytes, after);
    return status == (fault ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK) &&
        state->fault.valid == (fault ? LIB_TRUE : LIB_FALSE);
}

static lib_u32 legacy_alu_mask(lib_u8 width)
{
    return width == 8u ? 0xffu : (width == 16u ? 0xffffu : 0xffffffffu);
}

static lib_u32 legacy_alu_parity(lib_u32 value)
{
    lib_u8 byte = X86_CPU_MASK_U8(value);
    lib_u8 bit;
    lib_u32 parity = 1u;

    for (bit = 0u; bit != 8u; ++bit)
        parity ^= (byte >> bit) & 1u;
    return parity;
}

static lib_u32 legacy_alu_flags(legacy_alu_operation operation,
    lib_u32 left, lib_u32 right, lib_u32 carry,
    lib_u8 width, lib_u32 result, lib_u32 before)
{
    const lib_u32 mask = legacy_alu_mask(width);
    const lib_u32 sign = width == 8u ? 0x80u :
        (width == 16u ? 0x8000u : 0x80000000u);
    lib_u32 flags = before & ~LEGACY_ALU_FLAGS;
    lib_u32 cf = 0u;
    lib_u32 of = 0u;
    lib_u32 af = 0u;

    left &= mask;
    right &= mask;
    result &= mask;
    if (operation == LEGACY_ALU_ADD || operation == LEGACY_ALU_ADC) {
        const lib_u64 sum = (lib_u64)left + right +
            (operation == LEGACY_ALU_ADC ? carry : 0u);
        cf = sum > mask;
        of = ((~(left ^ right) & (left ^ result)) & sign) != 0u;
        af = ((left ^ right ^ result) & 0x10u) != 0u;
    } else if (operation == LEGACY_ALU_SUB || operation == LEGACY_ALU_SBB ||
        operation == LEGACY_ALU_CMP) {
        const lib_u64 subtrahend = (lib_u64)right +
            (operation == LEGACY_ALU_SBB ? carry : 0u);
        cf = left < subtrahend;
        of = (((left ^ right) & (left ^ result)) & sign) != 0u;
        af = ((left ^ right ^ result) & 0x10u) != 0u;
    }
    if (operation == LEGACY_ALU_ADD || operation == LEGACY_ALU_ADC ||
        operation == LEGACY_ALU_SUB || operation == LEGACY_ALU_SBB ||
        operation == LEGACY_ALU_CMP) {
        if (cf) flags |= VCPU_EFLAGS_CF;
        if (of) flags |= VCPU_EFLAGS_OF;
        if (af) flags |= VCPU_EFLAGS_AF;
    }
    if (result == 0u) flags |= VCPU_EFLAGS_ZF;
    if (result & sign) flags |= VCPU_EFLAGS_SF;
    if (legacy_alu_parity(result)) flags |= VCPU_EFLAGS_PF;
    return flags;
}

static lib_i32 legacy_alu_binary_case(x86_core_cpu_profile profile,
    legacy_alu_operation operation, lib_u8 encoding,
    lib_u8 width, lib_i32 memory, lib_u8 vector, lib_u8 carry_in)
{
    static const lib_u8 base[] = {
        0x00u, 0x08u, 0x10u, 0x18u, 0x20u, 0x28u, 0x30u, 0x38u
    };
    const lib_u32 mask = legacy_alu_mask(width);
    const lib_u32 sign = (mask >> 1u) + 1u;
    const lib_u32 operands[][2] = {
        {sign - 1u, 1u}, {mask, 1u}, {0u, mask}, {0u, 0u},
        {sign, 1u}, {mask, mask}, {0x55555555u & mask, 0xaaaaaaaau & mask},
        {sign - 1u, sign}
    };
    const lib_u32 left = operands[vector][0];
    const lib_u32 right = operands[vector][1];
    const lib_u32 carry = operation == LEGACY_ALU_ADC ||
        operation == LEGACY_ALU_SBB ? carry_in : 0u;
    lib_u32 expected;
    lib_u32 observed = 0u;
    lib_u8 code[6] = { 0u };
    lib_size bytes = 0u;
    legacy_alu_machine state;
    t_cpu after;
    t_cpu before;

    lib_i32 failed = 0;
    cpu_instruction_prepare(&state, profile);

    if (operation == LEGACY_ALU_ADD || operation == LEGACY_ALU_ADC)
        expected = (left + right + carry) & mask;
    else if (operation == LEGACY_ALU_SUB || operation == LEGACY_ALU_SBB ||
        operation == LEGACY_ALU_CMP)
        expected = (left - right - carry) & mask;
    else if (operation == LEGACY_ALU_OR)
        expected = left | right;
    else if (operation == LEGACY_ALU_AND)
        expected = left & right;
    else
        expected = left ^ right;
    if (width == 32u) code[bytes++] = 0x66u;
    code[bytes++] = (lib_u8)(base[operation] +
        (encoding == 0u ? (width == 8u ? 0u : 1u) :
        (width == 8u ? 2u : 3u)));
    if (memory) {
        code[bytes++] = encoding == 0u ? 0x0eu : 0x06u;
        code[bytes++] = X86_CPU_MASK_U8(LEGACY_ALU_MEMORY);
        code[bytes++] = X86_CPU_MASK_U8(LEGACY_ALU_MEMORY >> 8u);
    } else
        code[bytes++] = encoding == 0u ? 0xc8u : 0xc1u;
    state.cpu.data.eax = left;
    state.cpu.data.ecx = right;
    state.cpu.data.eflags = (carry_in ? VCPU_EFLAGS_CF : 0u) |
        VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
    if (memory)
        failed |= cpu_instruction_write(&state, LEGACY_ALU_MEMORY,
            encoding == 0u ? &left : &right,
            width == 8u ? 1u : (width == 16u ? 2u : 4u),
            X86_CORE_CPU_MEMORY_ACCESS_DATA, 0u) != LIB_STATUS_OK;
    before = state.cpu;
    failed |= !legacy_alu_run(&state, code, bytes, 0, &after) ||
        state.fault.valid || after.data.eip != bytes ||
        after.data.eflags != legacy_alu_flags(operation, left, right, carry,
            width, expected, before.data.eflags);
    if (memory)
        failed |= cpu_instruction_read(&state, LEGACY_ALU_MEMORY,
            &observed, width == 8u ? 1u : (width == 16u ? 2u : 4u),
            X86_CORE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE,
            LIB_FALSE, 0u) != LIB_STATUS_OK ||
            observed != (encoding == 0u && operation != LEGACY_ALU_CMP ?
                expected : (encoding == 0u ? left : right));
    else if (operation != LEGACY_ALU_CMP)
        failed |= (after.data.eax & mask) != expected;
    else
        failed |= (after.data.eax & mask) != left;

    return !failed;
}

static lib_i32 legacy_alu_test_binary_forms(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086, X86_CORE_CPU_PROFILE_80186,
        X86_CORE_CPU_PROFILE_80286, X86_CORE_CPU_PROFILE_80386,
        X86_CORE_CPU_PROFILE_80486SX
    };
    lib_u8 profile_index;
    lib_u8 operation;
    lib_u8 encoding;
    lib_u8 width_index;
    lib_u8 memory;
    lib_u8 vector;
    lib_u8 carry;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (operation = 0u; operation != 8u; ++operation)
    for (encoding = 0u; encoding != 2u; ++encoding)
    for (width_index = 0u; width_index != 2u; ++width_index)
    for (memory = 0u; memory != 2u; ++memory)
    for (vector = 0u; vector != 8u; ++vector)
    for (carry = 0u; carry != 2u; ++carry)
        if (!legacy_alu_binary_case(profiles[profile_index],
                (legacy_alu_operation)operation, encoding,
                width_index == 0u ? 8u : 16u, memory, vector, carry))
            return 0;
    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (operation = 0u; operation != 8u; ++operation)
    for (encoding = 0u; encoding != 2u; ++encoding)
    for (memory = 0u; memory != 2u; ++memory)
    for (vector = 0u; vector != 8u; ++vector)
    for (carry = 0u; carry != 2u; ++carry)
        if (profiles[profile_index] >= X86_CORE_CPU_PROFILE_80386 &&
            !legacy_alu_binary_case(profiles[profile_index],
                (legacy_alu_operation)operation, encoding, 32u, memory, vector, carry))
            return 0;
    return 1;
}

static lib_i32 legacy_alu_test_accumulator_immediate_forms(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086, X86_CORE_CPU_PROFILE_80186,
        X86_CORE_CPU_PROFILE_80286, X86_CORE_CPU_PROFILE_80386,
        X86_CORE_CPU_PROFILE_80486SX
    };
    static const lib_u8 base[] = {
        0x04u, 0x0cu, 0x14u, 0x1cu, 0x24u, 0x2cu, 0x34u, 0x3cu
    };
    lib_u8 profile_index;
    lib_u8 operation;
    lib_u8 width_index;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (operation = 0u; operation != 8u; ++operation)
    for (width_index = 0u; width_index != 3u; ++width_index) {
        const lib_u8 width = width_index == 0u ? 8u :
            (width_index == 1u ? 16u : 32u);
        const lib_u32 left = width == 8u ? 0x7fu :
            (width == 16u ? 0x7fffu : 0x7fffffffu);
        const lib_u32 right = 1u;
        const lib_u32 carry = operation == LEGACY_ALU_ADC ||
            operation == LEGACY_ALU_SBB ? 1u : 0u;
        const lib_u32 mask = legacy_alu_mask(width);
        const lib_u32 expected = operation == LEGACY_ALU_ADD ||
            operation == LEGACY_ALU_ADC ? (left + right + carry) & mask :
            (operation == LEGACY_ALU_SUB || operation == LEGACY_ALU_SBB ||
            operation == LEGACY_ALU_CMP ? (left - right - carry) & mask :
            (operation == LEGACY_ALU_OR ? left | right :
            (operation == LEGACY_ALU_AND ? left & right : left ^ right)));
        lib_u8 code[6] = { 0u };
        lib_size bytes = 0u;
        legacy_alu_machine state;
        t_cpu before;
        t_cpu after;

        lib_i32 failed;

        if (width == 32u && profiles[profile_index] < X86_CORE_CPU_PROFILE_80386)
            continue;
        if (width == 32u) code[bytes++] = 0x66u;
        code[bytes++] = (lib_u8)(base[operation] +
            (width == 8u ? 0u : 1u));
        code[bytes++] = 1u;
        if (width != 8u) {
            code[bytes++] = 0u;
            if (width == 32u) {
                code[bytes++] = 0u;
                code[bytes++] = 0u;
            }
        }
        cpu_instruction_prepare(&state, profiles[profile_index]);
        failed = 0;
        state.cpu.data.eax = width == 32u ? left :
            (0xaabb0000u | left);
        state.cpu.data.eflags = VCPU_EFLAGS_CF |
            VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
        before = state.cpu;
        failed |= !legacy_alu_run(&state, code, bytes, 0, &after) || state.fault.valid || after.data.eip != bytes ||
            after.data.eflags != legacy_alu_flags((legacy_alu_operation)operation,
            left, right, carry, width, expected, before.data.eflags) ||
            (after.data.eax & mask) != (operation == LEGACY_ALU_CMP ? left :
            expected) || (width == 8u && (after.data.eax & 0xffffff00u) !=
            (before.data.eax & 0xffffff00u));

        if (failed) return 0;
    }
    return 1;
}

static lib_i32 legacy_alu_group1_case(x86_core_cpu_profile profile,
    legacy_alu_operation operation, lib_u8 width,
    lib_i32 sign_extended, lib_i32 memory, lib_bool byte_alias, lib_u8 input_carry)
{
    const lib_u32 mask = legacy_alu_mask(width);
    const lib_u32 left = width == 8u ? 0x7fu :
        (width == 16u ? 0x7fffu : 0x7fffffffu);
    const lib_u32 right = sign_extended ? mask : 0x0001u;
    const lib_u32 carry = operation == LEGACY_ALU_ADC ||
        operation == LEGACY_ALU_SBB ? input_carry : 0u;
    lib_u32 expected;
    lib_u32 observed = 0u;
    lib_u8 code[9] = { 0u };
    lib_size bytes = 0u;
    legacy_alu_machine state;
    t_cpu after;
    t_cpu before;

    lib_i32 failed = 0;
    cpu_instruction_prepare(&state, profile);

    if (operation == LEGACY_ALU_ADD || operation == LEGACY_ALU_ADC)
        expected = (left + right + carry) & mask;
    else if (operation == LEGACY_ALU_SUB || operation == LEGACY_ALU_SBB ||
        operation == LEGACY_ALU_CMP)
        expected = (left - right - carry) & mask;
    else if (operation == LEGACY_ALU_OR)
        expected = left | right;
    else if (operation == LEGACY_ALU_AND)
        expected = left & right;
    else
        expected = left ^ right;
    if (width == 32u) code[bytes++] = 0x66u;
    code[bytes++] = width == 8u ? (byte_alias ? 0x82u : 0x80u) :
        (sign_extended ? 0x83u : 0x81u);
    code[bytes++] = (lib_u8)(operation << 3u) | (memory ? 0x06u : 0xc0u);
    if (memory) {
        code[bytes++] = X86_CPU_MASK_U8(LEGACY_ALU_MEMORY);
        code[bytes++] = X86_CPU_MASK_U8(LEGACY_ALU_MEMORY >> 8u);
    }
    if (width == 8u || sign_extended)
        code[bytes++] = sign_extended ? 0xffu : 0x01u;
    else {
        code[bytes++] = 0x01u;
        for (lib_u8 byte = 1u; byte < width / 8u; ++byte) code[bytes++] = 0u;
    }
    state.cpu.data.eax = left;
    state.cpu.data.eflags = (input_carry ? VCPU_EFLAGS_CF : 0u) |
        VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
    if (memory)
        failed |= cpu_instruction_write(&state, LEGACY_ALU_MEMORY, &left, width / 8u,
            X86_CORE_CPU_MEMORY_ACCESS_DATA, 0u) != LIB_STATUS_OK;
    before = state.cpu;
    failed |= !legacy_alu_run(&state, code, bytes, 0, &after) ||
        state.fault.valid || after.data.eip != bytes ||
        after.data.eflags != legacy_alu_flags(operation, left, right, carry,
            width, expected, before.data.eflags);
    if (memory)
        failed |= cpu_instruction_read(&state, LEGACY_ALU_MEMORY, &observed, width / 8u,
            X86_CORE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE, 0u) != LIB_STATUS_OK ||
            observed != (operation == LEGACY_ALU_CMP ? left : expected);
    else if (operation != LEGACY_ALU_CMP)
        failed |= (after.data.eax & mask) != expected;
    else
        failed |= (after.data.eax & mask) != left;

    return !failed;
}

static lib_i32 legacy_alu_test_group1_forms(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086, X86_CORE_CPU_PROFILE_80186,
        X86_CORE_CPU_PROFILE_80286, X86_CORE_CPU_PROFILE_80386,
        X86_CORE_CPU_PROFILE_80486SX
    };
    lib_u8 profile_index;
    lib_u8 operation;
    lib_u8 width_index;
    lib_u8 sign_extended;
    lib_u8 memory;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (operation = 0u; operation != 8u; ++operation)
    for (width_index = 0u; width_index != 3u; ++width_index)
    for (sign_extended = 0u; sign_extended != (width_index == 0u ? 1u : 2u);
        ++sign_extended)
    for (memory = 0u; memory != 2u; ++memory)
    for (lib_u8 carry = 0u; carry < 2u; ++carry)
    for (lib_u8 alias = 0u; alias < (width_index == 0u &&
         profiles[profile_index] == X86_CORE_CPU_PROFILE_80486SX ? 2u : 1u); ++alias)
        if ((width_index < 2u || profiles[profile_index] >= X86_CORE_CPU_PROFILE_80386) &&
            !legacy_alu_group1_case(profiles[profile_index],
                (legacy_alu_operation)operation, width_index == 0u ? 8u :
                    (width_index == 1u ? 16u : 32u),
                sign_extended, memory, alias != 0u, carry))
            return 0;
    return 1;
}

static lib_u32 legacy_alu_jcc_flags(lib_u8 condition,
    lib_i32 taken)
{
    lib_u32 flags = 0u;

    switch (condition) {
    case 0u: flags = taken ? VCPU_EFLAGS_OF : 0u; break;
    case 1u: flags = taken ? 0u : VCPU_EFLAGS_OF; break;
    case 2u: flags = taken ? VCPU_EFLAGS_CF : 0u; break;
    case 3u: flags = taken ? 0u : VCPU_EFLAGS_CF; break;
    case 4u: flags = taken ? VCPU_EFLAGS_ZF : 0u; break;
    case 5u: flags = taken ? 0u : VCPU_EFLAGS_ZF; break;
    case 6u: flags = taken ? VCPU_EFLAGS_CF : 0u; break;
    case 7u: flags = taken ? 0u : VCPU_EFLAGS_CF; break;
    case 8u: flags = taken ? VCPU_EFLAGS_SF : 0u; break;
    case 9u: flags = taken ? 0u : VCPU_EFLAGS_SF; break;
    case 10u: flags = taken ? VCPU_EFLAGS_PF : 0u; break;
    case 11u: flags = taken ? 0u : VCPU_EFLAGS_PF; break;
    case 12u: flags = taken ? VCPU_EFLAGS_SF : 0u; break;
    case 13u: flags = taken ? 0u : VCPU_EFLAGS_SF; break;
    case 14u: flags = taken ? VCPU_EFLAGS_ZF : 0u; break;
    case 15u: flags = taken ? 0u : VCPU_EFLAGS_ZF; break;
    default: break;
    }
    return flags | VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
}

static lib_i32 legacy_alu_test_condition_forms(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086, X86_CORE_CPU_PROFILE_80186
    };
    lib_u8 profile_index;
    lib_u8 condition;
    lib_i32 taken;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (condition = 0u; condition != 16u; ++condition)
    for (taken = 0; taken != 2; ++taken) {
        const lib_u8 code[] = { (lib_u8)(0x70u + condition),
            0x02u, 0x90u, 0x90u };
        legacy_alu_machine state;
        t_cpu before;
        t_cpu after;

        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, profiles[profile_index]);

        state.cpu.data.eax = 0x11223344u;
        state.cpu.data.ecx = 0x55667788u;
        state.cpu.data.eflags = legacy_alu_jcc_flags(condition,
            taken);
        before = state.cpu;
        failed |= !legacy_alu_run(&state, code, sizeof(code), 0, &after) || state.fault.valid ||
            after.data.eip != (taken ? 4u : 2u) ||
            after.data.eax != before.data.eax ||
            after.data.ecx != before.data.ecx ||
            after.data.eflags != before.data.eflags;

        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 legacy_alu_test_loop_forms(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086, X86_CORE_CPU_PROFILE_80186
    };
    lib_u8 profile_index;
    lib_u8 operation;
    lib_i32 taken;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (operation = 0u; operation != 4u; ++operation)
    for (taken = 0; taken != 2; ++taken) {
        const lib_u8 code[] = { (lib_u8)(0xe0u + operation),
            0x02u, 0x90u, 0x90u };
        const lib_u32 flags = (operation == 0u && !taken) ||
            (operation == 1u && taken) ? VCPU_EFLAGS_ZF : 0u;
        const lib_u32 ecx = operation == 3u ? (taken ? 0u : 1u) :
            (taken ? 2u : 1u);
        const lib_u32 expected_ecx = operation == 3u ? ecx : ecx - 1u;
        legacy_alu_machine state;
        t_cpu before;
        t_cpu after;

        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, profiles[profile_index]);

        state.cpu.data.eax = 0x11223344u;
        state.cpu.data.ecx = 0xaabb0000u | ecx;
        state.cpu.data.eflags = flags | VCPU_EFLAGS_IF |
            VCPU_EFLAGS_DF;
        before = state.cpu;
        failed |= !legacy_alu_run(&state, code, sizeof(code), 0, &after) || state.fault.valid ||
            after.data.eip != (taken ? 4u : 2u) || after.data.eax !=
            before.data.eax || after.data.ecx != (0xaabb0000u | expected_ecx) ||
            after.data.eflags != before.data.eflags;

        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 legacy_alu_test_test_forms(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086, X86_CORE_CPU_PROFILE_80186
    };
    static const lib_u8 forms[][4] = {
        { 0x84u, 0xc8u }, { 0x85u, 0xc8u }, { 0xa8u, 0x0fu },
        { 0xa9u, 0x0fu, 0x00u }, { 0xf6u, 0xc0u, 0x0fu },
        { 0xf7u, 0xc0u, 0x0fu, 0x00u }
    };
    static const lib_u8 lengths[] = { 2u, 2u, 2u, 3u, 3u, 4u };
    lib_u8 profile_index;
    lib_u8 form;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (form = 0u; form != sizeof(lengths); ++form) {
        const lib_u8 width = (form == 0u || form == 2u || form == 4u) ?
            8u : 16u;
        const lib_u32 result = width == 8u ? 0x08u : 0x0008u;
        const lib_u32 expected_flags = (legacy_alu_parity(result) ?
            VCPU_EFLAGS_PF : 0u) | VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
        legacy_alu_machine state;
        t_cpu before;
        t_cpu after;

        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, profiles[profile_index]);

        state.cpu.data.eax = 0xaabbcc08u;
        state.cpu.data.ecx = 0x1122330fu;
        state.cpu.data.eflags = VCPU_EFLAGS_CF |
            VCPU_EFLAGS_OF | VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
        before = state.cpu;
        failed |= !legacy_alu_run(&state, forms[form], lengths[form], 0,
            &after) || state.fault.valid ||
            after.data.eip != lengths[form] || after.data.eax != before.data.eax ||
            after.data.ecx != before.data.ecx ||
            (after.data.eflags & (VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
            VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF |
            VCPU_EFLAGS_IF | VCPU_EFLAGS_DF)) != expected_flags;

        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 legacy_alu_test_adjust_and_xlat_forms(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086, X86_CORE_CPU_PROFILE_80186,
        X86_CORE_CPU_PROFILE_80286, X86_CORE_CPU_PROFILE_80386,
        X86_CORE_CPU_PROFILE_80486SX
    };
    static const lib_u8 codes[][2] = {
        { 0x27u }, { 0x2fu }, { 0x37u }, { 0x3fu }, { 0xd4u, 10u },
        { 0xd5u, 10u }, { 0xd7u }
    };
    static const lib_u8 lengths[] = { 1u, 1u, 1u, 1u, 2u, 2u, 1u };
    lib_u8 profile_index;
    lib_u8 form;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (form = 0u; form != sizeof(lengths); ++form) {
        const lib_u32 initial_eax = form == 0u ? 0x1122009au :
            (form == 1u ? 0x11220000u : (form == 2u ? 0x1122000au :
            (form == 3u ? 0x1122010au : (form == 4u ? 0x1122002au :
            (form == 5u ? 0x11220402u : 0x11220002u)))));
        const lib_u32 expected_eax = form == 0u ? 0x11220000u :
            (form == 1u ? (profiles[profile_index] == X86_CORE_CPU_PROFILE_80486SX ?
                0x112200fau : 0x1122009au) : (form == 2u ? 0x11220100u :
            (form == 3u ? 0x11220004u : (form == 4u ? 0x11220402u :
            (form == 5u ? 0x1122002au : 0x112200a5u)))));
        legacy_alu_machine state;
        t_cpu before;
        t_cpu after;

        lib_u8 table_value = 0xa5u;
        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, profiles[profile_index]);

        state.cpu.data.eax = initial_eax;
        state.cpu.data.ebx = LEGACY_ALU_MEMORY;
        state.cpu.data.eflags = VCPU_EFLAGS_AF |
            VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
        if (form == 6u)
            failed |= cpu_instruction_write(&state,
                LEGACY_ALU_MEMORY + 2u, &table_value,
                sizeof(table_value), X86_CORE_CPU_MEMORY_ACCESS_DATA, 0u) !=
                LIB_STATUS_OK;
        before = state.cpu;
        failed |= !legacy_alu_run(&state, codes[form], lengths[form], 0,
            &after) || state.fault.valid ||
            after.data.eip != lengths[form] || after.data.eax != expected_eax ||
            after.data.ebx != before.data.ebx ||
            (after.data.eflags & (VCPU_EFLAGS_IF | VCPU_EFLAGS_DF)) !=
            (before.data.eflags & (VCPU_EFLAGS_IF | VCPU_EFLAGS_DF));
        if (form == 0u)
            failed |= (after.data.eflags & (VCPU_EFLAGS_CF | VCPU_EFLAGS_AF |
                VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF | VCPU_EFLAGS_SF)) !=
                (VCPU_EFLAGS_CF | VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF |
                VCPU_EFLAGS_PF);
        if (form == 1u)
            failed |= (after.data.eflags & (VCPU_EFLAGS_CF | VCPU_EFLAGS_AF |
                VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF | VCPU_EFLAGS_SF)) !=
                (VCPU_EFLAGS_CF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF |
                VCPU_EFLAGS_SF);

        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 legacy_alu_test_decimal_arithmetic(void)
{
    const lib_u32 kept_flags = VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
    const lib_u32 defined_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_AF |
        VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF | kept_flags;
    lib_u8 subtract;

    for (subtract = 0u; subtract != 2u; ++subtract) {
        legacy_alu_machine state;
        lib_u32 offset;
        lib_u16 left;
        lib_u16 right;
        lib_u8 carry;

        cpu_instruction_prepare(&state, X86_CORE_CPU_PROFILE_80486SX);
        /* One immutable guest program avoids rebuilding a large fixture for
         * every pair. Each case still executes the real arithmetic and adjust. */
        for (offset = 0u; offset != 60000u; offset += 3u) {
            state.memory[offset] = subtract ? 0x18u : 0x10u;
            state.memory[offset + 1u] = 0xc8u;
            state.memory[offset + 2u] = subtract ? 0x2fu : 0x27u;
        }
        for (left = 0u; left != 100u; ++left)
        for (right = 0u; right != 100u; ++right)
        for (carry = 0u; carry != 2u; ++carry) {
            const lib_i32 decimal = subtract ?
                (lib_i32)left - right - carry : (lib_i32)left + right + carry;
            const lib_u16 normalized = (lib_u16)((decimal + 100) % 100);
            const lib_u8 packed = (lib_u8)(((normalized / 10u) << 4u) |
                (normalized % 10u));
            const lib_bool decimal_carry = subtract ? decimal < 0 : decimal >= 100;
            const lib_bool digit_carry = subtract ?
                left % 10u < right % 10u + carry :
                left % 10u + right % 10u + carry >= 10u;
            const lib_u32 expected_flags = kept_flags |
                (decimal_carry ? VCPU_EFLAGS_CF : 0u) |
                (digit_carry ? VCPU_EFLAGS_AF : 0u) |
                (packed == 0u ? VCPU_EFLAGS_ZF : 0u) |
                ((packed & 0x80u) ? VCPU_EFLAGS_SF : 0u) |
                (legacy_alu_parity(packed) ? VCPU_EFLAGS_PF : 0u);
            const lib_u32 pc = state.cpu.data.eip;

            state.cpu.data.eax = 0x11220000u |
                ((lib_u32)(left / 10u) << 4u) | (left % 10u);
            state.cpu.data.ecx = 0x55660000u |
                ((lib_u32)(right / 10u) << 4u) | (right % 10u);
            state.cpu.data.eflags = kept_flags | (carry ? VCPU_EFLAGS_CF : 0u);
            x86_core_cpu_execution_refresh(&state.execution);
            x86_core_cpu_execution_refresh(&state.execution);
            if (state.execution.stop_requested || state.fault.valid ||
                state.delivered_exception.valid || state.cpu.data.eip != pc + 3u ||
                state.cpu.data.eax != (0x11220000u | packed) ||
                (state.cpu.data.eflags & defined_flags) != expected_flags ||
                state.cpu.data.ecx != (0x55660000u |
                    ((lib_u32)(right / 10u) << 4u) | (right % 10u))) {
                lib_c_fprintf(lib_c_stderr,
                    "DECIMAL: subtract=%u left=%u right=%u carry=%u eax=%08X flags=%08X\n",
                    (lib_u32)subtract, (lib_u32)left, (lib_u32)right, (lib_u32)carry,
                    state.cpu.data.eax, state.cpu.data.eflags);
                return 0;
            }
        }
    }
    return 1;
}

static lib_bool legacy_alu_test_ascii_arithmetic(void)
{
    static const lib_u8 high_bytes[] = {0u, 1u, 0x7fu, 0xffu};
    const lib_u32 kept = VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
    const lib_u32 checked = kept | VCPU_EFLAGS_CF | VCPU_EFLAGS_AF;

    for (x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_80386;
         profile <= X86_CORE_CPU_PROFILE_80486SX; ++profile)
    for (lib_u8 subtract = 0u; subtract < 2u; ++subtract) {
        legacy_alu_machine state;

        cpu_instruction_prepare(&state, profile);
        for (lib_u32 offset = 0u; offset < 1200u; offset += 3u) {
            state.memory[offset] = subtract ? 0x28u : 0x00u;
            state.memory[offset + 1u] = 0xc8u;
            state.memory[offset + 2u] = subtract ? 0x3fu : 0x37u;
        }
        for (lib_u8 left = 0u; left < 10u; ++left)
        for (lib_u8 right = 0u; right < 10u; ++right)
        for (lib_size high = 0u; high < sizeof(high_bytes); ++high) {
            const lib_i32 decimal = subtract ? (lib_i32)left - right : left + right;
            const lib_bool carry = subtract ? decimal < 0 : decimal >= 10;
            const lib_u8 digit = (lib_u8)((decimal + 10) % 10);
            const lib_u8 upper = (lib_u8)(high_bytes[high] +
                (carry ? (subtract ? -1 : 1) : 0));
            const lib_u32 expected = 0x11220000u | ((lib_u32)upper << 8u) | digit;
            const lib_u32 flags = kept | (carry ? VCPU_EFLAGS_CF | VCPU_EFLAGS_AF : 0u);
            const lib_u32 pc = state.cpu.data.eip;

            state.cpu.data.eax = 0x11220000u | ((lib_u32)high_bytes[high] << 8u) | left;
            state.cpu.data.ecx = 0x55660000u | right;
            state.cpu.data.eflags = kept;
            x86_core_cpu_execution_refresh(&state.execution);
            x86_core_cpu_execution_refresh(&state.execution);
            if (state.execution.stop_requested || state.fault.valid ||
                state.delivered_exception.valid || state.cpu.data.eip != pc + 3u ||
                state.cpu.data.eax != expected ||
                state.cpu.data.ecx != (0x55660000u | right) ||
                (state.cpu.data.eflags & checked) != flags) {
                lib_c_printf("ASCII arithmetic: profile=%u subtract=%u left=%u right=%u high=%u\n",
                    (lib_u32)profile, (lib_u32)subtract, (lib_u32)left,
                    (lib_u32)right, (lib_u32)high_bytes[high]);
                return LIB_FALSE;
            }
        }
    }
    return LIB_TRUE;
}

static lib_bool legacy_alu_test_adjust_inputs(void)
{
    static const lib_u8 opcodes[] = {0x37u, 0x3fu, 0x27u, 0x2fu};
    static const lib_u8 lows[] = {0u, 5u, 6u, 9u, 10u, 0x99u,
        0x9au, 0x9fu, 0xa0u, 0xf9u, 0xfau, 0xffu};
    static const lib_u8 highs[] = {0u, 1u, 0x7fu, 0xffu};
    const lib_u32 kept = VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;

    for (x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_80386;
         profile <= X86_CORE_CPU_PROFILE_80486SX; ++profile)
    for (lib_u8 operation = 0u; operation < sizeof(opcodes); ++operation)
    for (lib_u8 mode = 0u; mode < 6u; ++mode)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 controls = 0u; controls < 4u; ++controls)
    for (lib_u8 prefixes = 0u; prefixes < (mode == 0u && !code32 ? 1u : 4u); ++prefixes) {
        cpu_instruction_fixture fixture;
        const lib_bool packed = operation >= 2u;
        const lib_bool subtract = (operation & 1u) != 0u;
        const lib_bool exhaustive = mode == 0u && !code32;
        const lib_u32 images = exhaustive ? (packed ? 256u : 65536u) :
            sizeof(lows) * sizeof(highs);
        const lib_u32 checked = kept | VCPU_EFLAGS_CF | VCPU_EFLAGS_AF | VCPU_EFLAGS_VM |
            (packed ? VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF : 0u);
        lib_u8 code[3], bytes = 0u;

        if (mode == 5u && code32) continue;
        if (packed && profile != X86_CORE_CPU_PROFILE_80486SX) continue;
        cpu_instruction_prepare(&fixture, profile);
        if (mode) cpu_instruction_prepare_protected_exception(&fixture, mode - 1u);
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        fixture.cpu.data.ecx = 0x55667788u;
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = opcodes[operation];
        for (lib_u32 slot = 0u; slot < 256u; ++slot)
            lib_memory_copy(fixture.memory + 0x1000u + slot * bytes, code, bytes);
        for (lib_u32 image = 0u; image < images; ++image) {
            const lib_u8 low = exhaustive ? (lib_u8)image : lows[image % sizeof(lows)];
            const lib_u8 high = exhaustive ? (packed ? 0x5au : (lib_u8)(image >> 8u)) :
                highs[image / sizeof(lows)];
            const lib_bool adjust = (low & 0x0fu) > 9u || (controls & 1u);
            const lib_u8 delta = adjust ? 1u +
                (profile == X86_CORE_CPU_PROFILE_80486SX &&
                    (subtract ? low < 6u : low > 0xf9u)) : 0u;
            lib_u8 result_low = (lib_u8)(adjust ?
                (subtract ? low - 6 : low + 6) : low) & 0x0fu;
            lib_u8 result_high = (lib_u8)(high + (subtract ? -(lib_i32)delta : delta));
            lib_u32 flags = kept | (mode == 5u ? VCPU_EFLAGS_VM : 0u) |
                (adjust ? VCPU_EFLAGS_CF | VCPU_EFLAGS_AF : 0u);
            const lib_u32 pc = 0x1000u + (image & 0xffu) * bytes;

            if (packed) {
                const lib_bool high_adjust = low > 0x99u || (controls & 2u);
                const lib_i32 correction = (adjust ? 6 : 0) + (high_adjust ? 0x60 : 0);
                const lib_bool carry = high_adjust || (subtract && adjust && low < 6u);

                result_low = (lib_u8)(low + (subtract ? -correction : correction));
                result_high = high;
                flags = kept | (mode == 5u ? VCPU_EFLAGS_VM : 0u) |
                    (adjust ? VCPU_EFLAGS_AF : 0u) | (carry ? VCPU_EFLAGS_CF : 0u) |
                    (result_low == 0u ? VCPU_EFLAGS_ZF : 0u) |
                    ((result_low & 0x80u) ? VCPU_EFLAGS_SF : 0u) |
                    (legacy_alu_parity(result_low) ? VCPU_EFLAGS_PF : 0u);
            }
            const lib_u32 expected = 0x11220000u | ((lib_u32)result_high << 8u) | result_low;
            if ((image & 0xffu) == 0u) {
                fixture.cpu.data.eip = pc;
                x86_core_cpu_execution_invalidate_prefetch(&fixture.execution);
            }
            fixture.cpu.data.eax = 0x11220000u | ((lib_u32)high << 8u) | low;
            fixture.cpu.data.eflags = kept | (mode == 5u ? VCPU_EFLAGS_VM : 0u) |
                ((controls & 1u) ? VCPU_EFLAGS_AF : 0u) |
                ((controls & 2u) ? VCPU_EFLAGS_CF : 0u);
            x86_core_cpu_execution_refresh(&fixture.execution);
            if (fixture.execution.stop_requested || fixture.fault.valid ||
                fixture.delivered_exception.valid || fixture.cpu.data.eip != pc + bytes ||
                fixture.cpu.data.eax != expected || fixture.cpu.data.ecx != 0x55667788u ||
                (fixture.cpu.data.eflags & checked) != flags || fixture.writes != 0u) {
                lib_c_printf("BCD adjust input: profile=%u opcode=%02x mode=%u code32=%u prefix=%u controls=%u ax=%04x eax=%08x expected=%08x\n",
                    (lib_u32)profile, (lib_u32)opcodes[operation], (lib_u32)mode, (lib_u32)code32,
                    (lib_u32)prefixes, (lib_u32)controls, ((lib_u32)high << 8u) | low,
                    fixture.cpu.data.eax, expected);
                return LIB_FALSE;
            }
        }
    }
    return LIB_TRUE;
}

static lib_bool legacy_alu_test_aam_aad_results(void)
{
    const lib_u32 kept = VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
    const lib_u32 checked = kept | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF;

    for (x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_80386;
         profile <= X86_CORE_CPU_PROFILE_80486SX; ++profile)
    for (lib_u8 multiply = 0u; multiply < 2u; ++multiply)
    for (lib_u16 radix = 0u; radix < 256u; ++radix) {
        legacy_alu_machine state;

        if (multiply && radix == 0u) continue;
        cpu_instruction_prepare(&state, profile);
        for (lib_u32 offset = 0u; offset < 512u; offset += 2u) {
            state.memory[offset] = multiply ? 0xd4u : 0xd5u;
            state.memory[offset + 1u] = (lib_u8)radix;
        }
        for (lib_u16 high = 0u; high < (multiply ? 2u : radix == 10u ? 256u : 3u); ++high) {
            state.cpu.data.eip = 0u;
            x86_core_cpu_execution_invalidate_prefetch(&state.execution);
            for (lib_u16 low = 0u; low < 256u; ++low) {
                const lib_u8 upper = multiply ? (high ? 0xffu : 0u) :
                    (radix != 10u && high == 2u ? 0xffu : (lib_u8)high);
                const lib_u16 result = multiply ?
                    (lib_u16)(((low / radix) << 8u) | (low % radix)) :
                    (lib_u8)(upper * radix + low);
                const lib_u8 byte = (lib_u8)result;
                const lib_u32 flags = kept | (byte == 0u ? VCPU_EFLAGS_ZF : 0u) |
                    ((byte & 0x80u) ? VCPU_EFLAGS_SF : 0u) |
                    (legacy_alu_parity(byte) ? VCPU_EFLAGS_PF : 0u);
                const lib_u32 pc = state.cpu.data.eip;

                state.cpu.data.eax = 0x11220000u | ((lib_u32)upper << 8u) | low;
                state.cpu.data.ecx = 0x55667788u;
                state.cpu.data.eflags = kept | LEGACY_ALU_FLAGS;
                x86_core_cpu_execution_refresh(&state.execution);
                if (state.execution.stop_requested || state.fault.valid ||
                    state.delivered_exception.valid || state.cpu.data.eip != pc + 2u ||
                    state.cpu.data.eax != (0x11220000u | result) ||
                    state.cpu.data.ecx != 0x55667788u ||
                    (state.cpu.data.eflags & checked) != flags) {
                    lib_c_printf("AAM/AAD: profile=%u multiply=%u radix=%u high=%u low=%u\n",
                        (lib_u32)profile, (lib_u32)multiply, (lib_u32)radix,
                        (lib_u32)upper, (lib_u32)low);
                    return LIB_FALSE;
                }
            }
        }
    }
    for (x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_80386;
         profile <= X86_CORE_CPU_PROFILE_80486SX; ++profile)
    for (lib_u8 mode = 0u; mode < 6u; ++mode)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes)
    for (lib_u8 locked = 0u; locked < 2u; ++locked) {
        cpu_instruction_fixture fixture;
        t_cpu before;
        lib_u8 code[5], bytes = 0u;
        const lib_u8 vector = locked ? 6u : 0u;
        const lib_u32 stack = mode == 5u ? 0x1000u - 36u :
            0x800u - (mode ? 12u : 6u);
        const lib_u32 writes = (mode == 5u ? 9u : 3u) +
            (profile == X86_CORE_CPU_PROFILE_80386 && mode ?
                (mode == 5u ? 2u : 1u) : 0u);

        if (mode == 5u && code32) continue;
        cpu_instruction_prepare(&fixture, profile);
        if (mode) {
            cpu_instruction_prepare_protected_exception(&fixture, mode - 1u);
            lib_memory_copy(fixture.memory + 0x400u + vector * 8u,
                fixture.memory + 0x468u, 8u);
        } else {
            const lib_u16 handler[] = {0x300u, 0u};

            lib_memory_copy(fixture.memory + vector * 4u, handler, sizeof(handler));
            fixture.cpu.data.esp = 0x800u;
        }
        fixture.cpu.data.cs.seg.exec.defsize = code32;
        fixture.cpu.data.eip = 0x1000u;
        fixture.cpu.data.eax = 0x11223456u;
        before = fixture.cpu;
        if (locked) code[bytes++] = 0xf0u;
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = 0xd4u;
        code[bytes++] = 0u;
        lib_memory_copy(fixture.memory + before.data.eip, code, bytes);
        x86_core_cpu_execution_refresh(&fixture.execution);
        if (fixture.execution.stop_requested || fixture.fault.valid ||
            !fixture.delivered_exception.valid ||
            fixture.delivered_exception.exception_mask !=
                (locked ? VCPUINS_EXCEPT_UD : VCPUINS_EXCEPT_DE) ||
            fixture.delivered_exception.exception_code != 0u ||
            fixture.delivered_exception.point.eip != before.data.eip ||
            fixture.cpu.data.eip != 0x300u || fixture.cpu.data.esp != stack ||
            fixture.cpu.data.eax != before.data.eax ||
            fixture.writes != writes) {
            lib_c_printf("AAM zero: profile=%u mode=%u code32=%u prefix=%u locked=%u event=%x/%x point=%x ip=%x sp=%x writes=%u\n",
                (lib_u32)profile, (lib_u32)mode, (lib_u32)code32, (lib_u32)prefixes, (lib_u32)locked,
                fixture.delivered_exception.exception_mask,
                fixture.delivered_exception.exception_code, fixture.delivered_exception.point.eip,
                fixture.cpu.data.eip, fixture.cpu.data.esp, fixture.writes);
            return LIB_FALSE;
        }
        lib_u32 saved_ip = 0u;
        lib_memory_copy(&saved_ip, fixture.memory + stack, mode ? 4u : 2u);
        if (saved_ip != before.data.eip) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool legacy_alu_test_salc(void)
{
    static const lib_u32 values[] = {0u, 0xffffffffu, 0x12345678u, 0x80000080u};
    static const lib_u32 arithmetic_flags[] = {VCPU_EFLAGS_CF, VCPU_EFLAGS_PF,
        VCPU_EFLAGS_AF, VCPU_EFLAGS_ZF, VCPU_EFLAGS_SF, VCPU_EFLAGS_OF};

    for (lib_u8 mode = 0u; mode < 6u; ++mode)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes)
    for (lib_u8 repeat = 0u; repeat < 3u; ++repeat) {
        legacy_alu_machine state;
        x86_core_cpu_instruction_lexeme lexeme;
        lib_u8 code[4], bytes = 0u;

        if (mode == 5u && code32) continue;
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        if (repeat) code[bytes++] = repeat == 1u ? 0xf2u : 0xf3u;
        code[bytes++] = 0xd6u;
        if (!x86_core_cpu_instruction_lexeme_scan(code, bytes,
                X86_CORE_CPU_PROFILE_80486SX, code32, &lexeme) ||
            !lexeme.available || lexeme.byte_count != bytes) {
            lib_c_printf("SALC admission: mode=%u code32=%u prefixes=%u repeat=%u\n",
                (lib_u32)mode, (lib_u32)code32, (lib_u32)prefixes, (lib_u32)repeat);
            return LIB_FALSE;
        }
        cpu_instruction_prepare(&state, X86_CORE_CPU_PROFILE_80486SX);
        state.cpu.data.cs.seg.exec.defsize = code32;
        if (mode) {
            const lib_u8 cpl = mode == 5u ? 3u : mode - 1u;

            state.cpu.data.cr0 |= VCPU_CR0_PE;
            state.cpu.data.cs.selector = (lib_u16)(8u | cpl);
            state.cpu.data.cs.dpl = cpl;
        }
        for (lib_u32 offset = 0u; offset < bytes * 256u; offset += bytes)
            lib_memory_copy(state.memory + offset, code, bytes);
        for (lib_u8 flags_index = 0u; flags_index < 64u; ++flags_index)
        for (lib_size value = 0u; value < sizeof(values) / sizeof(values[0]); ++value) {
            lib_u32 flags = 2u | VCPU_EFLAGS_IF | VCPU_EFLAGS_DF |
                (mode == 5u ? VCPU_EFLAGS_VM : 0u);
            const lib_u32 expected = (values[value] & 0xffffff00u) |
                ((flags_index & 1u) ? 0xffu : 0u);
            const lib_u32 pc = state.cpu.data.eip;

            for (lib_u8 bit = 0u; bit < 6u; ++bit)
                if (flags_index & (1u << bit)) flags |= arithmetic_flags[bit];
            state.cpu.data.eax = values[value];
            state.cpu.data.ecx = 0x55667788u;
            state.cpu.data.eflags = flags;
            x86_core_cpu_execution_refresh(&state.execution);
            if (state.execution.stop_requested || state.fault.valid ||
                state.delivered_exception.valid || state.cpu.data.eip != pc + bytes ||
                state.cpu.data.eax != expected || state.cpu.data.eflags != flags ||
                state.cpu.data.ecx != 0x55667788u || state.writes != 0u) {
                lib_c_printf("SALC result: mode=%u code32=%u prefixes=%u repeat=%u flags=%u value=%u\n",
                    (lib_u32)mode, (lib_u32)code32, (lib_u32)prefixes,
                    (lib_u32)repeat, (lib_u32)flags_index, (lib_u32)value);
                return LIB_FALSE;
            }
        }
    }
    for (x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_8086;
         profile < X86_CORE_CPU_PROFILE_80486SX; ++profile) {
        legacy_alu_machine state;
        x86_core_cpu_instruction_lexeme lexeme;
        const lib_u8 code[] = {0xd6u};

        cpu_instruction_prepare(&state, profile);
        if (x86_core_cpu_instruction_lexeme_scan(code, sizeof(code), profile,
                LIB_FALSE, &lexeme) ||
            !cpu_instruction_expect_real_fault(&state, code, sizeof(code), 6u))
            return LIB_FALSE;
    }
    {
        legacy_alu_machine state;
        const lib_u8 code[] = {0xf0u, 0xd6u};

        cpu_instruction_prepare(&state, X86_CORE_CPU_PROFILE_80486SX);
        if (!cpu_instruction_expect_real_fault(&state, code, sizeof(code), 6u))
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool legacy_alu_test_sx_test_aliases(void)
{
    static const lib_u32 values[] = {0u, 1u, 0x80018081u, 0xffffffffu};

    for (lib_u8 mode = 0u; mode < 6u; ++mode)
    for (lib_u8 code32 = 0u; code32 < 2u; ++code32)
    for (lib_u8 prefixes = 0u; prefixes < 4u; ++prefixes)
    for (lib_u8 memory = 0u; memory < 2u; ++memory)
    for (lib_u8 wide = 0u; wide < 2u; ++wide)
    for (lib_size left = 0u; left < sizeof(values) / sizeof(values[0]); ++left)
    for (lib_size right = 0u; right < sizeof(values) / sizeof(values[0]); ++right) {
        legacy_alu_machine state;
        t_cpu canonical;
        lib_u8 code[12], bytes = 0u, modrm_index;
        const lib_bool address32 = code32 != ((prefixes & 2u) != 0u);
        const lib_u8 width = wide ?
            ((code32 != ((prefixes & 1u) != 0u)) ? 32u : 16u) : 8u;
        const lib_u32 result = values[left] & values[right] & legacy_alu_mask(width);
        const lib_u32 checked = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_SF |
            VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF | VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
        const lib_u32 flags = VCPU_EFLAGS_IF | VCPU_EFLAGS_DF |
            (result == 0u ? VCPU_EFLAGS_ZF : 0u) |
            ((result & (1u << (width - 1u))) ? VCPU_EFLAGS_SF : 0u) |
            (legacy_alu_parity(result) ? VCPU_EFLAGS_PF : 0u);

        if (mode == 5u && code32) continue;
        if (prefixes & 1u) code[bytes++] = 0x66u;
        if (prefixes & 2u) code[bytes++] = 0x67u;
        code[bytes++] = wide ? 0xf7u : 0xf6u;
        modrm_index = bytes;
        code[bytes++] = memory ? (address32 ? 5u : 6u) : 0xc0u;
        if (memory) {
            code[bytes++] = 0u;
            code[bytes++] = 0x40u;
            if (address32) { code[bytes++] = 0u; code[bytes++] = 0u; }
        }
        for (lib_u8 byte = 0u; byte < width / 8u; ++byte)
            code[bytes++] = (lib_u8)(values[right] >> (byte * 8u));
        for (lib_u8 alias = 0u; alias < 2u; ++alias) {
            x86_core_cpu_instruction_lexeme lexeme;
            t_cpu after;
            lib_u32 observed = 0u;

            code[modrm_index] = (lib_u8)((code[modrm_index] & 0xc7u) | (alias ? 8u : 0u));
            cpu_instruction_prepare(&state, X86_CORE_CPU_PROFILE_80486SX);
            if (mode) {
                const lib_u8 cpl = mode == 5u ? 3u : mode - 1u;

                cpu_instruction_prepare_protected_exception(&state, mode == 5u ? 4u : cpl);
                state.cpu.data.ds.selector = (lib_u16)(0x10u | cpl);
                state.cpu.data.ds.dpl = cpl;
            }
            state.cpu.data.cs.seg.exec.defsize = code32;
            state.cpu.data.eax = values[left];
            state.cpu.data.eflags |= LEGACY_ALU_FLAGS | VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
            if (memory && cpu_instruction_write(&state, 0x4000u, &values[left], width / 8u,
                    X86_CORE_CPU_MEMORY_ACCESS_DATA, 0u) != LIB_STATUS_OK) return LIB_FALSE;
            if (!x86_core_cpu_instruction_lexeme_scan(code, bytes,
                    X86_CORE_CPU_PROFILE_80486SX, code32, &lexeme) ||
                lexeme.byte_count != bytes ||
                cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK ||
                state.fault.valid || state.delivered_exception.valid) {
                lib_c_printf("TEST alias admission: mode=%u code32=%u prefix=%u memory=%u width=%u alias=%u\n",
                    (lib_u32)mode, (lib_u32)code32, (lib_u32)prefixes,
                    (lib_u32)memory, (lib_u32)width, (lib_u32)alias);
                return LIB_FALSE;
            }
            if (memory && (cpu_instruction_read(&state, 0x4000u, &observed, width / 8u,
                    X86_CORE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE, 0u) != LIB_STATUS_OK ||
                observed != (values[left] & legacy_alu_mask(width)))) return LIB_FALSE;
            if (state.writes != (memory ? 1u : 0u) || after.data.eax != values[left] ||
                after.data.eip != bytes || (after.data.eflags & checked) != flags) return LIB_FALSE;
            if (!alias) canonical = after;
            else if (lib_memory_compare(&canonical, &after, sizeof(after)) != 0) return LIB_FALSE;
        }
    }
    for (x86_core_cpu_profile profile = X86_CORE_CPU_PROFILE_8086;
         profile <= X86_CORE_CPU_PROFILE_80486SX; ++profile)
    for (lib_u8 wide = 0u; wide < 2u; ++wide) {
        legacy_alu_machine state;
        x86_core_cpu_instruction_lexeme lexeme;
        const lib_u8 code[] = {0xf0u, wide ? 0xf7u : 0xf6u, 0xc8u, 1u, 0u};
        const lib_u8 start = profile == X86_CORE_CPU_PROFILE_80486SX ? 0u : 1u;
        const lib_u8 bytes = (lib_u8)(sizeof(code) - start - (wide ? 0u : 1u));

        cpu_instruction_prepare(&state, profile);
        if (x86_core_cpu_instruction_lexeme_scan(code + start, bytes, profile,
                LIB_FALSE, &lexeme) || lexeme.available ||
            !cpu_instruction_expect_real_fault(&state, code + start, bytes, 6u))
            return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_i32 legacy_alu_test_group3_forms(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086, X86_CORE_CPU_PROFILE_80186,
        X86_CORE_CPU_PROFILE_80286, X86_CORE_CPU_PROFILE_80386,
        X86_CORE_CPU_PROFILE_80486SX
    };
    lib_u8 profile_index;
    lib_u8 width_index;
    lib_u8 extension;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (width_index = 0u; width_index != 2u; ++width_index)
    for (extension = 2u; extension != 8u; ++extension) {
        const lib_u8 width = width_index == 0u ? 8u : 16u;
        const lib_u8 code[] = { width == 8u ? 0xf6u : 0xf7u,
            (lib_u8)(0xc1u | (extension << 3u)) };
        legacy_alu_machine state;
        t_cpu before;
        t_cpu after;

        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, profiles[profile_index]);

        state.cpu.data.eax = extension == 7u ?
            0x1122fff7u : (extension == 6u ? 0x11220009u : 0x11220003u);
        state.cpu.data.ecx = extension == 7u ?
            (width == 8u ? 0x5566fffdu : 0x5566fffdu) : 0x55660003u;
        state.cpu.data.edx = extension >= 6u ?
            (extension == 7u ? 0x7788ffffu : 0x77880000u) : 0x77880000u;
        state.cpu.data.eflags = VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
        before = state.cpu;
        failed |= !legacy_alu_run(&state, code, sizeof(code), 0, &after) || state.fault.valid || after.data.eip !=
            sizeof(code) || (after.data.eflags & (VCPU_EFLAGS_IF | VCPU_EFLAGS_DF))
            != before.data.eflags;
        if (extension == 2u)
            failed |= (after.data.ecx & (width == 8u ? 0xffu : 0xffffu)) !=
                (width == 8u ? 0xfcu : 0xfffcu);
        else if (extension == 3u)
            failed |= (after.data.ecx & (width == 8u ? 0xffu : 0xffffu)) !=
                (width == 8u ? 0xfdu : 0xfffdu) ||
                (after.data.eflags & VCPU_EFLAGS_CF) == 0u;
        else if (extension == 4u || extension == 5u)
            failed |= (after.data.eax & 0xffffu) != 9u ||
                (after.data.eflags & (VCPU_EFLAGS_CF | VCPU_EFLAGS_OF)) != 0u;
        else
            failed |= (after.data.eax & (width == 8u ? 0xffffu : 0xffffu)) !=
                3u || (width == 16u && (after.data.edx & 0xffffu) != 0u);

        if (failed) {
            return 0;
        }
    }
    return 1;
}

static lib_bool legacy_alu_test_unary_boundaries(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_80386, X86_CORE_CPU_PROFILE_80486SX
    };
    lib_u8 profile, mode, code32, address32, width_index, vector, flags, negate, memory;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (mode = 0u; mode < 6u; ++mode)
    for (code32 = 0u; code32 < (mode == 5u ? 1u : 2u); ++code32)
    for (address32 = 0u; address32 < 2u; ++address32)
    for (width_index = 0u; width_index < 3u; ++width_index)
    for (vector = 0u; vector < 6u; ++vector)
    for (flags = 0u; flags < 2u; ++flags)
    for (negate = 0u; negate < 2u; ++negate)
    for (memory = 0u; memory < 2u; ++memory) {
        const lib_u8 width = (lib_u8)(8u << width_index);
        const lib_u32 mask = legacy_alu_mask(width);
        const lib_u32 sign = (mask >> 1u) + 1u;
        const lib_u32 values[] = {0u, 1u, sign, sign - 1u, mask, 0xa5a5a5a5u & mask};
        const lib_u32 value = values[vector];
        const lib_u32 result = (negate ? 0u - value : ~value) & mask;
        const lib_u32 source = (0x5a5a5a5au & ~mask) | value;
        const lib_u32 expected = (source & ~mask) | result;
        lib_u8 code[8] = {0u};
        lib_u8 bytes = 0u;
        lib_u32 observed = 0u;
        legacy_alu_machine state;
        t_cpu before, after;
        lib_bool passed;

        cpu_instruction_prepare(&state, profiles[profile]);
        if (mode != 0u) {
            const lib_u8 cpl = mode == 5u ? 3u : mode - 1u;
            cpu_instruction_prepare_protected_exception(&state, mode == 5u ? 4u : cpl);
            state.cpu.data.ds.selector = (lib_u16)(0x10u | cpl);
            state.cpu.data.ds.dpl = cpl;
        }
        state.cpu.data.cs.seg.exec.defsize = code32;
        state.cpu.data.ecx = source;
        state.cpu.data.eax = 0x12345678u;
        state.cpu.data.edx = 0x87654321u;
        state.cpu.data.eflags |= 2u | VCPU_EFLAGS_IF | VCPU_EFLAGS_DF |
            (flags ? LEGACY_ALU_FLAGS : 0u);
        if (width != 8u && (width == 32u) != code32) code[bytes++] = 0x66u;
        if (address32 != code32) code[bytes++] = 0x67u;
        code[bytes++] = width == 8u ? 0xf6u : 0xf7u;
        code[bytes++] = (lib_u8)((negate ? 3u : 2u) << 3u) |
            (memory ? (address32 ? 5u : 6u) : 0xc1u);
        if (memory) {
            code[bytes++] = X86_CPU_MASK_U8(LEGACY_ALU_MEMORY);
            code[bytes++] = X86_CPU_MASK_U8(LEGACY_ALU_MEMORY >> 8u);
            if (address32) { code[bytes++] = 0u; code[bytes++] = 0u; }
        }
        lib_memory_copy(state.memory + LEGACY_ALU_MEMORY, &source, sizeof(source));
        state.memory[LEGACY_ALU_MEMORY - 1u] = 0x96u;
        state.memory[LEGACY_ALU_MEMORY + 4u] = 0x69u;
        before = state.cpu;
        passed = legacy_alu_run(&state, code, bytes, 0, &after) &&
            !state.fault.valid && !state.delivered_exception.valid &&
            after.data.eip == bytes && after.data.eax == before.data.eax &&
            after.data.edx == before.data.edx && after.data.esp == before.data.esp &&
            after.data.ecx == (memory ? source : expected) &&
            after.data.eflags == (negate ? legacy_alu_flags(LEGACY_ALU_SUB,
                0u, value, 0u, width, result, before.data.eflags) : before.data.eflags);
        lib_memory_copy(&observed, state.memory + LEGACY_ALU_MEMORY, sizeof(observed));
        passed = passed && observed == (memory ? expected : source) &&
            state.memory[LEGACY_ALU_MEMORY - 1u] == 0x96u &&
            state.memory[LEGACY_ALU_MEMORY + 4u] == 0x69u;
        x86_core_cpu_execution_finalize(&state.execution);
        if (!passed) {
            lib_c_printf("unary profile=%u mode=%u code32=%u address32=%u width=%u "
                "vector=%u flags=%u negate=%u memory=%u\n", profiles[profile], mode,
                code32, address32, width, vector, flags, negate, memory);
            return LIB_FALSE;
        }
    }
    return LIB_TRUE;
}

static lib_i32 legacy_alu_test_inc_dec_and_shift_extensions(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086, X86_CORE_CPU_PROFILE_80186
    };
    lib_u8 profile_index;
    lib_u8 register_index;
    lib_u8 decrement;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (register_index = 0u; register_index != 8u; ++register_index)
    for (decrement = 0u; decrement != 2u; ++decrement) {
        const lib_u8 code[] = { (lib_u8)((decrement ? 0x48u :
            0x40u) + register_index) };
        const lib_u32 value = decrement ? 0x55668000u : 0x55667fffu;
        const lib_u32 expected = decrement ? 0x55667fffu : 0x55668000u;
        legacy_alu_machine state;
        t_cpu before;
        t_cpu after;

        lib_u32 *reg;
        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, profiles[profile_index]);

        reg = legacy_alu_register(&state.cpu, register_index);
        *reg = value;
        state.cpu.data.eflags = VCPU_EFLAGS_CF |
            VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
        before = state.cpu;
        failed |= !legacy_alu_run(&state, code, sizeof(code), 0, &after) || state.fault.valid || after.data.eip != 1u ||
            *legacy_alu_register(&after, register_index) != expected ||
            (after.data.eflags & VCPU_EFLAGS_CF) != VCPU_EFLAGS_CF ||
            (after.data.eflags & (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF |
            VCPU_EFLAGS_PF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF)) !=
            (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF |
            (decrement ? 0u : VCPU_EFLAGS_SF)) ||
            (after.data.eflags & (VCPU_EFLAGS_IF | VCPU_EFLAGS_DF)) !=
            (before.data.eflags & (VCPU_EFLAGS_IF | VCPU_EFLAGS_DF));

        if (failed)
            return 0;
    }
    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (decrement = 0u; decrement != 2u; ++decrement) {
        const lib_u8 code[] = { 0xfeu, decrement ? 0x0eu : 0x06u,
            X86_CPU_MASK_U8(LEGACY_ALU_MEMORY),
            X86_CPU_MASK_U8(LEGACY_ALU_MEMORY >> 8u) };
        lib_u8 value = decrement ? 0x80u : 0x7fu;
        lib_u8 observed = 0u;
        legacy_alu_machine state;
        t_cpu after;

        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, profiles[profile_index]);

        state.cpu.data.eflags = VCPU_EFLAGS_CF |
            VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
        failed |= cpu_instruction_write(&state, LEGACY_ALU_MEMORY,
            &value, sizeof(value),
            X86_CORE_CPU_MEMORY_ACCESS_DATA, 0u) != LIB_STATUS_OK || !legacy_alu_run(&state,
            code, sizeof(code), 0, &after) ||
            state.fault.valid || after.data.eip != sizeof(code) ||
            cpu_instruction_read(&state, LEGACY_ALU_MEMORY, &observed,
            sizeof(observed), X86_CORE_CPU_MEMORY_ACCESS_DATA,
            LIB_FALSE, LIB_FALSE, 0u) != LIB_STATUS_OK || observed != (decrement ?
            0x7fu : 0x80u) || (after.data.eflags & VCPU_EFLAGS_CF) !=
            VCPU_EFLAGS_CF;

        if (failed)
            return 0;
    }
    for (decrement = 0u; decrement != 2u; ++decrement) {
        const lib_u8 code[] = { decrement ? 0xc1u : 0xc0u,
            decrement ? 0xe0u : 0xc0u, 1u };
        legacy_alu_machine state;
        t_cpu after;

        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, X86_CORE_CPU_PROFILE_80186);

        state.cpu.data.eax = 0x11220081u;
        failed |= !legacy_alu_run(&state, code, sizeof(code), 0, &after) || state.fault.valid || after.data.eip !=
            sizeof(code) || (after.data.eax & (decrement ? 0xffffu : 0xffu)) !=
            (decrement ? 0x0102u : 0x0003u);

        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 legacy_alu_test_flags_and_sign_forms(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086, X86_CORE_CPU_PROFILE_80186
    };
    static const lib_u8 flag_codes[] = { 0xf5u, 0xf8u, 0xf9u, 0xfcu, 0xfdu };
    lib_u8 profile_index;
    lib_u8 form;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (form = 0u; form != sizeof(flag_codes); ++form) {
        const lib_u32 initial_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
            VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
        const lib_u32 expected_flags = form == 0u || form == 1u ?
            (initial_flags & ~VCPU_EFLAGS_CF) : (form == 2u ?
            initial_flags : (form == 3u ? (initial_flags & ~VCPU_EFLAGS_DF) :
            initial_flags | VCPU_EFLAGS_DF));
        legacy_alu_machine state;
        t_cpu before;
        t_cpu after;

        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, profiles[profile_index]);

        state.cpu.data.eax = 0x11223344u;
        state.cpu.data.eflags = initial_flags;
        before = state.cpu;
        failed |= !legacy_alu_run(&state, &flag_codes[form], 1u, 0, &after) || state.fault.valid || after.data.eip != 1u ||
            after.data.eax != before.data.eax || after.data.eflags != expected_flags;

        if (failed)
            return 0;
    }
    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (form = 0u; form != 4u; ++form) {
        const lib_u8 code = form < 2u ?
            (form == 0u ? 0x98u : 0x99u) : (form == 2u ? 0x9fu : 0x9eu);
        legacy_alu_machine state;
        t_cpu before;
        t_cpu after;

        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, profiles[profile_index]);

        state.cpu.data.eax = form == 0u ? 0x11220080u :
            (form == 1u ? 0x11228000u : (form == 2u ? 0x11220044u :
            0x1122d744u));
        state.cpu.data.edx = 0x55660000u;
        state.cpu.data.eflags = form == 3u ?
            (VCPU_EFLAGS_OF | VCPU_EFLAGS_IF | VCPU_EFLAGS_DF) :
            (VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF |
            VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_IF |
            VCPU_EFLAGS_DF);
        before = state.cpu;
        failed |= !legacy_alu_run(&state, &code, 1u, 0, &after) || state.fault.valid || after.data.eip != 1u;
        if (form == 0u)
            failed |= after.data.eax != 0x1122ff80u || after.data.edx !=
                before.data.edx || after.data.eflags != before.data.eflags;
        else if (form == 1u)
            failed |= after.data.eax != before.data.eax || after.data.edx !=
                0x5566ffffu || after.data.eflags != before.data.eflags;
        else if (form == 2u)
            failed |= after.data.eax != 0x1122d744u || after.data.edx !=
                before.data.edx || after.data.eflags != before.data.eflags;
        else
            failed |= after.data.eax != before.data.eax || after.data.edx !=
                before.data.edx || after.data.eflags !=
                (VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF |
                VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF |
                VCPU_EFLAGS_IF | VCPU_EFLAGS_DF);

        if (failed)
            return 0;
    }
    return 1;
}

static lib_u16 legacy_alu_shift_result(lib_u8 extension,
    lib_u16 value, lib_u8 count, lib_u8 width,
    lib_u8 *carry)
{
    lib_u8 index;
    const lib_u16 mask = width == 8u ? 0xffu : 0xffffu;
    const lib_u8 high = (lib_u8)(width - 1u);

    for (index = 0u; index != count; ++index) {
        if (extension == 0u) {
            *carry = (value >> high) & 1u;
            value = (lib_u16)(((value << 1u) | *carry) & mask);
        } else if (extension == 1u) {
            *carry = value & 1u;
            value = (lib_u16)((value >> 1u) | (*carry << high));
        } else if (extension == 2u) {
            const lib_u8 next = (value >> high) & 1u;
            value = (lib_u16)(((value << 1u) | *carry) & mask);
            *carry = next;
        } else if (extension == 3u) {
            const lib_u8 next = value & 1u;
            value = (lib_u16)((value >> 1u) | (*carry << high));
            *carry = next;
        } else if (extension == 4u) {
            *carry = (value >> high) & 1u;
            value = (lib_u16)((value << 1u) & mask);
        } else if (extension == 5u) {
            *carry = value & 1u;
            value >>= 1u;
        } else {
            *carry = value & 1u;
            value = (lib_u16)((value >> 1u) | (value & (1u << high)));
        }
    }
    return value;
}

static lib_i32 legacy_alu_test_group2_forms(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086, X86_CORE_CPU_PROFILE_80186
    };
    lib_u8 profile_index;
    lib_u8 extension;
    lib_u8 variant;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (extension = 0u; extension != 8u; ++extension)
    for (variant = 0u; variant != 4u; ++variant) {
        const lib_u8 opcode = variant == 0u ? 0xd0u : variant == 1u ?
            0xd1u : variant == 2u ? 0xd2u : 0xd3u;
        const lib_u8 width = variant == 0u || variant == 2u ? 8u : 16u;
        const lib_u8 code[] = { opcode,
            (lib_u8)(0xc0u | (extension << 3u)) };
        const lib_u8 count = variant < 2u ? 1u : 2u;
        lib_u8 carry = 1u;
        const lib_u16 expected = extension == 6u ?
            (width == 8u ? 0x23u : 0x8123u) : legacy_alu_shift_result(extension,
            width == 8u ? 0x23u : 0x8123u, count, width, &carry);
        legacy_alu_machine state;
        t_cpu before;
        t_cpu after;

        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, profiles[profile_index]);

        state.cpu.data.eax = 0x11228123u;
        state.cpu.data.ecx = 0x55660002u;
        state.cpu.data.eflags = VCPU_EFLAGS_CF |
            VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
        before = state.cpu;
        if (extension == 6u) {
            if (profiles[profile_index] == X86_CORE_CPU_PROFILE_8086)
                failed |= !legacy_alu_run(&state, code, sizeof(code), 1, &after) ||
                    !state.fault.valid || !X86_CPU_BIT_IS_SET(
                    state.fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                    after.data.eip != 0u || after.data.eax != before.data.eax ||
                    after.data.ecx != before.data.ecx || after.data.eflags !=
                    before.data.eflags;
            else
                failed |= !cpu_instruction_expect_real_fault(&state, code,
                    sizeof(code), 6u);
        } else {
            failed |= !legacy_alu_run(&state, code, sizeof(code), 0, &after);
            failed |= state.fault.valid || after.data.eip != sizeof(code) ||
                (after.data.eax & (width == 8u ? 0xffu : 0xffffu)) != expected ||
                (after.data.eflags & VCPU_EFLAGS_CF) != (carry ?
                VCPU_EFLAGS_CF : 0u) || (after.data.eflags &
                (VCPU_EFLAGS_IF | VCPU_EFLAGS_DF)) != (before.data.eflags &
                (VCPU_EFLAGS_IF | VCPU_EFLAGS_DF));
        }

        if (failed)
            return 0;
    }
    for (variant = 0u; variant != 2u; ++variant) {
        const lib_u8 code[] = { variant == 0u ? 0xc0u : 0xc1u,
            0xc0u, 1u };
        legacy_alu_machine state;
        t_cpu after;

        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, X86_CORE_CPU_PROFILE_8086);

        state.cpu.data.eax = 0x11228123u;
        failed |= !legacy_alu_run(&state, code, sizeof(code), 1, &after) || !state.fault.valid || !X86_CPU_BIT_IS_SET(
            state.fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eip != 0u || after.data.eax != 0x11228123u;

        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 legacy_alu_test_group2_immediate_extensions(void)
{
    lib_u8 extension;
    lib_u8 width_index;

    for (width_index = 0u; width_index != 2u; ++width_index)
    for (extension = 0u; extension != 8u; ++extension) {
        const lib_u8 width = width_index == 0u ? 8u : 16u;
        const lib_u8 code[] = { width == 8u ? 0xc0u : 0xc1u,
            (lib_u8)(0xc0u | (extension << 3u)), 2u };
        lib_u8 carry = 1u;
        const lib_u16 expected = extension == 6u ?
            (width == 8u ? 0x23u : 0x8123u) : legacy_alu_shift_result(extension,
            width == 8u ? 0x23u : 0x8123u, 2u, width, &carry);
        legacy_alu_machine state;
        t_cpu after;

        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, X86_CORE_CPU_PROFILE_80186);

        state.cpu.data.eax = 0x11228123u;
        state.cpu.data.eflags = VCPU_EFLAGS_CF |
            VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
        if (extension == 6u)
            failed |= !cpu_instruction_expect_real_fault(&state, code,
                sizeof(code), 6u);
        else {
            failed |= !legacy_alu_run(&state, code, sizeof(code), 0, &after);
            failed |= state.fault.valid || after.data.eip != sizeof(code) ||
                (after.data.eax & (width == 8u ? 0xffu : 0xffffu)) != expected ||
                (after.data.eflags & VCPU_EFLAGS_CF) != (carry ?
                VCPU_EFLAGS_CF : 0u);
        }

        if (failed)
            return 0;
    }
    for (width_index = 0u; width_index != 2u; ++width_index) {
        const lib_u8 code[] = { width_index == 0u ? 0xc0u : 0xc1u,
            0xc0u, 1u };
        legacy_alu_machine state;
        t_cpu after;

        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, X86_CORE_CPU_PROFILE_8086);

        state.cpu.data.eax = 0x11228123u;
        failed |= !legacy_alu_run(&state, code, sizeof(code), 1, &after) || !state.fault.valid || !X86_CPU_BIT_IS_SET(
            state.fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eip != 0u || after.data.eax != 0x11228123u;

        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 legacy_alu_test_reserved_and_attribute_rejections(void)
{
    static const x86_core_cpu_profile profiles[] = {
        X86_CORE_CPU_PROFILE_8086, X86_CORE_CPU_PROFILE_80186
    };
    static const lib_u8 reserved[][2] = {
        { 0xd6u }, { 0xf1u }, { 0xf6u, 0xc8u }, { 0xf7u, 0xc8u }
    };
    static const lib_u8 lengths[] = { 1u, 1u, 2u, 2u };
    static const lib_u8 attributes[][3] = {
        { 0x66u, 0x01u, 0xc8u }, { 0x67u, 0x01u, 0xc8u },
        { 0x66u, 0x67u, 0x01u }
    };
    static const lib_u8 attribute_lengths[] = { 3u, 3u, 4u };
    lib_u8 profile_index;
    lib_u8 form;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (form = 0u; form != sizeof(lengths); ++form) {
        legacy_alu_machine state;
        t_cpu after;

        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, profiles[profile_index]);

        state.cpu.data.eax = 0x11223344u;
        state.cpu.data.ecx = 0x55667788u;
        state.cpu.data.eflags = VCPU_EFLAGS_CF |
            VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
        if (profiles[profile_index] == X86_CORE_CPU_PROFILE_8086)
            failed |= !legacy_alu_run(&state, reserved[form], lengths[form], 1,
                &after) || !state.fault.valid ||
                !X86_CPU_BIT_IS_SET(state.fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.eip != 0u || after.data.eax != 0x11223344u ||
                after.data.ecx != 0x55667788u || after.data.eflags !=
                (VCPU_EFLAGS_CF | VCPU_EFLAGS_IF | VCPU_EFLAGS_DF);
        else
            failed |= !cpu_instruction_expect_real_fault(&state, reserved[form],
                lengths[form], 6u);

        if (failed)
            return 0;
    }
    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (form = 0u; form != sizeof(attribute_lengths); ++form) {
        legacy_alu_machine state;
        t_cpu after;

        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, profiles[profile_index]);

        state.cpu.data.eax = 0x11223344u;
        state.cpu.data.ecx = 0x55667788u;
        state.cpu.data.eflags = VCPU_EFLAGS_CF |
            VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
        if (profiles[profile_index] == X86_CORE_CPU_PROFILE_8086)
            failed |= !legacy_alu_run(&state, attributes[form],
                attribute_lengths[form], 1, &after) || !state.fault.valid ||
                !X86_CPU_BIT_IS_SET(state.fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.eip != 0u || after.data.eax != 0x11223344u ||
                after.data.ecx != 0x55667788u || after.data.eflags !=
                (VCPU_EFLAGS_CF | VCPU_EFLAGS_IF | VCPU_EFLAGS_DF);
        else
            failed |= !cpu_instruction_expect_real_fault(&state, attributes[form],
                attribute_lengths[form], 6u);

        if (failed)
            return 0;
    }
    return 1;
}

int main(void)
{
    if (!legacy_alu_test_binary_forms() ||
        !legacy_alu_test_accumulator_immediate_forms() || !legacy_alu_test_group1_forms() ||
        !legacy_alu_test_condition_forms() || !legacy_alu_test_loop_forms() ||
        !legacy_alu_test_test_forms() || !legacy_alu_test_adjust_and_xlat_forms() ||
        !legacy_alu_test_decimal_arithmetic() ||
        !legacy_alu_test_ascii_arithmetic() || !legacy_alu_test_aam_aad_results() ||
        !legacy_alu_test_salc() ||
        !legacy_alu_test_sx_test_aliases() || !legacy_alu_test_adjust_inputs() ||
        !legacy_alu_test_group3_forms() || !legacy_alu_test_unary_boundaries() ||
        !legacy_alu_test_inc_dec_and_shift_extensions() ||
        !legacy_alu_test_flags_and_sign_forms() || !legacy_alu_test_group2_forms() ||
        !legacy_alu_test_group2_immediate_extensions() ||
        !legacy_alu_test_reserved_and_attribute_rejections()) {
        lib_c_fprintf(lib_c_stderr, "LEGACY-ALU:FAILED\n");
        return 1;
    }
    lib_c_printf("LEGACY-ALU:OK\n");
    lib_c_printf("DECIMAL-ADJUST-PROFILES:OK\n");
    return 0;
}
