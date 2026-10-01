#include "lib/types/types_interface.h"
#include <stdio.h>
#include "support/cpu_instruction_fixture.h"
/* T337_REAL_UD_TERMINAL_CPU_OWNER: original invalid forms stay CPU-owned. */

#define INC_DEC_MEMORY 0x5000u
#define INC_DEC_DEFINED_FLAGS (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | \
    VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF)
#define TEST_DEFINED_FLAGS (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | \
    VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF | VCPU_EFLAGS_CF)
#define MUL_DEFINED_FLAGS (VCPU_EFLAGS_CF | VCPU_EFLAGS_OF)

typedef cpu_instruction_fixture inc_dec_machine;

static lib_i32 inc_dec_prepare(core_machine_cpu_profile profile,
    inc_dec_machine *state)
{
    cpu_instruction_prepare(state, profile);
    return 1;
}

static lib_i32 inc_dec_run(inc_dec_machine *state, const lib_u8 *code,
    lib_size bytes, lib_i32 fault, t_cpu *out,
    core_machine_cpu_diagnostic *diagnostic)
{
    lib_status status;

    if (fault) state->cpu.data.idtr.limit = 0x17u;
    status = cpu_instruction_run(state, code, (lib_u8)bytes, out);
    *diagnostic = (core_machine_cpu_diagnostic){0};
    diagnostic->first_fault = state->fault;
    return status == (fault ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK);
}

static lib_u32 *inc_dec_register(t_cpu *cpu, lib_u8 index)
{
    switch (index) {
    case 0: return &cpu->data.eax;
    case 1: return &cpu->data.ecx;
    case 2: return &cpu->data.edx;
    case 3: return &cpu->data.ebx;
    case 4: return &cpu->data.esp;
    case 5: return &cpu->data.ebp;
    case 6: return &cpu->data.esi;
    case 7: return &cpu->data.edi;
    default: return LIB_NULL;
    }
}

static lib_i32 inc_dec_flags_match(lib_u32 flags, lib_u32 expected,
    lib_u32 preserved)
{
    return (flags & INC_DEC_DEFINED_FLAGS) == expected &&
        (flags & VCPU_EFLAGS_CF) == preserved;
}

