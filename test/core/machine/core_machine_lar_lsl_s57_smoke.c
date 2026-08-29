#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define LAR_LSL_S57_GDT_POINTER_ADDRESS 0x0100u
#define LAR_LSL_S57_GDT_ADDRESS 0x0300u
#define LAR_LSL_S57_CODE_ADDRESS 0x2000u

typedef struct lar_lsl_s57_machine {
    core_machine *machine;
} lar_lsl_s57_machine;

static C_VOID lar_lsl_s57_reset(C_VOID *opaque)
{
    lar_lsl_s57_machine *state = (lar_lsl_s57_machine *)opaque;

    if (state != STD_NULL) {
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
    }
}

static const core_machine_execution_provider lar_lsl_s57_execution_provider = {
    lar_lsl_s57_reset,
    STD_NULL
};

static C_INT lar_lsl_s57_prepare(lar_lsl_s57_machine *state,
    core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE,
        .ticks_per_instruction = 29u,
        .instruction_timing = { 29u, 7u, 31u, 37u, 41u, 43u }
    };

    if (state == STD_NULL) {
        return 0;
    }
    STD_MEMSET(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK) {
        return 0;
    }
    if (!test_core_machine_fixture_bind_freeze_reset(state->machine,
            &lar_lsl_s57_execution_provider, state)) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT lar_lsl_s57_install_gdt(core_machine *machine)
{
    static const type_unsigned_8 pointer[] = {
        0x37u, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u
    };
    static const type_unsigned_8 gdt[] = {
        0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x20u, 0x00u, 0x9au, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x30u, 0x00u, 0x92u, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x40u, 0x00u, 0x12u, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x50u, 0x00u, 0x92u, 0x00u, 0x00u,
        0x10u, 0x00u, 0x00u, 0x60u, 0x00u, 0x92u, 0xc0u, 0x00u,
        0x0fu, 0x00u, 0x00u, 0x05u, 0x00u, 0x82u, 0x00u, 0x00u
    };

    return core_machine_memory_write(machine, LAR_LSL_S57_GDT_POINTER_ADDRESS,
        pointer, sizeof(pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, LAR_LSL_S57_GDT_ADDRESS, gdt,
            sizeof(gdt)) == TYPE_STATUS_OK;
}

static C_INT lar_lsl_s57_run_protected(lar_lsl_s57_machine *state,
    const type_unsigned_8 *code, STD_SIZE_T code_size, t_cpu *out_cpu)
{
    static const type_unsigned_8 real_code[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0x0fu, 0x01u, 0x1eu, 0x10u, 0x01u,
        0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x10u, 0x00u,
        0x8eu, 0xd8u,
        0x8eu, 0xc0u,
        0x8eu, 0xd0u,
        0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    static const type_unsigned_8 idt_pointer[] = {
        0x07u, 0x01u, 0x00u, 0x04u, 0x00u, 0x00u
    };
    const core_machine_run_budget budget = { 64u, 0u };
    core_machine_run_result result;
    type_unsigned_8 idt[0x108u] = { 0u };

    idt[0x100u] = 0x00u;
    idt[0x101u] = 0x01u;
    idt[0x102u] = 0x08u;
    idt[0x105u] = 0x8eu;

    if (state == STD_NULL || state->machine == STD_NULL ||
        code == STD_NULL || out_cpu == STD_NULL ||
        !lar_lsl_s57_install_gdt(state->machine) ||
        core_machine_memory_write(state->machine, 0x0110u, idt_pointer,
            sizeof(idt_pointer)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, 0x0400u, idt,
            sizeof(idt)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, LAR_LSL_S57_CODE_ADDRESS +
            0x100u, (const type_unsigned_8[]){ 0xf4u }, 1u) != TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, 0u, real_code,
            sizeof(real_code)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, LAR_LSL_S57_CODE_ADDRESS,
            code, code_size) != TYPE_STATUS_OK ||
        core_machine_run(state->machine, budget, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
        return 0;
    }
    *out_cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static C_INT lar_lsl_s57_boot_protected(lar_lsl_s57_machine *state,
    t_cpu *out_cpu)
{
    static const type_unsigned_8 hlt[] = { 0xf4u };

    return lar_lsl_s57_run_protected(state, hlt, sizeof(hlt), out_cpu);
}

static C_INT lar_lsl_s57_resume(lar_lsl_s57_machine *state,
    const type_unsigned_8 *code, STD_SIZE_T code_size, const core_machine_run_budget *budget,
    core_machine_run_result *result)
{
    if (state == STD_NULL || state->machine == STD_NULL || code == STD_NULL ||
        budget == STD_NULL || result == STD_NULL ||
        core_machine_memory_write(state->machine, LAR_LSL_S57_CODE_ADDRESS,
            code, code_size) != TYPE_STATUS_OK) {
        return 0;
    }
    test_core_machine_fixture_resume_after_halt_at(state->machine, 0u);
    return core_machine_run(state->machine, *budget, result) == TYPE_STATUS_OK;
}

static C_INT lar_lsl_s57_sregs_same(const t_cpu *before, const t_cpu *after)
{
    return STD_MEMCMP(&before->data.es, &after->data.es,
        sizeof(before->data.es)) == 0 &&
        STD_MEMCMP(&before->data.cs, &after->data.cs,
        sizeof(before->data.cs)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after->data.ss,
        sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after->data.ds,
        sizeof(before->data.ds)) == 0 &&
        STD_MEMCMP(&before->data.fs, &after->data.fs,
        sizeof(before->data.fs)) == 0 &&
        STD_MEMCMP(&before->data.gs, &after->data.gs,
        sizeof(before->data.gs)) == 0;
}

static C_INT lar_lsl_s57_nonstack_gprs_same(const t_cpu *before,
    const t_cpu *after, C_INT allow_eax, C_INT allow_ecx)
{
    return (allow_eax || before->data.eax == after->data.eax) &&
        (allow_ecx || before->data.ecx == after->data.ecx) &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi;
}

static C_INT lar_lsl_s57_gprs_same_except_eax_ecx(const t_cpu *before,
    const t_cpu *after)
{
    return before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi;
}

static C_INT lar_lsl_s57_pic_frame_matches(const type_unsigned_32 *frame,
    type_unsigned_32 expected_ip, const t_cpu *before, type_unsigned_8 expected_zf)
{
    return frame[0u] == expected_ip &&
        ((frame[2u] & ~VCPU_EFLAGS_ZF) ==
        (before->data.eflags & ~VCPU_EFLAGS_ZF)) &&
        !!TYPE_GET_BIT(frame[2u], VCPU_EFLAGS_ZF) == expected_zf;
}

static C_INT lar_lsl_s57_test_default(C_VOID)
{
    static const type_unsigned_8 lar[] = { 0x0fu, 0x02u, 0xc1u, 0xf4u };
    static const type_unsigned_8 lsl[] = { 0x0fu, 0x03u, 0xc1u, 0xf4u };
    const type_unsigned_32 expected[] = { 0xa1a19300u, 0xa1afffffu };
    const type_unsigned_8 *codes[] = { lar, lsl };
    type_unsigned_8 profile;
    type_unsigned_8 form;

    for (profile = 0u; profile != 2u; ++profile) {
        for (form = 0u; form != 2u; ++form) {
            lar_lsl_s57_machine state;
            t_cpu before;
            t_cpu after;
            C_INT failed = !lar_lsl_s57_prepare(&state,
                profile ? CORE_MACHINE_CPU_PROFILE_80386 :
                CORE_MACHINE_CPU_PROFILE_80286);

            if (!failed) {
                state.machine->executor_cpu.data.eax = 0xa1a10000u;
                state.machine->executor_cpu.data.ecx = 0x10u;
                state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_IF |
                    VCPU_EFLAGS_CF;
                before = state.machine->executor_cpu;
                failed = !lar_lsl_s57_run_protected(&state, codes[form], 4u,
                    &after);
                failed |= after.data.eip != 4u;
                failed |= !TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_ZF);
                failed |= after.data.eax !=
                    ((before.data.eax & 0xffff0000u) |
                    (expected[form] & 0xffffu));
                failed |= after.data.ecx != before.data.ecx;
            }
            core_machine_destroy(state.machine);
            if (failed) {
                return 0;
            }
        }
    }
    return 1;
}

static C_INT lar_lsl_s57_test_invalid_selector(C_VOID)
{
    static const type_unsigned_8 lar[] = { 0x0fu, 0x02u, 0xc1u, 0xf4u };
    static const type_unsigned_8 lsl[] = { 0x0fu, 0x03u, 0xc1u, 0xf4u };
    const type_unsigned_8 *codes[] = { lar, lsl };
    type_unsigned_8 form;

    for (form = 0u; form != 2u; ++form) {
        lar_lsl_s57_machine state;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        C_INT failed = !lar_lsl_s57_prepare(&state,
            CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            failed = !lar_lsl_s57_boot_protected(&state, &before);
        }
        if (!failed) {
            state.machine->executor_cpu.data.eax = 0xa1a10000u;
            state.machine->executor_cpu.data.ecx = 0x00000000u;
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_CF;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed = !lar_lsl_s57_resume(&state, codes[form], 4u,
                &(const core_machine_run_budget){ 4u, 0u }, &result);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            failed |= after.data.eip != 4u;
            failed |= TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_ZF);
            failed |= after.data.eax != before.data.eax;
            failed |= after.data.ecx != before.data.ecx;
            failed |= (after.data.eflags & ~VCPU_EFLAGS_ZF) !=
                (before.data.eflags & ~VCPU_EFLAGS_ZF);
            failed |= !lar_lsl_s57_sregs_same(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed) {
            return 0;
        }
    }
    return 1;
}

static C_INT lar_lsl_s57_test_attributes(C_VOID)
{
    static const type_unsigned_8 lar_66[] = { 0x66u, 0x0fu, 0x02u, 0xc1u, 0xf4u };
    static const type_unsigned_8 lar_67[] = { 0x67u, 0x0fu, 0x02u, 0xc1u, 0xf4u };
    static const type_unsigned_8 lar_6667[] = {
        0x66u, 0x67u, 0x0fu, 0x02u, 0xc1u, 0xf4u
    };
    static const type_unsigned_8 lsl_66[] = { 0x66u, 0x0fu, 0x03u, 0xc1u, 0xf4u };
    static const type_unsigned_8 lsl_67[] = { 0x67u, 0x0fu, 0x03u, 0xc1u, 0xf4u };
    static const type_unsigned_8 lsl_6667[] = {
        0x66u, 0x67u, 0x0fu, 0x03u, 0xc1u, 0xf4u
    };
    const type_unsigned_8 *codes[] = {
        lar_66, lar_67, lar_6667, lsl_66, lsl_67, lsl_6667
    };
    const type_unsigned_8 lengths[] = { 5u, 5u, 6u, 5u, 5u, 6u };
    const type_unsigned_32 expected[] = {
        0x00009300u, 0xa1a19300u, 0x00009300u,
        0x0000ffffu, 0xa1a1ffffu, 0x0000ffffu
    };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form) {
        lar_lsl_s57_machine state;
        t_cpu after;
        C_INT failed = !lar_lsl_s57_prepare(&state,
            CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            state.machine->executor_cpu.data.eax = 0xa1a10000u;
            state.machine->executor_cpu.data.ecx = 0x10u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_IF |
                VCPU_EFLAGS_CF;
            failed = !lar_lsl_s57_run_protected(&state, codes[form],
                lengths[form], &after);
            failed |= after.data.eip != lengths[form];
            failed |= after.data.eax != expected[form];
            failed |= after.data.ecx != 0x10u;
            failed |= !TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_ZF);
            failed |= !TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_IF);
            failed |= !TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_CF);
        }
        core_machine_destroy(state.machine);
        if (failed) {
            return 0;
        }
    }
    return 1;
}

