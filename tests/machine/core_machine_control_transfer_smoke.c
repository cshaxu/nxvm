#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define CT_GDT_POINTER 0x0100u
#define CT_GDT_ADDRESS 0x0300u
#define CT_CODE_ADDRESS 0x2000u

typedef struct ct_machine {
    core_machine *machine;
} ct_machine;

typedef struct ct_jcc_case {
    uint8_t opcode;
    uint32_t flags;
} ct_jcc_case;

static C_VOID ct_reset(C_VOID *opaque)
{
    ct_machine *state = (ct_machine *)opaque;

    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider ct_provider = {
    ct_reset, STD_NULL, STD_NULL
};

static C_INT ct_write(ct_machine *state, uint32_t address, const C_VOID *bytes,
    STD_SIZE_T byte_count)
{
    return state != STD_NULL && state->machine != STD_NULL &&
        core_machine_memory_write(state->machine, address, bytes, byte_count) ==
            TYPE_STATUS_OK;
}

static C_INT ct_read_private(ct_machine *state, uint32_t address, C_VOID *bytes,
    STD_SIZE_T byte_count)
{
    return state != STD_NULL && state->machine != STD_NULL &&
        core_machine_memory_read_physical(&state->machine->executor_memory, address,
            (type_virtual_address)bytes, byte_count) == TYPE_STATUS_OK;
}

static C_INT ct_prepare(ct_machine *state, core_machine_cpu_profile profile,
    C_INT code32)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    const uint8_t gdt_pointer[] = { 0x2fu,0,0,0x03u,0,0 };
    uint8_t gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0,0,
        0xffu,0xffu,0,0x30u,0,0x92u,0xcfu,0,
        0xffu,0xffu,0,0x40u,0,0x92u,0x40u,0,
        0xffu,0xffu,0,0x20u,0,0xbau,0,0,
        0xffu,0xffu,0,0x20u,0,0x1au,0,0
    };
    const uint8_t real_code[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,
        0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,
        0xb8u,0x18u,0x00u,0x8eu,0xd0u,
        0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    const uint8_t halt[] = { 0xf4u };
    const core_machine_run_budget budget = { 96u, 0u };
    core_machine_run_result result;

    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    gdt[14] = code32 ? 0x40u : 0u;
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK ||
        core_machine_bind_execution_provider(state->machine, &ct_provider,
            state) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(state->machine) != TYPE_STATUS_OK ||
        core_machine_reset(state->machine) != TYPE_STATUS_OK ||
        !ct_write(state, CT_GDT_POINTER, gdt_pointer, sizeof(gdt_pointer)) ||
        !ct_write(state, CT_GDT_ADDRESS, gdt, sizeof(gdt)) ||
        !ct_write(state, 0u, real_code, sizeof(real_code)) ||
        !ct_write(state, CT_CODE_ADDRESS, halt, sizeof(halt)) ||
        core_machine_run(state->machine, budget, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT ct_prepare_real(ct_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    static const uint8_t reset_jump[] = {0xeau,0,0,0,0};

    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK ||
        test_core_machine_fixture_register_reset_mapping(state->machine,
            0xfffffff0u, 0x000ffff0u, 64u) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(state->machine) != TYPE_STATUS_OK ||
        core_machine_reset(state->machine) != TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, 0xfffffff0u, reset_jump,
            sizeof(reset_jump)) != TYPE_STATUS_OK) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT ct_run(ct_machine *state, const uint8_t *code,
    STD_SIZE_T code_size, core_machine_stop_reason expected_reason, t_cpu *out_cpu)
{
    const core_machine_run_budget budget = { 48u, 0u };
    core_machine_run_result result;
    type_status status;

    if (!ct_write(state, CT_CODE_ADDRESS, code, code_size)) return 0;
    test_core_machine_fixture_resume_after_halt_at(state->machine, 0u);
    status = core_machine_run(state->machine, budget, &result);
    if ((expected_reason == CORE_MACHINE_STOP_FAULT && status != TYPE_STATUS_FAULT) ||
        (expected_reason != CORE_MACHINE_STOP_FAULT && status != TYPE_STATUS_OK) ||
        result.reason != expected_reason) return 0;
    *out_cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static C_INT ct_run_gp(ct_machine *state, const uint8_t *code,
    STD_SIZE_T code_size, t_cpu *out_cpu)
{
    core_machine_cpu_diagnostic diagnostic;

    return ct_run(state, code, code_size, CORE_MACHINE_STOP_FAULT, out_cpu) &&
        core_machine_get_cpu_diagnostic(state->machine, &diagnostic) == TYPE_STATUS_OK &&
        diagnostic.first_fault.valid &&
        TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_GP);
}

static C_INT ct_run_np(ct_machine *state, const uint8_t *code,
    STD_SIZE_T code_size, t_cpu *out_cpu)
{
    core_machine_cpu_diagnostic diagnostic;

    return ct_run(state, code, code_size, CORE_MACHINE_STOP_FAULT, out_cpu) &&
        core_machine_get_cpu_diagnostic(state->machine, &diagnostic) == TYPE_STATUS_OK &&
        diagnostic.first_fault.valid &&
        TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_NP);
}