static lib_i32 inc_dec_test_register_forms(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_u8 profile;
    lib_u8 index;
    lib_u8 decrement;
    lib_u8 operand32;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (index = 0u; index != 8u; ++index)
    for (decrement = 0u; decrement != 2u; ++decrement)
    for (operand32 = 0u; operand32 != (profiles[profile] ==
        CORE_MACHINE_CPU_PROFILE_80386 ? 2u : 1u); ++operand32) {
        lib_u8 code[2] = {(lib_u8)(0x40u + index + decrement * 8u),0u};
        const lib_u32 before = decrement ? (operand32 ? 0x80000000u :
            0xaabb8000u) : (operand32 ? 0x7fffffffu : 0xaabb7fffu);
        const lib_u32 expected = decrement ? (operand32 ? 0x7fffffffu :
            0xaabb7fffu) : (operand32 ? 0x80000000u : 0xaabb8000u);
        const lib_u32 flags = decrement ?
            (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF) :
            (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
        inc_dec_machine state;
        t_cpu after = {0};
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 *reg;
        lib_i32 failed = !inc_dec_prepare(profiles[profile], &state);

        if (operand32) { code[0] = 0x66u; code[1] = (lib_u8)(0x40u + index + decrement * 8u); }
        if (!failed) {
            reg = inc_dec_register(&state.cpu, index);
            *reg = before;
            state.cpu.data.eflags = VCPU_EFLAGS_CF;
            failed |= !inc_dec_run(&state, code, operand32 ? 2u : 1u, 0,
                &after, &diagnostic) || diagnostic.first_fault.valid ||
                *inc_dec_register(&after, index) != expected ||
                !inc_dec_flags_match(after.data.eflags, flags, VCPU_EFLAGS_CF);
        }
        if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_rm_forms(void)
{
    static const lib_u8 forms[][6] = {
        { 0xfeu, 0xc0u }, { 0xfeu, 0xc8u }, { 0xffu, 0xc0u }, { 0xffu, 0xc8u },
        { 0xfeu, 0x06u, 0x00u, 0x50u }, { 0xfeu, 0x0eu, 0x00u, 0x50u },
        { 0xffu, 0x06u, 0x00u, 0x50u }, { 0xffu, 0x0eu, 0x00u, 0x50u },
        { 0x66u, 0xffu, 0x06u, 0x00u, 0x50u },
        { 0x66u, 0xffu, 0x0eu, 0x00u, 0x50u }
    };
    static const lib_u8 lengths[] = { 2u, 2u, 2u, 2u, 4u, 4u, 4u, 4u, 5u, 5u };
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_u8 profile;
    lib_u8 form;
    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (form = 0u; form != sizeof(lengths); ++form) {
        if (form >= 8u && profiles[profile] != CORE_MACHINE_CPU_PROFILE_80386) continue;
        const lib_i32 decrement = (form & 1u) != 0u;
        const lib_u8 bytes = form < 2u || (form >= 4u && form < 6u) ? 1u :
            (form < 8u ? 2u : 4u);
        const lib_u32 before = decrement ? (bytes == 1u ? 0x80u :
            (bytes == 2u ? 0x8000u : 0x80000000u)) : (bytes == 1u ? 0x7fu :
            (bytes == 2u ? 0x7fffu : 0x7fffffffu));
        const lib_u32 expected = decrement ? (before - 1u) : (before + 1u);
        const lib_u32 flags = decrement ?
            (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF |
                (bytes == 1u ? 0u : VCPU_EFLAGS_PF)) :
            (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF |
                (bytes == 1u ? 0u : VCPU_EFLAGS_PF));
        inc_dec_machine state;
        t_cpu after = {0};
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 observed = 0u;
        lib_i32 failed = !inc_dec_prepare(profiles[profile], &state);

        if (!failed) {
            state.cpu.data.eax = before;
            state.cpu.data.eflags = decrement ? 0u : VCPU_EFLAGS_CF;
            if (form >= 4u) failed |= cpu_instruction_write(&state,
                INC_DEC_MEMORY, &before, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                !inc_dec_flags_match(after.data.eflags, flags,
                    decrement ? 0u : VCPU_EFLAGS_CF);
            if (form < 4u) {
                const lib_u32 mask = bytes == 1u ? 0xffu : 0xffffu;
                failed |= (after.data.eax & mask) != expected;
            } else {
                failed |= cpu_instruction_read(&state, INC_DEC_MEMORY,
                    &observed, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || observed != expected;
            }
        }
        if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_address_and_profile(void)
{
    static const lib_u8 address_code[] = { 0x67u,0x66u,0xffu,0x06u,
        0x00u,0x50u,0x00u,0x00u };
    static const lib_u8 rejected_prefixes[][2] = {
        {0x66u,0x40u}, {0x66u,0x48u}
    };
    static const core_machine_cpu_profile legacy_profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 accepted_legacy[] = { 0xffu,0xc0u };
    inc_dec_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    lib_u32 value = 0x7fffffffu;
    lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.cpu.data.esi = INC_DEC_MEMORY;
        state.cpu.data.eflags = VCPU_EFLAGS_CF;
        failed |= cpu_instruction_write(&state, INC_DEC_MEMORY,
            &value, sizeof(value), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            !inc_dec_run(&state, address_code, sizeof(address_code), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
            cpu_instruction_read(&state, INC_DEC_MEMORY,
                &value, sizeof(value), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
            value != 0x80000000u || !inc_dec_flags_match(after.data.eflags,
                VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF |
                VCPU_EFLAGS_PF, VCPU_EFLAGS_CF);
    }
    if (failed) return 0;

    {
        lib_u8 profile;
        lib_u8 opcode;
        for (profile = 0u; profile != sizeof(legacy_profiles) / sizeof(legacy_profiles[0]);
            ++profile)
        for (opcode = 0u; opcode != sizeof(rejected_prefixes) / sizeof(rejected_prefixes[0]);
            ++opcode) {
            failed = !inc_dec_prepare(legacy_profiles[profile], &state);
            if (!failed) {
                state.cpu.data.eax = 0x11227fffu;
                state.cpu.data.eflags = VCPU_EFLAGS_CF;
                failed |= !inc_dec_run(&state, rejected_prefixes[opcode],
                    sizeof(rejected_prefixes[opcode]), 1, &after, &diagnostic) ||
                    !diagnostic.first_fault.valid || !X86_CPU_BIT_IS_SET(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                    after.data.eax != 0x11227fffu ||
                    after.data.eflags != VCPU_EFLAGS_CF || after.data.eip != 0u;
            }
            if (failed) return 0;
        }
    }

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);
    if (!failed) {
        state.cpu.data.eax = 0xaabb7fffu;
        state.cpu.data.eflags = VCPU_EFLAGS_CF;
        failed |= !inc_dec_run(&state, accepted_legacy, sizeof(accepted_legacy), 0,
            &after, &diagnostic) || diagnostic.first_fault.valid ||
            after.data.eax != 0xaabb8000u || !inc_dec_flags_match(after.data.eflags,
                VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF |
                VCPU_EFLAGS_PF, VCPU_EFLAGS_CF);
    }
    return !failed;
}

static lib_i32 inc_dec_test_not_neg_forms(void)
{
    static const lib_u8 forms[][6] = {
        { 0xf6u,0xd0u }, { 0xf6u,0xd8u }, { 0xf7u,0xd0u }, { 0xf7u,0xd8u },
        { 0x66u,0xf7u,0xd0u }, { 0x66u,0xf7u,0xd8u },
        { 0xf6u,0x16u,0x00u,0x50u }, { 0xf6u,0x1eu,0x00u,0x50u },
        { 0xf7u,0x16u,0x00u,0x50u }, { 0xf7u,0x1eu,0x00u,0x50u },
        { 0x66u,0xf7u,0x16u,0x00u,0x50u },
        { 0x66u,0xf7u,0x1eu,0x00u,0x50u }
    };
    static const lib_u8 lengths[] = { 2u,2u,2u,2u,3u,3u,4u,4u,4u,4u,5u,5u };
    const lib_u32 saved_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF;
    lib_u8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const lib_i32 negate = (form & 1u) != 0u;
        const lib_u8 bytes = form == 0u || form == 1u || form == 6u || form == 7u ?
            1u : (form == 2u || form == 3u || form == 8u || form == 9u ? 2u : 4u);
        const lib_u32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const lib_u32 before = negate ? (bytes == 1u ? 0x80u :
            (bytes == 2u ? 0x8000u : 0x80000000u)) :
            (bytes == 1u ? 0x5au : (bytes == 2u ? 0xa55au : 0x5aa55aa5u));
        const lib_u32 expected = negate ? (0u - before) & mask : (~before) & mask;
        const lib_u32 neg_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
            VCPU_EFLAGS_SF | (bytes == 1u ? 0u : VCPU_EFLAGS_PF);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 observed = 0u;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.eax = before;
            state.cpu.data.eflags = saved_flags;
            if (form >= 6u) failed |= cpu_instruction_write(&state,
                INC_DEC_MEMORY, &before, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid;
            if (negate) {
                failed |= (after.data.eflags & (INC_DEC_DEFINED_FLAGS | VCPU_EFLAGS_CF)) !=
                    neg_flags;
            } else {
                failed |= after.data.eflags != saved_flags;
            }
            if (form < 6u) {
                failed |= (after.data.eax & mask) != expected;
            } else {
                failed |= cpu_instruction_read(&state, INC_DEC_MEMORY,
                    &observed, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || observed != expected;
            }
        }
        if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_not_neg_address_and_profile(void)
{
    static const lib_u8 address_code[] = { 0x67u,0x66u,0xf7u,0x1eu,
        0x00u,0x50u,0x00u,0x00u };
    static const lib_u8 rejected_prefix[] = { 0x66u,0xf7u,0xd0u };
    static const lib_u8 accepted_legacy[] = { 0xf7u,0xd0u };
    const lib_u32 saved_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF;
    inc_dec_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    lib_u32 value = 0x80000000u;
    lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.cpu.data.esi = INC_DEC_MEMORY;
        state.cpu.data.eflags = saved_flags;
        failed |= cpu_instruction_write(&state, INC_DEC_MEMORY,
            &value, sizeof(value), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            !inc_dec_run(&state, address_code, sizeof(address_code), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
            cpu_instruction_read(&state, INC_DEC_MEMORY, &value,
                sizeof(value), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || value != 0x80000000u ||
            (after.data.eflags & (INC_DEC_DEFINED_FLAGS | VCPU_EFLAGS_CF)) !=
                (VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_SF |
                    VCPU_EFLAGS_PF);
    }
    if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);
    if (!failed) {
        state.cpu.data.eax = 0x11225aa5u;
        state.cpu.data.eflags = saved_flags;
        failed |= !inc_dec_run(&state, rejected_prefix, sizeof(rejected_prefix), 1,
            &after, &diagnostic) || !diagnostic.first_fault.valid || !X86_CPU_BIT_IS_SET(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eax != 0x11225aa5u || after.data.eflags != saved_flags ||
            after.data.eip != 0u;
    }
    if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);
    if (!failed) {
        state.cpu.data.eax = 0xaabb5aa5u;
        state.cpu.data.eflags = saved_flags;
        failed |= !inc_dec_run(&state, accepted_legacy, sizeof(accepted_legacy), 0,
            &after, &diagnostic) || diagnostic.first_fault.valid ||
            after.data.eax != 0xaabba55au || after.data.eflags != saved_flags;
    }
    return !failed;
}

static lib_i32 inc_dec_test_test_forms(void)
{
    static const lib_u8 forms[][9] = {
        { 0xa8u,0x80u }, { 0xa9u,0u,0x80u },
        { 0x66u,0xa9u,0u,0u,0u,0x80u },
        { 0xf6u,0xc0u,0x80u }, { 0xf7u,0xc0u,0u,0x80u },
        { 0x66u,0xf7u,0xc0u,0u,0u,0u,0x80u },
        { 0xf6u,0x06u,0u,0x50u,0x80u },
        { 0xf7u,0x06u,0u,0x50u,0u,0x80u },
        { 0x66u,0xf7u,0x06u,0u,0x50u,0u,0u,0u,0x80u }
    };
    static const lib_u8 lengths[] = { 2u,3u,6u,3u,4u,7u,5u,6u,9u };
    const lib_u32 initial_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF;
    lib_u8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const lib_u8 bytes = form == 0u || form == 3u || form == 6u ? 1u :
            (form == 1u || form == 4u || form == 7u ? 2u : 4u);
        const lib_u32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const lib_u32 expected_flags = VCPU_EFLAGS_SF |
            (bytes == 1u ? 0u : VCPU_EFLAGS_PF);
        const lib_i32 memory = form >= 6u;
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 before = 0xffffffffu;
        lib_u32 observed = 0u;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.eax = before;
            state.cpu.data.eflags = initial_flags;
            if (memory) failed |= cpu_instruction_write(&state,
                INC_DEC_MEMORY, &before, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                after.data.eax != before ||
                (after.data.eflags & TEST_DEFINED_FLAGS) != expected_flags;
            if (memory) failed |= cpu_instruction_read(&state,
                INC_DEC_MEMORY, &observed, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
                observed != (before & mask);
        }
        if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_accumulator_profiles(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 forms[][3] = {
        {0xa8u, 0x80u, 0u}, {0xa9u, 0u, 0x80u}
    };
    static const lib_u8 lengths[] = {2u, 3u};
    static const lib_u8 dword[] = {0x66u, 0xa9u, 0u, 0u, 0u, 0x80u};
    const lib_u32 initial_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF;
    lib_u8 profile;
    lib_u8 form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (form = 0u; form != sizeof(lengths); ++form) {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 expected = form == 0u ? VCPU_EFLAGS_SF :
            VCPU_EFLAGS_SF | VCPU_EFLAGS_PF;
        lib_i32 failed = !inc_dec_prepare(profiles[profile], &state);

        if (!failed) {
            state.cpu.data.eax = 0xaabbffffu;
            state.cpu.data.eflags = initial_flags;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.eip !=
                lengths[form] || after.data.eax != 0xaabbffffu ||
                (after.data.eflags & TEST_DEFINED_FLAGS) != expected ||
                (after.data.eflags & ~TEST_DEFINED_FLAGS) !=
                (initial_flags & ~TEST_DEFINED_FLAGS);
        }
        if (failed)
            return 0;
    }
    for (profile = 0u; profile != 3u; ++profile) {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !inc_dec_prepare(profiles[profile], &state);

        if (!failed) {
            state.cpu.data.eax = 0xaabbffffu;
            state.cpu.data.eflags = initial_flags;
            failed |= !inc_dec_run(&state, dword, sizeof(dword), 1, &after,
                &diagnostic) || !diagnostic.first_fault.valid || !X86_CPU_BIT_IS_SET(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.eip != 0u || after.data.eax != 0xaabbffffu ||
                after.data.eflags != initial_flags;
        }
        if (failed)
            return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.eax = 0xffffffffu;
            state.cpu.data.eflags = initial_flags;
            failed |= !inc_dec_run(&state, dword, sizeof(dword), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.eip !=
                sizeof(dword) || after.data.eax != 0xffffffffu ||
                (after.data.eflags & TEST_DEFINED_FLAGS) !=
                (VCPU_EFLAGS_SF | VCPU_EFLAGS_PF) ||
                (after.data.eflags & ~TEST_DEFINED_FLAGS) !=
                (initial_flags & ~TEST_DEFINED_FLAGS);
        }
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_test_address_and_profile(void)
{
    static const lib_u8 address_code[] = { 0x67u,0x66u,0xf7u,0x06u,
        0u,0u,0u,0x80u };
    static const lib_u8 rejected_prefix[] = { 0x66u,0xa9u,0u,0u,0u,0x80u };
    static const lib_u8 accepted_legacy[] = { 0xa9u,0u,0x80u };
    const lib_u32 initial_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF;
    inc_dec_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    lib_u32 value = 0xffffffffu;
    lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.cpu.data.esi = INC_DEC_MEMORY;
        state.cpu.data.eax = value;
        state.cpu.data.eflags = initial_flags;
        failed |= cpu_instruction_write(&state, INC_DEC_MEMORY,
            &value, sizeof(value), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            !inc_dec_run(&state, address_code, sizeof(address_code), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.eax != value ||
            cpu_instruction_read(&state, INC_DEC_MEMORY, &value,
                sizeof(value), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || value != 0xffffffffu ||
            (after.data.eflags & TEST_DEFINED_FLAGS) !=
                (VCPU_EFLAGS_SF | VCPU_EFLAGS_PF);
    }
    if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);
    if (!failed) {
        state.cpu.data.eax = 0x1122ffffu;
        state.cpu.data.eflags = initial_flags;
        failed |= !inc_dec_run(&state, rejected_prefix, sizeof(rejected_prefix), 1,
            &after, &diagnostic) || !diagnostic.first_fault.valid || !X86_CPU_BIT_IS_SET(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eax != 0x1122ffffu || after.data.eflags != initial_flags ||
            after.data.eip != 0u;
    }
    if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);
    if (!failed) {
        state.cpu.data.eax = 0xaabbffffu;
        state.cpu.data.eflags = initial_flags;
        failed |= !inc_dec_run(&state, accepted_legacy, sizeof(accepted_legacy), 0,
            &after, &diagnostic) || diagnostic.first_fault.valid ||
            after.data.eax != 0xaabbffffu ||
            (after.data.eflags & TEST_DEFINED_FLAGS) !=
                (VCPU_EFLAGS_SF | VCPU_EFLAGS_PF);
    }
    return !failed;
}

static lib_i32 inc_dec_test_mul_imul_forms(void)
{
    static const lib_u8 forms[][5] = {
        { 0xf6u,0xe1u }, { 0xf6u,0xe9u }, { 0xf7u,0xe1u }, { 0xf7u,0xe9u },
        { 0x66u,0xf7u,0xe1u }, { 0x66u,0xf7u,0xe9u },
        { 0xf6u,0x26u,0u,0x50u }, { 0xf6u,0x2eu,0u,0x50u },
        { 0xf7u,0x26u,0u,0x50u }, { 0xf7u,0x2eu,0u,0x50u },
        { 0x66u,0xf7u,0x26u,0u,0x50u },
        { 0x66u,0xf7u,0x2eu,0u,0x50u }
    };
    static const lib_u8 lengths[] = { 2u,2u,2u,2u,3u,3u,4u,4u,4u,4u,5u,5u };
    lib_u8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const lib_i32 signed_multiply = (form & 1u) != 0u;
        const lib_i32 memory = form >= 6u;
        const lib_u8 bytes = form == 0u || form == 1u || form == 6u || form == 7u ?
            1u : (form == 2u || form == 3u || form == 8u || form == 9u ? 2u : 4u);
        const lib_u32 source = memory ? 2u : 2u;
        const lib_u32 accumulator = memory ? (bytes == 1u ? 0x80u :
            (bytes == 2u ? 0x8000u : 0x80000000u)) : 2u;
        const lib_u32 expected_lo = memory ? (bytes == 1u ? (signed_multiply ?
            0xff00u : 0x0100u) : 0u) : 4u;
        const lib_u32 expected_hi = memory ? (bytes == 1u ? 0u :
            (signed_multiply ? (bytes == 2u ? 0xffffu : 0xffffffffu) : 1u)) : 0u;
        const lib_u32 expected_flags = memory ? MUL_DEFINED_FLAGS : 0u;
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 observed = 0u;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.eax = accumulator;
            state.cpu.data.ecx = source;
            state.cpu.data.edx = 0xaabbccddu;
            state.cpu.data.eflags = MUL_DEFINED_FLAGS;
            if (memory) failed |= cpu_instruction_write(&state,
                INC_DEC_MEMORY, &source, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                (after.data.eflags & MUL_DEFINED_FLAGS) != expected_flags;
            if (bytes == 1u) {
                failed |= after.data.ax != expected_lo;
            } else if (bytes == 2u) {
                failed |= after.data.ax != expected_lo || after.data.dx != expected_hi;
            } else {
                failed |= after.data.eax != expected_lo || after.data.edx != expected_hi;
            }
            if (memory) failed |= cpu_instruction_read(&state,
                INC_DEC_MEMORY, &observed, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || observed != source;
        }
        if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_imul_sign_extension_profiles(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_u8 profile_index;
    lib_u8 width_index;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (width_index = 0u; width_index != (profiles[profile_index] ==
        CORE_MACHINE_CPU_PROFILE_80386 ? 3u : 2u); ++width_index) {
        static const lib_u8 code[][3] = {
            { 0xf6u, 0xe9u }, { 0xf7u, 0xe9u }, { 0x66u, 0xf7u, 0xe9u }
        };
        const lib_u8 bytes = width_index == 0u ? 1u :
            (width_index == 1u ? 2u : 4u);
        const lib_u32 initial_eax = bytes == 1u ? 0x112233ffu :
            (bytes == 2u ? 0x1122ffffu : 0xffffffffu);
        const lib_u32 initial_edx = 0xaabbccddu;
        const lib_u32 expected_eax = bytes != 4u ? 0x1122ffffu :
            0xffffffffu;
        const lib_u32 expected_edx = bytes == 1u ? initial_edx :
            (bytes == 2u ? 0xaabbffffu : 0xffffffffu);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_i32 failed = !inc_dec_prepare(profiles[profile_index], &state);

        if (!failed) {
            state.cpu.data.eax = initial_eax;
            state.cpu.data.ecx = 1u;
            state.cpu.data.edx = initial_edx;
            state.cpu.data.eflags = MUL_DEFINED_FLAGS |
                VCPU_EFLAGS_ZF;
            failed |= !inc_dec_run(&state, code[width_index], bytes == 4u ?
                3u : 2u, 0, &after, &diagnostic) || diagnostic.first_fault.valid ||
                after.data.eax != expected_eax || after.data.edx != expected_edx ||
                (after.data.eflags & MUL_DEFINED_FLAGS) != 0u ||
                after.data.cx != 1u;
        }
        if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_mul_imul_address_and_profile(void)
{
    static const lib_u8 address_code[] = { 0x67u,0x66u,0xf7u,0x2eu };
    static const lib_u8 rejected_prefix[] = { 0x66u,0xf7u,0xe0u };
    static const lib_u8 accepted_legacy[] = { 0xf7u,0xe0u };
    inc_dec_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    lib_u32 source = 2u;
    lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.cpu.data.esi = INC_DEC_MEMORY;
        state.cpu.data.eax = 0x80000000u;
        state.cpu.data.edx = 0x11223344u;
        state.cpu.data.eflags = 0u;
        failed |= cpu_instruction_write(&state, INC_DEC_MEMORY, &source,
            sizeof(source), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK || !inc_dec_run(&state, address_code,
                sizeof(address_code), 0, &after, &diagnostic) ||
            diagnostic.first_fault.valid || after.data.eax != 0u ||
            after.data.edx != 0xffffffffu ||
            (after.data.eflags & MUL_DEFINED_FLAGS) != MUL_DEFINED_FLAGS ||
            cpu_instruction_read(&state, INC_DEC_MEMORY, &source,
                sizeof(source), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK || source != 2u;
    }
    if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);
    if (!failed) {
        state.cpu.data.eax = 0x11220002u;
        state.cpu.data.edx = 0xaabbccddu;
        state.cpu.data.eflags = MUL_DEFINED_FLAGS;
        failed |= !inc_dec_run(&state, rejected_prefix, sizeof(rejected_prefix), 1,
            &after, &diagnostic) || !diagnostic.first_fault.valid || !X86_CPU_BIT_IS_SET(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eax != 0x11220002u || after.data.edx != 0xaabbccddu ||
            after.data.eflags != MUL_DEFINED_FLAGS || after.data.eip != 0u;
    }
    if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);
    if (!failed) {
        state.cpu.data.eax = 0xaabb0002u;
        state.cpu.data.edx = 0x11223344u;
        state.cpu.data.eflags = MUL_DEFINED_FLAGS;
        failed |= !inc_dec_run(&state, accepted_legacy, sizeof(accepted_legacy), 0,
            &after, &diagnostic) || diagnostic.first_fault.valid ||
            after.data.ax != 4u || after.data.dx != 0u ||
            (after.data.eflags & MUL_DEFINED_FLAGS) != 0u;
    }
    return !failed;
}

static lib_i32 inc_dec_test_div_idiv_forms(void)
{
    static const lib_u8 forms[][5] = {
        { 0xf6u,0xf1u }, { 0xf6u,0xf9u }, { 0xf7u,0xf1u }, { 0xf7u,0xf9u },
        { 0x66u,0xf7u,0xf1u }, { 0x66u,0xf7u,0xf9u },
        { 0xf6u,0x36u,0u,0x50u }, { 0xf6u,0x3eu,0u,0x50u },
        { 0xf7u,0x36u,0u,0x50u }, { 0xf7u,0x3eu,0u,0x50u },
        { 0x66u,0xf7u,0x36u,0u,0x50u },
        { 0x66u,0xf7u,0x3eu,0u,0x50u }
    };
    static const lib_u8 lengths[] = { 2u,2u,2u,2u,3u,3u,4u,4u,4u,4u,5u,5u };
    lib_u8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const lib_i32 signed_divide = (form & 1u) != 0u;
        const lib_i32 memory = form >= 6u;
        const lib_u8 bytes = form == 0u || form == 1u || form == 6u || form == 7u ?
            1u : (form == 2u || form == 3u || form == 8u || form == 9u ? 2u : 4u);
        const lib_u32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const lib_u32 source = signed_divide && memory ? mask - 1u : 2u;
        const lib_u32 quotient = signed_divide ? (memory ? 2u : mask - 1u) : 2u;
        const lib_u32 remainder = signed_divide ? mask : 1u;
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_u32 observed = 0u;
        lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.cpu.data.eax = signed_divide ?
                (bytes == 1u ? 0xfffbu : mask - 4u) : 5u;
            state.cpu.data.edx = signed_divide && bytes != 1u ? mask : 0u;
            state.cpu.data.ecx = source;
            state.cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
            if (memory) failed |= cpu_instruction_write(&state,
                INC_DEC_MEMORY, &source, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid;
            if (bytes == 1u) {
                failed |= after.data.al != quotient || after.data.ah != remainder;
            } else if (bytes == 2u) {
                failed |= after.data.ax != quotient || after.data.dx != remainder;
            } else {
                failed |= after.data.eax != quotient || after.data.edx != remainder;
            }
            if (memory) failed |= cpu_instruction_read(&state,
                INC_DEC_MEMORY, &observed, bytes, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
                observed != (source & mask);
        }
        if (failed) return 0;
    }
    return 1;
}

static lib_i32 inc_dec_test_div_idiv_attribute_and_profile(void)
{
    static const lib_u8 address_code[] = { 0x67u,0x66u,0xf7u,0x3eu };
    static const lib_u8 rejected_prefix[] = { 0x66u,0xf7u,0xf1u };
    static const lib_u8 accepted_legacy[] = { 0xf7u,0xf1u };
    inc_dec_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    lib_u32 source = 0xfffffffeu;
    lib_i32 failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.cpu.data.esi = INC_DEC_MEMORY;
        state.cpu.data.eax = 0xfffffffbu;
        state.cpu.data.edx = 0xffffffffu;
        failed |= cpu_instruction_write(&state, INC_DEC_MEMORY, &source,
            sizeof(source), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK || !inc_dec_run(&state, address_code,
                sizeof(address_code), 0, &after, &diagnostic) ||
            diagnostic.first_fault.valid || after.data.eax != 2u ||
            after.data.edx != 0xffffffffu;
    }
    if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);
    if (!failed) {
        state.cpu.data.eax = 0x11220005u;
        state.cpu.data.edx = 0xaabbccddu;
        state.cpu.data.ecx = 2u;
        state.cpu.data.eflags = VCPU_EFLAGS_CF;
        failed |= !inc_dec_run(&state, rejected_prefix, sizeof(rejected_prefix), 1,
            &after, &diagnostic) || !diagnostic.first_fault.valid || !X86_CPU_BIT_IS_SET(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eax != 0x11220005u || after.data.edx != 0xaabbccddu ||
            after.data.ecx != 2u || after.data.eflags != VCPU_EFLAGS_CF ||
            after.data.eip != 0u;
    }
    if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);
    if (!failed) {
        state.cpu.data.eax = 5u;
        state.cpu.data.edx = 0u;
        state.cpu.data.ecx = 2u;
        failed |= !inc_dec_run(&state, accepted_legacy, sizeof(accepted_legacy), 0,
            &after, &diagnostic) || diagnostic.first_fault.valid ||
            after.data.ax != 2u || after.data.dx != 1u;
    }
    return !failed;
}

int main(void)
{
    if (!inc_dec_test_register_forms() ||
        !inc_dec_test_rm_forms() ||
        !inc_dec_test_address_and_profile() ||
        !inc_dec_test_not_neg_forms() ||
        !inc_dec_test_not_neg_address_and_profile() ||
        !inc_dec_test_test_forms() ||
        !inc_dec_test_accumulator_profiles() ||
        !inc_dec_test_test_address_and_profile() ||
        !inc_dec_test_mul_imul_forms() ||
        !inc_dec_test_imul_sign_extension_profiles() ||
        !inc_dec_test_mul_imul_address_and_profile() ||
        !inc_dec_test_div_idiv_forms() ||
        !inc_dec_test_div_idiv_attribute_and_profile()) {
        fputs("M5:T539:S33:CPU-INC-DEC-GROUP:FAIL\n", stderr);
        return 1;
    }
    puts("M5:T316:S2:INC-DEC:OK");
    puts("M5:T316:S3:NOT-NEG:OK");
    puts("M5:T316:S4:TEST:OK");
    puts("M5:T316:S5:MUL-IMUL:OK");
    puts("M5:T401:S9:IMUL-SIGN-EXTENSION-PROFILES:OK");
    puts("M5:T316:S6:DIV-IDIV:OK");
    puts("M5:T401:S10:GROUP45-INC-DEC-PROFILES:OK");
    puts("M5:T401:S11:PRIMARY-INC-DEC-PROFILES:OK");
    puts("M5:T401:S20:ACCUMULATOR-TEST-PROFILES:OK");
    puts("M5:T539:S33:CPU-INC-DEC-GROUP:OK");
    return 0;
}