static C_INT lar_lsl_s57_test_profile_and_lock_rejects(C_VOID)
{
    static const type_unsigned_8 forms[][6] = {
        { 0x0fu, 0x02u, 0xc1u },
        { 0x0fu, 0x03u, 0xc1u },
        { 0x66u, 0x0fu, 0x02u, 0xc1u },
        { 0x67u, 0x0fu, 0x02u, 0xc1u },
        { 0x66u, 0x67u, 0x0fu, 0x02u, 0xc1u },
        { 0x66u, 0x0fu, 0x03u, 0xc1u },
        { 0x67u, 0x0fu, 0x03u, 0xc1u },
        { 0x66u, 0x67u, 0x0fu, 0x03u, 0xc1u },
        { 0xf0u, 0x0fu, 0x02u, 0xc1u },
        { 0xf0u, 0x0fu, 0x03u, 0xc1u },
        { 0xf0u, 0x66u, 0x0fu, 0x02u, 0xc1u },
        { 0xf0u, 0x66u, 0x0fu, 0x03u, 0xc1u },
        { 0xf0u, 0x67u, 0x0fu, 0x02u, 0xc1u },
        { 0xf0u, 0x67u, 0x0fu, 0x03u, 0xc1u },
        { 0xf0u, 0x66u, 0x67u, 0x0fu, 0x02u, 0xc1u },
        { 0xf0u, 0x66u, 0x67u, 0x0fu, 0x03u, 0xc1u }
    };
    static const type_unsigned_8 lengths[] = {
        3u, 3u, 4u, 4u, 5u, 4u, 4u, 5u, 4u, 4u,
        5u, 5u, 5u, 5u, 6u, 6u
    };
    const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    type_unsigned_8 profile;
    type_unsigned_8 form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        for (form = 0u; form != 8u; ++form) {
            lar_lsl_s57_machine state;
            core_machine_cpu_diagnostic diagnostic;
            core_machine_run_result result;
            t_cpu before;
            t_cpu after;
            const core_machine_run_budget budget = { 1u, 0u };
            C_INT failed = !lar_lsl_s57_prepare(&state, profiles[profile]);

            if (!failed) {
                failed |= core_machine_memory_write(state.machine, 0u,
                    forms[form], lengths[form]) != TYPE_STATUS_OK;
                failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                    state.machine);
                before = state.machine->executor_cpu;
                failed |= core_machine_run(state.machine, budget, &result) !=
                    TYPE_STATUS_FAULT;
                after = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed |= core_machine_get_cpu_diagnostic(state.machine,
                    &diagnostic) != TYPE_STATUS_OK ||
                    !diagnostic.first_fault.valid ||
                    !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                        VCPUINS_EXCEPT_UD);
                failed |= STD_MEMCMP(&before, &after, sizeof(before)) != 0;
            }
            core_machine_destroy(state.machine);
            if (failed) {
                return 0;
            }
        }
    }
    for (form = 2u; form != sizeof(forms) / sizeof(forms[0]); ++form) {
        lar_lsl_s57_machine state;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        const core_machine_run_budget budget = { 1u, 0u };
        C_INT failed = !lar_lsl_s57_prepare(&state,
            CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            failed |= core_machine_memory_write(state.machine, 0u,
                forms[form], lengths[form]) != TYPE_STATUS_OK;
            failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                state.machine);
            before = state.machine->executor_cpu;
            failed |= core_machine_run(state.machine, budget, &result) !=
                TYPE_STATUS_FAULT;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_get_cpu_diagnostic(state.machine,
                &diagnostic) != TYPE_STATUS_OK ||
                !diagnostic.first_fault.valid ||
                !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_UD);
            failed |= STD_MEMCMP(&before, &after, sizeof(before)) != 0;
        }
        core_machine_destroy(state.machine);
        if (failed) {
            return 0;
        }
    }
    return 1;
}