static C_INT ct_run_ud(ct_machine *state, const uint8_t *code,
    STD_SIZE_T code_size, t_cpu *out_cpu)
{
    core_machine_cpu_diagnostic diagnostic;

    return ct_run(state, code, code_size, CORE_MACHINE_STOP_FAULT, out_cpu) &&
        core_machine_get_cpu_diagnostic(state->machine, &diagnostic) == TYPE_STATUS_OK &&
        diagnostic.first_fault.valid &&
        TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD);
}

static C_VOID ct_set_stack32(ct_machine *state, uint32_t esp)
{
    state->machine->executor_cpu.data.ss.seg.data.big = TYPE_TRUE;
    state->machine->executor_cpu.data.ss.limit = 0xffffffffu;
    state->machine->executor_cpu.data.esp = esp;
}

static C_INT ct_test_jcc_short(C_VOID)
{
    static const ct_jcc_case cases[] = {
        {0x70u,VCPU_EFLAGS_OF}, {0x71u,0}, {0x72u,VCPU_EFLAGS_CF}, {0x73u,0},
        {0x74u,VCPU_EFLAGS_ZF}, {0x75u,0}, {0x76u,VCPU_EFLAGS_CF}, {0x77u,0},
        {0x78u,VCPU_EFLAGS_SF}, {0x79u,0}, {0x7au,VCPU_EFLAGS_PF}, {0x7bu,0},
        {0x7cu,VCPU_EFLAGS_SF}, {0x7du,0}, {0x7eu,VCPU_EFLAGS_ZF}, {0x7fu,0}
    };
    uint8_t code[] = {0,2,0xb0u,0,0xf4u};
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        ct_machine state;
        t_cpu before;
        t_cpu after;
        C_INT failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);

        if (!failed) {
            code[0] = cases[index].opcode;
            state.machine->executor_cpu.data.eflags = 0x00000002u | cases[index].flags;
            state.machine->executor_cpu.data.eax = 0x123456a5u;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed = !ct_run(&state, code, sizeof(code),
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) ||
                after.data.eip != sizeof(code) || after.data.eax != before.data.eax ||
                after.data.eflags != before.data.eflags;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT ct_test_near_and_short_jumps(C_VOID)
{
    static const uint8_t short_jump[] = {0xebu,2,0xb0u,0,0xf4u};
    static const uint8_t jump32[] = {0xe9u,2,0,0,0,0xb0u,0,0xf4u};
    static const uint8_t jump16[] = {0x66u,0xe9u,2,0,0xb0u,0,0xf4u};
    static const uint8_t jz32[] = {0x0fu,0x84u,2,0,0,0,0xb0u,0,0xf4u};
    static const uint8_t jz16[] = {0x66u,0x0fu,0x84u,2,0,0xb0u,0,0xf4u};
    const uint8_t *const programs[] = {
        short_jump, jump32, jump16, jz32, jz16
    };
    const STD_SIZE_T sizes[] = {
        sizeof(short_jump), sizeof(jump32), sizeof(jump16), sizeof(jz32), sizeof(jz16)
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(programs) / sizeof(programs[0]); ++index) {
        ct_machine state;
        t_cpu after;
        C_INT failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);

        if (!failed) {
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_ZF | 0x00000002u;
            state.machine->executor_cpu.data.eax = 0x123456a5u;
            failed = !ct_run(&state, programs[index], sizes[index],
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) ||
                after.data.eip != sizes[index] || after.data.eax != 0x123456a5u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT ct_test_jcc_near_conditions(C_VOID)
{
    static const ct_jcc_case cases[] = {
        {0x80u,VCPU_EFLAGS_OF}, {0x81u,0}, {0x82u,VCPU_EFLAGS_CF}, {0x83u,0},
        {0x84u,VCPU_EFLAGS_ZF}, {0x85u,0}, {0x86u,VCPU_EFLAGS_CF}, {0x87u,0},
        {0x88u,VCPU_EFLAGS_SF}, {0x89u,0}, {0x8au,VCPU_EFLAGS_PF}, {0x8bu,0},
        {0x8cu,VCPU_EFLAGS_SF}, {0x8du,0}, {0x8eu,VCPU_EFLAGS_ZF}, {0x8fu,0}
    };
    uint8_t code[] = {0x0fu,0,2,0,0,0,0xb0u,0,0xf4u};
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index) {
        ct_machine state;
        t_cpu before;
        t_cpu after;
        C_INT failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);

        if (!failed) {
            code[1] = cases[index].opcode;
            state.machine->executor_cpu.data.eflags = 0x00000002u | cases[index].flags;
            state.machine->executor_cpu.data.eax = 0x123456a5u;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed = !ct_run(&state, code, sizeof(code),
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) ||
                after.data.eip != sizeof(code) || after.data.eax != before.data.eax ||
                after.data.eflags != before.data.eflags;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT ct_test_16bit_code_and_real_mode(C_VOID)
{
    static const uint8_t jump16[] = {0xe9u,2,0,0xb0u,0,0xf4u};
    static const uint8_t jump32[] = {0x66u,0xe9u,2,0,0,0,0xb0u,0,0xf4u};
    static const uint8_t jz16[] = {0x0fu,0x84u,2,0,0xb0u,0,0xf4u};
    static const uint8_t jz32[] = {0x66u,0x0fu,0x84u,2,0,0,0,0xb0u,0,0xf4u};
    const uint8_t *const protected_programs[] = {jump16, jump32, jz16, jz32};
    const STD_SIZE_T protected_sizes[] = {
        sizeof(jump16), sizeof(jump32), sizeof(jz16), sizeof(jz32)
    };
    static const uint8_t real_jz32[] = {0x66u,0x0fu,0x84u,2,0,0,0,0xb0u,0,0xf4u};
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(protected_programs) / sizeof(protected_programs[0]); ++index) {
        ct_machine state;
        t_cpu after;
        C_INT failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 0);

        if (!failed) {
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_ZF | 0x00000002u;
            state.machine->executor_cpu.data.eax = 0x123456a5u;
            failed = !ct_run(&state, protected_programs[index], protected_sizes[index],
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) ||
                after.data.eip != protected_sizes[index] ||
                after.data.eax != 0x123456a5u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        static const uint8_t loop16[] = {0xe2u,2,0xb0u,0,0xf4u};
        ct_machine state;
        t_cpu after;
        C_INT failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 0);

        if (!failed) {
            state.machine->executor_cpu.data.ecx = 0xabcd0002u;
            failed = !ct_run(&state, loop16, sizeof(loop16),
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) ||
                after.data.ecx != 0xabcd0001u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        ct_machine state;
        t_cpu after;
        const core_machine_run_budget budget = {48u,0u};
        core_machine_run_result result;
        C_INT failed = !ct_prepare_real(&state);

        if (!failed) {
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_ZF | 0x00000002u;
            state.machine->executor_cpu.data.eax = 0x123456a5u;
            failed = core_machine_memory_write(state.machine, 0u, real_jz32,
                sizeof(real_jz32)) != TYPE_STATUS_OK ||
                core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= after.data.eax != 0x123456a5u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT ct_test_loop_and_jcxz(C_VOID)
{
    static const uint8_t loop[] = {0xe2u,2,0xb0u,0,0xf4u};
    static const uint8_t loopnz[] = {0xe0u,2,0xb0u,0,0xf4u};
    static const uint8_t loopz[] = {0xe1u,2,0xb0u,0,0xf4u};
    static const uint8_t jcxz[] = {0xe3u,2,0xb0u,0,0xf4u};
    static const uint8_t jecxz[] = {0x67u,0xe3u,2,0xb0u,0,0xf4u};
    ct_machine state;
    t_cpu after;
    C_INT failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);

    if (!failed) {
        state.machine->executor_cpu.data.ecx = 2u;
        failed |= !ct_run(&state, loop, sizeof(loop),
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) || after.data.ecx != 1u;
    }
    core_machine_destroy(state.machine);
    if (!failed) failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.machine->executor_cpu.data.ecx = 2u;
        failed |= !ct_run(&state, (const uint8_t[]){0x66u,0xe2u,2,0xb0u,0,0xf4u}, 6u,
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) || after.data.ecx != 1u;
    }
    core_machine_destroy(state.machine);
    if (!failed) failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.machine->executor_cpu.data.ecx = 0x12340002u;
        failed |= !ct_run(&state, (const uint8_t[]){0x67u,0xe2u,2,0xb0u,0,0xf4u}, 6u,
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) ||
            after.data.ecx != 0x12340001u;
    }
    core_machine_destroy(state.machine);
    if (!failed) failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.machine->executor_cpu.data.ecx = 2u;
        state.machine->executor_cpu.data.eflags = 0x00000002u;
        failed |= !ct_run(&state, loopnz, sizeof(loopnz),
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) || after.data.ecx != 1u;
    }
    core_machine_destroy(state.machine);
    if (!failed) failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.machine->executor_cpu.data.ecx = 2u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_ZF | 0x00000002u;
        failed |= !ct_run(&state, loopz, sizeof(loopz),
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) || after.data.ecx != 1u;
    }
    core_machine_destroy(state.machine);
    if (!failed) failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.machine->executor_cpu.data.ecx = 0x12340000u;
        failed |= !ct_run(&state, jcxz, sizeof(jcxz),
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) ||
            after.data.ecx != 0x12340000u;
    }
    core_machine_destroy(state.machine);
    if (!failed) failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.machine->executor_cpu.data.ecx = 0u;
        failed |= !ct_run(&state, jecxz, sizeof(jecxz),
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) || after.data.ecx != 0u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT ct_test_loop_target_fault_is_atomic(C_VOID)
{
    static const uint8_t loop_fault[] = {0xe2u,0x7fu};
    ct_machine state;
    t_cpu before;
    t_cpu after;
    C_INT failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);

    if (!failed) {
        state.machine->executor_cpu.data.cs.limit = 0x007fu;
        state.machine->executor_cpu.data.ecx = 2u;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        before.data.eip = 0u;
        failed = !ct_run_gp(&state, loop_fault, sizeof(loop_fault), &after) ||
            after.data.ecx != before.data.ecx || after.data.eip != before.data.eip ||
            after.data.eflags != before.data.eflags;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT ct_test_jcc_limit_boundaries(C_VOID)
{
    static const uint8_t taken_fault[] = {0x74u,0x7fu};
    static const uint8_t not_taken[] = {0x74u,0x7fu,0xf4u};
    ct_machine state;
    t_cpu before;
    t_cpu after;
    C_INT failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);

    if (!failed) {
        state.machine->executor_cpu.data.cs.limit = 0x007fu;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_ZF | 0x00000002u;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        before.data.eip = 0u;
        failed = !ct_run_gp(&state, taken_fault, sizeof(taken_fault), &after) ||
            after.data.eip != before.data.eip || after.data.eflags != before.data.eflags;
    }
    core_machine_destroy(state.machine);
    if (!failed) failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.machine->executor_cpu.data.cs.limit = 0x007fu;
        state.machine->executor_cpu.data.eflags = 0x00000002u;
        failed = !ct_run(&state, not_taken, sizeof(not_taken),
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) || after.data.eip != sizeof(not_taken);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT ct_test_pre386_near_jcc_is_ud(C_VOID)
{
    static const uint8_t near_jcc[] = {0x0fu,0x84u,0,0};
    ct_machine state;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    C_INT failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80286, 0);

    if (!failed) {
        failed = !ct_run(&state, near_jcc, sizeof(near_jcc),
            CORE_MACHINE_STOP_FAULT, &after) ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK ||
            !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT ct_test_ret_target_fault_is_atomic(C_VOID)
{
    static const uint8_t ret[] = {0xc3u};
    static const uint8_t target[] = {0x80u,0,0,0};
    ct_machine state;
    t_cpu before;
    t_cpu after;
    C_INT failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);

    if (!failed) {
        state.machine->executor_cpu.data.cs.limit = 0x007fu;
        failed = !ct_write(&state, 0x0000c000u, target, sizeof(target));
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        before.data.eip = 0u;
        if (!failed) failed = !ct_run_gp(&state, ret, sizeof(ret), &after) ||
            after.data.eip != before.data.eip || after.data.sp != before.data.sp ||
            after.data.eflags != before.data.eflags;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT ct_test_near_call_and_ret_forms(C_VOID)
{
    static const uint8_t code32_call32[] = {0xe8u,3,0,0,0,0xb0u,0xa5u,0xf4u,0xc3u};
    static const uint8_t code32_call16[] = {0x66u,0xe8u,3,0,0xb0u,0xa5u,0xf4u,0x66u,0xc3u};
    static const uint8_t code16_call16[] = {0xe8u,3,0,0xb0u,0xa5u,0xf4u,0xc3u};
    static const uint8_t code16_call32[] = {0x66u,0xe8u,3,0,0,0,0xb0u,0xa5u,0xf4u,0x66u,0xc3u};
    static const uint8_t ret16_imm[] = {0x66u,0xc2u,4,0,0xf4u};
    static const uint8_t ret32_imm[] = {0xc2u,4,0,0xf4u};
    const uint8_t *const programs[] = {
        code32_call32, code32_call16, code16_call16, code16_call32
    };
    const STD_SIZE_T sizes[] = {
        sizeof(code32_call32), sizeof(code32_call16), sizeof(code16_call16),
        sizeof(code16_call32)
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(programs) / sizeof(programs[0]); ++index) {
        ct_machine state;
        t_cpu before;
        t_cpu after;
        C_INT failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386,
            index < 2u);

        if (!failed) {
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed = !ct_run(&state, programs[index], sizes[index],
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) ||
                after.data.eax != 0x000000a5u || after.data.sp != before.data.sp ||
                after.data.eflags != before.data.eflags;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        static const uint8_t target16[] = {4,0};
        static const uint8_t target32[] = {3,0,0,0};
        ct_machine state;
        t_cpu before;
        t_cpu after;
        C_INT failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);

        if (!failed) {
            ct_set_stack32(&state, 0x00008000u);
            failed = !ct_write(&state, 0x0000c000u, target16, sizeof(target16));
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            if (!failed) failed = !ct_run(&state, ret16_imm, sizeof(ret16_imm),
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) ||
                after.data.esp != before.data.esp + 6u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
        failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
        if (!failed) {
            ct_set_stack32(&state, 0x00008000u);
            failed = !ct_write(&state, 0x0000c000u, target32, sizeof(target32));
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            if (!failed) failed = !ct_run(&state, ret32_imm, sizeof(ret32_imm),
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) ||
                after.data.esp != before.data.esp + 8u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT ct_test_near_indirect_and_fault_boundaries(C_VOID)
{
    static const uint8_t call_register[] = {
        0xb8u,10,0,0,0,0xffu,0xd0u,0xb0u,0xa5u,0xf4u,0xc3u
    };
    static const uint8_t jmp_memory[] = {0xffu,0x25u,0,1,0,0,0xb0u,0,0xf4u};
    static const uint8_t call16_register[] = {
        0xb8u,8,0,0xffu,0xd0u,0xb0u,0xa5u,0xf4u,0xc3u
    };
    static const uint8_t jmp16_memory[] = {0xffu,0x26u,0,1,0xb0u,0,0xf4u};
    static const uint8_t call_fault[] = {0xe8u,0x7bu,0,0,0};
    static const uint8_t jmp_fault[] = {0xffu,0xe0u};
    static const uint8_t jmp_target[] = {8,0,0,0};
    ct_machine state;
    t_cpu before;
    t_cpu after;
    C_INT failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);

    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed = !ct_run(&state, call_register, sizeof(call_register),
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) ||
            after.data.eax != 0x000000a5u || after.data.sp != before.data.sp;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    if (!failed) failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 0);
    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed = !ct_run(&state, call16_register, sizeof(call16_register),
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) ||
            after.data.eax != 0x000000a5u || after.data.sp != before.data.sp;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    if (!failed) failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        failed = !ct_write(&state, 0x00003100u, jmp_target, sizeof(jmp_target));
        if (!failed) failed = !ct_run(&state, jmp_memory, sizeof(jmp_memory),
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) || after.data.eip != 9u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    if (!failed) failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 0);
    if (!failed) {
        static const uint8_t target16[] = {6,0};
        failed = !ct_write(&state, 0x00003100u, target16, sizeof(target16));
        if (!failed) failed = !ct_run(&state, jmp16_memory, sizeof(jmp16_memory),
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) || after.data.eip != 7u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    if (!failed) failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.machine->executor_cpu.data.cs.limit = 0x007fu;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        before.data.eip = 0u;
        failed = !ct_run_gp(&state, call_fault, sizeof(call_fault), &after) ||
            after.data.eip != before.data.eip || after.data.sp != before.data.sp ||
            after.data.eflags != before.data.eflags;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    if (!failed) failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        state.machine->executor_cpu.data.cs.limit = 0x007fu;
        state.machine->executor_cpu.data.eax = 0x00000080u;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        before.data.eip = 0u;
        failed = !ct_run_gp(&state, jmp_fault, sizeof(jmp_fault), &after) ||
            after.data.eip != before.data.eip || after.data.eax != before.data.eax ||
            after.data.eflags != before.data.eflags;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT ct_test_far_same_cpl_return_validation(C_VOID)
{
    static const uint8_t retf[] = {0xcbu};
    static const uint8_t dpl_mismatch[] = {0,0,0,0,0x20u,0,0,0};
    static const uint8_t nonpresent[] = {0,0,0,0,0x28u,0,0,0};
    ct_machine state;
    t_cpu before;
    t_cpu after;
    uint8_t descriptor_access = 0u;
    C_INT failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);

    if (!failed) {
        failed = !ct_write(&state, 0x0000c000u, dpl_mismatch,
            sizeof(dpl_mismatch));
        failed |= !ct_read_private(&state, 0x0325u, &descriptor_access,
            sizeof(descriptor_access)) || descriptor_access != 0xbau;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        before.data.eip = 0u;
        if (!failed) {
            C_INT run_ok = ct_run_gp(&state, retf, sizeof(retf), &after);
            failed = !run_ok || after.data.eip != before.data.eip ||
                after.data.esp != before.data.esp || after.data.eflags != before.data.eflags ||
                after.data.cs.selector != before.data.cs.selector ||
                after.data.cs.base != before.data.cs.base || after.data.cs.limit != before.data.cs.limit ||
                after.data.cs.seg.executable != before.data.cs.seg.executable ||
                after.data.cs.seg.exec.conform != before.data.cs.seg.exec.conform ||
                after.data.cs.seg.exec.defsize != before.data.cs.seg.exec.defsize ||
                !ct_read_private(&state, 0x0325u, &descriptor_access,
                    sizeof(descriptor_access)) || descriptor_access != 0xbau;
        }
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
    if (!failed) {
        failed = !ct_write(&state, 0x0000c000u, nonpresent, sizeof(nonpresent));
        failed |= !ct_read_private(&state, 0x032du, &descriptor_access,
            sizeof(descriptor_access)) || descriptor_access != 0x1au;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        before.data.eip = 0u;
        if (!failed) {
            C_INT run_ok = ct_run_np(&state, retf, sizeof(retf), &after);
            failed = !run_ok || after.data.eip != before.data.eip ||
                after.data.esp != before.data.esp || after.data.eflags != before.data.eflags ||
                after.data.cs.selector != before.data.cs.selector ||
                after.data.cs.base != before.data.cs.base || after.data.cs.limit != before.data.cs.limit ||
                after.data.cs.seg.executable != before.data.cs.seg.executable ||
                after.data.cs.seg.exec.conform != before.data.cs.seg.exec.conform ||
                after.data.cs.seg.exec.defsize != before.data.cs.seg.exec.defsize ||
                !ct_read_private(&state, 0x032du, &descriptor_access,
                    sizeof(descriptor_access)) || descriptor_access != 0x1au;
        }
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT ct_test_far_immediate_forms(C_VOID)
{
    static const uint8_t jmp32[] = {0xeau,7,0,0,0,8,0,0xf4u};
    static const uint8_t jmp16[] = {0x66u,0xeau,6,0,8,0,0xf4u};
    static const uint8_t call32[] = {
        0x9au,10,0,0,0,8,0,0xb0u,0xa5u,0xf4u,0xcbu
    };
    static const uint8_t call16[] = {
        0x66u,0x9au,9,0,8,0,0xb0u,0xa5u,0xf4u,0x66u,0xcbu
    };
    const uint8_t *const programs[] = {jmp32,jmp16,call32,call16};
    const STD_SIZE_T sizes[] = {
        sizeof(jmp32),sizeof(jmp16),sizeof(call32),sizeof(call16)
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(programs) / sizeof(programs[0]); ++index) {
        ct_machine state;
        t_cpu before;
        t_cpu after;
        C_INT failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);

        if (!failed) {
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed = !ct_run(&state, programs[index], sizes[index],
                CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after);
            failed |= after.data.cs.selector != before.data.cs.selector ||
                after.data.cs.base != before.data.cs.base ||
                (index >= 2u && (after.data.eax != 0x000000a5u ||
                    after.data.esp != before.data.esp));
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT ct_test_far_indirect_forms(C_VOID)
{
    static const uint8_t call32[] = {0xffu,0x1du,0,1,0,0,0xb0u,0xa5u,0xf4u,0xcbu};
    static const uint8_t call16[] = {0x66u,0xffu,0x1du,0,1,0,0,0xb0u,0xa5u,0xf4u,0x66u,0xcbu};
    static const uint8_t jmp32[] = {0xffu,0x2du,0,1,0,0,0xf4u};
    static const uint8_t jmp16[] = {0x66u,0xffu,0x2du,0,1,0,0,0xf4u};
    static const uint8_t pointer_call32[] = {9,0,0,0,8,0};
    static const uint8_t pointer_call16[] = {10,0,8,0};
    static const uint8_t pointer_jmp32[] = {6,0,0,0,8,0};
    static const uint8_t pointer_jmp16[] = {7,0,8,0};
    const uint8_t *const programs[] = {call32,call16,jmp32,jmp16};
    const uint8_t *const pointers[] = {pointer_call32,pointer_call16,pointer_jmp32,pointer_jmp16};
    const STD_SIZE_T sizes[] = {sizeof(call32),sizeof(call16),sizeof(jmp32),sizeof(jmp16)};
    const STD_SIZE_T pointer_sizes[] = {sizeof(pointer_call32),sizeof(pointer_call16),sizeof(pointer_jmp32),sizeof(pointer_jmp16)};
    STD_SIZE_T index;

    for (index = 0u; index < 4u; ++index) {
        ct_machine state;
        t_cpu before;
        t_cpu after;
        C_INT failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
        if (!failed) {
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed = !ct_write(&state, 0x00003100u, pointers[index], pointer_sizes[index]) ||
                !ct_run(&state, programs[index], sizes[index],
                    CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT, &after) ||
                (index < 2u && (after.data.eax != 0x000000a5u || after.data.esp != before.data.esp));
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    {
        ct_machine state;
        t_cpu after;
        C_INT failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
        if (!failed) failed = !ct_run_ud(&state, (const uint8_t[]){0xffu,0xd8u}, 2u, &after);
        core_machine_destroy(state.machine);
        if (failed) return 0;
        failed = !ct_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386, 1);
        if (!failed) failed = !ct_run_ud(&state, (const uint8_t[]){0xffu,0xe8u}, 2u, &after);
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

static C_INT ct_test_far_real_mode(C_VOID)
{
    static const uint8_t jmp[] = {0xeau,0,0,0,1};
    static const uint8_t call[] = {0x9au,0,0,0,1,0xf4u};
    static const uint8_t halt[] = {0xf4u};
    static const uint8_t retf[] = {0xcbu};
    const core_machine_run_budget budget = {48u,0u};
    core_machine_run_result result;
    ct_machine state;
    t_cpu after;
    C_INT failed = !ct_prepare_real(&state);

    if (!failed) {
        failed = !ct_write(&state, 0u, jmp, sizeof(jmp)) ||
            !ct_write(&state, 0x1000u, halt, sizeof(halt)) ||
            core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= after.data.cs.selector != 0x0100u || after.data.cs.base != 0x1000u;
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    failed = !ct_prepare_real(&state);
    if (!failed) {
        failed = !ct_write(&state, 0u, call, sizeof(call)) ||
            !ct_write(&state, 0x1000u, retf, sizeof(retf)) ||
            core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= after.data.cs.selector != 0u || after.data.cs.base != 0u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

C_INT main(C_VOID)
{
    if (!ct_test_jcc_short() || !ct_test_near_and_short_jumps() ||
        !ct_test_jcc_near_conditions() || !ct_test_16bit_code_and_real_mode() ||
        !ct_test_loop_and_jcxz() || !ct_test_jcc_limit_boundaries() ||
        !ct_test_loop_target_fault_is_atomic() || !ct_test_pre386_near_jcc_is_ud() ||
        !ct_test_ret_target_fault_is_atomic() || !ct_test_near_call_and_ret_forms() ||
        !ct_test_near_indirect_and_fault_boundaries() ||
        !ct_test_far_same_cpl_return_validation() || !ct_test_far_immediate_forms() ||
        !ct_test_far_indirect_forms() || !ct_test_far_real_mode()) return 1;
    STD_PRINTF("M5:T303:CONTROL-TRANSFER:OK\n");
    return 0;
}
