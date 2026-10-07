#include "lib/types/types_interface.h"
#include "lib/types/file.h"
#include "support/cpu_instruction_fixture.h"
/* T337_REAL_UD_TERMINAL_CPU_OWNER: legacy invalid opcodes are CPU-owned. */

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
        const lib_u32 sum = left + right +
            (operation == LEGACY_ALU_ADC ? carry : 0u);
        cf = sum > mask;
        of = ((~(left ^ right) & (left ^ result)) & sign) != 0u;
        af = ((left ^ right ^ result) & 0x10u) != 0u;
    } else if (operation == LEGACY_ALU_SUB || operation == LEGACY_ALU_SBB ||
        operation == LEGACY_ALU_CMP) {
        const lib_u32 subtrahend = right +
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

static lib_i32 legacy_alu_binary_case(core_machine_cpu_profile profile,
    legacy_alu_operation operation, lib_u8 encoding,
    lib_u8 width, lib_i32 memory)
{
    static const lib_u8 base[] = {
        0x00u, 0x08u, 0x10u, 0x18u, 0x20u, 0x28u, 0x30u, 0x38u
    };
    const lib_u32 left = width == 8u ? 0x7fu :
        (width == 16u ? 0x7fffu : 0x7fffffffu);
    const lib_u32 right = width == 8u ? 0x01u : 0x0001u;
    const lib_u32 carry = operation == LEGACY_ALU_ADC ||
        operation == LEGACY_ALU_SBB ? 1u : 0u;
    const lib_u32 mask = legacy_alu_mask(width);
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
    state.cpu.data.eflags = VCPU_EFLAGS_CF |
        VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
    if (memory)
        failed |= cpu_instruction_write(&state, LEGACY_ALU_MEMORY,
            encoding == 0u ? &left : &right,
            width == 8u ? 1u : (width == 16u ? 2u : 4u),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
    before = state.cpu;
    failed |= !legacy_alu_run(&state, code, bytes, 0, &after) ||
        state.fault.valid || after.data.eip != bytes ||
        after.data.eflags != legacy_alu_flags(operation, left, right, carry,
            width, expected, before.data.eflags);
    if (memory)
        failed |= cpu_instruction_read(&state, LEGACY_ALU_MEMORY,
            &observed, width == 8u ? 1u : (width == 16u ? 2u : 4u),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE,
            LIB_FALSE) != LIB_STATUS_OK ||
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
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_u8 profile_index;
    lib_u8 operation;
    lib_u8 encoding;
    lib_u8 width_index;
    lib_u8 memory;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (operation = 0u; operation != 8u; ++operation)
    for (encoding = 0u; encoding != 2u; ++encoding)
    for (width_index = 0u; width_index != 2u; ++width_index)
    for (memory = 0u; memory != 2u; ++memory)
        if (!legacy_alu_binary_case(profiles[profile_index],
                (legacy_alu_operation)operation, encoding,
                width_index == 0u ? 8u : 16u, memory))
            return 0;
    for (operation = 0u; operation != 8u; ++operation)
    for (encoding = 0u; encoding != 2u; ++encoding)
    for (memory = 0u; memory != 2u; ++memory)
        if (!legacy_alu_binary_case(CORE_MACHINE_CPU_PROFILE_80386,
                (legacy_alu_operation)operation, encoding, 32u, memory))
            return 0;
    return 1;
}

static lib_i32 legacy_alu_test_accumulator_immediate_forms(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
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

        if (width == 32u && profiles[profile_index] != CORE_MACHINE_CPU_PROFILE_80386)
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

static lib_i32 legacy_alu_group1_case(core_machine_cpu_profile profile,
    legacy_alu_operation operation, lib_u8 width,
    lib_i32 sign_extended, lib_i32 memory)
{
    const lib_u32 left = width == 8u ? 0x7fu : 0x7fffu;
    const lib_u32 right = sign_extended ? 0xffffu : 0x0001u;
    const lib_u32 carry = operation == LEGACY_ALU_ADC ||
        operation == LEGACY_ALU_SBB ? 1u : 0u;
    const lib_u32 mask = legacy_alu_mask(width);
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
    code[bytes++] = width == 8u ? 0x80u : (sign_extended ? 0x83u : 0x81u);
    code[bytes++] = (lib_u8)(operation << 3u) | (memory ? 0x06u : 0xc0u);
    if (memory) {
        code[bytes++] = X86_CPU_MASK_U8(LEGACY_ALU_MEMORY);
        code[bytes++] = X86_CPU_MASK_U8(LEGACY_ALU_MEMORY >> 8u);
    }
    if (width == 8u || sign_extended)
        code[bytes++] = sign_extended ? 0xffu : 0x01u;
    else {
        code[bytes++] = 0x01u;
        code[bytes++] = 0x00u;
    }
    state.cpu.data.eax = left;
    state.cpu.data.eflags = VCPU_EFLAGS_CF |
        VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
    if (memory)
        failed |= cpu_instruction_write(&state, LEGACY_ALU_MEMORY, &left, width == 8u ? 1u : 2u,
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
    before = state.cpu;
    failed |= !legacy_alu_run(&state, code, bytes, 0, &after) ||
        state.fault.valid || after.data.eip != bytes ||
        after.data.eflags != legacy_alu_flags(operation, left, right, carry,
            width, expected, before.data.eflags);
    if (memory)
        failed |= cpu_instruction_read(&state, LEGACY_ALU_MEMORY, &observed, width == 8u ? 1u : 2u,
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
            observed != (operation == LEGACY_ALU_CMP ? left : expected);
    else if (operation != LEGACY_ALU_CMP)
        failed |= (after.data.eax & mask) != expected;
    else
        failed |= (after.data.eax & mask) != left;

    return !failed;
}

static lib_i32 legacy_alu_test_group1_forms(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186
    };
    lib_u8 profile_index;
    lib_u8 operation;
    lib_u8 width_index;
    lib_u8 sign_extended;
    lib_u8 memory;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (operation = 0u; operation != 8u; ++operation)
    for (width_index = 0u; width_index != 2u; ++width_index)
    for (sign_extended = 0u; sign_extended != (width_index == 0u ? 1u : 2u);
        ++sign_extended)
    for (memory = 0u; memory != 2u; ++memory)
        if (!legacy_alu_group1_case(profiles[profile_index],
                (legacy_alu_operation)operation, width_index == 0u ? 8u : 16u,
                sign_extended, memory))
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
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186
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
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186
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
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186
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
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
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
            (form == 1u ? 0x1122009au : (form == 2u ? 0x11220100u :
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
                sizeof(table_value), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) !=
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

static lib_i32 legacy_alu_test_group3_forms(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186
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

static lib_i32 legacy_alu_test_inc_dec_and_shift_extensions(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186
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
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK || !legacy_alu_run(&state,
            code, sizeof(code), 0, &after) ||
            state.fault.valid || after.data.eip != sizeof(code) ||
            cpu_instruction_read(&state, LEGACY_ALU_MEMORY, &observed,
            sizeof(observed), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA,
            LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || observed != (decrement ?
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
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80186);

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
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186
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
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186
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
            if (profiles[profile_index] == CORE_MACHINE_CPU_PROFILE_8086)
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
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_8086);

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
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80186);

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
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_8086);

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
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186
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
        if (profiles[profile_index] == CORE_MACHINE_CPU_PROFILE_8086)
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
        if (profiles[profile_index] == CORE_MACHINE_CPU_PROFILE_8086)
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
        !legacy_alu_test_group3_forms() ||
        !legacy_alu_test_inc_dec_and_shift_extensions() ||
        !legacy_alu_test_flags_and_sign_forms() || !legacy_alu_test_group2_forms() ||
        !legacy_alu_test_group2_immediate_extensions() ||
        !legacy_alu_test_reserved_and_attribute_rejections()) {
        lib_c_fprintf(lib_c_stderr, "M5:T338:S2:LEGACY-ALU:FAILED\n");
        return 1;
    }
    lib_c_printf("M5:T338:S2:LEGACY-ALU:OK\n");
    lib_c_printf("M5:T401:S34:DECIMAL-ADJUST-PROFILES:OK\n");
    return 0;
}