static C_INT lar_lsl_s57_test_real_mode_rejects(C_VOID)
{
    static const type_unsigned_8 forms[][3] = {
        { 0x0fu, 0x02u, 0xc1u },
        { 0x0fu, 0x03u, 0xc1u }
    };
    const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;
    type_unsigned_8 form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        for (form = 0u; form != sizeof(forms) / sizeof(forms[0]); ++form) {
            lar_lsl_s57_machine state;
            core_machine_cpu_diagnostic diagnostic;
            core_machine_run_result result;
            t_cpu before;
            t_cpu after;
            const core_machine_run_budget budget = { 1u, 0u };
            C_INT failed = !lar_lsl_s57_prepare(&state, profiles[profile]);

            if (!failed) {
                failed |= core_machine_memory_write(state.machine, 0u,
                    forms[form], sizeof(forms[form])) != TYPE_STATUS_OK;
                state.machine->executor_cpu.data.eax = 0xa1a10010u;
                state.machine->executor_cpu.data.ecx = 0x00000010u;
                state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_IF |
                    VCPU_EFLAGS_CF;
                failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                    state.machine);
                before = state.machine->executor_cpu;
                failed |= core_machine_run(state.machine, budget, &result) !=
                    TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT;
                after = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed |= core_machine_get_cpu_diagnostic(state.machine,
                    &diagnostic) != TYPE_STATUS_OK ||
                    !diagnostic.first_fault.valid ||
                    !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                        VCPUINS_EXCEPT_UD);
                failed |= after.data.eax != before.data.eax ||
                    after.data.ecx != before.data.ecx ||
                    after.data.eflags != before.data.eflags ||
                    !lar_lsl_s57_sregs_same(&before, &after);
            }
            core_machine_destroy(state.machine);
            if (failed) {
                return 0;
            }
        }
    }
    return 1;
}

