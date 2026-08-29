#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define INC_DEC_MEMORY 0x5000u
#define INC_DEC_DEFINED_FLAGS (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | \
    VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF)
#define TEST_DEFINED_FLAGS (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | \
    VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF | VCPU_EFLAGS_CF)
#define OR_DEFINED_FLAGS (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | \
    VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF | VCPU_EFLAGS_CF)
#define AND_DEFINED_FLAGS (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | \
    VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF | VCPU_EFLAGS_CF)
#define MUL_DEFINED_FLAGS (VCPU_EFLAGS_CF | VCPU_EFLAGS_OF)
#define ADD_DEFINED_FLAGS (VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | \
    VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF)

typedef struct inc_dec_machine { core_machine *machine; } inc_dec_machine;

static C_VOID inc_dec_reset(C_VOID *opaque)
{
    inc_dec_machine *state = (inc_dec_machine *)opaque;
    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider inc_dec_provider = {
    inc_dec_reset, STD_NULL
};

static C_INT inc_dec_prepare(core_machine_cpu_profile profile, inc_dec_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    if (!test_core_machine_fixture_create_bind_freeze_reset(&config,
            &inc_dec_provider, state, &state->machine)) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT inc_dec_run(inc_dec_machine *state, const type_unsigned_8 *code,
    STD_SIZE_T bytes, C_INT fault, t_cpu *out,
    core_machine_cpu_diagnostic *diagnostic)
{
    core_machine_run_result result;
    type_status status;
    if (state == STD_NULL || state->machine == STD_NULL ||
        !test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u) ||
        core_machine_memory_write(state->machine, 0u, code, bytes) != TYPE_STATUS_OK)
        return 0;
    if (fault && !test_core_machine_fixture_preflight_real_ud_terminal(
            state->machine)) return 0;
    status = core_machine_run(state->machine, (core_machine_run_budget){ 1u, 0u },
        &result);
    if (status != (fault ? TYPE_STATUS_FAULT : TYPE_STATUS_OK) || result.reason !=
        (fault ? CORE_MACHINE_STOP_FAULT : CORE_MACHINE_STOP_BUDGET) ||
        core_machine_get_cpu_diagnostic(state->machine, diagnostic) != TYPE_STATUS_OK)
        return 0;
    *out = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static C_INT inc_dec_run_delivered_de(inc_dec_machine *state,
    const type_unsigned_8 *code, STD_SIZE_T bytes, t_cpu *out,
    core_machine_cpu_diagnostic *diagnostic)
{
    static const type_unsigned_8 handler[] = { 0xf4u };
    const type_unsigned_16 handler_offset = 0x0100u;
    const type_unsigned_16 code_offset = 0x0200u;
    const type_unsigned_16 handler_segment = 0u;
    core_machine_run_result result;
    t_cpu before;
    const type_unsigned_8 frame_width = code[0] == 0x66u ? 4u : 2u;
    type_unsigned_16 frame16[3] = { 0u, 0u, 0u };
    type_unsigned_32 frame32[3] = { 0u, 0u, 0u };

    if (state == STD_NULL || state->machine == STD_NULL ||
        !test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u) ||
        core_machine_memory_write(state->machine, code_offset, code, bytes) !=
            TYPE_STATUS_OK || core_machine_memory_write(state->machine, 0u,
            &handler_offset, sizeof(handler_offset)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, 2u, &handler_segment,
            sizeof(handler_segment)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, handler_offset, handler,
            sizeof(handler)) != TYPE_STATUS_OK) {
        return 0;
    }
    test_core_machine_fixture_resume_after_halt_at(state->machine, code_offset);
    state->machine->executor_cpu.data.esp = 0x00008000u;
    before = state->machine->executor_cpu;
    if (core_machine_run(state->machine, (core_machine_run_budget){ 1u, 0u },
            &result) != TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
        core_machine_get_cpu_diagnostic(state->machine, diagnostic) !=
            TYPE_STATUS_OK) {
        return 0;
    }
    *out = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    if (!(!diagnostic->first_fault.valid &&
        diagnostic->last_delivered_exception.valid && TYPE_GET_BIT(
            diagnostic->last_delivered_exception.exception_mask,
            VCPUINS_EXCEPT_DE) && out->data.eip == handler_offset &&
        out->data.esp == ((before.data.esp & 0xffff0000u) |
            (type_unsigned_16)(before.data.esp - 3u * frame_width)))) {
        return 0;
    }
    if (frame_width == 2u) {
        return test_core_machine_fixture_read_linear(state->machine,
            out->data.ss.base + (type_unsigned_16)out->data.esp,
            TYPE_REFERENCE_OF(frame16), sizeof(frame16)) &&
            frame16[0] == code_offset && frame16[1] == before.data.cs.selector &&
            frame16[2] == (type_unsigned_16)((before.data.eflags &
                ~VCPU_EFLAGS_RESERVED) | 0x02u);
    }
    return test_core_machine_fixture_read_linear(state->machine,
        out->data.ss.base + out->data.esp, TYPE_REFERENCE_OF(frame32),
        sizeof(frame32)) && frame32[0] == code_offset &&
        frame32[1] == before.data.cs.selector && frame32[2] ==
            ((before.data.eflags & ~VCPU_EFLAGS_RESERVED) | 0x02u);
    return 1;
}

static type_unsigned_32 *inc_dec_register(t_cpu *cpu, type_unsigned_8 index)
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
    default: return STD_NULL;
    }
}

static C_INT inc_dec_flags_match(type_unsigned_32 flags, type_unsigned_32 expected,
    type_unsigned_32 preserved)
{
    return (flags & INC_DEC_DEFINED_FLAGS) == expected &&
        (flags & VCPU_EFLAGS_CF) == preserved;
}

