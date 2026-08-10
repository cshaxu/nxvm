#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define INC_DEC_MEMORY 0x5000u
#define INC_DEC_DEFINED_FLAGS (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | \
    VCPU_EFLAGS_ZF | VCPU_EFLAGS_AF | VCPU_EFLAGS_PF)
#define TEST_DEFINED_FLAGS (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | \
    VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF | VCPU_EFLAGS_CF)
#define MUL_DEFINED_FLAGS (VCPU_EFLAGS_CF | VCPU_EFLAGS_OF)

typedef struct inc_dec_machine { core_machine *machine; } inc_dec_machine;

static C_VOID inc_dec_reset(C_VOID *opaque)
{
    inc_dec_machine *state = (inc_dec_machine *)opaque;
    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider inc_dec_provider = {
    inc_dec_reset, STD_NULL, STD_NULL
};

static C_INT inc_dec_prepare(core_machine_cpu_profile profile, inc_dec_machine *state)
{
    const core_machine_config config = { CORE_MACHINE_MINIMUM_MEMORY_BYTES, profile,
        CORE_MACHINE_FPU_PROFILE_NONE };
    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK ||
        core_machine_bind_execution_provider(state->machine, &inc_dec_provider, state) !=
            TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(state->machine) != TYPE_STATUS_OK ||
        core_machine_reset(state->machine) != TYPE_STATUS_OK) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT inc_dec_run(inc_dec_machine *state, const uint8_t *code,
    STD_SIZE_T bytes, C_INT fault, t_cpu *out,
    core_machine_cpu_diagnostic *diagnostic)
{
    core_machine_run_result result;
    type_status status;
    if (state == STD_NULL || state->machine == STD_NULL ||
        !test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u) ||
        core_machine_memory_write(state->machine, 0u, code, bytes) != TYPE_STATUS_OK)
        return 0;
    status = core_machine_run(state->machine, (core_machine_run_budget){ 1u, 0u },
        &result);
    if (status != (fault ? TYPE_STATUS_FAULT : TYPE_STATUS_OK) || result.reason !=
        (fault ? CORE_MACHINE_STOP_FAULT : CORE_MACHINE_STOP_BUDGET) ||
        core_machine_get_cpu_diagnostic(state->machine, diagnostic) != TYPE_STATUS_OK)
        return 0;
    *out = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static uint32_t *inc_dec_register(t_cpu *cpu, uint8_t index)
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

static C_INT inc_dec_flags_match(uint32_t flags, uint32_t expected,
    uint32_t preserved)
{
    return (flags & INC_DEC_DEFINED_FLAGS) == expected &&
        (flags & VCPU_EFLAGS_CF) == preserved;
}

static C_INT inc_dec_test_register_forms(C_VOID)
{
    uint8_t index;
    uint8_t operand32;
    for (index = 0u; index != 8u; ++index) {
        for (operand32 = 0u; operand32 != 2u; ++operand32) {
            uint8_t code[2] = { 0x40u + index, 0u };
            inc_dec_machine state;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            uint32_t *reg;
            C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

            if (operand32) { code[0] = 0x66u; code[1] = 0x40u + index; }
            if (!failed) {
                reg = inc_dec_register(&state.machine->executor_cpu, index);
                *reg = operand32 ? 0x7fffffffu : 0xaabb7fffu;
                state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
                failed |= !inc_dec_run(&state, code, operand32 ? 2u : 1u, 0,
                    &after, &diagnostic) || diagnostic.first_fault.valid ||
                    *inc_dec_register(&after, index) != (operand32 ? 0x80000000u :
                        0xaabb8000u) || !inc_dec_flags_match(after.data.eflags,
                        VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF |
                        VCPU_EFLAGS_PF,
                        VCPU_EFLAGS_CF);
            }
            core_machine_destroy(state.machine);
            if (failed) return 0;
        }
    }
    return 1;
}

static C_INT inc_dec_test_rm_forms(C_VOID)
{
    static const uint8_t forms[][6] = {
        { 0xfeu, 0xc0u }, { 0xfeu, 0xc8u }, { 0xffu, 0xc0u }, { 0xffu, 0xc8u },
        { 0xfeu, 0x06u, 0x00u, 0x50u }, { 0xfeu, 0x0eu, 0x00u, 0x50u },
        { 0xffu, 0x06u, 0x00u, 0x50u }, { 0xffu, 0x0eu, 0x00u, 0x50u },
        { 0x66u, 0xffu, 0x06u, 0x00u, 0x50u },
        { 0x66u, 0xffu, 0x0eu, 0x00u, 0x50u }
    };
    static const uint8_t lengths[] = { 2u, 2u, 2u, 2u, 4u, 4u, 4u, 4u, 5u, 5u };
    uint8_t form;
    for (form = 0u; form != sizeof(lengths); ++form) {
        const C_INT decrement = (form & 1u) != 0u;
        const uint8_t bytes = form < 2u || (form >= 4u && form < 6u) ? 1u :
            (form < 8u ? 2u : 4u);
        const uint32_t before = decrement ? (bytes == 1u ? 0x80u :
            (bytes == 2u ? 0x8000u : 0x80000000u)) : (bytes == 1u ? 0x7fu :
            (bytes == 2u ? 0x7fffu : 0x7fffffffu));
        const uint32_t expected = decrement ? (before - 1u) : (before + 1u);
        const uint32_t flags = decrement ?
            (VCPU_EFLAGS_OF | VCPU_EFLAGS_AF |
                (bytes == 1u ? 0u : VCPU_EFLAGS_PF)) :
            (VCPU_EFLAGS_OF | VCPU_EFLAGS_SF | VCPU_EFLAGS_AF |
                (bytes == 1u ? 0u : VCPU_EFLAGS_PF));
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        uint32_t observed = 0u;
        C_INT failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

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
                const uint32_t mask = bytes == 1u ? 0xffu : 0xffffu;
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
    static const uint8_t address_code[] = { 0x67u,0x66u,0xffu,0x06u,
        0x00u,0x50u,0x00u,0x00u };
    static const uint8_t rejected_prefix[] = { 0x66u,0x40u };
    static const uint8_t accepted_legacy[] = { 0xffu,0xc0u };
    inc_dec_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    uint32_t value = 0x7fffffffu;
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

    failed = !inc_dec_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);
    if (!failed) {
        state.machine->executor_cpu.data.eax = 0x11227fffu;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF;
        failed |= !inc_dec_run(&state, rejected_prefix, sizeof(rejected_prefix), 1,
            &after, &diagnostic) || !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eax != 0x11227fffu || after.data.eflags != VCPU_EFLAGS_CF ||
            after.data.eip != 0u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;

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
    static const uint8_t gdt_pointer[] = { 0x1fu,0u,0u,0x03u,0u,0u };
    uint8_t gdt[] = {
        0u,0u,0u,0u,0u,0u,0u,0u,
        0xffu,0xffu,0u,0x20u,0u,0x9au,0u,0u,
        0xffu,0xffu,0u,0x30u,0u,0x92u,0u,0u,
        0xffu,0xffu,0u,0x40u,0u,0x92u,0u,0u
    };
    static const uint8_t bootstrap[] = {
        0x0fu,0x01u,0x16u,0u,0x01u,
        0xb8u,0x01u,0u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0u,0x8eu,0xd8u,0x8eu,0xc0u,
        0xb8u,0x18u,0u,0x8eu,0xd0u,
        0xbcu,0u,0x80u,0xeau,0u,0u,0x08u,0u
    };
    static const uint8_t halt[] = { 0xf4u };
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
    static const uint8_t code[] = { 0xffu,0x06u,0x10u,0u };
    const uint32_t flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
    uint8_t pass;

    for (pass = 0u; pass != 2u; ++pass) {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        uint16_t before = 0x7fffu;
        uint16_t observed = 0u;
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
    static const uint8_t forms[][6] = {
        { 0xf6u,0xd0u }, { 0xf6u,0xd8u }, { 0xf7u,0xd0u }, { 0xf7u,0xd8u },
        { 0x66u,0xf7u,0xd0u }, { 0x66u,0xf7u,0xd8u },
        { 0xf6u,0x16u,0x00u,0x50u }, { 0xf6u,0x1eu,0x00u,0x50u },
        { 0xf7u,0x16u,0x00u,0x50u }, { 0xf7u,0x1eu,0x00u,0x50u },
        { 0x66u,0xf7u,0x16u,0x00u,0x50u },
        { 0x66u,0xf7u,0x1eu,0x00u,0x50u }
    };
    static const uint8_t lengths[] = { 2u,2u,2u,2u,3u,3u,4u,4u,4u,4u,5u,5u };
    const uint32_t saved_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF;
    uint8_t form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const C_INT negate = (form & 1u) != 0u;
        const uint8_t bytes = form == 0u || form == 1u || form == 6u || form == 7u ?
            1u : (form == 2u || form == 3u || form == 8u || form == 9u ? 2u : 4u);
        const uint32_t mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const uint32_t before = negate ? (bytes == 1u ? 0x80u :
            (bytes == 2u ? 0x8000u : 0x80000000u)) :
            (bytes == 1u ? 0x5au : (bytes == 2u ? 0xa55au : 0x5aa55aa5u));
        const uint32_t expected = negate ? (0u - before) & mask : (~before) & mask;
        const uint32_t neg_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
            VCPU_EFLAGS_SF | (bytes == 1u ? 0u : VCPU_EFLAGS_PF);
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        uint32_t observed = 0u;
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
    static const uint8_t address_code[] = { 0x67u,0x66u,0xf7u,0x1eu,
        0x00u,0x50u,0x00u,0x00u };
    static const uint8_t rejected_prefix[] = { 0x66u,0xf7u,0xd0u };
    static const uint8_t accepted_legacy[] = { 0xf7u,0xd0u };
    const uint32_t saved_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF;
    inc_dec_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    uint32_t value = 0x80000000u;
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
    static const uint8_t code[][4] = {
        { 0xf7u,0x16u,0x10u,0u }, { 0xf7u,0x1eu,0x10u,0u }
    };
    const uint32_t flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
    uint8_t operation;
    uint8_t pass;

    for (operation = 0u; operation != 2u; ++operation) {
        for (pass = 0u; pass != 2u; ++pass) {
            inc_dec_machine state;
            t_cpu after;
            core_machine_cpu_diagnostic diagnostic;
            core_machine_run_result result;
            uint16_t before = operation ? 0x7fffu : 0x55aau;
            uint16_t observed = 0u;
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
    static const uint8_t forms[][9] = {
        { 0xa8u,0x80u }, { 0xa9u,0u,0x80u },
        { 0x66u,0xa9u,0u,0u,0u,0x80u },
        { 0xf6u,0xc0u,0x80u }, { 0xf7u,0xc0u,0u,0x80u },
        { 0x66u,0xf7u,0xc0u,0u,0u,0u,0x80u },
        { 0xf6u,0x06u,0u,0x50u,0x80u },
        { 0xf7u,0x06u,0u,0x50u,0u,0x80u },
        { 0x66u,0xf7u,0x06u,0u,0x50u,0u,0u,0u,0x80u }
    };
    static const uint8_t lengths[] = { 2u,3u,6u,3u,4u,7u,5u,6u,9u };
    const uint32_t initial_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF;
    uint8_t form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const uint8_t bytes = form == 0u || form == 3u || form == 6u ? 1u :
            (form == 1u || form == 4u || form == 7u ? 2u : 4u);
        const uint32_t mask = bytes == 1u ? 0xffu :
            (bytes == 2u ? 0xffffu : 0xffffffffu);
        const uint32_t expected_flags = VCPU_EFLAGS_SF |
            (bytes == 1u ? 0u : VCPU_EFLAGS_PF);
        const C_INT memory = form >= 6u;
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        uint32_t before = 0xffffffffu;
        uint32_t observed = 0u;
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

static C_INT inc_dec_test_test_address_and_profile(C_VOID)
{
    static const uint8_t address_code[] = { 0x67u,0x66u,0xf7u,0x06u,
        0u,0u,0u,0x80u };
    static const uint8_t rejected_prefix[] = { 0x66u,0xa9u,0u,0u,0u,0x80u };
    static const uint8_t accepted_legacy[] = { 0xa9u,0u,0x80u };
    const uint32_t initial_flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_PF;
    inc_dec_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    uint32_t value = 0xffffffffu;
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
    static const uint8_t code[] = { 0xf7u,0x06u,0x10u,0u,0xffu,0xffu };
    const uint32_t flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_OF | VCPU_EFLAGS_AF;
    inc_dec_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    uint16_t before = 0xffffu;
    uint16_t observed = 0u;
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
    static const uint8_t forms[][5] = {
        { 0xf6u,0xe1u }, { 0xf6u,0xe9u }, { 0xf7u,0xe1u }, { 0xf7u,0xe9u },
        { 0x66u,0xf7u,0xe1u }, { 0x66u,0xf7u,0xe9u },
        { 0xf6u,0x26u,0u,0x50u }, { 0xf6u,0x2eu,0u,0x50u },
        { 0xf7u,0x26u,0u,0x50u }, { 0xf7u,0x2eu,0u,0x50u },
        { 0x66u,0xf7u,0x26u,0u,0x50u },
        { 0x66u,0xf7u,0x2eu,0u,0x50u }
    };
    static const uint8_t lengths[] = { 2u,2u,2u,2u,3u,3u,4u,4u,4u,4u,5u,5u };
    uint8_t form;

    for (form = 0u; form != sizeof(lengths); ++form) {
        const C_INT signed_multiply = (form & 1u) != 0u;
        const C_INT memory = form >= 6u;
        const uint8_t bytes = form == 0u || form == 1u || form == 6u || form == 7u ?
            1u : (form == 2u || form == 3u || form == 8u || form == 9u ? 2u : 4u);
        const uint32_t source = memory ? 2u : 2u;
        const uint32_t accumulator = memory ? (bytes == 1u ? 0x80u :
            (bytes == 2u ? 0x8000u : 0x80000000u)) : 2u;
        const uint32_t expected_lo = memory ? (bytes == 1u ? (signed_multiply ?
            0xff00u : 0x0100u) : 0u) : 4u;
        const uint32_t expected_hi = memory ? (bytes == 1u ? 0u :
            (signed_multiply ? (bytes == 2u ? 0xffffu : 0xffffffffu) : 1u)) : 0u;
        const uint32_t expected_flags = memory ? MUL_DEFINED_FLAGS : 0u;
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        uint32_t observed = 0u;
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

static C_INT inc_dec_test_mul_imul_address_and_profile(C_VOID)
{
    static const uint8_t address_code[] = { 0x67u,0x66u,0xf7u,0x2eu };
    static const uint8_t rejected_prefix[] = { 0x66u,0xf7u,0xe0u };
    static const uint8_t accepted_legacy[] = { 0xf7u,0xe0u };
    inc_dec_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    uint32_t source = 2u;
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
    static const uint8_t code[][4] = {
        { 0xf7u,0x26u,0x10u,0u }, { 0xf7u,0x2eu,0x10u,0u }
    };
    const uint32_t flags = MUL_DEFINED_FLAGS | VCPU_EFLAGS_ZF;
    uint8_t operation;

    for (operation = 0u; operation != 2u; ++operation) {
        inc_dec_machine state;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        uint16_t before = 2u;
        uint16_t observed = 0u;
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

C_INT main(C_VOID)
{
    if (!inc_dec_test_register_forms() || !inc_dec_test_rm_forms() ||
        !inc_dec_test_address_and_profile() || !inc_dec_test_fault_nonpublication() ||
        !inc_dec_test_not_neg_forms() || !inc_dec_test_not_neg_address_and_profile() ||
        !inc_dec_test_not_neg_fault_nonpublication() || !inc_dec_test_test_forms() ||
        !inc_dec_test_test_address_and_profile() ||
        !inc_dec_test_test_fault_nonpublication() || !inc_dec_test_mul_imul_forms() ||
        !inc_dec_test_mul_imul_address_and_profile() ||
        !inc_dec_test_mul_imul_fault_nonpublication()) return 1;
    STD_PRINTF("M5:T316:S2:INC-DEC:OK\n");
    STD_PRINTF("M5:T316:S3:NOT-NEG:OK\n");
    STD_PRINTF("M5:T316:S4:TEST:OK\n");
    STD_PRINTF("M5:T316:S5:MUL-IMUL:OK\n");
    return 0;
}