static C_INT lar_lsl_s57_test_descriptor_matrix(C_VOID)
{
    static const type_unsigned_8 nonpresent_lar[] = {
        0xb8u, 0x18u, 0x00u, 0xb9u, 0x34u, 0x12u,
        0x0fu, 0x02u, 0xc8u, 0xf4u
    };
    static const type_unsigned_8 privilege_lsl[] = {
        0xb8u, 0x23u, 0x00u, 0xb9u, 0x34u, 0x12u,
        0x0fu, 0x03u, 0xc8u, 0xf4u
    };
    static const type_unsigned_8 granular_lsl[] = {
        0xb8u, 0x28u, 0x00u, 0xb9u, 0x34u, 0x12u,
        0x0fu, 0x03u, 0xc8u, 0xf4u
    };
    const type_unsigned_8 *codes[] = {
        nonpresent_lar, privilege_lsl, granular_lsl
    };
    const type_unsigned_32 expected_ecx[] = {
        0x00001234u, 0x00001234u, 0x00000fffu
    };
    const type_unsigned_8 expected_zf[] = { 0u, 0u, 1u };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form) {
        lar_lsl_s57_machine state;
        t_cpu after;
        C_INT failed = !lar_lsl_s57_prepare(&state,
            CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            failed = !lar_lsl_s57_run_protected(&state, codes[form], 10u,
                &after);
            failed |= after.data.eip != 10u;
            failed |= after.data.ecx != expected_ecx[form];
            failed |= !!TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_ZF) !=
                expected_zf[form];
        }
        core_machine_destroy(state.machine);
        if (failed) {
            return 0;
        }
    }
    return 1;
}

static C_INT lar_lsl_s57_test_memory_source(C_VOID)
{
    static const type_unsigned_8 lar[] = {
        0xb9u, 0x34u, 0x12u,
        0x0fu, 0x02u, 0x0eu, 0x00u, 0x01u,
        0xf4u
    };
    static const type_unsigned_8 lsl[] = {
        0xb9u, 0x34u, 0x12u,
        0x0fu, 0x03u, 0x0eu, 0x00u, 0x01u,
        0xf4u
    };
    const type_unsigned_8 *codes[] = { lar, lsl };
    const type_unsigned_16 selector = 0x0010u;
    const type_unsigned_32 expected[] = { 0x00009300u, 0x0000ffffu };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form) {
        lar_lsl_s57_machine state;
        t_cpu after;
        type_unsigned_16 source = selector;
        C_INT failed = !lar_lsl_s57_prepare(&state,
            CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            failed |= core_machine_memory_write(state.machine, 0x3100u,
                &source, sizeof(source)) != TYPE_STATUS_OK;
            failed = !lar_lsl_s57_run_protected(&state, codes[form], 9u,
                &after);
            failed |= core_machine_memory_read(state.machine, 0x3100u,
                &source, sizeof(source)) != TYPE_STATUS_OK;
            failed |= after.data.eip != 9u || after.data.ecx != expected[form] ||
                !TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_ZF) ||
                source != selector;
        }
        core_machine_destroy(state.machine);
        if (failed) {
            return 0;
        }
    }
    return 1;
}

