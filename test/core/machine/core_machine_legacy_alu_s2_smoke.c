#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define LEGACY_ALU_MEMORY 0x5000u
#define LEGACY_ALU_FLAGS (VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF | \
    VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF)

typedef struct legacy_alu_machine {
    core_machine *machine;
} legacy_alu_machine;

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

static type_unsigned_32 *legacy_alu_register(t_cpu *cpu, type_unsigned_8 index)
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
    default: return STD_NULL;
    }
}

static C_VOID legacy_alu_reset(C_VOID *opaque)
{
    legacy_alu_machine *state = (legacy_alu_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider legacy_alu_provider = {
    legacy_alu_reset, STD_NULL
};

static C_INT legacy_alu_prepare(core_machine_cpu_profile profile,
    legacy_alu_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    if (state == STD_NULL)
        return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    if (!test_core_machine_fixture_create_bind_freeze_reset(&config,
            &legacy_alu_provider, state, &state->machine)) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT legacy_alu_run(legacy_alu_machine *state,
    const type_unsigned_8 *code, STD_SIZE_T bytes, C_INT fault, t_cpu *after,
    core_machine_cpu_diagnostic *diagnostic)
{
    core_machine_run_result result;
    type_status status;

    if (state == STD_NULL || state->machine == STD_NULL || code == STD_NULL ||
        after == STD_NULL || diagnostic == STD_NULL ||
        !test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u) ||
        core_machine_memory_write(state->machine, 0u, code, bytes) != TYPE_STATUS_OK)
        return 0;
    if (fault && !test_core_machine_fixture_preflight_real_ud_terminal(
            state->machine))
        return 0;
    status = core_machine_run(state->machine,
        (core_machine_run_budget){ 1u, 0u }, &result);
    if (status != (fault ? TYPE_STATUS_FAULT : TYPE_STATUS_OK) ||
        result.reason != (fault ? CORE_MACHINE_STOP_FAULT :
            CORE_MACHINE_STOP_BUDGET) ||
        core_machine_get_cpu_diagnostic(state->machine, diagnostic) != TYPE_STATUS_OK)
        return 0;
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static type_unsigned_32 legacy_alu_mask(type_unsigned_8 width)
{
    return width == 8u ? 0xffu : (width == 16u ? 0xffffu : 0xffffffffu);
}

static type_unsigned_16 legacy_alu_real_flags_image(type_unsigned_32 flags)
{
    type_unsigned_16 image = TYPE_MASK_UNSIGNED_16((flags &
        ~VCPU_EFLAGS_RESERVED) | 0x02u);

    return image;
}

static type_unsigned_16 legacy_alu_real_flags_known_mask(
    core_machine_cpu_profile profile)
{
    return profile < CORE_MACHINE_CPU_PROFILE_80286 ? 0x0fd5u : 0x7fd5u;
}

static type_unsigned_32 legacy_alu_parity(type_unsigned_32 value)
{
    type_unsigned_8 byte = TYPE_MASK_UNSIGNED_8(value);
    type_unsigned_8 bit;
    type_unsigned_32 parity = 1u;

    for (bit = 0u; bit != 8u; ++bit)
        parity ^= (byte >> bit) & 1u;
    return parity;
}

static type_unsigned_32 legacy_alu_flags(legacy_alu_operation operation,
    type_unsigned_32 left, type_unsigned_32 right, type_unsigned_32 carry,
    type_unsigned_8 width, type_unsigned_32 result, type_unsigned_32 before)
{
    const type_unsigned_32 mask = legacy_alu_mask(width);
    const type_unsigned_32 sign = width == 8u ? 0x80u :
        (width == 16u ? 0x8000u : 0x80000000u);
    type_unsigned_32 flags = before & ~LEGACY_ALU_FLAGS;
    type_unsigned_32 cf = 0u;
    type_unsigned_32 of = 0u;
    type_unsigned_32 af = 0u;

    left &= mask;
    right &= mask;
    result &= mask;
    if (operation == LEGACY_ALU_ADD || operation == LEGACY_ALU_ADC) {
        const type_unsigned_32 sum = left + right +
            (operation == LEGACY_ALU_ADC ? carry : 0u);
        cf = sum > mask;
        of = ((~(left ^ right) & (left ^ result)) & sign) != 0u;
        af = ((left ^ right ^ result) & 0x10u) != 0u;
    } else if (operation == LEGACY_ALU_SUB || operation == LEGACY_ALU_SBB ||
        operation == LEGACY_ALU_CMP) {
        const type_unsigned_32 subtrahend = right +
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

static C_INT legacy_alu_binary_case(core_machine_cpu_profile profile,
    legacy_alu_operation operation, type_unsigned_8 encoding,
    type_unsigned_8 width, C_INT memory)
{
    static const type_unsigned_8 base[] = {
        0x00u, 0x08u, 0x10u, 0x18u, 0x20u, 0x28u, 0x30u, 0x38u
    };
    const type_unsigned_32 left = width == 8u ? 0x7fu :
        (width == 16u ? 0x7fffu : 0x7fffffffu);
    const type_unsigned_32 right = width == 8u ? 0x01u : 0x0001u;
    const type_unsigned_32 carry = operation == LEGACY_ALU_ADC ||
        operation == LEGACY_ALU_SBB ? 1u : 0u;
    const type_unsigned_32 mask = legacy_alu_mask(width);
    type_unsigned_32 expected;
    type_unsigned_32 observed = 0u;
    type_unsigned_8 code[6] = { 0u };
    STD_SIZE_T bytes = 0u;
    legacy_alu_machine state;
    t_cpu after;
    t_cpu before;
    core_machine_cpu_diagnostic diagnostic;
    C_INT failed = !legacy_alu_prepare(profile, &state);

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
    code[bytes++] = (type_unsigned_8)(base[operation] +
        (encoding == 0u ? (width == 8u ? 0u : 1u) :
        (width == 8u ? 2u : 3u)));
    if (memory) {
        code[bytes++] = encoding == 0u ? 0x0eu : 0x06u;
        code[bytes++] = TYPE_MASK_UNSIGNED_8(LEGACY_ALU_MEMORY);
        code[bytes++] = TYPE_MASK_UNSIGNED_8(LEGACY_ALU_MEMORY >> 8u);
    } else
        code[bytes++] = encoding == 0u ? 0xc8u : 0xc1u;
    if (!failed) {
        state.machine->executor_cpu.data.eax = left;
        state.machine->executor_cpu.data.ecx = right;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
            VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
        if (memory)
            failed |= core_machine_memory_write(state.machine, LEGACY_ALU_MEMORY,
                encoding == 0u ? &left : &right,
                width == 8u ? 1u : (width == 16u ? 2u : 4u)) != TYPE_STATUS_OK;
        before = state.machine->executor_cpu;
        failed |= !legacy_alu_run(&state, code, bytes, 0, &after, &diagnostic) ||
            diagnostic.first_fault.valid || after.data.eip != bytes ||
            after.data.eflags != legacy_alu_flags(operation, left, right, carry,
                width, expected, before.data.eflags);
        if (memory)
            failed |= core_machine_memory_read(state.machine, LEGACY_ALU_MEMORY,
                &observed, width == 8u ? 1u : (width == 16u ? 2u : 4u)) != TYPE_STATUS_OK ||
                observed != (encoding == 0u && operation != LEGACY_ALU_CMP ?
                    expected : (encoding == 0u ? left : right));
        else if (operation != LEGACY_ALU_CMP)
            failed |= (after.data.eax & mask) != expected;
        else
            failed |= (after.data.eax & mask) != left;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT legacy_alu_test_binary_forms(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile_index;
    type_unsigned_8 operation;
    type_unsigned_8 encoding;
    type_unsigned_8 width_index;
    type_unsigned_8 memory;

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

static C_INT legacy_alu_test_accumulator_immediate_forms(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 base[] = {
        0x04u, 0x0cu, 0x14u, 0x1cu, 0x24u, 0x2cu, 0x34u, 0x3cu
    };
    type_unsigned_8 profile_index;
    type_unsigned_8 operation;
    type_unsigned_8 width_index;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (operation = 0u; operation != 8u; ++operation)
    for (width_index = 0u; width_index != 3u; ++width_index) {
        const type_unsigned_8 width = width_index == 0u ? 8u :
            (width_index == 1u ? 16u : 32u);
        const type_unsigned_32 left = width == 8u ? 0x7fu :
            (width == 16u ? 0x7fffu : 0x7fffffffu);
        const type_unsigned_32 right = 1u;
        const type_unsigned_32 carry = operation == LEGACY_ALU_ADC ||
            operation == LEGACY_ALU_SBB ? 1u : 0u;
        const type_unsigned_32 mask = legacy_alu_mask(width);
        const type_unsigned_32 expected = operation == LEGACY_ALU_ADD ||
            operation == LEGACY_ALU_ADC ? (left + right + carry) & mask :
            (operation == LEGACY_ALU_SUB || operation == LEGACY_ALU_SBB ||
            operation == LEGACY_ALU_CMP ? (left - right - carry) & mask :
            (operation == LEGACY_ALU_OR ? left | right :
            (operation == LEGACY_ALU_AND ? left & right : left ^ right)));
        type_unsigned_8 code[6] = { 0u };
        STD_SIZE_T bytes = 0u;
        legacy_alu_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed;

        if (width == 32u && profiles[profile_index] != CORE_MACHINE_CPU_PROFILE_80386)
            continue;
        if (width == 32u) code[bytes++] = 0x66u;
        code[bytes++] = (type_unsigned_8)(base[operation] +
            (width == 8u ? 0u : 1u));
        code[bytes++] = 1u;
        if (width != 8u) {
            code[bytes++] = 0u;
            if (width == 32u) {
                code[bytes++] = 0u;
                code[bytes++] = 0u;
            }
        }
        failed = !legacy_alu_prepare(profiles[profile_index], &state);
        if (!failed) {
            state.machine->executor_cpu.data.eax = width == 32u ? left :
                (0xaabb0000u | left);
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
                VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
            before = state.machine->executor_cpu;
            failed |= !legacy_alu_run(&state, code, bytes, 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.eip != bytes ||
                after.data.eflags != legacy_alu_flags((legacy_alu_operation)operation,
                left, right, carry, width, expected, before.data.eflags) ||
                (after.data.eax & mask) != (operation == LEGACY_ALU_CMP ? left :
                expected) || (width == 8u && (after.data.eax & 0xffffff00u) !=
                (before.data.eax & 0xffffff00u));
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT legacy_alu_group1_case(core_machine_cpu_profile profile,
    legacy_alu_operation operation, type_unsigned_8 width,
    C_INT sign_extended, C_INT memory)
{
    const type_unsigned_32 left = width == 8u ? 0x7fu : 0x7fffu;
    const type_unsigned_32 right = sign_extended ? 0xffffu : 0x0001u;
    const type_unsigned_32 carry = operation == LEGACY_ALU_ADC ||
        operation == LEGACY_ALU_SBB ? 1u : 0u;
    const type_unsigned_32 mask = legacy_alu_mask(width);
    type_unsigned_32 expected;
    type_unsigned_32 observed = 0u;
    type_unsigned_8 code[6] = { 0u };
    STD_SIZE_T bytes = 0u;
    legacy_alu_machine state;
    t_cpu after;
    t_cpu before;
    core_machine_cpu_diagnostic diagnostic;
    C_INT failed = !legacy_alu_prepare(profile, &state);

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
    code[bytes++] = (type_unsigned_8)(operation << 3u) | (memory ? 0x06u : 0xc0u);
    if (memory) {
        code[bytes++] = TYPE_MASK_UNSIGNED_8(LEGACY_ALU_MEMORY);
        code[bytes++] = TYPE_MASK_UNSIGNED_8(LEGACY_ALU_MEMORY >> 8u);
    }
    if (width == 8u || sign_extended)
        code[bytes++] = sign_extended ? 0xffu : 0x01u;
    else {
        code[bytes++] = 0x01u;
        code[bytes++] = 0x00u;
    }
    if (!failed) {
        state.machine->executor_cpu.data.eax = left;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
            VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
        if (memory)
            failed |= core_machine_memory_write(state.machine, LEGACY_ALU_MEMORY,
                &left, width == 8u ? 1u : 2u) != TYPE_STATUS_OK;
        before = state.machine->executor_cpu;
        failed |= !legacy_alu_run(&state, code, bytes, 0, &after, &diagnostic) ||
            diagnostic.first_fault.valid || after.data.eip != bytes ||
            after.data.eflags != legacy_alu_flags(operation, left, right, carry,
                width, expected, before.data.eflags);
        if (memory)
            failed |= core_machine_memory_read(state.machine, LEGACY_ALU_MEMORY,
                &observed, width == 8u ? 1u : 2u) != TYPE_STATUS_OK ||
                observed != (operation == LEGACY_ALU_CMP ? left : expected);
        else if (operation != LEGACY_ALU_CMP)
            failed |= (after.data.eax & mask) != expected;
        else
            failed |= (after.data.eax & mask) != left;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT legacy_alu_test_group1_forms(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186
    };
    type_unsigned_8 profile_index;
    type_unsigned_8 operation;
    type_unsigned_8 width_index;
    type_unsigned_8 sign_extended;
    type_unsigned_8 memory;

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

static type_unsigned_32 legacy_alu_jcc_flags(type_unsigned_8 condition,
    C_INT taken)
{
    type_unsigned_32 flags = 0u;

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

static C_INT legacy_alu_test_condition_forms(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186
    };
    type_unsigned_8 profile_index;
    type_unsigned_8 condition;
    C_INT taken;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (condition = 0u; condition != 16u; ++condition)
    for (taken = 0; taken != 2; ++taken) {
        const type_unsigned_8 code[] = { (type_unsigned_8)(0x70u + condition),
            0x02u, 0x90u, 0x90u };
        legacy_alu_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !legacy_alu_prepare(profiles[profile_index], &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0x11223344u;
            state.machine->executor_cpu.data.ecx = 0x55667788u;
            state.machine->executor_cpu.data.eflags = legacy_alu_jcc_flags(condition,
                taken);
            before = state.machine->executor_cpu;
            failed |= !legacy_alu_run(&state, code, sizeof(code), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                after.data.eip != (taken ? 4u : 2u) ||
                after.data.eax != before.data.eax ||
                after.data.ecx != before.data.ecx ||
                after.data.eflags != before.data.eflags;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT legacy_alu_test_loop_forms(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186
    };
    type_unsigned_8 profile_index;
    type_unsigned_8 operation;
    C_INT taken;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (operation = 0u; operation != 4u; ++operation)
    for (taken = 0; taken != 2; ++taken) {
        const type_unsigned_8 code[] = { (type_unsigned_8)(0xe0u + operation),
            0x02u, 0x90u, 0x90u };
        const type_unsigned_32 flags = (operation == 0u && !taken) ||
            (operation == 1u && taken) ? VCPU_EFLAGS_ZF : 0u;
        const type_unsigned_32 ecx = operation == 3u ? (taken ? 0u : 1u) :
            (taken ? 2u : 1u);
        const type_unsigned_32 expected_ecx = operation == 3u ? ecx : ecx - 1u;
        legacy_alu_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !legacy_alu_prepare(profiles[profile_index], &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0x11223344u;
            state.machine->executor_cpu.data.ecx = 0xaabb0000u | ecx;
            state.machine->executor_cpu.data.eflags = flags | VCPU_EFLAGS_IF |
                VCPU_EFLAGS_DF;
            before = state.machine->executor_cpu;
            failed |= !legacy_alu_run(&state, code, sizeof(code), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                after.data.eip != (taken ? 4u : 2u) || after.data.eax !=
                before.data.eax || after.data.ecx != (0xaabb0000u | expected_ecx) ||
                after.data.eflags != before.data.eflags;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT legacy_alu_test_test_forms(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186
    };
    static const type_unsigned_8 forms[][4] = {
        { 0x84u, 0xc8u }, { 0x85u, 0xc8u }, { 0xa8u, 0x0fu },
        { 0xa9u, 0x0fu, 0x00u }, { 0xf6u, 0xc0u, 0x0fu },
        { 0xf7u, 0xc0u, 0x0fu, 0x00u }
    };
    static const type_unsigned_8 lengths[] = { 2u, 2u, 2u, 3u, 3u, 4u };
    type_unsigned_8 profile_index;
    type_unsigned_8 form;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (form = 0u; form != sizeof(lengths); ++form) {
        const type_unsigned_8 width = (form == 0u || form == 2u || form == 4u) ?
            8u : 16u;
        const type_unsigned_32 result = width == 8u ? 0x08u : 0x0008u;
        const type_unsigned_32 expected_flags = (legacy_alu_parity(result) ?
            VCPU_EFLAGS_PF : 0u) | VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
        legacy_alu_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !legacy_alu_prepare(profiles[profile_index], &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0xaabbcc08u;
            state.machine->executor_cpu.data.ecx = 0x1122330fu;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
                VCPU_EFLAGS_OF | VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
            before = state.machine->executor_cpu;
            failed |= !legacy_alu_run(&state, forms[form], lengths[form], 0,
                &after, &diagnostic) || diagnostic.first_fault.valid ||
                after.data.eip != lengths[form] || after.data.eax != before.data.eax ||
                after.data.ecx != before.data.ecx ||
                (after.data.eflags & (VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
                VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF |
                VCPU_EFLAGS_IF | VCPU_EFLAGS_DF)) != expected_flags;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT legacy_alu_test_adjust_and_xlat_forms(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 codes[][2] = {
        { 0x27u }, { 0x2fu }, { 0x37u }, { 0x3fu }, { 0xd4u, 10u },
        { 0xd5u, 10u }, { 0xd7u }
    };
    static const type_unsigned_8 lengths[] = { 1u, 1u, 1u, 1u, 2u, 2u, 1u };
    type_unsigned_8 profile_index;
    type_unsigned_8 form;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (form = 0u; form != sizeof(lengths); ++form) {
        const type_unsigned_32 initial_eax = form == 0u ? 0x1122009au :
            (form == 1u ? 0x11220000u : (form == 2u ? 0x1122000au :
            (form == 3u ? 0x1122010au : (form == 4u ? 0x1122002au :
            (form == 5u ? 0x11220402u : 0x11220002u)))));
        const type_unsigned_32 expected_eax = form == 0u ? 0x11220000u :
            (form == 1u ? 0x1122009au : (form == 2u ? 0x11220100u :
            (form == 3u ? 0x11220004u : (form == 4u ? 0x11220402u :
            (form == 5u ? 0x1122002au : 0x112200a5u)))));
        legacy_alu_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_8 table_value = 0xa5u;
        C_INT failed = !legacy_alu_prepare(profiles[profile_index], &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = initial_eax;
            state.machine->executor_cpu.data.ebx = LEGACY_ALU_MEMORY;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_AF |
                VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
            if (form == 6u)
                failed |= core_machine_memory_write(state.machine,
                    LEGACY_ALU_MEMORY + 2u, &table_value, sizeof(table_value)) !=
                    TYPE_STATUS_OK;
            before = state.machine->executor_cpu;
            failed |= !legacy_alu_run(&state, codes[form], lengths[form], 0,
                &after, &diagnostic) || diagnostic.first_fault.valid ||
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
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT legacy_alu_test_group3_forms(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186
    };
    type_unsigned_8 profile_index;
    type_unsigned_8 width_index;
    type_unsigned_8 extension;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (width_index = 0u; width_index != 2u; ++width_index)
    for (extension = 2u; extension != 8u; ++extension) {
        const type_unsigned_8 width = width_index == 0u ? 8u : 16u;
        const type_unsigned_8 code[] = { width == 8u ? 0xf6u : 0xf7u,
            (type_unsigned_8)(0xc1u | (extension << 3u)) };
        legacy_alu_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !legacy_alu_prepare(profiles[profile_index], &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = extension == 7u ?
                0x1122fff7u : (extension == 6u ? 0x11220009u : 0x11220003u);
            state.machine->executor_cpu.data.ecx = extension == 7u ?
                (width == 8u ? 0x5566fffdu : 0x5566fffdu) : 0x55660003u;
            state.machine->executor_cpu.data.edx = extension >= 6u ?
                (extension == 7u ? 0x7788ffffu : 0x77880000u) : 0x77880000u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
            before = state.machine->executor_cpu;
            failed |= !legacy_alu_run(&state, code, sizeof(code), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.eip !=
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
        }
        core_machine_destroy(state.machine);
        if (failed) {
            return 0;
        }
    }
    return 1;
}

static C_INT legacy_alu_test_inc_dec_and_shift_extensions(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186
    };
    type_unsigned_8 profile_index;
    type_unsigned_8 register_index;
    type_unsigned_8 decrement;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (register_index = 0u; register_index != 8u; ++register_index)
    for (decrement = 0u; decrement != 2u; ++decrement) {
        const type_unsigned_8 code[] = { (type_unsigned_8)((decrement ? 0x48u :
            0x40u) + register_index) };
        const type_unsigned_32 value = decrement ? 0x55668000u : 0x55667fffu;
        const type_unsigned_32 expected = decrement ? 0x55667fffu : 0x55668000u;
        legacy_alu_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 *reg;
        C_INT failed = !legacy_alu_prepare(profiles[profile_index], &state);

        if (!failed) {
            reg = legacy_alu_register(&state.machine->executor_cpu, register_index);
            *reg = value;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
                VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
            before = state.machine->executor_cpu;
            failed |= !legacy_alu_run(&state, code, sizeof(code), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.eip != 1u ||
                *legacy_alu_register(&after, register_index) != expected ||
                (after.data.eflags & VCPU_EFLAGS_CF) != VCPU_EFLAGS_CF ||
                (after.data.eflags & (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF |
                VCPU_EFLAGS_PF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF)) !=
                (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF |
                (decrement ? 0u : VCPU_EFLAGS_SF)) ||
                (after.data.eflags & (VCPU_EFLAGS_IF | VCPU_EFLAGS_DF)) !=
                (before.data.eflags & (VCPU_EFLAGS_IF | VCPU_EFLAGS_DF));
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (decrement = 0u; decrement != 2u; ++decrement) {
        const type_unsigned_8 code[] = { 0xfeu, decrement ? 0x0eu : 0x06u,
            TYPE_MASK_UNSIGNED_8(LEGACY_ALU_MEMORY),
            TYPE_MASK_UNSIGNED_8(LEGACY_ALU_MEMORY >> 8u) };
        type_unsigned_8 value = decrement ? 0x80u : 0x7fu;
        type_unsigned_8 observed = 0u;
        legacy_alu_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !legacy_alu_prepare(profiles[profile_index], &state);

        if (!failed) {
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
                VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
            failed |= core_machine_memory_write(state.machine, LEGACY_ALU_MEMORY,
                &value, sizeof(value)) != TYPE_STATUS_OK || !legacy_alu_run(&state,
                code, sizeof(code), 0, &after, &diagnostic) ||
                diagnostic.first_fault.valid || after.data.eip != sizeof(code) ||
                core_machine_memory_read(state.machine, LEGACY_ALU_MEMORY, &observed,
                sizeof(observed)) != TYPE_STATUS_OK || observed != (decrement ?
                0x7fu : 0x80u) || (after.data.eflags & VCPU_EFLAGS_CF) !=
                VCPU_EFLAGS_CF;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    for (decrement = 0u; decrement != 2u; ++decrement) {
        const type_unsigned_8 code[] = { decrement ? 0xc1u : 0xc0u,
            decrement ? 0xe0u : 0xc0u, 1u };
        legacy_alu_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !legacy_alu_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0x11220081u;
            failed |= !legacy_alu_run(&state, code, sizeof(code), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.eip !=
                sizeof(code) || (after.data.eax & (decrement ? 0xffffu : 0xffu)) !=
                (decrement ? 0x0102u : 0x0003u);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT legacy_alu_test_flags_and_sign_forms(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186
    };
    static const type_unsigned_8 flag_codes[] = { 0xf5u, 0xf8u, 0xf9u, 0xfcu, 0xfdu };
    type_unsigned_8 profile_index;
    type_unsigned_8 form;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (form = 0u; form != sizeof(flag_codes); ++form) {
        const type_unsigned_32 initial_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
            VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
        const type_unsigned_32 expected_flags = form == 0u || form == 1u ?
            (initial_flags & ~VCPU_EFLAGS_CF) : (form == 2u ?
            initial_flags : (form == 3u ? (initial_flags & ~VCPU_EFLAGS_DF) :
            initial_flags | VCPU_EFLAGS_DF));
        legacy_alu_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !legacy_alu_prepare(profiles[profile_index], &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0x11223344u;
            state.machine->executor_cpu.data.eflags = initial_flags;
            before = state.machine->executor_cpu;
            failed |= !legacy_alu_run(&state, &flag_codes[form], 1u, 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.eip != 1u ||
                after.data.eax != before.data.eax || after.data.eflags != expected_flags;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (form = 0u; form != 4u; ++form) {
        const type_unsigned_8 code = form < 2u ?
            (form == 0u ? 0x98u : 0x99u) : (form == 2u ? 0x9fu : 0x9eu);
        legacy_alu_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !legacy_alu_prepare(profiles[profile_index], &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = form == 0u ? 0x11220080u :
                (form == 1u ? 0x11228000u : (form == 2u ? 0x11220044u :
                0x1122d744u));
            state.machine->executor_cpu.data.edx = 0x55660000u;
            state.machine->executor_cpu.data.eflags = form == 3u ?
                (VCPU_EFLAGS_OF | VCPU_EFLAGS_IF | VCPU_EFLAGS_DF) :
                (VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_AF |
                VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_IF |
                VCPU_EFLAGS_DF);
            before = state.machine->executor_cpu;
            failed |= !legacy_alu_run(&state, &code, 1u, 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.eip != 1u;
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
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static type_unsigned_16 legacy_alu_shift_result(type_unsigned_8 extension,
    type_unsigned_16 value, type_unsigned_8 count, type_unsigned_8 width,
    type_unsigned_8 *carry)
{
    type_unsigned_8 index;
    const type_unsigned_16 mask = width == 8u ? 0xffu : 0xffffu;
    const type_unsigned_8 high = (type_unsigned_8)(width - 1u);

    for (index = 0u; index != count; ++index) {
        if (extension == 0u) {
            *carry = (value >> high) & 1u;
            value = (type_unsigned_16)(((value << 1u) | *carry) & mask);
        } else if (extension == 1u) {
            *carry = value & 1u;
            value = (type_unsigned_16)((value >> 1u) | (*carry << high));
        } else if (extension == 2u) {
            const type_unsigned_8 next = (value >> high) & 1u;
            value = (type_unsigned_16)(((value << 1u) | *carry) & mask);
            *carry = next;
        } else if (extension == 3u) {
            const type_unsigned_8 next = value & 1u;
            value = (type_unsigned_16)((value >> 1u) | (*carry << high));
            *carry = next;
        } else if (extension == 4u) {
            *carry = (value >> high) & 1u;
            value = (type_unsigned_16)((value << 1u) & mask);
        } else if (extension == 5u) {
            *carry = value & 1u;
            value >>= 1u;
        } else {
            *carry = value & 1u;
            value = (type_unsigned_16)((value >> 1u) | (value & (1u << high)));
        }
    }
    return value;
}

static C_INT legacy_alu_test_group2_forms(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186
    };
    type_unsigned_8 profile_index;
    type_unsigned_8 extension;
    type_unsigned_8 variant;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (extension = 0u; extension != 8u; ++extension)
    for (variant = 0u; variant != 4u; ++variant) {
        const type_unsigned_8 opcode = variant == 0u ? 0xd0u : variant == 1u ?
            0xd1u : variant == 2u ? 0xd2u : 0xd3u;
        const type_unsigned_8 width = variant == 0u || variant == 2u ? 8u : 16u;
        const type_unsigned_8 code[] = { opcode,
            (type_unsigned_8)(0xc0u | (extension << 3u)) };
        const type_unsigned_8 count = variant < 2u ? 1u : 2u;
        type_unsigned_8 carry = 1u;
        const type_unsigned_16 expected = extension == 6u ?
            (width == 8u ? 0x23u : 0x8123u) : legacy_alu_shift_result(extension,
            width == 8u ? 0x23u : 0x8123u, count, width, &carry);
        legacy_alu_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !legacy_alu_prepare(profiles[profile_index], &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0x11228123u;
            state.machine->executor_cpu.data.ecx = 0x55660002u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
                VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
            before = state.machine->executor_cpu;
            failed |= !legacy_alu_run(&state, code, sizeof(code), extension == 6u,
                &after, &diagnostic);
            if (extension == 6u)
                failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                    after.data.eip != 0u || after.data.eax != before.data.eax ||
                    after.data.ecx != before.data.ecx || after.data.eflags !=
                    before.data.eflags;
            else
                failed |= diagnostic.first_fault.valid || after.data.eip != sizeof(code) ||
                    (after.data.eax & (width == 8u ? 0xffu : 0xffffu)) != expected ||
                    (after.data.eflags & VCPU_EFLAGS_CF) != (carry ?
                    VCPU_EFLAGS_CF : 0u) || (after.data.eflags &
                    (VCPU_EFLAGS_IF | VCPU_EFLAGS_DF)) != (before.data.eflags &
                    (VCPU_EFLAGS_IF | VCPU_EFLAGS_DF));
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    for (variant = 0u; variant != 2u; ++variant) {
        const type_unsigned_8 code[] = { variant == 0u ? 0xc0u : 0xc1u,
            0xc0u, 1u };
        legacy_alu_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !legacy_alu_prepare(CORE_MACHINE_CPU_PROFILE_8086, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0x11228123u;
            failed |= !legacy_alu_run(&state, code, sizeof(code), 1, &after,
                &diagnostic) || !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.eip != 0u || after.data.eax != 0x11228123u;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT legacy_alu_test_group2_immediate_extensions(C_VOID)
{
    type_unsigned_8 extension;
    type_unsigned_8 width_index;

    for (width_index = 0u; width_index != 2u; ++width_index)
    for (extension = 0u; extension != 8u; ++extension) {
        const type_unsigned_8 width = width_index == 0u ? 8u : 16u;
        const type_unsigned_8 code[] = { width == 8u ? 0xc0u : 0xc1u,
            (type_unsigned_8)(0xc0u | (extension << 3u)), 2u };
        type_unsigned_8 carry = 1u;
        const type_unsigned_16 expected = extension == 6u ?
            (width == 8u ? 0x23u : 0x8123u) : legacy_alu_shift_result(extension,
            width == 8u ? 0x23u : 0x8123u, 2u, width, &carry);
        legacy_alu_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !legacy_alu_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0x11228123u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
                VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
            before = state.machine->executor_cpu;
            failed |= !legacy_alu_run(&state, code, sizeof(code), extension == 6u,
                &after, &diagnostic);
            if (extension == 6u)
                failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                    after.data.eip != 0u || after.data.eax != before.data.eax ||
                    after.data.eflags != before.data.eflags;
            else
                failed |= diagnostic.first_fault.valid || after.data.eip != sizeof(code) ||
                    (after.data.eax & (width == 8u ? 0xffu : 0xffffu)) != expected ||
                    (after.data.eflags & VCPU_EFLAGS_CF) != (carry ?
                    VCPU_EFLAGS_CF : 0u);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    for (width_index = 0u; width_index != 2u; ++width_index) {
        const type_unsigned_8 code[] = { width_index == 0u ? 0xc0u : 0xc1u,
            0xc0u, 1u };
        legacy_alu_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !legacy_alu_prepare(CORE_MACHINE_CPU_PROFILE_8086, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0x11228123u;
            failed |= !legacy_alu_run(&state, code, sizeof(code), 1, &after,
                &diagnostic) || !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.eip != 0u || after.data.eax != 0x11228123u;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT legacy_alu_test_reserved_and_attribute_rejections(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186
    };
    static const type_unsigned_8 reserved[][2] = {
        { 0xd6u }, { 0xf1u }, { 0xf6u, 0xc8u }, { 0xf7u, 0xc8u }
    };
    static const type_unsigned_8 lengths[] = { 1u, 1u, 2u, 2u };
    static const type_unsigned_8 attributes[][3] = {
        { 0x66u, 0x01u, 0xc8u }, { 0x67u, 0x01u, 0xc8u },
        { 0x66u, 0x67u, 0x01u }
    };
    static const type_unsigned_8 attribute_lengths[] = { 3u, 3u, 4u };
    type_unsigned_8 profile_index;
    type_unsigned_8 form;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (form = 0u; form != sizeof(lengths); ++form) {
        legacy_alu_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !legacy_alu_prepare(profiles[profile_index], &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0x11223344u;
            state.machine->executor_cpu.data.ecx = 0x55667788u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
                VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
            failed |= !legacy_alu_run(&state, reserved[form], lengths[form], 1,
                &after, &diagnostic) || !diagnostic.first_fault.valid ||
                !TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.eip != 0u || after.data.eax != 0x11223344u ||
                after.data.ecx != 0x55667788u || after.data.eflags !=
                (VCPU_EFLAGS_CF | VCPU_EFLAGS_IF | VCPU_EFLAGS_DF);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (form = 0u; form != sizeof(attribute_lengths); ++form) {
        legacy_alu_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !legacy_alu_prepare(profiles[profile_index], &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0x11223344u;
            state.machine->executor_cpu.data.ecx = 0x55667788u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
                VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
            failed |= !legacy_alu_run(&state, attributes[form],
                attribute_lengths[form], 1, &after, &diagnostic) ||
                !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.eip != 0u || after.data.eax != 0x11223344u ||
                after.data.ecx != 0x55667788u || after.data.eflags !=
                (VCPU_EFLAGS_CF | VCPU_EFLAGS_IF | VCPU_EFLAGS_DF);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT legacy_alu_test_divide_error_delivery(C_VOID)
{
    static const type_unsigned_8 code[] = { 0xf6u, 0xf1u };
    static const type_unsigned_8 handler[] = { 0xf4u };
    const type_unsigned_16 code_offset = 0x0200u;
    const type_unsigned_16 handler_offset = 0x0100u;
    type_unsigned_8 profile_index;
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186
    };

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index) {
        legacy_alu_machine state;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_16 frame[3] = { 0u, 0u, 0u };
        C_INT failed = !legacy_alu_prepare(profiles[profile_index], &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 5u;
            state.machine->executor_cpu.data.ecx = 0u;
            state.machine->executor_cpu.data.edx = 0xaabbccddu;
            state.machine->executor_cpu.data.esp = 0x00008000u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
                VCPU_EFLAGS_IF | VCPU_EFLAGS_DF;
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                state.machine, 0u) || core_machine_memory_write(state.machine,
                code_offset, code, sizeof(code)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0u, &handler_offset,
                sizeof(handler_offset)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, handler_offset, handler,
                sizeof(handler)) != TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, code_offset);
            before = state.machine->executor_cpu;
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){ 1u, 0u }, &result) != TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= diagnostic.first_fault.valid ||
                !diagnostic.last_delivered_exception.valid || !TYPE_GET_BIT(
                diagnostic.last_delivered_exception.exception_mask, VCPUINS_EXCEPT_DE) ||
                after.data.eip != handler_offset || after.data.eax != before.data.eax ||
                after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
                after.data.eflags != (before.data.eflags & ~(VCPU_EFLAGS_IF |
                VCPU_EFLAGS_TF)) || after.data.esp !=
                ((before.data.esp & 0xffff0000u) |
                (type_unsigned_16)(before.data.esp - 6u)) ||
                !test_core_machine_fixture_read_linear(state.machine,
                after.data.ss.base + (type_unsigned_16)after.data.esp,
                TYPE_REFERENCE_OF(frame), sizeof(frame)) || frame[0] != code_offset ||
                frame[1] != before.data.cs.selector || (frame[2] &
                legacy_alu_real_flags_known_mask(profiles[profile_index])) !=
                (legacy_alu_real_flags_image(before.data.eflags) &
                    legacy_alu_real_flags_known_mask(
                    profiles[profile_index]));
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

int main(C_VOID)
{
    if (!legacy_alu_test_binary_forms() ||
        !legacy_alu_test_accumulator_immediate_forms() || !legacy_alu_test_group1_forms() ||
        !legacy_alu_test_condition_forms() || !legacy_alu_test_loop_forms() ||
        !legacy_alu_test_test_forms() || !legacy_alu_test_adjust_and_xlat_forms() ||
        !legacy_alu_test_group3_forms() ||
        !legacy_alu_test_inc_dec_and_shift_extensions() ||
        !legacy_alu_test_flags_and_sign_forms() || !legacy_alu_test_group2_forms() ||
        !legacy_alu_test_group2_immediate_extensions() ||
        !legacy_alu_test_reserved_and_attribute_rejections() ||
        !legacy_alu_test_divide_error_delivery()) {
        STD_FPRINTF(stderr, "M5:T338:S2:LEGACY-ALU:FAILED\n");
        return 1;
    }
    STD_PRINTF("M5:T338:S2:LEGACY-ALU:OK\n");
    STD_PRINTF("M5:T401:S34:DECIMAL-ADJUST-PROFILES:OK\n");
    return 0;
}