static C_INT inc_dec_test_register_forms(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;
    type_unsigned_8 index;
    type_unsigned_8 decrement;
    type_unsigned_8 operand32;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (index = 0u; index != 8u; ++index)
    for (decrement = 0u; decrement != 2u; ++decrement)
    for (operand32 = 0u; operand32 != (profiles[profile] ==
        CORE_MACHINE_CPU_PROFILE_80386 ? 2u : 1u); ++operand32) {
        type_unsigned_8 code[2] = {(type_unsigned_8)(0x40u + index + decrement * 8u),0u};
        const type_unsigned_32 before = decrement ? (operand32 ? 0x80000000u :
            0xaabb8000u) : (operand32 ? 0x7fffffffu : 0xaabb7fffu);
        const type_unsigned_32 expected = decrement ? (operand32 ? 0x7fffffffu :
            0xaabb7fffu) : (operand32 ? 0x80000000u : 0xaabb8000u);
        const type_unsigned_32 flags = decrement ?
            (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF) :
            (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
        inc_dec_machine state;
        t_cpu after = {0};
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 *reg;
        C_INT failed = !inc_dec_prepare(profiles[profile], &state);

        if (operand32) { code[0] = 0x66u; code[1] = (type_unsigned_8)(0x40u + index + decrement * 8u); }
        if (!failed) {
            reg = inc_dec_register(&state.machine->executor_cpu, index);
            *reg = before;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
            failed |= !inc_dec_run(&state, code, operand32 ? 2u : 1u, 0,
                &after, &diagnostic) || diagnostic.first_fault.valid ||
                *inc_dec_register(&after, index) != expected ||
                !inc_dec_flags_match(after.data.eflags, flags, VCPU_EFLAGS_CF);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}
static C_INT inc_dec_test_rm_forms(C_VOID)
{
    static const type_unsigned_8 forms[][6] = {
        { 0xfeu, 0xc0u }, { 0xfeu, 0xc8u }, { 0xffu, 0xc0u }, { 0xffu, 0xc8u },
        { 0xfeu, 0x06u, 0x00u, 0x50u }, { 0xfeu, 0x0eu, 0x00u, 0x50u },
        { 0xffu, 0x06u, 0x00u, 0x50u }, { 0xffu, 0x0eu, 0x00u, 0x50u },
        { 0x66u, 0xffu, 0x06u, 0x00u, 0x50u },
        { 0x66u, 0xffu, 0x0eu, 0x00u, 0x50u }
    };
    static const type_unsigned_8 lengths[] = { 2u, 2u, 2u, 2u, 4u, 4u, 4u, 4u, 5u, 5u };
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;
    type_unsigned_8 form;
    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (form = 0u; form != sizeof(lengths); ++form) {
        if (form >= 8u && profiles[profile] != CORE_MACHINE_CPU_PROFILE_80386) continue;
        const C_INT decrement = (form & 1u) != 0u;
        const type_unsigned_8 bytes = form < 2u || (form >= 4u && form < 6u) ? 1u :
            (form < 8u ? 2u : 4u);
        const type_unsigned_32 before = decrement ? (bytes == 1u ? 0x80u :
            (bytes == 2u ? 0x8000u : 0x80000000u)) : (bytes == 1u ? 0x7fu :
            (bytes == 2u ? 0x7fffu : 0x7fffffffu));
        const type_unsigned_32 expected = decrement ? (before - 1u) : (before + 1u);
        const type_unsigned_32 flags = decrement ?
            (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF |
                (bytes == 1u ? 0u : VCPU_EFLAGS_PF)) :
            (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF |
                (bytes == 1u ? 0u : VCPU_EFLAGS_PF));
        inc_dec_machine state;
        t_cpu after = {0};
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 observed = 0u;
        C_INT failed = !inc_dec_prepare(profiles[profile], &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = before;
            state.machine->executor_cpu.data.eflags = decrement ? 0u : VCPU_EFLAGS_CF;
            if (form >= 4u) failed |= core_machine_memory_write(state.machine,
                INC_DEC_MEMORY, &before, bytes) != TYPE_STATUS_OK;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                !inc_dec_flags_match(after.data.eflags, flags,
                    decrement ? 0u : VCPU_EFLAGS_CF);
            if (form < 4u) {
                const type_unsigned_32 mask = bytes == 1u ? 0xffu : 0xffffu;
                failed |= (after.data.eax & mask) != expected;
            } else {
                failed |= core_machine_memory_read(state.machine, INC_DEC_MEMORY,
                    &observed, bytes) != TYPE_STATUS_OK || observed != expected;
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_address_and_profile(C_VOID)
{
    static const type_unsigned_8 address_code[] = { 0x67u,0x66u,0xffu,0x06u,
        0x00u,0x50u,0x00u,0x00u };
    static const type_unsigned_8 rejected_prefixes[][2] = {
        {0x66u,0x40u}, {0x66u,0x48u}
    };
    static const core_machine_cpu_profile legacy_profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const type_unsigned_8 accepted_legacy[] = { 0xffu,0xc0u };
    inc_dec_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    type_unsigned_32 value = 0x7fffffffu;
    C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.machine->executor_cpu.data.esi = INC_DEC_MEMORY;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
        failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY,
            &value, sizeof(value)) != TYPE_STATUS_OK ||
            !inc_dec_run(&state, address_code, sizeof(address_code), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
            core_machine_memory_read(state.machine, INC_DEC_MEMORY,
                &value, sizeof(value)) != TYPE_STATUS_OK ||
            value != 0x80000000u || !inc_dec_flags_match(after.data.eflags,
                VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF |
                VCPU_EFLAGS_PF, VCPU_EFLAGS_CF);
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;

    {
        type_unsigned_8 profile;
        type_unsigned_8 opcode;
        for (profile = 0u; profile != sizeof(legacy_profiles) / sizeof(legacy_profiles[0]);
            ++profile)
        for (opcode = 0u; opcode != sizeof(rejected_prefixes) / sizeof(rejected_prefixes[0]);
            ++opcode) {
            failed = !inc_dec_prepare(legacy_profiles[profile], &state);
            if (!failed) {
                state.machine->executor_cpu.data.eax = 0x11227fffu;
                state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
                failed |= !inc_dec_run(&state, rejected_prefixes[opcode],
                    sizeof(rejected_prefixes[opcode]), 1, &after, &diagnostic) ||
                    !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                    after.data.eax != 0x11227fffu ||
                    after.data.eflags != VCPU_EFLAGS_CF || after.data.eip != 0u;
            }
            core_machine_destroy(state.machine);
            if (failed) return 0;
        }
    }

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);
    if (!failed) {
        state.machine->executor_cpu.data.eax = 0xaabb7fffu;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
        failed |= !inc_dec_run(&state, accepted_legacy, sizeof(accepted_legacy), 0,
            &after, &diagnostic) || diagnostic.first_fault.valid ||
            after.data.eax != 0xaabb8000u || !inc_dec_flags_match(after.data.eflags,
                VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF |
                VCPU_EFLAGS_PF, VCPU_EFLAGS_CF);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT inc_dec_prepare_protected(C_INT writable, C_INT out_of_limit,
    inc_dec_machine *state)
{
    static const type_unsigned_8 gdt_pointer[] = { 0x1fu,0u,0u,0x03u,0u,0u };
    type_unsigned_8 gdt[] = {
        0u,0u,0u,0u,0u,0u,0u,0u,
        0xffu,0xffu,0u,0x20u,0u,0x9au,0u,0u,
        0xffu,0xffu,0u,0x30u,0u,0x92u,0u,0u,
        0xffu,0xffu,0u,0x40u,0u,0x92u,0u,0u
    };
    static const type_unsigned_8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0u,0x01u,
        0xb8u,0x01u,0u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0u,0x8eu,0xd8u,0x8eu,0xc0u,
        0xb8u,0x18u,0u,0x8eu,0xd0u,
        0xbcu,0u,0x80u,0xeau,0u,0u,0x08u,0u
    };
    static const type_unsigned_8 halt[] = { 0xf4u };
    core_machine_run_result result;

    gdt[16u] = out_of_limit ? 0x0fu : 0xffu;
    gdt[17u] = out_of_limit ? 0u : 0xffu;
    gdt[21u] = writable ? 0x92u : 0x90u;
    return inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, state) &&
        core_machine_memory_write(state->machine, 0x0100u, gdt_pointer,
            sizeof(gdt_pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x0300u, gdt, sizeof(gdt)) ==
            TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, bootstrap,
            sizeof(bootstrap)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x2000u, halt, sizeof(halt)) ==
            TYPE_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){ 96u, 0u },
            &result) == TYPE_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT inc_dec_test_fault_nonpublication(C_VOID)
{
    static const type_unsigned_8 code[] = { 0xffu,0x06u,0x10u,0u };
    const type_unsigned_32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
    type_unsigned_8 pass;

    for (pass = 0u; pass != 2u; ++pass) {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        type_unsigned_16 before = 0x7fffu;
        type_unsigned_16 observed = 0u;
        C_INT failed = !inc_dec_prepare_protected(pass == 0u, pass == 0u, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eflags = flags;
            failed |= core_machine_memory_write(state.machine, 0x3010u, &before,
                sizeof(before)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x2000u, code,
                    sizeof(code)) != TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
                &result) != TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                    TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                    0x3010u, (type_virtual_address)&observed, sizeof(observed)) !=
                    TYPE_STATUS_OK || observed != before || after.data.eflags != flags ||
                after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_not_neg_forms(C_VOID)
{
    static const type_unsigned_8 forms[][6] = {
        { 0xf6u,0xd0u }, { 0xf6u,0xd8u }, { 0xf7u,0xd0u }, { 0xf7u,0xd8u },
        { 0x66u,0xf7u,0xd0u }, { 0x66u,0xf7u,0xd8u },
        { 0xf6u,0x16u,0x00u,0x50u }, { 0xf6u,0x1eu,0x00u,0x50u },
        { 0xf7u,0x16u,0x00u,0x50u }, { 0xf7u,0x1eu,0x00u,0x50u },
        { 0x66u,0xf7u,0x16u,0x00u,0x50u },
        { 0x66u,0xf7u,0x1eu,0x00u,0x50u }
    };
    static const type_unsigned_8 lengths[] = { 2u,2u,2u,2u,3u,3u,4u,4u,4u,4u,5u,5u };
    const type_unsigned_32 saved_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF;
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const C_INT negate = (form & 1u) != 0u;
        const type_unsigned_8 bytes = form == 0u || form == 1u || form == 6u || form == 7u ?
            1u : (form == 2u || form == 3u || form == 8u || form == 9u ? 2u : 4u);
        const type_unsigned_32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const type_unsigned_32 before = negate ? (bytes == 1u ? 0x80u :
            (bytes == 2u ? 0x8000u : 0x80000000u)) :
            (bytes == 1u ? 0x5au : (bytes == 2u ? 0xa55au : 0x5aa55aa5u));
        const type_unsigned_32 expected = negate ? (0u - before) & mask : (~before) & mask;
        const type_unsigned_32 neg_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
            VCPU_EFLAGS_SF | (bytes == 1u ? 0u : VCPU_EFLAGS_PF);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 observed = 0u;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = before;
            state.machine->executor_cpu.data.eflags = saved_flags;
            if (form >= 6u) failed |= core_machine_memory_write(state.machine,
                INC_DEC_MEMORY, &before, bytes) != TYPE_STATUS_OK;
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
                failed |= core_machine_memory_read(state.machine, INC_DEC_MEMORY,
                    &observed, bytes) != TYPE_STATUS_OK || observed != expected;
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_not_neg_address_and_profile(C_VOID)
{
    static const type_unsigned_8 address_code[] = { 0x67u,0x66u,0xf7u,0x1eu,
        0x00u,0x50u,0x00u,0x00u };
    static const type_unsigned_8 rejected_prefix[] = { 0x66u,0xf7u,0xd0u };
    static const type_unsigned_8 accepted_legacy[] = { 0xf7u,0xd0u };
    const type_unsigned_32 saved_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF;
    inc_dec_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    type_unsigned_32 value = 0x80000000u;
    C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.machine->executor_cpu.data.esi = INC_DEC_MEMORY;
        state.machine->executor_cpu.data.eflags = saved_flags;
        failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY,
            &value, sizeof(value)) != TYPE_STATUS_OK ||
            !inc_dec_run(&state, address_code, sizeof(address_code), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
            core_machine_memory_read(state.machine, INC_DEC_MEMORY, &value,
                sizeof(value)) != TYPE_STATUS_OK || value != 0x80000000u ||
            (after.data.eflags & (INC_DEC_DEFINED_FLAGS | VCPU_EFLAGS_CF)) !=
                (VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_SF |
                    VCPU_EFLAGS_PF);
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);
    if (!failed) {
        state.machine->executor_cpu.data.eax = 0x11225aa5u;
        state.machine->executor_cpu.data.eflags = saved_flags;
        failed |= !inc_dec_run(&state, rejected_prefix, sizeof(rejected_prefix), 1,
            &after, &diagnostic) || !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eax != 0x11225aa5u || after.data.eflags != saved_flags ||
            after.data.eip != 0u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);
    if (!failed) {
        state.machine->executor_cpu.data.eax = 0xaabb5aa5u;
        state.machine->executor_cpu.data.eflags = saved_flags;
        failed |= !inc_dec_run(&state, accepted_legacy, sizeof(accepted_legacy), 0,
            &after, &diagnostic) || diagnostic.first_fault.valid ||
            after.data.eax != 0xaabba55au || after.data.eflags != saved_flags;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT inc_dec_test_not_neg_fault_nonpublication(C_VOID)
{
    static const type_unsigned_8 code[][4] = {
        { 0xf7u,0x16u,0x10u,0u }, { 0xf7u,0x1eu,0x10u,0u }
    };
    const type_unsigned_32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
    type_unsigned_8 operation;
    type_unsigned_8 pass;

    for (operation = 0u; operation != 2u; ++operation) {
        for (pass = 0u; pass != 2u; ++pass) {
            inc_dec_machine state;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            core_machine_run_result result;
            type_unsigned_16 before = operation ? 0x7fffu : 0x55aau;
            type_unsigned_16 observed = 0u;
            C_INT failed = !inc_dec_prepare_protected(pass == 0u, pass == 0u, &state);

            if (!failed) {
                state.machine->executor_cpu.data.eflags = flags;
                failed |= core_machine_memory_write(state.machine, 0x3010u, &before,
                    sizeof(before)) != TYPE_STATUS_OK || core_machine_memory_write(
                        state.machine, 0x2000u, code[operation], sizeof(code[operation])) !=
                    TYPE_STATUS_OK;
                test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
                failed |= core_machine_run(state.machine,
                    (core_machine_run_budget){ 1u, 0u }, &result) != TYPE_STATUS_FAULT ||
                    result.reason != CORE_MACHINE_STOP_FAULT ||
                    core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                        TYPE_STATUS_OK;
                after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                    core_machine_memory_read_physical(&state.machine->executor_memory,
                        0x3010u, (type_virtual_address)&observed, sizeof(observed)) !=
                        TYPE_STATUS_OK || observed != before || after.data.eflags != flags ||
                    after.data.eip != 0u;
            }
            core_machine_destroy(state.machine);
            if (failed) return 0;
        }
    }
    return 1;
}

static C_INT inc_dec_test_test_forms(C_VOID)
{
    static const type_unsigned_8 forms[][9] = {
        { 0xa8u,0x80u }, { 0xa9u,0u,0x80u },
        { 0x66u,0xa9u,0u,0u,0u,0x80u },
        { 0xf6u,0xc0u,0x80u }, { 0xf7u,0xc0u,0u,0x80u },
        { 0x66u,0xf7u,0xc0u,0u,0u,0u,0x80u },
        { 0xf6u,0x06u,0u,0x50u,0x80u },
        { 0xf7u,0x06u,0u,0x50u,0u,0x80u },
        { 0x66u,0xf7u,0x06u,0u,0x50u,0u,0u,0u,0x80u }
    };
    static const type_unsigned_8 lengths[] = { 2u,3u,6u,3u,4u,7u,5u,6u,9u };
    const type_unsigned_32 initial_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF;
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const type_unsigned_8 bytes = form == 0u || form == 3u || form == 6u ? 1u :
            (form == 1u || form == 4u || form == 7u ? 2u : 4u);
        const type_unsigned_32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const type_unsigned_32 expected_flags = VCPU_EFLAGS_SF |
            (bytes == 1u ? 0u : VCPU_EFLAGS_PF);
        const C_INT memory = form >= 6u;
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 before = 0xffffffffu;
        type_unsigned_32 observed = 0u;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = before;
            state.machine->executor_cpu.data.eflags = initial_flags;
            if (memory) failed |= core_machine_memory_write(state.machine,
                INC_DEC_MEMORY, &before, bytes) != TYPE_STATUS_OK;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                after.data.eax != before ||
                (after.data.eflags & TEST_DEFINED_FLAGS) != expected_flags;
            if (memory) failed |= core_machine_memory_read(state.machine,
                INC_DEC_MEMORY, &observed, bytes) != TYPE_STATUS_OK ||
                observed != (before & mask);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_accumulator_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 forms[][3] = {
        {0xa8u, 0x80u, 0u}, {0xa9u, 0u, 0x80u}
    };
    static const type_unsigned_8 lengths[] = {2u, 3u};
    static const type_unsigned_8 dword[] = {0x66u, 0xa9u, 0u, 0u, 0u, 0x80u};
    const type_unsigned_32 initial_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF;
    type_unsigned_8 profile;
    type_unsigned_8 form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (form = 0u; form != sizeof(lengths); ++form) {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 expected = form == 0u ? VCPU_EFLAGS_SF :
            VCPU_EFLAGS_SF | VCPU_EFLAGS_PF;
        C_INT failed = !inc_dec_prepare(profiles[profile], &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0xaabbffffu;
            state.machine->executor_cpu.data.eflags = initial_flags;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.eip !=
                lengths[form] || after.data.eax != 0xaabbffffu ||
                (after.data.eflags & TEST_DEFINED_FLAGS) != expected ||
                (after.data.eflags & ~TEST_DEFINED_FLAGS) !=
                (initial_flags & ~TEST_DEFINED_FLAGS);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    for (profile = 0u; profile != 3u; ++profile) {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !inc_dec_prepare(profiles[profile], &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0xaabbffffu;
            state.machine->executor_cpu.data.eflags = initial_flags;
            failed |= !inc_dec_run(&state, dword, sizeof(dword), 1, &after,
                &diagnostic) || !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.eip != 0u || after.data.eax != 0xaabbffffu ||
                after.data.eflags != initial_flags;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0xffffffffu;
            state.machine->executor_cpu.data.eflags = initial_flags;
            failed |= !inc_dec_run(&state, dword, sizeof(dword), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.eip !=
                sizeof(dword) || after.data.eax != 0xffffffffu ||
                (after.data.eflags & TEST_DEFINED_FLAGS) !=
                (VCPU_EFLAGS_SF | VCPU_EFLAGS_PF) ||
                (after.data.eflags & ~TEST_DEFINED_FLAGS) !=
                (initial_flags & ~TEST_DEFINED_FLAGS);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}
static C_INT inc_dec_test_test_address_and_profile(C_VOID)
{
    static const type_unsigned_8 address_code[] = { 0x67u,0x66u,0xf7u,0x06u,
        0u,0u,0u,0x80u };
    static const type_unsigned_8 rejected_prefix[] = { 0x66u,0xa9u,0u,0u,0u,0x80u };
    static const type_unsigned_8 accepted_legacy[] = { 0xa9u,0u,0x80u };
    const type_unsigned_32 initial_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF;
    inc_dec_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    type_unsigned_32 value = 0xffffffffu;
    C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.machine->executor_cpu.data.esi = INC_DEC_MEMORY;
        state.machine->executor_cpu.data.eax = value;
        state.machine->executor_cpu.data.eflags = initial_flags;
        failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY,
            &value, sizeof(value)) != TYPE_STATUS_OK ||
            !inc_dec_run(&state, address_code, sizeof(address_code), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.eax != value ||
            core_machine_memory_read(state.machine, INC_DEC_MEMORY, &value,
                sizeof(value)) != TYPE_STATUS_OK || value != 0xffffffffu ||
            (after.data.eflags & TEST_DEFINED_FLAGS) !=
                (VCPU_EFLAGS_SF | VCPU_EFLAGS_PF);
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);
    if (!failed) {
        state.machine->executor_cpu.data.eax = 0x1122ffffu;
        state.machine->executor_cpu.data.eflags = initial_flags;
        failed |= !inc_dec_run(&state, rejected_prefix, sizeof(rejected_prefix), 1,
            &after, &diagnostic) || !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eax != 0x1122ffffu || after.data.eflags != initial_flags ||
            after.data.eip != 0u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);
    if (!failed) {
        state.machine->executor_cpu.data.eax = 0xaabbffffu;
        state.machine->executor_cpu.data.eflags = initial_flags;
        failed |= !inc_dec_run(&state, accepted_legacy, sizeof(accepted_legacy), 0,
            &after, &diagnostic) || diagnostic.first_fault.valid ||
            after.data.eax != 0xaabbffffu ||
            (after.data.eflags & TEST_DEFINED_FLAGS) !=
                (VCPU_EFLAGS_SF | VCPU_EFLAGS_PF);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT inc_dec_test_test_fault_nonpublication(C_VOID)
{
    static const type_unsigned_8 code[] = { 0xf7u,0x06u,0x10u,0u,0xffu,0xffu };
    const type_unsigned_32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_AF;
    inc_dec_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_unsigned_16 before = 0xffffu;
    type_unsigned_16 observed = 0u;
    C_INT failed = !inc_dec_prepare_protected(1, 1, &state);

    if (!failed) {
        state.machine->executor_cpu.data.eflags = flags;
        failed |= core_machine_memory_write(state.machine, 0x3010u, &before,
            sizeof(before)) != TYPE_STATUS_OK || core_machine_memory_write(
                state.machine, 0x2000u, code, sizeof(code)) != TYPE_STATUS_OK;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
            &result) != TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
            core_machine_memory_read_physical(&state.machine->executor_memory, 0x3010u,
                (type_virtual_address)&observed, sizeof(observed)) != TYPE_STATUS_OK ||
            observed != before || after.data.eflags != flags || after.data.eip != 0u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT inc_dec_test_mul_imul_forms(C_VOID)
{
    static const type_unsigned_8 forms[][5] = {
        { 0xf6u,0xe1u }, { 0xf6u,0xe9u }, { 0xf7u,0xe1u }, { 0xf7u,0xe9u },
        { 0x66u,0xf7u,0xe1u }, { 0x66u,0xf7u,0xe9u },
        { 0xf6u,0x26u,0u,0x50u }, { 0xf6u,0x2eu,0u,0x50u },
        { 0xf7u,0x26u,0u,0x50u }, { 0xf7u,0x2eu,0u,0x50u },
        { 0x66u,0xf7u,0x26u,0u,0x50u },
        { 0x66u,0xf7u,0x2eu,0u,0x50u }
    };
    static const type_unsigned_8 lengths[] = { 2u,2u,2u,2u,3u,3u,4u,4u,4u,4u,5u,5u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const C_INT signed_multiply = (form & 1u) != 0u;
        const C_INT memory = form >= 6u;
        const type_unsigned_8 bytes = form == 0u || form == 1u || form == 6u || form == 7u ?
            1u : (form == 2u || form == 3u || form == 8u || form == 9u ? 2u : 4u);
        const type_unsigned_32 source = memory ? 2u : 2u;
        const type_unsigned_32 accumulator = memory ? (bytes == 1u ? 0x80u :
            (bytes == 2u ? 0x8000u : 0x80000000u)) : 2u;
        const type_unsigned_32 expected_lo = memory ? (bytes == 1u ? (signed_multiply ?
            0xff00u : 0x0100u) : 0u) : 4u;
        const type_unsigned_32 expected_hi = memory ? (bytes == 1u ? 0u :
            (signed_multiply ? (bytes == 2u ? 0xffffu : 0xffffffffu) : 1u)) : 0u;
        const type_unsigned_32 expected_flags = memory ? MUL_DEFINED_FLAGS : 0u;
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 observed = 0u;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = accumulator;
            state.machine->executor_cpu.data.ecx = source;
            state.machine->executor_cpu.data.edx = 0xaabbccddu;
            state.machine->executor_cpu.data.eflags = MUL_DEFINED_FLAGS;
            if (memory) failed |= core_machine_memory_write(state.machine,
                INC_DEC_MEMORY, &source, bytes) != TYPE_STATUS_OK;
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
            if (memory) failed |= core_machine_memory_read(state.machine,
                INC_DEC_MEMORY, &observed, bytes) != TYPE_STATUS_OK || observed != source;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_imul_sign_extension_profiles(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile_index;
    type_unsigned_8 width_index;

    for (profile_index = 0u; profile_index != sizeof(profiles) / sizeof(profiles[0]);
        ++profile_index)
    for (width_index = 0u; width_index != (profiles[profile_index] ==
        CORE_MACHINE_CPU_PROFILE_80386 ? 3u : 2u); ++width_index) {
        static const type_unsigned_8 code[][3] = {
            { 0xf6u, 0xe9u }, { 0xf7u, 0xe9u }, { 0x66u, 0xf7u, 0xe9u }
        };
        const type_unsigned_8 bytes = width_index == 0u ? 1u :
            (width_index == 1u ? 2u : 4u);
        const type_unsigned_32 initial_eax = bytes == 1u ? 0x112233ffu :
            (bytes == 2u ? 0x1122ffffu : 0xffffffffu);
        const type_unsigned_32 initial_edx = 0xaabbccddu;
        const type_unsigned_32 expected_eax = bytes != 4u ? 0x1122ffffu :
            0xffffffffu;
        const type_unsigned_32 expected_edx = bytes == 1u ? initial_edx :
            (bytes == 2u ? 0xaabbffffu : 0xffffffffu);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !inc_dec_prepare(profiles[profile_index], &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = initial_eax;
            state.machine->executor_cpu.data.ecx = 1u;
            state.machine->executor_cpu.data.edx = initial_edx;
            state.machine->executor_cpu.data.eflags = MUL_DEFINED_FLAGS |
                VCPU_EFLAGS_ZF;
            failed |= !inc_dec_run(&state, code[width_index], bytes == 4u ?
                3u : 2u, 0, &after, &diagnostic) || diagnostic.first_fault.valid ||
                after.data.eax != expected_eax || after.data.edx != expected_edx ||
                (after.data.eflags & MUL_DEFINED_FLAGS) != 0u ||
                after.data.cx != 1u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}
static C_INT inc_dec_test_mul_imul_address_and_profile(C_VOID)
{
    static const type_unsigned_8 address_code[] = { 0x67u,0x66u,0xf7u,0x2eu };
    static const type_unsigned_8 rejected_prefix[] = { 0x66u,0xf7u,0xe0u };
    static const type_unsigned_8 accepted_legacy[] = { 0xf7u,0xe0u };
    inc_dec_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    type_unsigned_32 source = 2u;
    C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.machine->executor_cpu.data.esi = INC_DEC_MEMORY;
        state.machine->executor_cpu.data.eax = 0x80000000u;
        state.machine->executor_cpu.data.edx = 0x11223344u;
        state.machine->executor_cpu.data.eflags = 0u;
        failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY, &source,
            sizeof(source)) != TYPE_STATUS_OK || !inc_dec_run(&state, address_code,
                sizeof(address_code), 0, &after, &diagnostic) ||
            diagnostic.first_fault.valid || after.data.eax != 0u ||
            after.data.edx != 0xffffffffu ||
            (after.data.eflags & MUL_DEFINED_FLAGS) != MUL_DEFINED_FLAGS ||
            core_machine_memory_read(state.machine, INC_DEC_MEMORY, &source,
                sizeof(source)) != TYPE_STATUS_OK || source != 2u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);
    if (!failed) {
        state.machine->executor_cpu.data.eax = 0x11220002u;
        state.machine->executor_cpu.data.edx = 0xaabbccddu;
        state.machine->executor_cpu.data.eflags = MUL_DEFINED_FLAGS;
        failed |= !inc_dec_run(&state, rejected_prefix, sizeof(rejected_prefix), 1,
            &after, &diagnostic) || !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eax != 0x11220002u || after.data.edx != 0xaabbccddu ||
            after.data.eflags != MUL_DEFINED_FLAGS || after.data.eip != 0u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);
    if (!failed) {
        state.machine->executor_cpu.data.eax = 0xaabb0002u;
        state.machine->executor_cpu.data.edx = 0x11223344u;
        state.machine->executor_cpu.data.eflags = MUL_DEFINED_FLAGS;
        failed |= !inc_dec_run(&state, accepted_legacy, sizeof(accepted_legacy), 0,
            &after, &diagnostic) || diagnostic.first_fault.valid ||
            after.data.ax != 4u || after.data.dx != 0u ||
            (after.data.eflags & MUL_DEFINED_FLAGS) != 0u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT inc_dec_test_mul_imul_fault_nonpublication(C_VOID)
{
    static const type_unsigned_8 code[][4] = {
        { 0xf7u,0x26u,0x10u,0u }, { 0xf7u,0x2eu,0x10u,0u }
    };
    const type_unsigned_32 flags = MUL_DEFINED_FLAGS | VCPU_EFLAGS_ZF;
    type_unsigned_8 operation;

    for (operation = 0u; operation != 2u; ++operation) {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        type_unsigned_16 before = 2u;
        type_unsigned_16 observed = 0u;
        C_INT failed = !inc_dec_prepare_protected(1, 1, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0xaabb8000u;
            state.machine->executor_cpu.data.edx = 0x11223344u;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= core_machine_memory_write(state.machine, 0x3010u, &before,
                sizeof(before)) != TYPE_STATUS_OK || core_machine_memory_write(
                    state.machine, 0x2000u, code[operation], sizeof(code[operation])) !=
                TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
                &result) != TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                core_machine_memory_read_physical(&state.machine->executor_memory, 0x3010u,
                    (type_virtual_address)&observed, sizeof(observed)) != TYPE_STATUS_OK ||
                observed != before || after.data.eax != 0xaabb8000u ||
                after.data.edx != 0x11223344u || after.data.eflags != flags ||
                after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_div_idiv_forms(C_VOID)
{
    static const type_unsigned_8 forms[][5] = {
        { 0xf6u,0xf1u }, { 0xf6u,0xf9u }, { 0xf7u,0xf1u }, { 0xf7u,0xf9u },
        { 0x66u,0xf7u,0xf1u }, { 0x66u,0xf7u,0xf9u },
        { 0xf6u,0x36u,0u,0x50u }, { 0xf6u,0x3eu,0u,0x50u },
        { 0xf7u,0x36u,0u,0x50u }, { 0xf7u,0x3eu,0u,0x50u },
        { 0x66u,0xf7u,0x36u,0u,0x50u },
        { 0x66u,0xf7u,0x3eu,0u,0x50u }
    };
    static const type_unsigned_8 lengths[] = { 2u,2u,2u,2u,3u,3u,4u,4u,4u,4u,5u,5u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const C_INT signed_divide = (form & 1u) != 0u;
        const C_INT memory = form >= 6u;
        const type_unsigned_8 bytes = form == 0u || form == 1u || form == 6u || form == 7u ?
            1u : (form == 2u || form == 3u || form == 8u || form == 9u ? 2u : 4u);
        const type_unsigned_32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const type_unsigned_32 source = signed_divide && memory ? mask - 1u : 2u;
        const type_unsigned_32 quotient = signed_divide ? (memory ? 2u : mask - 1u) : 2u;
        const type_unsigned_32 remainder = signed_divide ? mask : 1u;
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 observed = 0u;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = signed_divide ?
                (bytes == 1u ? 0xfffbu : mask - 4u) : 5u;
            state.machine->executor_cpu.data.edx = signed_divide && bytes != 1u ? mask : 0u;
            state.machine->executor_cpu.data.ecx = source;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
            if (memory) failed |= core_machine_memory_write(state.machine,
                INC_DEC_MEMORY, &source, bytes) != TYPE_STATUS_OK;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid;
            if (bytes == 1u) {
                failed |= after.data.al != quotient || after.data.ah != remainder;
            } else if (bytes == 2u) {
                failed |= after.data.ax != quotient || after.data.dx != remainder;
            } else {
                failed |= after.data.eax != quotient || after.data.edx != remainder;
            }
            if (memory) failed |= core_machine_memory_read(state.machine,
                INC_DEC_MEMORY, &observed, bytes) != TYPE_STATUS_OK ||
                observed != (source & mask);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_div_idiv_attribute_and_profile(C_VOID)
{
    static const type_unsigned_8 address_code[] = { 0x67u,0x66u,0xf7u,0x3eu };
    static const type_unsigned_8 rejected_prefix[] = { 0x66u,0xf7u,0xf1u };
    static const type_unsigned_8 accepted_legacy[] = { 0xf7u,0xf1u };
    inc_dec_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    type_unsigned_32 source = 0xfffffffeu;
    C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.machine->executor_cpu.data.esi = INC_DEC_MEMORY;
        state.machine->executor_cpu.data.eax = 0xfffffffbu;
        state.machine->executor_cpu.data.edx = 0xffffffffu;
        failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY, &source,
            sizeof(source)) != TYPE_STATUS_OK || !inc_dec_run(&state, address_code,
                sizeof(address_code), 0, &after, &diagnostic) ||
            diagnostic.first_fault.valid || after.data.eax != 2u ||
            after.data.edx != 0xffffffffu;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);
    if (!failed) {
        state.machine->executor_cpu.data.eax = 0x11220005u;
        state.machine->executor_cpu.data.edx = 0xaabbccddu;
        state.machine->executor_cpu.data.ecx = 2u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
        failed |= !inc_dec_run(&state, rejected_prefix, sizeof(rejected_prefix), 1,
            &after, &diagnostic) || !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eax != 0x11220005u || after.data.edx != 0xaabbccddu ||
            after.data.ecx != 2u || after.data.eflags != VCPU_EFLAGS_CF ||
            after.data.eip != 0u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);
    if (!failed) {
        state.machine->executor_cpu.data.eax = 5u;
        state.machine->executor_cpu.data.edx = 0u;
        state.machine->executor_cpu.data.ecx = 2u;
        failed |= !inc_dec_run(&state, accepted_legacy, sizeof(accepted_legacy), 0,
            &after, &diagnostic) || diagnostic.first_fault.valid ||
            after.data.ax != 2u || after.data.dx != 1u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT inc_dec_test_div_idiv_de_nonpublication(C_VOID)
{
    static const type_unsigned_8 code[][3] = {
        { 0xf6u,0xf1u }, { 0xf6u,0xf9u }, { 0xf7u,0xf1u }, { 0xf7u,0xf9u },
        { 0x66u,0xf7u,0xf1u }, { 0x66u,0xf7u,0xf9u }
    };
    type_unsigned_8 fault_case;
    type_unsigned_8 form;

    for (fault_case = 0u; fault_case != 2u; ++fault_case) {
        for (form = 0u; form != sizeof(code) / sizeof(code[0]); ++form) {
            const C_INT signed_divide = (form & 1u) != 0u;
            const type_unsigned_8 bytes = form < 2u ? 1u : (form < 4u ? 2u : 4u);
            inc_dec_machine state;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            type_unsigned_32 eax = fault_case ? (signed_divide ?
                (bytes == 1u ? 0xff80u : (bytes == 2u ? 0x00008000u : 0x80000000u)) : 0u) : 5u;
            type_unsigned_32 edx = fault_case ? (bytes == 1u ? 0x11223344u :
                (signed_divide ? (bytes == 2u ? 0x0000ffffu : 0xffffffffu) : 1u)) :
                0xaabbccddu;
            type_unsigned_32 ecx = fault_case ? (signed_divide ?
                (bytes == 1u ? 0xffu : (bytes == 2u ? 0xffffu : 0xffffffffu)) : 1u) : 0u;
            const type_unsigned_32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
            C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

            if (!failed) {
                if (fault_case && !signed_divide && bytes == 1u) eax = 0x00000100u;
                state.machine->executor_cpu.data.eax = eax;
                state.machine->executor_cpu.data.edx = edx;
                state.machine->executor_cpu.data.ecx = ecx;
                state.machine->executor_cpu.data.eflags = flags;
                failed |= !inc_dec_run_delivered_de(&state, code[form],
                    form < 4u ? 2u : 3u, &after, &diagnostic) ||
                    after.data.eax != eax || after.data.edx != edx ||
                    after.data.ecx != ecx || after.data.eflags != flags ||
                    after.data.eip != 0x0100u;
            }
            core_machine_destroy(state.machine);
            if (failed) return 0;
        }
    }
    return 1;
}

static C_INT inc_dec_test_div_idiv_fault_nonpublication(C_VOID)
{
    static const type_unsigned_8 code[][4] = {
        { 0xf7u,0x36u,0x10u,0u }, { 0xf7u,0x3eu,0x10u,0u }
    };
    type_unsigned_8 operation;

    for (operation = 0u; operation != 2u; ++operation) {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        type_unsigned_16 before = 2u;
        type_unsigned_16 observed = 0u;
        const type_unsigned_32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
        C_INT failed = !inc_dec_prepare_protected(1, 1, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 5u;
            state.machine->executor_cpu.data.edx = 0xaabbccddu;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= core_machine_memory_write(state.machine, 0x3010u, &before,
                sizeof(before)) != TYPE_STATUS_OK || core_machine_memory_write(
                    state.machine, 0x2000u, code[operation], sizeof(code[operation])) !=
                TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
                &result) != TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                    0x3010u, (type_virtual_address)&observed, sizeof(observed)) !=
                    TYPE_STATUS_OK || observed != before ||
                after.data.eax != 5u || after.data.edx != 0xaabbccddu ||
                after.data.eflags != flags || after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_test_rm_reg_forms(C_VOID)
{
    static const type_unsigned_8 forms[][5] = {
        { 0x84u,0xd1u }, { 0x85u,0xd1u }, { 0x66u,0x85u,0xd1u },
        { 0x84u,0x16u,0u,0x50u }, { 0x85u,0x16u,0u,0x50u },
        { 0x66u,0x85u,0x16u,0u,0x50u }
    };
    static const type_unsigned_8 lengths[] = { 2u,2u,3u,4u,4u,5u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const type_unsigned_8 bytes = form == 0u || form == 3u ? 1u :
            (form == 1u || form == 4u ? 2u : 4u);
        const C_INT memory = form >= 3u;
        const type_unsigned_32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        type_unsigned_8 pass;

        for (pass = 0u; pass != 2u; ++pass) {
            const type_unsigned_32 destination = mask;
            const type_unsigned_32 source = pass ? 0u : (bytes == 1u ? 0x80u :
                (bytes == 2u ? 0x8000u : 0x80000000u));
            const type_unsigned_32 initial_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
                VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF;
            const type_unsigned_32 expected_flags = pass ?
                (VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF) :
                (VCPU_EFLAGS_SF | (bytes == 1u ? 0u : VCPU_EFLAGS_PF));
            inc_dec_machine state;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            type_unsigned_32 observed = 0u;
            C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

            if (!failed) {
                state.machine->executor_cpu.data.ecx = destination;
                state.machine->executor_cpu.data.edx = source;
                state.machine->executor_cpu.data.eflags = initial_flags;
                if (memory) failed |= core_machine_memory_write(state.machine,
                    INC_DEC_MEMORY, &destination, bytes) != TYPE_STATUS_OK;
                failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                    &diagnostic) || diagnostic.first_fault.valid ||
                    after.data.ecx != destination || after.data.edx != source ||
                    (after.data.eflags & TEST_DEFINED_FLAGS) != expected_flags;
                if (memory) failed |= core_machine_memory_read(state.machine,
                    INC_DEC_MEMORY, &observed, bytes) != TYPE_STATUS_OK ||
                    observed != (destination & mask);
            }
            core_machine_destroy(state.machine);
            if (failed) return 0;
        }
    }
    return 1;
}

static C_INT inc_dec_test_test_rm_reg_attribute_and_profile(C_VOID)
{
    static const type_unsigned_8 address_code[] = { 0x67u,0x66u,0x85u,0x16u };
    static const type_unsigned_8 rejected_prefix[] = { 0x66u,0x85u,0xd1u };
    static const type_unsigned_8 accepted_legacy[] = { 0x85u,0xd1u };
    const type_unsigned_32 initial_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF;
    inc_dec_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    type_unsigned_32 destination = 0xffffffffu;
    C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.machine->executor_cpu.data.esi = INC_DEC_MEMORY;
        state.machine->executor_cpu.data.edx = 0x80000000u;
        state.machine->executor_cpu.data.eflags = initial_flags;
        failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY,
            &destination, sizeof(destination)) != TYPE_STATUS_OK ||
            !inc_dec_run(&state, address_code, sizeof(address_code), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
            after.data.edx != 0x80000000u ||
            (after.data.eflags & TEST_DEFINED_FLAGS) !=
                (VCPU_EFLAGS_SF | VCPU_EFLAGS_PF) ||
            core_machine_memory_read(state.machine, INC_DEC_MEMORY, &destination,
                sizeof(destination)) != TYPE_STATUS_OK || destination != 0xffffffffu;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);
    if (!failed) {
        state.machine->executor_cpu.data.ecx = 0x1122ffffu;
        state.machine->executor_cpu.data.edx = 0xaabb8000u;
        state.machine->executor_cpu.data.eflags = initial_flags;
        failed |= !inc_dec_run(&state, rejected_prefix, sizeof(rejected_prefix), 1,
            &after, &diagnostic) || !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.ecx != 0x1122ffffu || after.data.edx != 0xaabb8000u ||
            after.data.eflags != initial_flags || after.data.eip != 0u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);
    if (!failed) {
        state.machine->executor_cpu.data.ecx = 0xaabbffffu;
        state.machine->executor_cpu.data.edx = 0x11228000u;
        state.machine->executor_cpu.data.eflags = initial_flags;
        failed |= !inc_dec_run(&state, accepted_legacy, sizeof(accepted_legacy), 0,
            &after, &diagnostic) || diagnostic.first_fault.valid ||
            after.data.ecx != 0xaabbffffu || after.data.edx != 0x11228000u ||
            (after.data.eflags & TEST_DEFINED_FLAGS) !=
                (VCPU_EFLAGS_SF | VCPU_EFLAGS_PF);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT inc_dec_test_test_rm_reg_fault_nonpublication(C_VOID)
{
    static const type_unsigned_8 code[][5] = {
        { 0x84u,0x16u,0x10u,0u }, { 0x85u,0x16u,0x10u,0u },
        { 0x66u,0x85u,0x16u,0x10u,0u }
    };
    static const type_unsigned_8 lengths[] = { 4u,4u,5u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        type_unsigned_16 before = 0xffffu;
        type_unsigned_16 observed = 0u;
        const type_unsigned_32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_AF;
        C_INT failed = !inc_dec_prepare_protected(1, 1, &state);

        if (!failed) {
            state.machine->executor_cpu.data.edx = 0x11228000u;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= core_machine_memory_write(state.machine, 0x3010u, &before,
                sizeof(before)) != TYPE_STATUS_OK || core_machine_memory_write(
                    state.machine, 0x2000u, code[form], lengths[form]) != TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
                &result) != TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                core_machine_memory_read_physical(&state.machine->executor_memory, 0x3010u,
                    (type_virtual_address)&observed, sizeof(observed)) != TYPE_STATUS_OK ||
                observed != before || after.data.edx != 0x11228000u ||
                after.data.eflags != flags || after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_add_rm_reg_forms(C_VOID)
{
    static const type_unsigned_8 forms[][5] = {
        { 0x00u,0xd1u }, { 0x01u,0xd1u }, { 0x66u,0x01u,0xd1u },
        { 0x02u,0xd1u }, { 0x03u,0xd1u }, { 0x66u,0x03u,0xd1u },
        { 0x00u,0x16u,0u,0x50u }, { 0x01u,0x16u,0u,0x50u },
        { 0x66u,0x01u,0x16u,0u,0x50u }, { 0x02u,0x16u,0u,0x50u },
        { 0x03u,0x16u,0u,0x50u }, { 0x66u,0x03u,0x16u,0u,0x50u }
    };
    static const type_unsigned_8 lengths[] = { 2u,2u,3u,2u,2u,3u,4u,4u,5u,4u,4u,5u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const type_unsigned_8 bytes = form == 0u || form == 3u || form == 6u || form == 9u ?
            1u : (form == 1u || form == 4u || form == 7u || form == 10u ? 2u : 4u);
        const C_INT memory = form >= 6u;
        const C_INT reg_destination = !memory && form >= 3u;
        const C_INT memory_source = memory && form >= 9u;
        const type_unsigned_32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const type_unsigned_32 destination = bytes == 1u ? 0x112233ffu :
            (bytes == 2u ? 0x1122ffffu : mask);
        const type_unsigned_32 expected = destination & ~mask;
        const type_unsigned_32 source = 1u;
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 observed = 0u;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.ecx = reg_destination ? source : destination;
            state.machine->executor_cpu.data.edx = (reg_destination || memory_source) ?
                destination : source;
            state.machine->executor_cpu.data.eflags = 0u;
            if (memory) failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY,
                &(type_unsigned_32){ memory_source ? source : destination }, bytes) != TYPE_STATUS_OK;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
            if (memory) {
                failed |= core_machine_memory_read(state.machine, INC_DEC_MEMORY, &observed,
                    bytes) != TYPE_STATUS_OK || observed != (memory_source ? source : 0u);
                if (memory_source) failed |= (after.data.edx & mask) != 0u;
            } else if (reg_destination) {
                failed |= after.data.edx != expected || after.data.ecx != source;
            } else {
                failed |= after.data.ecx != expected || after.data.edx != source;
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_add_immediate_forms(C_VOID)
{
    static const type_unsigned_8 forms[][9] = {
        { 0x04u,1u }, { 0x05u,1u,0u }, { 0x66u,0x05u,1u,0u,0u,0u },
        { 0x80u,0xc1u,1u }, { 0x81u,0xc1u,1u,0u },
        { 0x66u,0x81u,0xc1u,1u,0u,0u,0u }, { 0x83u,0xc1u,0xffu },
        { 0x66u,0x83u,0xc1u,0xffu }, { 0x80u,0x06u,0u,0x50u,1u },
        { 0x81u,0x06u,0u,0x50u,1u,0u },
        { 0x66u,0x81u,0x06u,0u,0x50u,1u,0u,0u,0u },
        { 0x83u,0x06u,0u,0x50u,0xffu },
        { 0x66u,0x83u,0x06u,0u,0x50u,0xffu }
    };
    static const type_unsigned_8 lengths[] = { 2u,3u,6u,3u,4u,7u,3u,4u,5u,6u,9u,5u,6u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const type_unsigned_8 bytes = form == 0u || form == 3u || form == 8u ? 1u :
            (form == 1u || form == 4u || form == 6u || form == 9u || form == 11u ? 2u : 4u);
        const C_INT memory = form >= 8u;
        const C_INT signed_immediate = form == 6u || form == 7u || form == 11u || form == 12u;
        const type_unsigned_32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const type_unsigned_32 destination = signed_immediate ? 0u : mask;
        const type_unsigned_32 expected = signed_immediate ? mask : 0u;
        const type_unsigned_32 flags = signed_immediate ?
            (VCPU_EFLAGS_SF | VCPU_EFLAGS_PF) :
            (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 observed = 0u;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = destination;
            state.machine->executor_cpu.data.ecx = destination;
            if (memory) failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY,
                &destination, bytes) != TYPE_STATUS_OK;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                (after.data.eflags & ADD_DEFINED_FLAGS) != flags;
            if (memory) failed |= core_machine_memory_read(state.machine, INC_DEC_MEMORY,
                &observed, bytes) != TYPE_STATUS_OK || observed != expected;
            else if (form < 3u) failed |= (after.data.eax & mask) != expected;
            else failed |= (after.data.ecx & mask) != expected;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_add_signed_overflow(C_VOID)
{
    static const type_unsigned_8 forms[][6] = {
        { 0x04u,1u }, { 0x05u,1u,0u }, { 0x66u,0x05u,1u,0u,0u,0u }
    };
    static const type_unsigned_8 lengths[] = { 2u,3u,6u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const type_unsigned_8 bytes = form == 0u ? 1u : (form == 1u ? 2u : 4u);
        const type_unsigned_32 initial = bytes == 1u ? 0x1122337fu :
            (bytes == 2u ? 0x11227fffu : 0x7fffffffu);
        const type_unsigned_32 expected = bytes == 1u ? 0x11223380u :
            (bytes == 2u ? 0x11228000u : 0x80000000u);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = initial;
            state.machine->executor_cpu.data.eflags = 0u;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.eax != expected ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF |
                        (bytes == 1u ? 0u : VCPU_EFLAGS_PF));
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_add_attribute_and_profile(C_VOID)
{
    static const type_unsigned_8 address_code[] = { 0x67u,0x66u,0x01u,0x16u };
    static const type_unsigned_8 rejected_prefix[] = { 0x66u,0x01u,0xd1u };
    static const type_unsigned_8 accepted_legacy[] = { 0x01u,0xd1u };
    inc_dec_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    type_unsigned_32 destination = 0xffffffffu;
    C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.machine->executor_cpu.data.esi = INC_DEC_MEMORY;
        state.machine->executor_cpu.data.edx = 1u;
        failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY, &destination,
            sizeof(destination)) != TYPE_STATUS_OK || !inc_dec_run(&state, address_code,
                sizeof(address_code), 0, &after, &diagnostic) ||
            diagnostic.first_fault.valid || after.data.edx != 1u ||
            (after.data.eflags & ADD_DEFINED_FLAGS) !=
                (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF) ||
            core_machine_memory_read(state.machine, INC_DEC_MEMORY, &destination,
                sizeof(destination)) != TYPE_STATUS_OK || destination != 0u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);
    if (!failed) {
        state.machine->executor_cpu.data.ecx = 0x1122ffffu;
        state.machine->executor_cpu.data.edx = 1u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_OF;
        failed |= !inc_dec_run(&state, rejected_prefix, sizeof(rejected_prefix), 1,
            &after, &diagnostic) || !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.ecx != 0x1122ffffu || after.data.edx != 1u ||
            after.data.eflags != VCPU_EFLAGS_OF || after.data.eip != 0u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);
    if (!failed) {
        state.machine->executor_cpu.data.ecx = 0xaabbffffu;
        state.machine->executor_cpu.data.edx = 1u;
        failed |= !inc_dec_run(&state, accepted_legacy, sizeof(accepted_legacy), 0,
            &after, &diagnostic) || diagnostic.first_fault.valid ||
            after.data.ecx != 0xaabb0000u || after.data.edx != 1u ||
            (after.data.eflags & ADD_DEFINED_FLAGS) !=
                (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT inc_dec_test_add_fault_nonpublication(C_VOID)
{
    static const type_unsigned_8 code[] = { 0x01u,0x16u,0x10u,0u };
    type_unsigned_8 pass;

    for (pass = 0u; pass != 2u; ++pass) {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        type_unsigned_16 before = 0xffffu;
        type_unsigned_16 observed = 0u;
        const type_unsigned_32 flags = VCPU_EFLAGS_OF | VCPU_EFLAGS_CF;
        C_INT failed = !inc_dec_prepare_protected(0, pass == 0u, &state);

        if (!failed) {
            state.machine->executor_cpu.data.edx = 1u;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= core_machine_memory_write(state.machine, 0x3010u, &before,
                sizeof(before)) != TYPE_STATUS_OK || core_machine_memory_write(
                    state.machine, 0x2000u, code, sizeof(code)) != TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
                &result) != TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                core_machine_memory_read_physical(&state.machine->executor_memory, 0x3010u,
                    (type_virtual_address)&observed, sizeof(observed)) != TYPE_STATUS_OK ||
                observed != before || after.data.edx != 1u || after.data.eflags != flags ||
                after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_adc_forms(C_VOID)
{
    static const type_unsigned_8 forms[][9] = {
        { 0x10u,0xd1u }, { 0x11u,0xd1u }, { 0x66u,0x11u,0xd1u },
        { 0x12u,0xd1u }, { 0x13u,0xd1u }, { 0x66u,0x13u,0xd1u },
        { 0x10u,0x16u,0u,0x50u }, { 0x11u,0x16u,0u,0x50u },
        { 0x66u,0x11u,0x16u,0u,0x50u }, { 0x12u,0x16u,0u,0x50u },
        { 0x13u,0x16u,0u,0x50u }, { 0x66u,0x13u,0x16u,0u,0x50u },
        { 0x14u,0u }, { 0x15u,0u,0u }, { 0x66u,0x15u,0u,0u,0u,0u },
        { 0x80u,0xd1u,0u }, { 0x81u,0xd1u,0u,0u },
        { 0x66u,0x81u,0xd1u,0u,0u,0u,0u }, { 0x83u,0xd1u,0u },
        { 0x66u,0x83u,0xd1u,0u }, { 0x80u,0x16u,0u,0x50u,0u },
        { 0x81u,0x16u,0u,0x50u,0u,0u },
        { 0x66u,0x81u,0x16u,0u,0x50u,0u,0u },
        { 0x83u,0x16u,0u,0x50u,0u }, { 0x66u,0x83u,0x16u,0u,0x50u,0u }
    };
    static const type_unsigned_8 lengths[] = { 2u,2u,3u,2u,2u,3u,4u,4u,5u,4u,4u,5u,
        2u,3u,6u,3u,4u,7u,3u,4u,5u,6u,9u,5u,6u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const type_unsigned_8 bytes = (form == 0u || form == 3u || form == 6u ||
            form == 9u || form == 12u || form == 15u || form == 20u) ? 1u :
            ((form == 1u || form == 4u || form == 7u || form == 10u ||
              form == 13u || form == 16u || form == 18u || form == 21u ||
              form == 23u) ? 2u : 4u);
        const C_INT memory = (form >= 6u && form < 12u) || form >= 20u;
        const C_INT register_destination = (form >= 3u && form < 6u) ||
            (form >= 9u && form < 12u);
        const C_INT memory_source = form >= 9u && form < 12u;
        const C_INT accumulator = form >= 12u && form < 15u;
        const C_INT immediate = form >= 12u;
        const type_unsigned_32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const type_unsigned_32 destination = bytes == 1u ? 0x112233ffu :
            (bytes == 2u ? 0x1122ffffu : mask);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 observed = 0u;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = destination;
            state.machine->executor_cpu.data.ecx = register_destination ? 0u : destination;
            state.machine->executor_cpu.data.edx = (register_destination || memory_source) ?
                destination : 0u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
            if (memory) {
                const type_unsigned_32 memory_value = memory_source ? 0u : destination;
                failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY,
                    &memory_value, bytes) != TYPE_STATUS_OK;
            }
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
            if (memory) failed |= core_machine_memory_read(state.machine, INC_DEC_MEMORY,
                &observed, bytes) != TYPE_STATUS_OK || observed != (memory_source ? 0u : 0u);
            else if (accumulator) failed |= (after.data.eax & mask) != 0u;
            else if (immediate || !register_destination) failed |= (after.data.ecx & mask) != 0u;
            else failed |= (after.data.edx & mask) != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_adc_signed_overflow(C_VOID)
{
    static const type_unsigned_8 forms[][6] = {
        { 0x14u,0u }, { 0x15u,0u,0u }, { 0x66u,0x15u,0u,0u,0u,0u }
    };
    static const type_unsigned_8 lengths[] = { 2u,3u,6u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const type_unsigned_8 bytes = form == 0u ? 1u : (form == 1u ? 2u : 4u);
        const type_unsigned_32 initial = bytes == 1u ? 0x1122337fu :
            (bytes == 2u ? 0x11227fffu : 0x7fffffffu);
        const type_unsigned_32 expected = bytes == 1u ? 0x11223380u :
            (bytes == 2u ? 0x11228000u : 0x80000000u);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = initial;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.eax != expected ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF |
                        (bytes == 1u ? 0u : VCPU_EFLAGS_PF));
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_adc_attribute_profile_fault(C_VOID)
{
    static const type_unsigned_8 address_code[] = { 0x67u,0x66u,0x11u,0x16u };
    static const type_unsigned_8 rejected[] = { 0x66u,0x11u,0xd1u };
    static const type_unsigned_8 legacy[] = { 0x11u,0xd1u };
    static const type_unsigned_8 fault_code[] = { 0x11u,0x16u,0x10u,0u };
    inc_dec_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_unsigned_32 value = 0xffffffffu;
    type_unsigned_16 before = 0xffffu;
    type_unsigned_16 observed = 0u;
    C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        state.machine->executor_cpu.data.esi = INC_DEC_MEMORY;
        state.machine->executor_cpu.data.edx = 0u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
        failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY, &value,
            sizeof(value)) != TYPE_STATUS_OK || !inc_dec_run(&state, address_code,
                sizeof(address_code), 0, &after, &diagnostic) || diagnostic.first_fault.valid ||
            after.data.edx != 0u || (after.data.eflags & ADD_DEFINED_FLAGS) !=
                (VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF) ||
            core_machine_memory_read(state.machine, INC_DEC_MEMORY, &value,
                sizeof(value)) != TYPE_STATUS_OK || value != 0u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);
    if (!failed) {
        state.machine->executor_cpu.data.ecx = 0x1122ffffu;
        state.machine->executor_cpu.data.edx = 0u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
        failed |= !inc_dec_run(&state, rejected, sizeof(rejected), 1, &after, &diagnostic) ||
            !diagnostic.first_fault.valid || !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_UD) || after.data.ecx != 0x1122ffffu ||
            after.data.edx != 0u || after.data.eflags != VCPU_EFLAGS_CF || after.data.eip != 0u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);
    if (!failed) {
        state.machine->executor_cpu.data.ecx = 0xaabbffffu;
        state.machine->executor_cpu.data.edx = 0u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
        failed |= !inc_dec_run(&state, legacy, sizeof(legacy), 0, &after, &diagnostic) ||
            diagnostic.first_fault.valid || after.data.ecx != 0xaabb0000u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    failed = !inc_dec_prepare_protected(0, 1, &state);
    if (!failed) {
        state.machine->executor_cpu.data.edx = 0u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
        failed |= core_machine_memory_write(state.machine, 0x3010u, &before,
            sizeof(before)) != TYPE_STATUS_OK || core_machine_memory_write(state.machine,
                0x2000u, fault_code, sizeof(fault_code)) != TYPE_STATUS_OK;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
            &result) != TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
            core_machine_memory_read_physical(&state.machine->executor_memory, 0x3010u,
                (type_virtual_address)&observed, sizeof(observed)) != TYPE_STATUS_OK ||
            observed != before || after.data.edx != 0u || after.data.eflags != VCPU_EFLAGS_CF ||
            after.data.eip != 0u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT inc_dec_test_sbb_forms(C_VOID)
{
    static const type_unsigned_8 forms[][9] = {
        { 0x18u, 0xd1u },
        { 0x19u, 0xd1u },
        { 0x66u, 0x19u, 0xd1u },
        { 0x1au, 0xd1u },
        { 0x1bu, 0xd1u },
        { 0x66u, 0x1bu, 0xd1u },
        { 0x18u, 0x16u, 0u, 0x50u },
        { 0x19u, 0x16u, 0u, 0x50u },
        { 0x66u, 0x19u, 0x16u, 0u, 0x50u },
        { 0x1au, 0x16u, 0u, 0x50u },
        { 0x1bu, 0x16u, 0u, 0x50u },
        { 0x66u, 0x1bu, 0x16u, 0u, 0x50u },
        { 0x1cu, 0u },
        { 0x1du, 0u, 0u },
        { 0x66u, 0x1du, 0u, 0u, 0u, 0u },
        { 0x80u, 0xd9u, 0u },
        { 0x81u, 0xd9u, 0u, 0u },
        { 0x66u, 0x81u, 0xd9u, 0u, 0u, 0u, 0u },
        { 0x83u, 0xd9u, 0u },
        { 0x66u, 0x83u, 0xd9u, 0u },
        { 0x80u, 0x1eu, 0u, 0x50u, 0u },
        { 0x81u, 0x1eu, 0u, 0x50u, 0u, 0u },
        { 0x66u, 0x81u, 0x1eu, 0u, 0x50u, 0u, 0u },
        { 0x83u, 0x1eu, 0u, 0x50u, 0u },
        { 0x66u, 0x83u, 0x1eu, 0u, 0x50u, 0u }
    };
    static const type_unsigned_8 lengths[] = {
        2u, 2u, 3u, 2u, 2u, 3u, 4u, 4u, 5u, 4u, 4u, 5u,
        2u, 3u, 6u, 3u, 4u, 7u, 3u, 4u, 5u, 6u, 9u, 5u, 6u
    };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const type_unsigned_8 bytes = (form == 0u || form == 3u || form == 6u ||
            form == 9u || form == 12u || form == 15u || form == 20u) ? 1u :
            ((form == 1u || form == 4u || form == 7u || form == 10u ||
              form == 13u || form == 16u || form == 18u || form == 21u ||
              form == 23u) ? 2u : 4u);
        const C_INT memory = (form >= 6u && form < 12u) || form >= 20u;
        const C_INT register_destination = (form >= 3u && form < 6u) ||
            (form >= 9u && form < 12u);
        const C_INT memory_source = form >= 9u && form < 12u;
        const C_INT accumulator = form >= 12u && form < 15u;
        const C_INT immediate = form >= 12u;
        const type_unsigned_32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 observed = 0u;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
        if (!failed) {
            state.machine->executor_cpu.data.eax = 0u;
            state.machine->executor_cpu.data.ecx = 0u;
            state.machine->executor_cpu.data.edx = 0u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
            if (memory) {
                const type_unsigned_32 value = 0u;
                failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY,
                    &value, bytes) != TYPE_STATUS_OK;
            }
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_CF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
            if (memory) {
                failed |= core_machine_memory_read(state.machine, INC_DEC_MEMORY,
                    &observed, bytes) != TYPE_STATUS_OK ||
                    observed != (memory_source ? 0u : mask);
            } else if (accumulator) {
                failed |= (after.data.eax & mask) != mask;
            } else if (immediate || !register_destination) {
                failed |= (after.data.ecx & mask) != mask;
            } else {
                failed |= (after.data.edx & mask) != mask;
            }
            if (!memory && !accumulator && !immediate && register_destination) {
                failed |= after.data.ecx != 0u;
            }
            if (!memory && !accumulator && !immediate && !register_destination) {
                failed |= after.data.edx != 0u;
            }
            if (memory && !memory_source) {
                failed |= after.data.edx != 0u;
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_sbb_boundaries(C_VOID)
{
    static const type_unsigned_8 overflow[][6] = {
        { 0x1cu, 0u },
        { 0x1du, 0u, 0u },
        { 0x66u, 0x1du, 0u, 0u, 0u, 0u }
    };
    static const type_unsigned_8 overflow_lengths[] = { 2u, 3u, 6u };
    static const type_unsigned_8 sign_extended_immediate[][4] = {
        { 0x80u, 0xd9u, 0xffu },
        { 0x83u, 0xd9u, 0xffu },
        { 0x66u, 0x83u, 0xd9u, 0xffu }
    };
    static const type_unsigned_8 sign_extended_immediate_lengths[] = { 3u, 3u, 4u };
    static const type_unsigned_8 address_code[] = { 0x67u, 0x66u, 0x19u, 0x16u };
    static const type_unsigned_8 rejected[] = { 0x66u, 0x19u, 0xd1u };
    static const type_unsigned_8 legacy[] = { 0x19u, 0xd1u };
    static const type_unsigned_8 fault_code[] = { 0x19u, 0x16u, 0x10u, 0u };
    type_unsigned_8 form;
    type_unsigned_8 pass;

    for (form = 0u; form != sizeof(overflow_lengths); ++form) {
        const type_unsigned_8 bytes = form == 0u ? 1u : (form == 1u ? 2u : 4u);
        const type_unsigned_32 initial = bytes == 1u ? 0x11223380u :
            (bytes == 2u ? 0x11228000u : 0x80000000u);
        const type_unsigned_32 expected = bytes == 1u ? 0x1122337fu :
            (bytes == 2u ? 0x11227fffu : 0x7fffffffu);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = initial;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
            failed |= !inc_dec_run(&state, overflow[form], overflow_lengths[form], 0,
                &after, &diagnostic) || diagnostic.first_fault.valid ||
                after.data.eax != expected || (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF |
                        (bytes == 1u ? 0u : VCPU_EFLAGS_PF));
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (form = 0u; form != sizeof(sign_extended_immediate_lengths); ++form) {
        const type_unsigned_32 initial = form == 2u ? 0u : 0xaabb0000u;
        const type_unsigned_32 expected = form == 2u ? 1u : 0xaabb0001u;
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.ecx = initial;
            state.machine->executor_cpu.data.edx = 0x55667788u;
            state.machine->executor_cpu.data.eflags = 0u;
            failed |= !inc_dec_run(&state, sign_extended_immediate[form],
                sign_extended_immediate_lengths[form], 0, &after, &diagnostic) ||
                diagnostic.first_fault.valid || after.data.ecx != expected ||
                after.data.edx != 0x55667788u ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_CF | VCPU_EFLAGS_AF);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 value = 0u;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.esi = INC_DEC_MEMORY;
            state.machine->executor_cpu.data.edx = 0u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
            failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY, &value,
                sizeof(value)) != TYPE_STATUS_OK || !inc_dec_run(&state, address_code,
                    sizeof(address_code), 0, &after, &diagnostic) ||
                diagnostic.first_fault.valid || core_machine_memory_read(state.machine,
                    INC_DEC_MEMORY, &value, sizeof(value)) != TYPE_STATUS_OK ||
                value != 0xffffffffu;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);

        if (!failed) {
            state.machine->executor_cpu.data.ecx = 0x11220000u;
            state.machine->executor_cpu.data.edx = 0u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
            failed |= !inc_dec_run(&state, rejected, sizeof(rejected), 1, &after,
                &diagnostic) || !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.ecx != 0x11220000u || after.data.edx != 0u ||
                after.data.eflags != VCPU_EFLAGS_CF || after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);

        if (!failed) {
            state.machine->executor_cpu.data.ecx = 0xaabb0000u;
            state.machine->executor_cpu.data.edx = 0u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
            failed |= !inc_dec_run(&state, legacy, sizeof(legacy), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                after.data.ecx != 0xaabbffffu || after.data.edx != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (pass = 0u; pass != 2u; ++pass) {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        type_unsigned_16 before = 0u;
        type_unsigned_16 observed = 0u;
        C_INT failed = !inc_dec_prepare_protected(0, pass == 0u, &state);

        if (!failed) {
            state.machine->executor_cpu.data.edx = 0u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
            failed |= core_machine_memory_write(state.machine, 0x3010u, &before,
                sizeof(before)) != TYPE_STATUS_OK || core_machine_memory_write(
                    state.machine, 0x2000u, fault_code, sizeof(fault_code)) !=
                TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
                &result) != TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                core_machine_memory_read_physical(&state.machine->executor_memory, 0x3010u,
                    (type_virtual_address)&observed, sizeof(observed)) != TYPE_STATUS_OK ||
                observed != before || after.data.edx != 0u ||
                after.data.eflags != VCPU_EFLAGS_CF || after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_or_rm_reg_forms(C_VOID)
{
    static const type_unsigned_8 forms[][6] = {
        { 0x08u, 0xd1u },
        { 0x09u, 0xd1u },
        { 0x66u, 0x09u, 0xd1u },
        { 0x0au, 0xd1u },
        { 0x0bu, 0xd1u },
        { 0x66u, 0x0bu, 0xd1u },
        { 0x08u, 0x16u, 0u, 0x50u },
        { 0x09u, 0x16u, 0u, 0x50u },
        { 0x66u, 0x09u, 0x16u, 0u, 0x50u },
        { 0x0au, 0x16u, 0u, 0x50u },
        { 0x0bu, 0x16u, 0u, 0x50u },
        { 0x66u, 0x0bu, 0x16u, 0u, 0x50u }
    };
    static const type_unsigned_8 lengths[] = {
        2u, 2u, 3u, 2u, 2u, 3u, 4u, 4u, 5u, 4u, 4u, 5u
    };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const type_unsigned_8 bytes = form == 0u || form == 3u || form == 6u ||
            form == 9u ? 1u : (form == 1u || form == 4u || form == 7u ||
                form == 10u ? 2u : 4u);
        const C_INT memory = form >= 6u;
        const C_INT register_destination = !memory && form >= 3u;
        const C_INT memory_source = memory && form >= 9u;
        const type_unsigned_32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const type_unsigned_32 destination = bytes == 1u ? 0x11223380u :
            (bytes == 2u ? 0x11228000u : 0x80000000u);
        const type_unsigned_32 expected = destination | 1u;
        const type_unsigned_32 flags = VCPU_EFLAGS_SF |
            (bytes == 1u ? VCPU_EFLAGS_PF : 0u);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 observed = 0u;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.ecx = register_destination || memory_source ?
                1u : destination;
            state.machine->executor_cpu.data.edx = register_destination || memory_source ?
                destination : 1u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_AF |
                VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
            if (memory) {
                const type_unsigned_32 value = memory_source ? 1u : destination;
                failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY,
                    &value, bytes) != TYPE_STATUS_OK;
            }
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                (after.data.eflags & OR_DEFINED_FLAGS) !=
                    flags;
            if (memory) {
                failed |= core_machine_memory_read(state.machine, INC_DEC_MEMORY,
                    &observed, bytes) != TYPE_STATUS_OK ||
                    observed != (memory_source ? 1u : (expected & mask));
                failed |= after.data.edx != (memory_source ? expected : 1u);
            } else if (register_destination) {
                failed |= after.data.edx != expected || after.data.ecx != 1u;
            } else {
                failed |= after.data.ecx != expected || after.data.edx != 1u;
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_or_immediate_forms(C_VOID)
{
    static const type_unsigned_8 forms[][9] = {
        { 0x0cu, 1u },
        { 0x0du, 1u, 0x80u },
        { 0x66u, 0x0du, 1u, 0u, 0u, 0x80u },
        { 0x80u, 0xc9u, 1u },
        { 0x81u, 0xc9u, 1u, 0x80u },
        { 0x66u, 0x81u, 0xc9u, 1u, 0u, 0u, 0x80u },
        { 0x83u, 0xc9u, 0xffu },
        { 0x66u, 0x83u, 0xc9u, 0xffu },
        { 0x80u, 0x0eu, 0u, 0x50u, 1u },
        { 0x81u, 0x0eu, 0u, 0x50u, 1u, 0x80u },
        { 0x66u, 0x81u, 0x0eu, 0u, 0x50u, 1u, 0u, 0u, 0x80u },
        { 0x83u, 0x0eu, 0u, 0x50u, 0xffu },
        { 0x66u, 0x83u, 0x0eu, 0u, 0x50u, 0xffu }
    };
    static const type_unsigned_8 lengths[] = {
        2u, 3u, 6u, 3u, 4u, 7u, 3u, 4u, 5u, 6u, 9u, 5u, 6u
    };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const type_unsigned_8 bytes = form == 0u || form == 3u || form == 8u ? 1u :
            (form == 1u || form == 4u || form == 6u || form == 9u ||
                form == 11u ? 2u : 4u);
        const C_INT memory = form >= 8u;
        const C_INT accumulator = form < 3u;
        const C_INT signed_immediate = form == 6u || form == 7u ||
            form == 11u || form == 12u;
        const type_unsigned_32 destination = signed_immediate ? 0u :
            (bytes == 1u ? 0x11223380u : (bytes == 2u ? 0x11228000u :
                0x80000000u));
        const type_unsigned_32 expected = signed_immediate ?
            (bytes == 2u ? 0xffffu : 0xffffffffu) : destination | 1u;
        const type_unsigned_32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const type_unsigned_32 flags = VCPU_EFLAGS_SF |
            (signed_immediate || bytes == 1u ? VCPU_EFLAGS_PF : 0u);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 observed = 0u;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = destination;
            state.machine->executor_cpu.data.ecx = destination;
            state.machine->executor_cpu.data.edx = 0x55667788u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_AF |
                VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
            if (memory) {
                failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY,
                    &destination, bytes) != TYPE_STATUS_OK;
            }
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                after.data.edx != 0x55667788u ||
                (after.data.eflags & OR_DEFINED_FLAGS) !=
                    flags;
            if (memory) {
                failed |= core_machine_memory_read(state.machine, INC_DEC_MEMORY,
                    &observed, bytes) != TYPE_STATUS_OK || observed != (expected & mask);
            } else if (accumulator) {
                failed |= after.data.eax != expected;
            } else {
                failed |= after.data.ecx != expected;
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_or_attribute_profile_fault(C_VOID)
{
    static const type_unsigned_8 address_code[] = { 0x67u, 0x66u, 0x09u, 0x16u };
    static const type_unsigned_8 rejected[] = { 0x66u, 0x09u, 0xd1u };
    static const type_unsigned_8 legacy[] = { 0x09u, 0xd1u };
    static const type_unsigned_8 fault_code[] = { 0x09u, 0x16u, 0x10u, 0u };
    type_unsigned_8 pass;

    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 value = 0x80000000u;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.esi = INC_DEC_MEMORY;
            state.machine->executor_cpu.data.edx = 1u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_AF |
                VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
            failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY, &value,
                sizeof(value)) != TYPE_STATUS_OK || !inc_dec_run(&state, address_code,
                    sizeof(address_code), 0, &after, &diagnostic) ||
                diagnostic.first_fault.valid || after.data.edx != 1u ||
                (after.data.eflags & OR_DEFINED_FLAGS) !=
                    VCPU_EFLAGS_SF ||
                core_machine_memory_read(state.machine, INC_DEC_MEMORY, &value,
                    sizeof(value)) != TYPE_STATUS_OK || value != 0x80000001u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);

        if (!failed) {
            state.machine->executor_cpu.data.ecx = 0x11228000u;
            state.machine->executor_cpu.data.edx = 1u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_AF |
                VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
            failed |= !inc_dec_run(&state, rejected, sizeof(rejected), 1, &after,
                &diagnostic) || !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.ecx != 0x11228000u || after.data.edx != 1u ||
                after.data.eflags != (VCPU_EFLAGS_AF | VCPU_EFLAGS_CF |
                    VCPU_EFLAGS_OF) || after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);

        if (!failed) {
            state.machine->executor_cpu.data.ecx = 0xaabb8000u;
            state.machine->executor_cpu.data.edx = 1u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_AF |
                VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
            failed |= !inc_dec_run(&state, legacy, sizeof(legacy), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                after.data.ecx != 0xaabb8001u || after.data.edx != 1u ||
                (after.data.eflags & OR_DEFINED_FLAGS) !=
                    VCPU_EFLAGS_SF;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (pass = 0u; pass != 2u; ++pass) {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        type_unsigned_16 before = 0x8000u;
        type_unsigned_16 observed = 0u;
        const type_unsigned_32 flags = VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
        C_INT failed = !inc_dec_prepare_protected(0, pass == 0u, &state);

        if (!failed) {
            state.machine->executor_cpu.data.edx = 1u;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= core_machine_memory_write(state.machine, 0x3010u, &before,
                sizeof(before)) != TYPE_STATUS_OK || core_machine_memory_write(
                    state.machine, 0x2000u, fault_code, sizeof(fault_code)) !=
                TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
                &result) != TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                core_machine_memory_read_physical(&state.machine->executor_memory, 0x3010u,
                    (type_virtual_address)&observed, sizeof(observed)) != TYPE_STATUS_OK ||
                observed != before || after.data.edx != 1u ||
                after.data.eflags != flags || after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_and_forms(C_VOID)
{
    static const type_unsigned_8 forms[][6] = {
        { 0x20u, 0xd1u }, { 0x21u, 0xd1u }, { 0x66u, 0x21u, 0xd1u },
        { 0x22u, 0xd1u }, { 0x23u, 0xd1u }, { 0x66u, 0x23u, 0xd1u },
        { 0x20u, 0x16u, 0u, 0x50u }, { 0x21u, 0x16u, 0u, 0x50u },
        { 0x66u, 0x21u, 0x16u, 0u, 0x50u }, { 0x22u, 0x16u, 0u, 0x50u },
        { 0x23u, 0x16u, 0u, 0x50u }, { 0x66u, 0x23u, 0x16u, 0u, 0x50u }
    };
    static const type_unsigned_8 lengths[] = { 2u, 2u, 3u, 2u, 2u, 3u, 4u, 4u,
        5u, 4u, 4u, 5u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const type_unsigned_8 bytes = form == 0u || form == 3u || form == 6u ||
            form == 9u ? 1u : (form == 1u || form == 4u || form == 7u ||
                form == 10u ? 2u : 4u);
        const C_INT memory = form >= 6u;
        const C_INT register_destination = !memory && form >= 3u;
        const C_INT memory_source = memory && form >= 9u;
        const type_unsigned_32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const type_unsigned_32 destination = bytes == 1u ? 0x11223381u :
            (bytes == 2u ? 0x11228081u : 0x80000081u);
        const type_unsigned_32 result = destination & mask;
        const type_unsigned_32 expected = (destination & ~mask) | result;
        inc_dec_machine state;
        t_cpu after = {0};
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 observed = 0u;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.ecx = register_destination || memory_source ?
                mask : destination;
            state.machine->executor_cpu.data.edx = register_destination || memory_source ?
                destination : mask;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_AF |
                VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
            if (memory) {
                const type_unsigned_32 value = memory_source ? mask : destination;
                failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY,
                    &value, bytes) != TYPE_STATUS_OK;
            }
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                (after.data.eflags & AND_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_SF | VCPU_EFLAGS_PF);
            if (memory) {
                failed |= core_machine_memory_read(state.machine, INC_DEC_MEMORY,
                    &observed, bytes) != TYPE_STATUS_OK ||
                    observed != (memory_source ? mask : result);
                failed |= after.data.edx != (memory_source ? expected : mask);
            } else if (register_destination) {
                failed |= after.data.edx != expected || after.data.ecx != mask;
            } else {
                failed |= after.data.ecx != expected || after.data.edx != mask;
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_and_immediate(C_VOID)
{
    static const type_unsigned_8 forms[][9] = {
        { 0x24u, 0xffu }, { 0x25u, 0xffu, 0xffu },
        { 0x66u, 0x25u, 0xffu, 0xffu, 0xffu, 0xffu },
        { 0x80u, 0xe1u, 0xffu }, { 0x81u, 0xe1u, 0xffu, 0xffu },
        { 0x66u, 0x81u, 0xe1u, 0xffu, 0xffu, 0xffu, 0xffu },
        { 0x83u, 0xe1u, 0xffu }, { 0x66u, 0x83u, 0xe1u, 0xffu },
        { 0x80u, 0x26u, 0u, 0x50u, 0xffu },
        { 0x81u, 0x26u, 0u, 0x50u, 0xffu, 0xffu },
        { 0x66u, 0x81u, 0x26u, 0u, 0x50u, 0xffu, 0xffu, 0xffu, 0xffu },
        { 0x83u, 0x26u, 0u, 0x50u, 0xffu },
        { 0x66u, 0x83u, 0x26u, 0u, 0x50u, 0xffu }
    };
    static const type_unsigned_8 lengths[] = { 2u, 3u, 6u, 3u, 4u, 7u, 3u, 4u,
        5u, 6u, 9u, 5u, 6u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const type_unsigned_8 bytes = form == 0u || form == 3u || form == 8u ? 1u :
            (form == 1u || form == 4u || form == 6u || form == 9u ||
                form == 11u ? 2u : 4u);
        const C_INT memory = form >= 8u;
        const C_INT accumulator = form < 3u;
        const type_unsigned_32 destination = bytes == 1u ? 0x11223381u :
            (bytes == 2u ? 0x11228081u : 0x80000081u);
        const type_unsigned_32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const type_unsigned_32 result = destination & mask;
        const type_unsigned_32 expected = (destination & ~mask) | result;
        inc_dec_machine state;
        t_cpu after = {0};
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 observed = 0u;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = destination;
            state.machine->executor_cpu.data.ecx = destination;
            state.machine->executor_cpu.data.edx = 0x55667788u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_AF |
                VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
            if (memory) failed |= core_machine_memory_write(state.machine,
                INC_DEC_MEMORY, &destination, bytes) != TYPE_STATUS_OK;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                after.data.edx != 0x55667788u ||
                (after.data.eflags & AND_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_SF | VCPU_EFLAGS_PF);
            if (memory) failed |= core_machine_memory_read(state.machine,
                INC_DEC_MEMORY, &observed, bytes) != TYPE_STATUS_OK ||
                observed != result;
            else if (accumulator) failed |= after.data.eax != expected;
            else failed |= after.data.ecx != expected;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_and_attribute_profile_fault(C_VOID)
{
    static const type_unsigned_8 address_code[] = { 0x67u, 0x66u, 0x21u, 0x16u };
    static const type_unsigned_8 rejected[] = { 0x66u, 0x21u, 0xd1u };
    static const type_unsigned_8 legacy[] = { 0x21u, 0xd1u };
    static const type_unsigned_8 fault_code[] = { 0x21u, 0x16u, 0x10u, 0u };
    type_unsigned_8 pass;

    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 value = 0x80000081u;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
        if (!failed) {
            state.machine->executor_cpu.data.esi = INC_DEC_MEMORY;
            state.machine->executor_cpu.data.edx = 0xffffffffu;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_AF |
                VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
            failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY, &value,
                sizeof(value)) != TYPE_STATUS_OK || !inc_dec_run(&state, address_code,
                    sizeof(address_code), 0, &after, &diagnostic) ||
                diagnostic.first_fault.valid || after.data.edx != 0xffffffffu ||
                (after.data.eflags & AND_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_SF | VCPU_EFLAGS_PF) ||
                core_machine_memory_read(state.machine, INC_DEC_MEMORY, &value,
                    sizeof(value)) != TYPE_STATUS_OK || value != 0x80000081u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        const type_unsigned_32 flags = VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);
        if (!failed) {
            state.machine->executor_cpu.data.ecx = 0x11228081u;
            state.machine->executor_cpu.data.edx = 0xffffu;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= !inc_dec_run(&state, rejected, sizeof(rejected), 1, &after,
                &diagnostic) || !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.ecx != 0x11228081u || after.data.edx != 0xffffu ||
                after.data.eflags != flags || after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);
        if (!failed) {
            state.machine->executor_cpu.data.ecx = 0xaabb8081u;
            state.machine->executor_cpu.data.edx = 0xffffu;
            failed |= !inc_dec_run(&state, legacy, sizeof(legacy), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                after.data.ecx != 0xaabb8081u || after.data.edx != 0xffffu ||
                (after.data.eflags & AND_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_SF | VCPU_EFLAGS_PF);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (pass = 0u; pass != 2u; ++pass) {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        type_unsigned_16 before = 0x8081u;
        type_unsigned_16 observed = 0u;
        const type_unsigned_32 flags = VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
        C_INT failed = !inc_dec_prepare_protected(0, pass == 0u, &state);
        if (!failed) {
            state.machine->executor_cpu.data.edx = 0xffffu;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= core_machine_memory_write(state.machine, 0x3010u, &before,
                sizeof(before)) != TYPE_STATUS_OK || core_machine_memory_write(
                    state.machine, 0x2000u, fault_code, sizeof(fault_code)) !=
                TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
                &result) != TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                core_machine_memory_read_physical(&state.machine->executor_memory, 0x3010u,
                    (type_virtual_address)&observed, sizeof(observed)) != TYPE_STATUS_OK ||
                observed != before || after.data.edx != 0xffffu ||
                after.data.eflags != flags || after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_and_zero_flags(C_VOID)
{
    static const type_unsigned_8 forms[][6] = {
        { 0x24u, 0u },
        { 0x25u, 0u, 0u },
        { 0x66u, 0x25u, 0u, 0u, 0u, 0u }
    };
    static const type_unsigned_8 lengths[] = { 2u, 3u, 6u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0xffffffffu;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_AF |
                VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                (after.data.eflags & AND_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF) ||
                (form == 0u ? after.data.eax != 0xffffff00u :
                    (form == 1u ? after.data.eax != 0xffff0000u :
                        after.data.eax != 0u));
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_sub_forms(C_VOID)
{
    static const type_unsigned_8 forms[][9] = {
        { 0x28u, 0xd1u }, { 0x29u, 0xd1u }, { 0x66u, 0x29u, 0xd1u },
        { 0x2au, 0xd1u }, { 0x2bu, 0xd1u }, { 0x66u, 0x2bu, 0xd1u },
        { 0x28u, 0x16u, 0u, 0x50u }, { 0x29u, 0x16u, 0u, 0x50u },
        { 0x66u, 0x29u, 0x16u, 0u, 0x50u }, { 0x2au, 0x16u, 0u, 0x50u },
        { 0x2bu, 0x16u, 0u, 0x50u }, { 0x66u, 0x2bu, 0x16u, 0u, 0x50u },
        { 0x2cu, 1u }, { 0x2du, 1u, 0u }, { 0x66u, 0x2du, 1u, 0u, 0u, 0u },
        { 0x80u, 0xe9u, 1u }, { 0x81u, 0xe9u, 1u, 0u },
        { 0x66u, 0x81u, 0xe9u, 1u, 0u, 0u, 0u }, { 0x83u, 0xe9u, 1u },
        { 0x66u, 0x83u, 0xe9u, 1u }, { 0x80u, 0x2eu, 0u, 0x50u, 1u },
        { 0x81u, 0x2eu, 0u, 0x50u, 1u, 0u },
        { 0x66u, 0x81u, 0x2eu, 0u, 0x50u, 1u, 0u, 0u, 0u },
        { 0x83u, 0x2eu, 0u, 0x50u, 1u }, { 0x66u, 0x83u, 0x2eu, 0u, 0x50u, 1u }
    };
    static const type_unsigned_8 lengths[] = { 2u, 2u, 3u, 2u, 2u, 3u, 4u, 4u, 5u, 4u,
        4u, 5u, 2u, 3u, 6u, 3u, 4u, 7u, 3u, 4u, 5u, 6u, 9u, 5u, 6u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const type_unsigned_8 bytes = (form == 0u || form == 3u || form == 6u ||
            form == 9u || form == 12u || form == 15u || form == 20u) ? 1u :
            ((form == 1u || form == 4u || form == 7u || form == 10u ||
              form == 13u || form == 16u || form == 18u || form == 21u ||
              form == 23u) ? 2u : 4u);
        const C_INT memory = (form >= 6u && form < 12u) || form >= 20u;
        const C_INT register_destination = (form >= 3u && form < 6u) ||
            (form >= 9u && form < 12u);
        const C_INT memory_source = form >= 9u && form < 12u;
        const C_INT accumulator = form >= 12u && form < 15u;
        const C_INT immediate = form >= 12u;
        const type_unsigned_32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 observed = 0u;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0u;
            state.machine->executor_cpu.data.ecx = register_destination || memory_source ?
                1u : 0u;
            state.machine->executor_cpu.data.edx = register_destination || memory_source ?
                0u : 1u;
            if (memory) {
                const type_unsigned_32 value = memory_source ? 1u : 0u;
                failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY,
                    &value, bytes) != TYPE_STATUS_OK;
            }
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_CF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
            if (memory) {
                failed |= core_machine_memory_read(state.machine, INC_DEC_MEMORY,
                    &observed, bytes) != TYPE_STATUS_OK ||
                    observed != (memory_source ? 1u : mask) ||
                    after.data.edx != (memory_source ? mask : 1u);
            } else if (accumulator) {
                failed |= (after.data.eax & mask) != mask;
            } else if (immediate || !register_destination) {
                failed |= (after.data.ecx & mask) != mask ||
                    (!immediate && after.data.edx != 1u);
            } else {
                failed |= (after.data.edx & mask) != mask || after.data.ecx != 1u;
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_sub_boundaries(C_VOID)
{
    static const type_unsigned_8 overflow[][6] = {
        { 0x2cu, 1u }, { 0x2du, 1u, 0u }, { 0x66u, 0x2du, 1u, 0u, 0u, 0u }
    };
    static const type_unsigned_8 lengths[] = { 2u, 3u, 6u };
    static const type_unsigned_8 signext[][4] = {
        { 0x80u, 0xe9u, 0xffu }, { 0x83u, 0xe9u, 0xffu },
        { 0x66u, 0x83u, 0xe9u, 0xffu }
    };
    static const type_unsigned_8 signlengths[] = { 3u, 3u, 4u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const type_unsigned_8 bytes = form == 0u ? 1u : (form == 1u ? 2u : 4u);
        const type_unsigned_32 initial = bytes == 1u ? 0x11223380u :
            (bytes == 2u ? 0x11228000u : 0x80000000u);
        const type_unsigned_32 expected = bytes == 1u ? 0x1122337fu :
            (bytes == 2u ? 0x11227fffu : 0x7fffffffu);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = initial;
            failed |= !inc_dec_run(&state, overflow[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.eax != expected ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF |
                        (bytes == 1u ? 0u : VCPU_EFLAGS_PF));
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (form = 0u; form != sizeof(signlengths); ++form) {
        const type_unsigned_32 expected = form == 2u ? 1u : 0xaabb0001u;
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.ecx = form == 2u ? 0u : 0xaabb0000u;
            state.machine->executor_cpu.data.edx = 0x55667788u;
            failed |= !inc_dec_run(&state, signext[form], signlengths[form], 0,
                &after, &diagnostic) || diagnostic.first_fault.valid ||
                after.data.ecx != expected || after.data.edx != 0x55667788u ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_CF | VCPU_EFLAGS_AF);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_sub_attribute_profile_fault(C_VOID)
{
    static const type_unsigned_8 address_code[] = { 0x67u, 0x66u, 0x29u, 0x16u };
    static const type_unsigned_8 rejected[] = { 0x66u, 0x29u, 0xd1u };
    static const type_unsigned_8 legacy[] = { 0x29u, 0xd1u };
    static const type_unsigned_8 fault_code[] = { 0x29u, 0x16u, 0x10u, 0u };
    type_unsigned_8 pass;
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 value = 0u;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
        if (!failed) {
            state.machine->executor_cpu.data.esi = INC_DEC_MEMORY;
            state.machine->executor_cpu.data.edx = 1u;
            failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY, &value,
                sizeof(value)) != TYPE_STATUS_OK || !inc_dec_run(&state, address_code,
                    sizeof(address_code), 0, &after, &diagnostic) ||
                diagnostic.first_fault.valid || after.data.edx != 1u ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_CF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF) ||
                core_machine_memory_read(state.machine, INC_DEC_MEMORY, &value,
                    sizeof(value)) != TYPE_STATUS_OK || value != 0xffffffffu;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        const type_unsigned_32 flags = VCPU_EFLAGS_CF;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);
        if (!failed) {
            state.machine->executor_cpu.data.ecx = 0x11220000u;
            state.machine->executor_cpu.data.edx = 1u;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= !inc_dec_run(&state, rejected, sizeof(rejected), 1, &after,
                &diagnostic) || !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.ecx != 0x11220000u || after.data.edx != 1u ||
                after.data.eflags != flags || after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);
        if (!failed) {
            state.machine->executor_cpu.data.ecx = 0xaabb0000u;
            state.machine->executor_cpu.data.edx = 1u;
            failed |= !inc_dec_run(&state, legacy, sizeof(legacy), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                after.data.ecx != 0xaabbffffu || after.data.edx != 1u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (pass = 0u; pass != 2u; ++pass) {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        type_unsigned_16 before = 0u;
        type_unsigned_16 observed = 0u;
        const type_unsigned_32 flags = VCPU_EFLAGS_CF;
        C_INT failed = !inc_dec_prepare_protected(0, pass == 0u, &state);
        if (!failed) {
            state.machine->executor_cpu.data.edx = 1u;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= core_machine_memory_write(state.machine, 0x3010u, &before,
                sizeof(before)) != TYPE_STATUS_OK || core_machine_memory_write(
                    state.machine, 0x2000u, fault_code, sizeof(fault_code)) !=
                TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
                &result) != TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                core_machine_memory_read_physical(&state.machine->executor_memory, 0x3010u,
                    (type_virtual_address)&observed, sizeof(observed)) != TYPE_STATUS_OK ||
                observed != before || after.data.edx != 1u ||
                after.data.eflags != flags || after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_xor_forms(C_VOID)
{
    static const type_unsigned_8 forms[][9] = {
        { 0x30u, 0xd1u }, { 0x31u, 0xd1u }, { 0x66u, 0x31u, 0xd1u },
        { 0x32u, 0xd1u }, { 0x33u, 0xd1u }, { 0x66u, 0x33u, 0xd1u },
        { 0x30u, 0x16u, 0u, 0x50u }, { 0x31u, 0x16u, 0u, 0x50u },
        { 0x66u, 0x31u, 0x16u, 0u, 0x50u }, { 0x32u, 0x16u, 0u, 0x50u },
        { 0x33u, 0x16u, 0u, 0x50u }, { 0x66u, 0x33u, 0x16u, 0u, 0x50u }
    };
    static const type_unsigned_8 lengths[] = { 2u, 2u, 3u, 2u, 2u, 3u, 4u, 4u, 5u,
        4u, 4u, 5u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const type_unsigned_8 bytes = form == 0u || form == 3u || form == 6u || form == 9u ?
            1u : (form == 1u || form == 4u || form == 7u || form == 10u ? 2u : 4u);
        const C_INT memory = form >= 6u;
        const C_INT register_destination = !memory && form >= 3u;
        const C_INT memory_source = memory && form >= 9u;
        const type_unsigned_32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const type_unsigned_32 value = bytes == 1u ? 0x11223381u :
            (bytes == 2u ? 0x11228081u : 0x80000081u);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 observed = 0u;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.ecx = value;
            state.machine->executor_cpu.data.edx = value;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_AF | VCPU_EFLAGS_CF |
                VCPU_EFLAGS_OF;
            if (memory) {
                failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY,
                    &value, bytes) != TYPE_STATUS_OK;
            }
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                (after.data.eflags & OR_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF);
            if (memory) {
                failed |= core_machine_memory_read(state.machine, INC_DEC_MEMORY, &observed,
                    bytes) != TYPE_STATUS_OK || observed != (memory_source ? (value & mask) : 0u) ||
                    (memory_source ? (after.data.edx & mask) : after.data.edx) !=
                        (memory_source ? 0u : value);
            } else if (register_destination) {
                failed |= (after.data.edx & mask) != 0u || after.data.ecx != value;
            } else {
                failed |= (after.data.ecx & mask) != 0u || after.data.edx != value;
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_xor_immediates(C_VOID)
{
    static const type_unsigned_8 forms[][9] = {
        { 0x34u, 0xffu }, { 0x35u, 0xffu, 0xffu },
        { 0x66u, 0x35u, 0xffu, 0xffu, 0xffu, 0xffu },
        { 0x80u, 0xf1u, 0xffu }, { 0x81u, 0xf1u, 0xffu, 0xffu },
        { 0x66u, 0x81u, 0xf1u, 0xffu, 0xffu, 0xffu, 0xffu },
        { 0x83u, 0xf1u, 0xffu }, { 0x66u, 0x83u, 0xf1u, 0xffu },
        { 0x80u, 0x36u, 0u, 0x50u, 0xffu },
        { 0x81u, 0x36u, 0u, 0x50u, 0xffu, 0xffu },
        { 0x66u, 0x81u, 0x36u, 0u, 0x50u, 0xffu, 0xffu, 0xffu, 0xffu },
        { 0x83u, 0x36u, 0u, 0x50u, 0xffu },
        { 0x66u, 0x83u, 0x36u, 0u, 0x50u, 0xffu }
    };
    static const type_unsigned_8 lengths[] = { 2u, 3u, 6u, 3u, 4u, 7u, 3u, 4u,
        5u, 6u, 9u, 5u, 6u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const type_unsigned_8 bytes = form == 0u || form == 3u || form == 8u ? 1u :
            (form == 1u || form == 4u || form == 6u || form == 9u ||
                form == 11u ? 2u : 4u);
        const C_INT memory = form >= 8u;
        const C_INT accumulator = form < 3u;
        const type_unsigned_32 value = bytes == 1u ? 0x112233ffu :
            (bytes == 2u ? 0x1122ffffu : 0xffffffffu);
        inc_dec_machine state;
        t_cpu after = {0};
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 observed = 0u;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
        if (!failed) {
            state.machine->executor_cpu.data.eax = value;
            state.machine->executor_cpu.data.ecx = value;
            state.machine->executor_cpu.data.edx = 0x55667788u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_AF | VCPU_EFLAGS_CF |
                VCPU_EFLAGS_OF;
            if (memory) failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY,
                &value, bytes) != TYPE_STATUS_OK;
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                after.data.edx != 0x55667788u ||
                (after.data.eflags & OR_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF);
            if (memory) {
                failed |= core_machine_memory_read(state.machine, INC_DEC_MEMORY,
                    &observed, bytes) != TYPE_STATUS_OK || observed != 0u;
            } else if (accumulator) {
                failed |= (after.data.eax & (bytes == 1u ? 0xffu :
                    (bytes == 2u ? 0xffffu : 0xffffffffu))) != 0u;
            } else {
                failed |= (after.data.ecx & (bytes == 1u ? 0xffu :
                    (bytes == 2u ? 0xffffu : 0xffffffffu))) != 0u;
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_xor_attribute_profile_fault(C_VOID)
{
    static const type_unsigned_8 address_code[] = { 0x67u, 0x66u, 0x31u, 0x16u };
    static const type_unsigned_8 rejected[] = { 0x66u, 0x31u, 0xd1u };
    static const type_unsigned_8 legacy[] = { 0x31u, 0xd1u };
    static const type_unsigned_8 fault_code[] = { 0x31u, 0x16u, 0x10u, 0u };
    type_unsigned_8 pass;

    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 value = 0x80000081u;
        const type_unsigned_32 flags = VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.esi = INC_DEC_MEMORY;
            state.machine->executor_cpu.data.edx = 0xffffffffu;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY, &value,
                sizeof(value)) != TYPE_STATUS_OK || !inc_dec_run(&state, address_code,
                    sizeof(address_code), 0, &after, &diagnostic) ||
                diagnostic.first_fault.valid || after.data.edx != 0xffffffffu ||
                (after.data.eflags & OR_DEFINED_FLAGS) != VCPU_EFLAGS_PF ||
                core_machine_memory_read(state.machine, INC_DEC_MEMORY, &value,
                    sizeof(value)) != TYPE_STATUS_OK || value != 0x7fffff7eu;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        const type_unsigned_32 flags = VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);

        if (!failed) {
            state.machine->executor_cpu.data.ecx = 0x11228081u;
            state.machine->executor_cpu.data.edx = 0xffffu;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= !inc_dec_run(&state, rejected, sizeof(rejected), 1, &after,
                &diagnostic) || !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.ecx != 0x11228081u || after.data.edx != 0xffffu ||
                after.data.eflags != flags || after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);

        if (!failed) {
            state.machine->executor_cpu.data.ecx = 0xaabb8081u;
            state.machine->executor_cpu.data.edx = 0xffffu;
            failed |= !inc_dec_run(&state, legacy, sizeof(legacy), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                after.data.ecx != 0xaabb7f7eu || after.data.edx != 0xffffu;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (pass = 0u; pass != 2u; ++pass) {
        const type_unsigned_32 flags = VCPU_EFLAGS_AF | VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        type_unsigned_16 observed = 0u;
        type_unsigned_32 value = 0u;
        C_INT failed;

        failed = !inc_dec_prepare_protected(0, pass == 0u, &state);
        if (!failed) {
            state.machine->executor_cpu.data.edx = 0xffffu;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= core_machine_memory_write(state.machine, 0x3010u, &observed,
                sizeof(observed)) != TYPE_STATUS_OK || core_machine_memory_write(
                state.machine, 0x2000u, fault_code, sizeof(fault_code)) != TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
                &result) != TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                core_machine_memory_read_physical(&state.machine->executor_memory, 0x3010u,
                    (type_virtual_address)&value, sizeof(observed)) != TYPE_STATUS_OK || value != 0u ||
                after.data.edx != 0xffffu || after.data.eflags != flags || after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_cmp_forms(C_VOID)
{
    static const type_unsigned_8 forms[][9] = {
        { 0x38u, 0xd1u }, { 0x39u, 0xd1u }, { 0x66u, 0x39u, 0xd1u },
        { 0x3au, 0xd1u }, { 0x3bu, 0xd1u }, { 0x66u, 0x3bu, 0xd1u },
        { 0x38u, 0x16u, 0u, 0x50u }, { 0x39u, 0x16u, 0u, 0x50u },
        { 0x66u, 0x39u, 0x16u, 0u, 0x50u }, { 0x3au, 0x16u, 0u, 0x50u },
        { 0x3bu, 0x16u, 0u, 0x50u }, { 0x66u, 0x3bu, 0x16u, 0u, 0x50u },
        { 0x3cu, 1u }, { 0x3du, 1u, 0u }, { 0x66u, 0x3du, 1u, 0u, 0u, 0u },
        { 0x80u, 0xf9u, 1u }, { 0x81u, 0xf9u, 1u, 0u },
        { 0x66u, 0x81u, 0xf9u, 1u, 0u, 0u, 0u }, { 0x83u, 0xf9u, 1u },
        { 0x66u, 0x83u, 0xf9u, 1u }, { 0x80u, 0x3eu, 0u, 0x50u, 1u },
        { 0x81u, 0x3eu, 0u, 0x50u, 1u, 0u },
        { 0x66u, 0x81u, 0x3eu, 0u, 0x50u, 1u, 0u, 0u, 0u },
        { 0x83u, 0x3eu, 0u, 0x50u, 1u }, { 0x66u, 0x83u, 0x3eu, 0u, 0x50u, 1u }
    };
    static const type_unsigned_8 lengths[] = { 2u, 2u, 3u, 2u, 2u, 3u, 4u, 4u, 5u, 4u,
        4u, 5u, 2u, 3u, 6u, 3u, 4u, 7u, 3u, 4u, 5u, 6u, 9u, 5u, 6u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const type_unsigned_8 bytes = (form == 0u || form == 3u || form == 6u ||
            form == 9u || form == 12u || form == 15u || form == 20u) ? 1u :
            ((form == 1u || form == 4u || form == 7u || form == 10u ||
              form == 13u || form == 16u || form == 18u || form == 21u ||
              form == 23u) ? 2u : 4u);
        const C_INT memory = (form >= 6u && form < 12u) || form >= 20u;
        const C_INT register_destination = (form >= 3u && form < 6u) ||
            (form >= 9u && form < 12u);
        const C_INT memory_source = form >= 9u && form < 12u;
        const C_INT accumulator = form >= 12u && form < 15u;
        const C_INT immediate = form >= 12u;
        const type_unsigned_32 mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const type_unsigned_32 destination = 0u;
        const type_unsigned_32 source = 1u;
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 observed = 0u;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = destination;
            state.machine->executor_cpu.data.ecx = register_destination || memory_source ?
                source : destination;
            state.machine->executor_cpu.data.edx = register_destination || memory_source ?
                destination : source;
            if (memory) {
                const type_unsigned_32 value = memory_source ? source : destination;
                failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY,
                    &value, bytes) != TYPE_STATUS_OK;
            }
            failed |= !inc_dec_run(&state, forms[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_CF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF);
            if (memory) {
                failed |= core_machine_memory_read(state.machine, INC_DEC_MEMORY,
                    &observed, bytes) != TYPE_STATUS_OK ||
                    observed != (memory_source ? source : destination) ||
                    after.data.edx != (memory_source ? destination : source);
            } else if (accumulator) {
                failed |= (after.data.eax & mask) != destination;
            } else if (immediate || !register_destination) {
                failed |= (after.data.ecx & mask) != destination ||
                    (!immediate && after.data.edx != source);
            } else {
                failed |= (after.data.edx & mask) != destination || after.data.ecx != source;
            }
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_cmp_boundaries(C_VOID)
{
    static const type_unsigned_8 overflow[][6] = {
        { 0x3cu, 1u }, { 0x3du, 1u, 0u }, { 0x66u, 0x3du, 1u, 0u, 0u, 0u }
    };
    static const type_unsigned_8 lengths[] = { 2u, 3u, 6u };
    static const type_unsigned_8 signext[][4] = {
        { 0x80u, 0xf9u, 0xffu }, { 0x83u, 0xf9u, 0xffu },
        { 0x66u, 0x83u, 0xf9u, 0xffu }
    };
    static const type_unsigned_8 signlengths[] = { 3u, 3u, 4u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const type_unsigned_8 bytes = form == 0u ? 1u : (form == 1u ? 2u : 4u);
        const type_unsigned_32 initial = bytes == 1u ? 0x11223380u :
            (bytes == 2u ? 0x11228000u : 0x80000000u);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = initial;
            failed |= !inc_dec_run(&state, overflow[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.eax != initial ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF |
                        (bytes == 1u ? 0u : VCPU_EFLAGS_PF));
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (form = 0u; form != sizeof(signlengths); ++form) {
        const type_unsigned_32 initial = form == 2u ? 0u : 0xaabb0000u;
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.ecx = initial;
            state.machine->executor_cpu.data.edx = 0x55667788u;
            failed |= !inc_dec_run(&state, signext[form], signlengths[form], 0,
                &after, &diagnostic) || diagnostic.first_fault.valid ||
                after.data.ecx != initial || after.data.edx != 0x55667788u ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_CF | VCPU_EFLAGS_AF);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_cmp_attribute_profile_fault(C_VOID)
{
    static const type_unsigned_8 address_code[] = { 0x67u, 0x66u, 0x39u, 0x16u };
    static const type_unsigned_8 rejected[] = { 0x66u, 0x39u, 0xd1u };
    static const type_unsigned_8 legacy[] = { 0x39u, 0xd1u };
    static const type_unsigned_8 fault_codes[][4] = {
        { 0x39u, 0x16u, 0x10u, 0u }, { 0x3bu, 0x16u, 0x10u, 0u }
    };
    type_unsigned_8 pass;

    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        type_unsigned_32 value = 0u;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.esi = INC_DEC_MEMORY;
            state.machine->executor_cpu.data.edx = 1u;
            failed |= core_machine_memory_write(state.machine, INC_DEC_MEMORY, &value,
                sizeof(value)) != TYPE_STATUS_OK || !inc_dec_run(&state, address_code,
                sizeof(address_code), 0, &after, &diagnostic) ||
                diagnostic.first_fault.valid || after.data.edx != 1u ||
                (after.data.eflags & ADD_DEFINED_FLAGS) !=
                    (VCPU_EFLAGS_CF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF) ||
                core_machine_memory_read(state.machine, INC_DEC_MEMORY, &value,
                    sizeof(value)) != TYPE_STATUS_OK || value != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        const type_unsigned_32 flags = VCPU_EFLAGS_CF;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);

        if (!failed) {
            state.machine->executor_cpu.data.ecx = 0x11220000u;
            state.machine->executor_cpu.data.edx = 1u;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= !inc_dec_run(&state, rejected, sizeof(rejected), 1, &after,
                &diagnostic) || !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.ecx != 0x11220000u || after.data.edx != 1u ||
                after.data.eflags != flags || after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80186, &state);

        if (!failed) {
            state.machine->executor_cpu.data.ecx = 0u;
            state.machine->executor_cpu.data.edx = 1u;
            failed |= !inc_dec_run(&state, legacy, sizeof(legacy), 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.ecx != 0u ||
                after.data.edx != 1u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    for (pass = 0u; pass != 2u; ++pass) {
        const type_unsigned_32 flags = VCPU_EFLAGS_CF;
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        type_unsigned_16 before = 0u;
        type_unsigned_16 observed = 0u;
        C_INT failed = !inc_dec_prepare_protected(0, 1, &state);

        if (!failed) {
            state.machine->executor_cpu.data.edx = 1u;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= core_machine_memory_write(state.machine, 0x3010u, &before,
                sizeof(before)) != TYPE_STATUS_OK || core_machine_memory_write(
                state.machine, 0x2000u, fault_codes[pass], sizeof(fault_codes[pass])) != TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
                &result) != TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                core_machine_memory_read_physical(&state.machine->executor_memory, 0x3010u,
                    (type_virtual_address)&observed, sizeof(observed)) != TYPE_STATUS_OK ||
                observed != before || after.data.edx != 1u ||
                after.data.eflags != flags || after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_decimal_adjust(C_VOID)
{
    static const type_unsigned_8 code[][2] = {
        { 0x27u, 0u }, { 0x27u, 0u }, { 0x2fu, 0u },
        { 0x37u, 0u }, { 0x3fu, 0u }, { 0xd4u, 0x10u }, { 0xd5u, 0x10u }
    };
    static const type_unsigned_8 lengths[] = { 1u, 1u, 1u, 1u, 1u, 2u, 2u };
    static const type_unsigned_32 eax[] = {
        0x1122330au, 0x1122339au, 0x11223300u, 0x11220a0au,
        0x11220a0au, 0x1122002fu, 0x1122020fu
    };
    static const type_unsigned_32 input_flags[] = { 0u, 0u, VCPU_EFLAGS_AF, 0u, 0u, 0u, 0u };
    static const type_unsigned_32 result_eax[] = {
        0x11223310u, 0x11223300u, 0x1122339au, 0x11220b00u,
        0x11220904u, 0x1122020fu, 0x1122002fu
    };
    static const type_unsigned_32 flag_masks[] = {
        VCPU_EFLAGS_CF | VCPU_EFLAGS_AF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF,
        VCPU_EFLAGS_CF | VCPU_EFLAGS_AF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF,
        VCPU_EFLAGS_CF | VCPU_EFLAGS_AF | VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF,
        VCPU_EFLAGS_CF | VCPU_EFLAGS_AF, VCPU_EFLAGS_CF | VCPU_EFLAGS_AF,
        VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF,
        VCPU_EFLAGS_SF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF
    };
    static const type_unsigned_32 expected_flags[] = {
        VCPU_EFLAGS_AF,
        VCPU_EFLAGS_CF | VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF,
        VCPU_EFLAGS_CF | VCPU_EFLAGS_AF | VCPU_EFLAGS_SF | VCPU_EFLAGS_PF,
        VCPU_EFLAGS_CF | VCPU_EFLAGS_AF, VCPU_EFLAGS_CF | VCPU_EFLAGS_AF,
        VCPU_EFLAGS_PF, 0u
    };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !inc_dec_prepare(form == 6u ? CORE_MACHINE_CPU_PROFILE_80186 :
            CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = eax[form];
            state.machine->executor_cpu.data.eflags = input_flags[form];
            failed |= !inc_dec_run(&state, code[form], lengths[form], 0, &after,
                &diagnostic) || diagnostic.first_fault.valid || after.data.eax != result_eax[form] ||
                (after.data.eflags & flag_masks[form]) != expected_flags[form];
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        static const type_unsigned_8 aam_zero[] = { 0xd4u, 0u };
        const type_unsigned_32 eax_before = 0x1122332fu;
        const type_unsigned_32 flags_before = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF;
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.eax = eax_before;
            state.machine->executor_cpu.data.eflags = flags_before;
            failed |= !inc_dec_run_delivered_de(&state, aam_zero,
                sizeof(aam_zero), &after, &diagnostic) ||
                after.data.eax != eax_before || after.data.eflags != flags_before ||
                after.data.eip != 0x0100u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_run_xlat_es(inc_dec_machine *state, const type_unsigned_8 *code,
    t_cpu *out, core_machine_cpu_diagnostic *diagnostic)
{
    core_machine_run_result result;
    if (state == STD_NULL || state->machine == STD_NULL ||
        !test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u) ||
        core_machine_cpu_execution_load_segment(&state->machine->executor_cpu_execution,
            &state->machine->executor_cpu.data.es, 0x10u) ||
        core_machine_memory_write(state->machine, 0u, code, 2u) != TYPE_STATUS_OK ||
        core_machine_run(state->machine, (core_machine_run_budget){ 1u, 0u },
            &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET ||
        core_machine_get_cpu_diagnostic(state->machine, diagnostic) != TYPE_STATUS_OK)
        return 0;
    *out = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static C_INT inc_dec_test_xlat(C_VOID)
{
    static const type_unsigned_8 code[][2] = { { 0xd7u, 0u }, { 0x67u, 0xd7u }, { 0x26u, 0xd7u } };
    type_unsigned_8 form;
    for (form = 0u; form != 3u; ++form) {
        const type_unsigned_32 base = form == 1u ? 0x00010000u : 0xaabb0010u;
        const type_unsigned_8 value = form == 1u ? 0x5au : (form == 2u ? 0x3cu : 0xa5u);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        C_INT failed = !inc_dec_prepare(form == 0u ? CORE_MACHINE_CPU_PROFILE_80186 : CORE_MACHINE_CPU_PROFILE_80386, &state);

        if (!failed) {
            state.machine->executor_cpu.data.ebx = base;
            state.machine->executor_cpu.data.eax = 0x11223304u;
            state.machine->executor_cpu.data.ecx = 0x55667788u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
            failed |= core_machine_memory_write(state.machine, form == 1u ? 0x10004u :
                (form == 2u ? 0x114u : 0x14u), &value, 1) != TYPE_STATUS_OK;
            if (form == 2u)
                failed |= !inc_dec_run_xlat_es(&state, code[form], &after, &diagnostic);
            else
                failed |= !inc_dec_run(&state, code[form], form == 0u ? 1u : 2u,
                    0, &after, &diagnostic);
            failed |= diagnostic.first_fault.valid ||
                after.data.eax != (0x11223300u | value) || after.data.ebx != base ||
                after.data.ecx != 0x55667788u || after.data.eflags != VCPU_EFLAGS_CF;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        static const type_unsigned_8 fault_code[] = { 0xd7u };
        const type_unsigned_32 eax = 0x11223304u;
        const type_unsigned_32 flags = VCPU_EFLAGS_CF;
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        C_INT failed = !inc_dec_prepare_protected(0, 1, &state);

        if (!failed) {
            state.machine->executor_cpu.data.ebx = 0x10u;
            state.machine->executor_cpu.data.eax = eax;
            state.machine->executor_cpu.data.eflags = flags;
            failed |= core_machine_memory_write(state.machine, 0x2000u, fault_code,
                sizeof(fault_code)) != TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine, (core_machine_run_budget){ 1u, 0u },
                &result) != TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                after.data.eax != eax || after.data.ebx != 0x10u ||
                after.data.eflags != flags || after.data.eip != 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT inc_dec_test_group1_profile_matrix(C_VOID)
{
    static const type_unsigned_8 opcodes[] = { 0x80u, 0x81u, 0x82u, 0x83u };
    core_machine_cpu_profile profile;
    type_unsigned_8 opcode_index;
    type_unsigned_8 selector;

    for (profile = CORE_MACHINE_CPU_PROFILE_8086;
        profile <= CORE_MACHINE_CPU_PROFILE_80386; ++profile) {
        for (opcode_index = 0u; opcode_index != sizeof(opcodes); ++opcode_index) {
            const type_unsigned_8 opcode = opcodes[opcode_index];
            const type_unsigned_8 operand_bytes = (opcode == 0x80u || opcode == 0x82u) ? 1u : 2u;
            const type_unsigned_8 immediate_bytes = opcode == 0x81u ? 2u : 1u;
            const type_unsigned_32 mask = operand_bytes == 1u ? 0xffu : 0xffffu;

            for (selector = 0u; selector != 8u; ++selector) {
                type_unsigned_8 code[] = { opcode,
                    (type_unsigned_8)(0xc0u | (selector << 3u)), 0xffu, 0xffu };
                const type_unsigned_32 expected = selector == 3u || selector == 5u ?
                    1u : (selector == 4u || selector == 7u ? 0u : mask);
                inc_dec_machine state;
                t_cpu after;
                core_machine_cpu_diagnostic diagnostic;
                C_INT failed = !inc_dec_prepare(profile, &state);

                if (!failed) {
                    state.machine->executor_cpu.data.eax = 0u;
                    state.machine->executor_cpu.data.eflags = 0u;
                    failed |= !inc_dec_run(&state, code, (STD_SIZE_T)(2u + immediate_bytes), 0,
                        &after, &diagnostic) || diagnostic.first_fault.valid ||
                        (after.data.eax & mask) != expected ||
                        after.data.eip != 2u + immediate_bytes;
                }
                core_machine_destroy(state.machine);
                if (failed) return 0;
            }
        }
    }
    return 1;
}
C_INT main(C_VOID)
{
    if (!inc_dec_test_register_forms() || !inc_dec_test_rm_forms() ||
        !inc_dec_test_address_and_profile() || !inc_dec_test_fault_nonpublication() ||
        !inc_dec_test_not_neg_forms() || !inc_dec_test_not_neg_address_and_profile() ||
        !inc_dec_test_not_neg_fault_nonpublication() || !inc_dec_test_test_forms() ||
        !inc_dec_test_test_address_and_profile() || !inc_dec_test_accumulator_profiles() ||
        !inc_dec_test_test_fault_nonpublication() || !inc_dec_test_mul_imul_forms() || !inc_dec_test_imul_sign_extension_profiles() ||
        !inc_dec_test_mul_imul_address_and_profile() ||
        !inc_dec_test_mul_imul_fault_nonpublication() || !inc_dec_test_div_idiv_forms() ||
        !inc_dec_test_div_idiv_attribute_and_profile() ||
        !inc_dec_test_div_idiv_de_nonpublication() ||
        !inc_dec_test_div_idiv_fault_nonpublication() ||
        !inc_dec_test_test_rm_reg_forms() ||
        !inc_dec_test_test_rm_reg_attribute_and_profile() ||
        !inc_dec_test_test_rm_reg_fault_nonpublication() ||
        !inc_dec_test_add_rm_reg_forms() || !inc_dec_test_add_immediate_forms() ||
        !inc_dec_test_add_signed_overflow() ||
        !inc_dec_test_add_attribute_and_profile() ||
        !inc_dec_test_add_fault_nonpublication() || !inc_dec_test_adc_forms() ||
        !inc_dec_test_adc_signed_overflow() ||
        !inc_dec_test_adc_attribute_profile_fault() || !inc_dec_test_sbb_forms() ||
        !inc_dec_test_sbb_boundaries() || !inc_dec_test_or_rm_reg_forms() ||
        !inc_dec_test_or_immediate_forms() ||
        !inc_dec_test_or_attribute_profile_fault() || !inc_dec_test_and_forms() ||
        !inc_dec_test_and_immediate() ||
        !inc_dec_test_and_attribute_profile_fault() ||
        !inc_dec_test_and_zero_flags() || !inc_dec_test_sub_forms() ||
        !inc_dec_test_sub_boundaries() ||
        !inc_dec_test_sub_attribute_profile_fault() || !inc_dec_test_xor_forms() ||
        !inc_dec_test_xor_immediates() || !inc_dec_test_xor_attribute_profile_fault() ||
        !inc_dec_test_cmp_forms() || !inc_dec_test_cmp_boundaries() ||
        !inc_dec_test_cmp_attribute_profile_fault() || !inc_dec_test_group1_profile_matrix() ||
        !inc_dec_test_decimal_adjust() || !inc_dec_test_xlat()) return 1;
    STD_PRINTF("M5:T316:S2:INC-DEC:OK\n");
    STD_PRINTF("M5:T316:S3:NOT-NEG:OK\n");
    STD_PRINTF("M5:T316:S4:TEST:OK\n");
    STD_PRINTF("M5:T316:S5:MUL-IMUL:OK\n");
    STD_PRINTF("M5:T401:S9:IMUL-SIGN-EXTENSION-PROFILES:OK\n");
    STD_PRINTF("M5:T316:S6:DIV-IDIV:OK\n");
    STD_PRINTF("M5:T316:S7:TEST-RM-REG:OK\n");
    STD_PRINTF("M5:T316:S8:ADD:OK\n");
    STD_PRINTF("M5:T316:S9:ADC:OK\n");
    STD_PRINTF("M5:T316:S10:SBB:OK\n");
    STD_PRINTF("M5:T316:S11:OR:OK\n");
    STD_PRINTF("M5:T316:S12:AND:OK\n");
    STD_PRINTF("M5:T316:S13:SUB:OK\n");
    STD_PRINTF("M5:T316:S14:XOR:OK\n");
    STD_PRINTF("M5:T316:S15:CMP:OK\n");
    STD_PRINTF("M5:T401:S7:GROUP1-PROFILE-MATRIX:OK\n");
    STD_PRINTF("M5:T401:S10:GROUP45-INC-DEC-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S11:PRIMARY-INC-DEC-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S20:ACCUMULATOR-TEST-PROFILES:OK\n");
    STD_PRINTF("M5:T316:S16:DECIMAL-ADJUST:OK\n");
    STD_PRINTF("M5:T316:S17:XLAT:OK\n");
    STD_PRINTF("M5:T401:S35:XLAT-PROFILES:OK\n");
    return 0;
}