static C_INT lar_lsl_s57_test_67_sib_ss_source(C_VOID)
{
    static const type_unsigned_8 code[] = {
        0x66u, 0xbcu, 0x00u, 0x01u, 0x00u, 0x00u,
        0x67u, 0x0fu, 0x02u, 0x0cu, 0x24u,
        0xf4u
    };
    const type_unsigned_16 selector = 0x0010u;
    lar_lsl_s57_machine state;
    t_cpu after;
    type_unsigned_16 source = selector;
    C_INT failed = !lar_lsl_s57_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0x3100u,
            &source, sizeof(source)) != TYPE_STATUS_OK;
        failed = !lar_lsl_s57_run_protected(&state, code, sizeof(code),
            &after);
        failed |= core_machine_memory_read(state.machine, 0x3100u,
            &source, sizeof(source)) != TYPE_STATUS_OK;
        failed |= after.data.eip != sizeof(code) ||
            after.data.ecx != 0x00009300u ||
            after.data.esp != 0x00000100u ||
            !TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_ZF) ||
            source != selector;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT lar_lsl_s57_test_bp_ss_source(C_VOID)
{
    static const type_unsigned_8 code[] = {
        0xbdu, 0x00u, 0x01u, 0xb9u, 0x34u, 0x12u,
        0x0fu, 0x03u, 0x4eu, 0x00u,
        0xf4u
    };
    const type_unsigned_16 selector = 0x0010u;
    lar_lsl_s57_machine state;
    t_cpu after;
    type_unsigned_16 source = selector;
    C_INT failed = !lar_lsl_s57_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0x3100u,
            &source, sizeof(source)) != TYPE_STATUS_OK;
        failed = !lar_lsl_s57_run_protected(&state, code, sizeof(code),
            &after);
        failed |= after.data.eip != sizeof(code) ||
            after.data.ecx != 0x0000ffffu ||
            after.data.ebp != 0x00000100u || source != selector;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT lar_lsl_s57_test_override_sources(C_VOID)
{
    static const type_unsigned_8 codes[][7] = {
        { 0x2eu, 0x0fu, 0x02u, 0x0eu, 0x20u, 0x01u, 0xf4u },
        { 0x26u, 0x0fu, 0x03u, 0x0eu, 0x20u, 0x01u, 0xf4u },
        { 0x64u, 0x0fu, 0x02u, 0x0eu, 0x20u, 0x01u, 0xf4u },
        { 0x65u, 0x0fu, 0x03u, 0x0eu, 0x20u, 0x01u, 0xf4u }
    };
    const type_unsigned_32 bases[] = { 0x2000u, 0x4000u, 0x5000u, 0x6000u };
    const type_unsigned_32 expected[] = {
        0x00009300u, 0x0000ffffu, 0x00009300u, 0x0000ffffu
    };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form) {
        lar_lsl_s57_machine state;
        t_cpu before;
        t_cpu after;
        type_unsigned_16 source = 0x0010u;
        C_INT failed = !lar_lsl_s57_prepare(&state,
            CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            failed = !lar_lsl_s57_boot_protected(&state, &before);
        }
        if (!failed) {
            state.machine->executor_cpu.data.es.base = 0x4000u;
            state.machine->executor_cpu.data.fs.base = 0x5000u;
            state.machine->executor_cpu.data.fs.selector = 0x0010u;
            state.machine->executor_cpu.data.fs.flagValid = TYPE_TRUE;
            state.machine->executor_cpu.data.gs.base = 0x6000u;
            state.machine->executor_cpu.data.gs.selector = 0x0010u;
            state.machine->executor_cpu.data.gs.flagValid = TYPE_TRUE;
            failed |= core_machine_memory_write(state.machine,
                bases[form] + 0x120u, &source, sizeof(source)) !=
                TYPE_STATUS_OK;
            state.machine->executor_cpu.data.ecx = 0x00001234u;
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_CF;
            before = state.machine->executor_cpu;
            failed |= !lar_lsl_s57_resume(&state, codes[form],
                sizeof(codes[form]), &(const core_machine_run_budget){ 8u, 0u },
                &(core_machine_run_result){ 0 });
            after = test_core_machine_fixture_capture_cpu_after_run(
                state.machine);
            failed |= core_machine_memory_read(state.machine,
                bases[form] + 0x120u, &source, sizeof(source)) !=
                TYPE_STATUS_OK;
            failed |= after.data.eip != sizeof(codes[form]) ||
                after.data.ecx != expected[form] || source != 0x0010u ||
                !TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_ZF) ||
                !lar_lsl_s57_nonstack_gprs_same(&before, &after, 0, 1) ||
                (after.data.eflags & ~VCPU_EFLAGS_ZF) !=
                (before.data.eflags & ~VCPU_EFLAGS_ZF) ||
                !lar_lsl_s57_sregs_same(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed) {
            return 0;
        }
    }
    return 1;
}

static C_INT lar_lsl_s57_test_source_limit(C_VOID)
{
    static const type_unsigned_8 codes[][5] = {
        { 0x0fu, 0x02u, 0x0eu, 0x10u, 0x00u },
        { 0x0fu, 0x03u, 0x0eu, 0x10u, 0x00u }
    };
    const core_machine_run_budget budget = { 64u, 0u };
    type_unsigned_8 form;

    for (form = 0u; form != 2u; ++form) {
        lar_lsl_s57_machine state;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        type_unsigned_16 source = 0x0010u;
        C_INT failed = !lar_lsl_s57_prepare(&state,
            CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            failed = !lar_lsl_s57_boot_protected(&state, &before);
        }
        if (!failed) {
            state.machine->executor_cpu.data.ds.limit = 0x000fu;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_write(state.machine, 0x3010u, &source,
                sizeof(source)) != TYPE_STATUS_OK;
            failed |= core_machine_memory_write(state.machine,
                LAR_LSL_S57_CODE_ADDRESS, codes[form], sizeof(codes[form])) !=
                TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine, budget, &result) !=
                TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                0x3010u, (type_virtual_address)&source, sizeof(source)) !=
                TYPE_STATUS_OK;
            failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK || !diagnostic.first_fault.valid ||
                !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_DF);
            failed |= after.data.eax != before.data.eax ||
                after.data.ecx != before.data.ecx || after.data.eip != 0u ||
                after.data.eflags != before.data.eflags ||
                !lar_lsl_s57_gprs_same_except_eax_ecx(&before, &after) ||
                !lar_lsl_s57_sregs_same(&before, &after) || source != 0x0010u;
        }
        core_machine_destroy(state.machine);
        if (failed) {
            return 0;
        }
    }
    return 1;
}

static C_INT lar_lsl_s57_test_vm86(C_VOID)
{
    static const type_unsigned_8 codes[][3] = {
        { 0x0fu, 0x02u, 0xc1u },
        { 0x0fu, 0x03u, 0xc1u }
    };
    const core_machine_run_budget budget = { 1u, 0u };
    type_unsigned_8 form;

    for (form = 0u; form != 2u; ++form) {
    lar_lsl_s57_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    C_INT failed = !lar_lsl_s57_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed = !test_core_machine_fixture_prepare_real_mode_execution(
            state.machine, 0u);
    }
    if (!failed) {
        state.machine->executor_cpu.data.cr0 |= VCPU_CR0_PE;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_VM |
            VCPU_EFLAGS_IOPL | VCPU_EFLAGS_IF | VCPU_EFLAGS_CF;
        state.machine->executor_cpu.data.cs.limit = 0xffffu;
        state.machine->executor_cpu.data.ds.limit = 0xffffu;
        state.machine->executor_cpu.data.ss.limit = 0xffffu;
        state.machine->executor_cpu.data.cs.flagValid = TYPE_TRUE;
        state.machine->executor_cpu.data.ds.flagValid = TYPE_TRUE;
        state.machine->executor_cpu.data.ss.flagValid = TYPE_TRUE;
        state.machine->executor_cpu.data.eax = 0xa1a10010u;
        state.machine->executor_cpu.data.ecx = 0x00000010u;
        before = state.machine->executor_cpu;
        failed |= core_machine_memory_write(state.machine, 0u, codes[form],
            sizeof(codes[form])) != TYPE_STATUS_OK;
        failed |= core_machine_run(state.machine, budget, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK || !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_UD) || after.data.eax != before.data.eax ||
            after.data.ecx != before.data.ecx ||
            after.data.eip != before.data.eip ||
            after.data.eflags != before.data.eflags ||
            !lar_lsl_s57_gprs_same_except_eax_ecx(&before, &after) ||
            !lar_lsl_s57_sregs_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    if (failed) return 0;
    }
    return 1;
}

static C_INT lar_lsl_s57_test_pic_lar(C_VOID)
{
    static const type_unsigned_8 code[] = { 0xfbu, 0x0fu, 0x02u, 0xc1u, 0x90u };
    lar_lsl_s57_machine state;
    core_machine_pic_irq_source source;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    type_unsigned_32 frame[3u] = { 0u };
    C_INT failed = !lar_lsl_s57_prepare(&state,
        CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed = !lar_lsl_s57_boot_protected(&state, &before);
    }
    if (!failed) {
        state.machine->executor_cpu.data.eax = 0xa1a10000u;
        state.machine->executor_cpu.data.ecx = 0x0010u;
        state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_write(state.machine,
            LAR_LSL_S57_CODE_ADDRESS, code, sizeof(code)) != TYPE_STATUS_OK;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        STD_MEMSET(&source, 0, sizeof(source));
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&source, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 2u, 0u }, &result) != TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= result.reason != CORE_MACHINE_STOP_BUDGET ||
            after.data.eip != 0x100u || after.data.eax != 0xa1a19300u ||
            !TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_ZF) ||
            !lar_lsl_s57_nonstack_gprs_same(&before, &after, 1, 0) ||
            !lar_lsl_s57_sregs_same(&before, &after) ||
            !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u)) ||
            TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
                VPIC_IRR_IRQ(0u));
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            after.data.ss.base + (type_unsigned_16)after.data.esp,
            (type_virtual_address)frame, sizeof(frame)) != TYPE_STATUS_OK ||
            !lar_lsl_s57_pic_frame_matches(frame, 4u, &before, 1u);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT lar_lsl_s57_test_80386_timing_granularity(C_VOID)
{
    static const type_unsigned_8 lsl[] = { 0x0fu, 0x03u, 0xc8u };
    static const type_unsigned_8 lsl_memory[] = {
        0x0fu, 0x03u, 0x0eu, 0x00u, 0x10u
    };
    const type_unsigned_8 *codes[] = { lsl, lsl, lsl_memory, lsl_memory };
    const core_machine_run_budget budget = { 1u, 0u };
    const type_unsigned_16 selectors[] = { 0x0010u, 0x0028u, 0x0010u, 0x0028u };
    const type_unsigned_64 expected[] = { 21u, 25u, 22u, 26u };
    type_unsigned_8 index;

    for (index = 0u; index < 4u; ++index) {
        lar_lsl_s57_machine state;
        core_machine_run_result result;
        t_cpu after;
        C_INT failed = !lar_lsl_s57_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386) ||
            !lar_lsl_s57_boot_protected(&state, &after);

        if (!failed) {
            state.machine->executor_cpu.data.eax = selectors[index];
            state.machine->executor_cpu.data.ecx = 0u;
            state.machine->elapsed_ticks = 0u;
            if (index >= 2u) failed |= core_machine_memory_write(state.machine,
                0x4000u, &selectors[index], sizeof(selectors[index])) != TYPE_STATUS_OK;
            failed |= !lar_lsl_s57_resume(&state, codes[index],
                index < 2u ? sizeof(lsl) : sizeof(lsl_memory), &budget, &result) ||
                result.reason != CORE_MACHINE_STOP_BUDGET || result.executed != 1u ||
                result.ticks != expected[index] || result.elapsed_ticks != expected[index] ||
                (state.machine->executor_cpu.data.eflags & VCPU_EFLAGS_ZF) == 0u;
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}
static C_INT lar_lsl_s57_test_pic_lsl_and_invalid(C_VOID)
{
    static const type_unsigned_8 codes[][8] = {
        { 0xfbu, 0x0fu, 0x03u, 0xc1u, 0x90u },
        { 0xfbu, 0x0fu, 0x02u, 0xc1u, 0x90u }
    };
    const type_unsigned_8 lengths[] = { 5u, 5u };
    const type_unsigned_32 expected[] = { 0xa1a1ffffu, 0xa1a10000u };
    const type_unsigned_8 expected_zf[] = { 1u, 0u };
    const type_unsigned_32 expected_ip[] = { 4u, 4u };
    type_unsigned_8 form;

    for (form = 0u; form != 2u; ++form) {
        lar_lsl_s57_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        type_unsigned_32 frame[3u] = { 0u };
        C_INT failed = !lar_lsl_s57_prepare(&state,
            CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            failed = !lar_lsl_s57_boot_protected(&state, &after);
        }
        if (!failed) {
            state.machine->executor_cpu.data.eax = form ? 0xa1a10000u :
                0xa1a10010u;
            state.machine->executor_cpu.data.ecx = form ? 0u : 0x0010u;
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_write(state.machine,
                LAR_LSL_S57_CODE_ADDRESS, codes[form], lengths[form]) != TYPE_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            STD_MEMSET(&source, 0, sizeof(source));
            state.machine->shared_pic_master.data.icw2 = 0x20u;
            core_machine_pic_irq_source_bind(&source, &state.machine->shared_pic_master,
                &state.machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){ 2u, 0u }, &result) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= result.reason != CORE_MACHINE_STOP_BUDGET ||
                after.data.eip != 0x100u || after.data.eax != expected[form] ||
                !!TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_ZF) != expected_zf[form] ||
                !lar_lsl_s57_nonstack_gprs_same(&before, &after, 1, 0) ||
                !lar_lsl_s57_sregs_same(&before, &after) ||
                !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u)) ||
                TYPE_GET_BIT(state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u));
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (type_unsigned_16)after.data.esp,
                (type_virtual_address)frame, sizeof(frame)) != TYPE_STATUS_OK ||
                !lar_lsl_s57_pic_frame_matches(frame, expected_ip[form],
                    &before, expected_zf[form]);
        }
        core_machine_destroy(state.machine);
        if (failed) return 0;
    }
    return 1;
}

/* PIC vectors use the protected IDT/IRQ0 contract after the local bootstrap. */

static C_INT lar_lsl_s57_test_ldt_selector(C_VOID)
{
    static const type_unsigned_8 ldt_descriptor[] = {
        0xffu, 0xffu, 0x00u, 0x70u, 0x00u, 0x92u, 0x00u, 0x00u
    };
    static const type_unsigned_8 lar[] = {
        0xb8u, 0x30u, 0x00u, 0x0fu, 0x00u, 0xd0u,
        0xb8u, 0x0cu, 0x00u, 0xb9u, 0x34u, 0x12u,
        0x0fu, 0x02u, 0xc8u, 0xf4u
    };
    static const type_unsigned_8 lsl[] = {
        0xb8u, 0x30u, 0x00u, 0x0fu, 0x00u, 0xd0u,
        0xb8u, 0x0cu, 0x00u, 0xb9u, 0x34u, 0x12u,
        0x0fu, 0x03u, 0xc8u, 0xf4u
    };
    const type_unsigned_8 *codes[] = { lar, lsl };
    const type_unsigned_32 expected[] = { 0x00009200u, 0x0000ffffu };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form) {
        lar_lsl_s57_machine state;
        t_cpu after;
        C_INT failed = !lar_lsl_s57_prepare(&state,
            CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            failed |= core_machine_memory_write(state.machine, 0x0508u,
                ldt_descriptor, sizeof(ldt_descriptor)) != TYPE_STATUS_OK;
            failed = !lar_lsl_s57_run_protected(&state, codes[form], 16u,
                &after);
            failed |= after.data.eip != 16u || after.data.ecx != expected[form] ||
                !TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_ZF) ||
                !after.data.ldtr.flagValid || after.data.ldtr.selector != 0x0030u;
        }
        core_machine_destroy(state.machine);
        if (failed) {
            return 0;
        }
    }
    return 1;
}

C_INT main(C_VOID)
{
    if (!lar_lsl_s57_test_default())
    {
        STD_FPRINTF(STD_STDERR, "S57 default failed\n");
        return 1;
    }
    if (!lar_lsl_s57_test_invalid_selector())
    {
        STD_FPRINTF(STD_STDERR, "S57 invalid failed\n");
        return 1;
    }
    if (!lar_lsl_s57_test_attributes())
    {
        STD_FPRINTF(STD_STDERR, "S57 attributes failed\n");
        return 1;
    }
    if (!lar_lsl_s57_test_profile_and_lock_rejects())
    {
        STD_FPRINTF(STD_STDERR, "S57 rejects failed\n");
        return 1;
    }
    if (!lar_lsl_s57_test_real_mode_rejects())
    {
        STD_FPRINTF(STD_STDERR, "S57 real-mode rejects failed\n");
        return 1;
    }
    if (!lar_lsl_s57_test_descriptor_matrix())
    {
        STD_FPRINTF(STD_STDERR, "S57 descriptor matrix failed\n");
        return 1;
    }
    if (!lar_lsl_s57_test_memory_source())
    {
        STD_FPRINTF(STD_STDERR, "S57 memory source failed\n");
        return 1;
    }
    if (!lar_lsl_s57_test_67_sib_ss_source())
    {
        STD_FPRINTF(STD_STDERR, "S57 67 SIB source failed\n");
        return 1;
    }
    if (!lar_lsl_s57_test_bp_ss_source())
    {
        STD_FPRINTF(STD_STDERR, "S57 BP SS source failed\n");
        return 1;
    }
    if (!lar_lsl_s57_test_override_sources())
    {
        STD_FPRINTF(STD_STDERR, "S57 override sources failed\n");
        return 1;
    }
    if (!lar_lsl_s57_test_source_limit())
    {
        STD_FPRINTF(STD_STDERR, "S57 source limit failed\n");
        return 1;
    }
    if (!lar_lsl_s57_test_vm86())
    {
        STD_FPRINTF(STD_STDERR, "S57 VM86 failed\n");
        return 1;
    }
    if (!lar_lsl_s57_test_pic_lar())
    {
        STD_FPRINTF(STD_STDERR, "S57 PIC LAR failed\n");
        return 1;
    }
    if (!lar_lsl_s57_test_80386_timing_granularity())
    {
        STD_FPRINTF(STD_STDERR, "S57 80386 LSL timing granularity failed\n");
        return 12;
    }
    if (!lar_lsl_s57_test_pic_lsl_and_invalid())
    {
        STD_FPRINTF(STD_STDERR, "S57 PIC LSL/invalid failed\n");
        return 1;
    }
    if (!lar_lsl_s57_test_ldt_selector())
    {
        STD_FPRINTF(STD_STDERR, "S57 LDT failed\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S57:LAR-LSL:OK\n");
    STD_PRINTF("M5:T401:S32:LAR-LSL-PROFILES:OK\n");
    return 0;
}
