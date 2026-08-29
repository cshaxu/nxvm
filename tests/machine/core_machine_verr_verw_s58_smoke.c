#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define VERR_VERW_S58_GDT_POINTER_ADDRESS 0x0100u
#define VERR_VERW_S58_GDT_ADDRESS 0x0300u
#define VERR_VERW_S58_CODE_ADDRESS 0x2000u

#define VERR_VERW_S58_VERR_MODRM 0xe0u
#define VERR_VERW_S58_VERW_MODRM 0xe8u

typedef struct verr_verw_s58_machine {
    core_machine *machine;
} verr_verw_s58_machine;

static C_VOID verr_verw_s58_reset(C_VOID *opaque)
{
    verr_verw_s58_machine *state = (verr_verw_s58_machine *)opaque;

    if (state != STD_NULL) {
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
    }
}

static const core_machine_execution_provider verr_verw_s58_execution_provider = {
    verr_verw_s58_reset,
    STD_NULL
};

static C_INT verr_verw_s58_prepare(verr_verw_s58_machine *state,
    core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    if (state == STD_NULL) {
        return 0;
    }
    STD_MEMSET(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK) {
        return 0;
    }
    if (!test_core_machine_fixture_bind_freeze_reset(state->machine,
            &verr_verw_s58_execution_provider, state)) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT verr_verw_s58_install_gdt(core_machine *machine)
{
    static const type_unsigned_8 pointer[] = {
        0x37u, 0x00u, 0x00u, 0x03u, 0x00u, 0x00u
    };
    static const type_unsigned_8 gdt[] = {
        0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x20u, 0x00u, 0x9au, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x30u, 0x00u, 0x92u, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x40u, 0x00u, 0x12u, 0x00u, 0x00u,
        0xffu, 0xffu, 0x00u, 0x50u, 0x00u, 0x98u, 0x00u, 0x00u,
        0x00u, 0x00u, 0x00u, 0x00u, 0x00u, 0x80u, 0x00u, 0x00u,
        0x0fu, 0x00u, 0x00u, 0x05u, 0x00u, 0x82u, 0x00u, 0x00u
    };

    return core_machine_memory_write(machine, VERR_VERW_S58_GDT_POINTER_ADDRESS,
        pointer, sizeof(pointer)) == TYPE_STATUS_OK &&
        core_machine_memory_write(machine, VERR_VERW_S58_GDT_ADDRESS, gdt,
            sizeof(gdt)) == TYPE_STATUS_OK;
}

static C_INT verr_verw_s58_boot_protected(verr_verw_s58_machine *state,
    t_cpu *out_cpu)
{
    static const type_unsigned_8 real_code[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u,
        0x0fu, 0x01u, 0x1eu, 0x10u, 0x01u,
        0xb8u, 0x01u, 0x00u, 0x0fu, 0x01u, 0xf0u,
        0xb8u, 0x10u, 0x00u, 0x8eu, 0xd8u, 0x8eu, 0xc0u, 0x8eu, 0xd0u,
        0xbcu, 0x00u, 0x80u, 0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    static const type_unsigned_8 idt_pointer[] = {
        0x07u, 0x01u, 0x00u, 0x04u, 0x00u, 0x00u
    };
    static const type_unsigned_8 halt[] = { 0xf4u };
    const core_machine_run_budget budget = { 96u, 0u };
    core_machine_run_result result;
    type_unsigned_8 idt[0x108u] = { 0u };

    idt[0x100u] = 0x00u;
    idt[0x101u] = 0x01u;
    idt[0x102u] = 0x08u;
    idt[0x105u] = 0x8eu;
    if (state == STD_NULL || state->machine == STD_NULL || out_cpu == STD_NULL ||
        !verr_verw_s58_install_gdt(state->machine) ||
        core_machine_memory_write(state->machine, 0x0110u, idt_pointer,
            sizeof(idt_pointer)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, 0x0400u, idt, sizeof(idt)) !=
            TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, VERR_VERW_S58_CODE_ADDRESS +
            0x100u, halt, sizeof(halt)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, 0u, real_code,
            sizeof(real_code)) != TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, VERR_VERW_S58_CODE_ADDRESS,
            halt, sizeof(halt)) != TYPE_STATUS_OK ||
        core_machine_run(state->machine, budget, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
        return 0;
    }
    *out_cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static C_INT verr_verw_s58_sregs_same(const t_cpu *before, const t_cpu *after)
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

static C_INT verr_verw_s58_non_eax_gprs_same(const t_cpu *before,
    const t_cpu *after)
{
    return before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi;
}

static C_INT verr_verw_s58_run_halt(verr_verw_s58_machine *state,
    const type_unsigned_8 *code, STD_SIZE_T code_size, t_cpu *out_cpu)
{
    const core_machine_run_budget budget = { 16u, 0u };
    core_machine_run_result result;

    if (state == STD_NULL || state->machine == STD_NULL || code == STD_NULL ||
        out_cpu == STD_NULL || core_machine_memory_write(state->machine,
            VERR_VERW_S58_CODE_ADDRESS, code, code_size) != TYPE_STATUS_OK) {
        return 0;
    }
    test_core_machine_fixture_resume_after_halt_at(state->machine, 0u);
    if (core_machine_run(state->machine, budget, &result) != TYPE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT) {
        return 0;
    }
    *out_cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return 1;
}

static C_INT verr_verw_s58_test_outcomes(C_VOID)
{
    static const type_unsigned_16 selectors[] = {
        0x0010u, 0x0008u, 0x0020u, 0x0018u, 0x0000u, 0x0013u
    };
    static const type_unsigned_8 expected_verr[] = { 1u, 1u, 0u, 0u, 0u, 0u };
    static const type_unsigned_8 expected_verw[] = { 1u, 0u, 0u, 0u, 0u, 0u };
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile_index;
    type_unsigned_8 query;
    type_unsigned_8 selector_index;

    for (profile_index = 0u; profile_index != 2u; ++profile_index) {
        for (query = 0u; query != 2u; ++query) {
            for (selector_index = 0u; selector_index != 6u; ++selector_index) {
                const type_unsigned_8 code[] = {
                    0xb8u, (type_unsigned_8)selectors[selector_index],
                    (type_unsigned_8)(selectors[selector_index] >> 8), 0x0fu, 0x00u,
                    query ? VERR_VERW_S58_VERW_MODRM : VERR_VERW_S58_VERR_MODRM,
                    0xf4u
                };
                verr_verw_s58_machine state;
                t_cpu before;
                t_cpu after;
                const type_unsigned_8 expected_zf = query ? expected_verw[selector_index] :
                    expected_verr[selector_index];
                C_INT failed = !verr_verw_s58_prepare(&state,
                    profiles[profile_index]);

                if (!failed) {
                    failed = !verr_verw_s58_boot_protected(&state, &before);
                }
                if (!failed) {
                    state.machine->executor_cpu.data.eax = 0xa1a10000u;
                    state.machine->executor_cpu.data.ecx = 0x11223344u;
                    state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
                        VCPU_EFLAGS_PF | VCPU_EFLAGS_AF | VCPU_EFLAGS_SF |
                        VCPU_EFLAGS_OF | VCPU_EFLAGS_IF;
                    before = test_core_machine_fixture_capture_cpu_after_run(
                        state.machine);
                    failed = !verr_verw_s58_run_halt(&state, code, sizeof(code),
                        &after);
                    failed |= after.data.eax != (0xa1a10000u |
                        selectors[selector_index]) || after.data.eip != 7u ||
                        !!TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_ZF) !=
                            expected_zf ||
                        (after.data.eflags & ~VCPU_EFLAGS_ZF) !=
                            (before.data.eflags & ~VCPU_EFLAGS_ZF) ||
                        !verr_verw_s58_non_eax_gprs_same(&before, &after) ||
                        !verr_verw_s58_sregs_same(&before, &after);
                }
                core_machine_destroy(state.machine);
                if (failed) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

static C_INT verr_verw_s58_expect_ud(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, STD_SIZE_T code_size)
{
    const core_machine_run_budget budget = { 16u, 0u };
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    verr_verw_s58_machine state;
    t_cpu before;
    t_cpu after;
    C_INT failed = !verr_verw_s58_prepare(&state, profile);

    if (!failed) {
        state.machine->executor_cpu.data.eax = 0xa1a10010u;
        state.machine->executor_cpu.data.ecx = 0x11223344u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
            VCPU_EFLAGS_PF | VCPU_EFLAGS_AF | VCPU_EFLAGS_SF |
            VCPU_EFLAGS_OF | VCPU_EFLAGS_IF;
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_write(state.machine, 0u, code, code_size) !=
            TYPE_STATUS_OK;
        failed |= core_machine_run(state.machine, budget, &result) !=
            TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK || !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_UD);
        failed |= STD_MEMCMP(&before, &after, sizeof(before)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT verr_verw_s58_test_prefix_and_rejection(C_VOID)
{
    static const type_unsigned_8 default_verr[] = { 0x0fu, 0x00u, 0xe0u };
    static const type_unsigned_8 default_verw[] = { 0x0fu, 0x00u, 0xe8u };
    static const type_unsigned_8 attr_verr[][5] = {
        { 0x66u, 0x0fu, 0x00u, 0xe0u, 0u },
        { 0x67u, 0x0fu, 0x00u, 0xe0u, 0u },
        { 0x66u, 0x67u, 0x0fu, 0x00u, 0xe0u }
    };
    static const type_unsigned_8 attr_verw[][5] = {
        { 0x66u, 0x0fu, 0x00u, 0xe8u, 0u },
        { 0x67u, 0x0fu, 0x00u, 0xe8u, 0u },
        { 0x66u, 0x67u, 0x0fu, 0x00u, 0xe8u }
    };
    static const type_unsigned_8 lengths[] = { 4u, 4u, 5u };
    const core_machine_cpu_profile legacy_profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286
    };
    type_unsigned_8 query;
    type_unsigned_8 attr;
    type_unsigned_8 profile;

    for (query = 0u; query != 2u; ++query) {
        const type_unsigned_8 *default_code = query ? default_verw : default_verr;
        const type_unsigned_8 (*attr_codes)[5] = query ? attr_verw : attr_verr;

        if (!verr_verw_s58_expect_ud(CORE_MACHINE_CPU_PROFILE_80186,
                default_code, 3u) ||
            !verr_verw_s58_expect_ud(CORE_MACHINE_CPU_PROFILE_80286,
                default_code, 3u) ||
            !verr_verw_s58_expect_ud(CORE_MACHINE_CPU_PROFILE_80386,
                default_code, 3u)) {
            return 0;
        }
        for (profile = 0u; profile != 2u; ++profile) {
            for (attr = 0u; attr != 3u; ++attr) {
                if (!verr_verw_s58_expect_ud(legacy_profiles[profile],
                        attr_codes[attr], lengths[attr])) {
                    return 0;
                }
            }
        }
    }
    for (query = 0u; query != 2u; ++query) {
        verr_verw_s58_machine state;
        t_cpu before;
        t_cpu after;
        const type_unsigned_8 (*attr_codes)[5] = query ? attr_verw : attr_verr;
        type_unsigned_8 code[6u] = { 0u };

        if (!verr_verw_s58_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386) ||
            !verr_verw_s58_boot_protected(&state, &before)) {
            if (state.machine != STD_NULL) {
                core_machine_destroy(state.machine);
            }
            return 0;
        }
        for (attr = 0u; attr != 3u; ++attr) {
            const type_unsigned_8 length = lengths[attr];
            const type_unsigned_8 prefix_count = length - 3u;

            STD_MEMCPY(code, attr_codes[attr], length);
            code[length] = 0xf4u;
            state.machine->executor_cpu.data.eax = 0xa1a10010u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
                VCPU_EFLAGS_IF;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            if (!verr_verw_s58_run_halt(&state, code, (STD_SIZE_T)length + 1u,
                    &after) || after.data.eip != (type_unsigned_32)length + 1u ||
                after.data.eax != before.data.eax ||
                !TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_ZF) ||
                (after.data.eflags & ~VCPU_EFLAGS_ZF) !=
                    (before.data.eflags & ~VCPU_EFLAGS_ZF) ||
                !verr_verw_s58_non_eax_gprs_same(&before, &after) ||
                !verr_verw_s58_sregs_same(&before, &after) || prefix_count == 0u) {
                core_machine_destroy(state.machine);
                return 0;
            }
        }
        core_machine_destroy(state.machine);
    }
    for (query = 0u; query != 2u; ++query) {
        type_unsigned_8 lock_code[] = { 0xf0u, 0x0fu, 0x00u,
            query ? VERR_VERW_S58_VERW_MODRM : VERR_VERW_S58_VERR_MODRM };

        if (!verr_verw_s58_expect_ud(CORE_MACHINE_CPU_PROFILE_80386,
                lock_code, sizeof(lock_code))) {
            return 0;
        }
    }
    return 1;
}
static C_INT verr_verw_s58_test_memory_sources(C_VOID)
{
    static const type_unsigned_8 verr_codes[][9] = {
        { 0x0fu, 0x00u, 0x26u, 0x10u, 0x00u, 0xf4u },
        { 0x0fu, 0x00u, 0x66u, 0x10u, 0xf4u },
        { 0x67u, 0x0fu, 0x00u, 0x25u, 0x10u, 0x00u, 0x00u, 0x00u, 0xf4u }
    };
    static const type_unsigned_8 verw_codes[][9] = {
        { 0x0fu, 0x00u, 0x2eu, 0x10u, 0x00u, 0xf4u },
        { 0x0fu, 0x00u, 0x6eu, 0x10u, 0xf4u },
        { 0x67u, 0x0fu, 0x00u, 0x2du, 0x10u, 0x00u, 0x00u, 0x00u, 0xf4u }
    };
    static const type_unsigned_8 lengths[] = { 6u, 5u, 9u };
    type_unsigned_8 query;
    type_unsigned_8 form;

    for (query = 0u; query != 2u; ++query) {
        const type_unsigned_8 (*codes)[9] = query ? verw_codes : verr_codes;

        for (form = 0u; form != 3u; ++form) {
            verr_verw_s58_machine state;
            t_cpu before;
            t_cpu after;
            type_unsigned_16 selector = 0x0010u;
            const type_unsigned_32 source_address = 0x3010u;
            C_INT failed = !verr_verw_s58_prepare(&state,
                CORE_MACHINE_CPU_PROFILE_80386);

            if (!failed) {
                failed = !verr_verw_s58_boot_protected(&state, &before);
            }
            if (!failed) {
                state.machine->executor_cpu.data.ebp = 0u;
                state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
                    VCPU_EFLAGS_IF;
                before = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed |= core_machine_memory_write(state.machine, source_address,
                    &selector, sizeof(selector)) != TYPE_STATUS_OK;
                failed = failed || !verr_verw_s58_run_halt(&state, codes[form],
                    lengths[form], &after);
                failed |= after.data.eip != lengths[form] ||
                    !TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_ZF) ||
                    (after.data.eflags & ~VCPU_EFLAGS_ZF) !=
                        (before.data.eflags & ~VCPU_EFLAGS_ZF) ||
                    !verr_verw_s58_non_eax_gprs_same(&before, &after) ||
                    !verr_verw_s58_sregs_same(&before, &after);
            }
            core_machine_destroy(state.machine);
            if (failed) {
                return 0;
            }
        }
    }
    return 1;
}
static C_INT verr_verw_s58_test_source_limit(C_VOID)
{
    static const type_unsigned_8 verr_codes[][5] = {
        { 0x0fu, 0x00u, 0x26u, 0x10u, 0x00u },
        { 0x0fu, 0x00u, 0x66u, 0x10u }
    };
    static const type_unsigned_8 verw_codes[][5] = {
        { 0x0fu, 0x00u, 0x2eu, 0x10u, 0x00u },
        { 0x0fu, 0x00u, 0x6eu, 0x10u }
    };
    static const type_unsigned_8 lengths[] = { 5u, 4u };
    const core_machine_run_budget budget = { 32u, 0u };
    type_unsigned_8 query;
    type_unsigned_8 form;

    for (query = 0u; query != 2u; ++query) {
        const type_unsigned_8 (*codes)[5] = query ? verw_codes : verr_codes;

        for (form = 0u; form != 2u; ++form) {
            verr_verw_s58_machine state;
            core_machine_run_result result;
            core_machine_cpu_diagnostic diagnostic;
            t_cpu before;
            t_cpu after;
            type_unsigned_16 selector = 0x0010u;
            C_INT failed = !verr_verw_s58_prepare(&state,
                CORE_MACHINE_CPU_PROFILE_80386);

            if (!failed) {
                failed = !verr_verw_s58_boot_protected(&state, &before);
            }
            if (!failed) {
                if (form == 0u) {
                    state.machine->executor_cpu.data.ds.limit = 0x000fu;
                } else {
                    state.machine->executor_cpu.data.ebp = 0u;
                    state.machine->executor_cpu.data.ss.limit = 0x000fu;
                }
                state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
                    VCPU_EFLAGS_IF;
                before = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed |= core_machine_memory_write(state.machine, 0x3010u,
                    &selector, sizeof(selector)) != TYPE_STATUS_OK;
                failed |= core_machine_memory_write(state.machine,
                    VERR_VERW_S58_CODE_ADDRESS, codes[form], lengths[form]) !=
                    TYPE_STATUS_OK;
                test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
                failed |= core_machine_run(state.machine, budget, &result) !=
                    TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT;
                after = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed |= core_machine_get_cpu_diagnostic(state.machine,
                    &diagnostic) != TYPE_STATUS_OK ||
                    !diagnostic.first_fault.valid ||
                    !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                        VCPUINS_EXCEPT_DF);                failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                    0x3010u, (type_virtual_address)&selector, sizeof(selector)) !=
                    TYPE_STATUS_OK;
                failed |= after.data.eip != 0u ||
                    after.data.eax != before.data.eax ||
                    after.data.eflags != before.data.eflags ||
                    !verr_verw_s58_non_eax_gprs_same(&before, &after) ||
                    !verr_verw_s58_sregs_same(&before, &after) ||
                    selector != 0x0010u;
            }
            core_machine_destroy(state.machine);
            if (failed) {
                return 0;
            }
        }
    }
    return 1;
}
static C_INT verr_verw_s58_test_vm86(C_VOID)
{
    static const type_unsigned_8 codes[][3] = {
        { 0x0fu, 0x00u, VERR_VERW_S58_VERR_MODRM },
        { 0x0fu, 0x00u, VERR_VERW_S58_VERW_MODRM }
    };
    const core_machine_run_budget budget = { 8u, 0u };
    type_unsigned_8 query;

    for (query = 0u; query != 2u; ++query) {
        verr_verw_s58_machine state;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        C_INT failed = !verr_verw_s58_prepare(&state,
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
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_write(state.machine, 0u, codes[query],
                sizeof(codes[query])) != TYPE_STATUS_OK;
            failed |= core_machine_run(state.machine, budget, &result) !=
                TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK || !diagnostic.first_fault.valid ||
                !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_UD);
            failed |= after.data.eip != before.data.eip ||
                after.data.eax != before.data.eax ||
                after.data.eflags != before.data.eflags ||
                !verr_verw_s58_non_eax_gprs_same(&before, &after) ||
                !verr_verw_s58_sregs_same(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed) {
            return 0;
        }
    }
    return 1;
}
static C_INT verr_verw_s58_test_pic_no_shadow(C_VOID)
{
    static const type_unsigned_8 codes[][5] = {
        { 0xfbu, 0x0fu, 0x00u, VERR_VERW_S58_VERR_MODRM, 0x90u },
        { 0xfbu, 0x0fu, 0x00u, VERR_VERW_S58_VERW_MODRM, 0x90u }
    };
    type_unsigned_8 query;

    for (query = 0u; query != 2u; ++query) {
        verr_verw_s58_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        type_unsigned_32 frame[3u] = { 0u };
        C_INT failed = !verr_verw_s58_prepare(&state,
            CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            failed = !verr_verw_s58_boot_protected(&state, &before);
        }
        if (!failed) {
            state.machine->executor_cpu.data.eax = 0xa1a10010u;
            state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_IF |
                VCPU_EFLAGS_CF;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_write(state.machine,
                VERR_VERW_S58_CODE_ADDRESS, codes[query], sizeof(codes[query])) !=
                TYPE_STATUS_OK;
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
                after.data.eip != 0x100u || after.data.eax != before.data.eax ||
                !TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_ZF) ||
                after.data.ecx != before.data.ecx ||
                after.data.edx != before.data.edx ||
                after.data.ebx != before.data.ebx ||
                after.data.ebp != before.data.ebp ||
                after.data.esi != before.data.esi ||
                after.data.edi != before.data.edi ||
                !verr_verw_s58_sregs_same(&before, &after) ||
                !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                    VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(
                        state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u));
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (type_unsigned_16)after.data.esp,
                (type_virtual_address)frame, sizeof(frame)) != TYPE_STATUS_OK ||
                frame[0u] != 4u || !TYPE_GET_BIT(frame[2u], VCPU_EFLAGS_ZF) ||
                !TYPE_GET_BIT(frame[2u], VCPU_EFLAGS_IF) ||
                !TYPE_GET_BIT(frame[2u], VCPU_EFLAGS_CF);
        }
        core_machine_destroy(state.machine);
        if (failed) {
            return 0;
        }
    }
    return 1;
}
static C_INT verr_verw_s58_test_ldt_selector(C_VOID)
{
    static const type_unsigned_8 ldt_descriptor[] = {
        0xffu, 0xffu, 0x00u, 0x70u, 0x00u, 0x92u, 0x00u, 0x00u
    };
    static const type_unsigned_8 codes[][16] = {
        { 0xb8u, 0x30u, 0x00u, 0x0fu, 0x00u, 0xd0u,
          0xb9u, 0x0cu, 0x00u, 0x0fu, 0x00u, 0xe1u, 0xf4u },
        { 0xb8u, 0x30u, 0x00u, 0x0fu, 0x00u, 0xd0u,
          0xb9u, 0x0cu, 0x00u, 0x0fu, 0x00u, 0xe9u, 0xf4u }
    };
    type_unsigned_8 query;

    for (query = 0u; query != 2u; ++query) {
        verr_verw_s58_machine state;
        t_cpu after;
        C_INT failed = !verr_verw_s58_prepare(&state,
            CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            failed |= core_machine_memory_write(state.machine, 0x0508u,
                ldt_descriptor, sizeof(ldt_descriptor)) != TYPE_STATUS_OK;
            failed = failed || !verr_verw_s58_boot_protected(&state, &after);
            failed = failed || !verr_verw_s58_run_halt(&state, codes[query], 13u,
                &after);
            failed |= after.data.eip != 13u ||
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
    if (!verr_verw_s58_test_outcomes() ||
        !verr_verw_s58_test_prefix_and_rejection() ||
        !verr_verw_s58_test_memory_sources() ||
        !verr_verw_s58_test_source_limit() ||
        !verr_verw_s58_test_vm86() ||
        !verr_verw_s58_test_pic_no_shadow() ||
        !verr_verw_s58_test_ldt_selector()) {
        STD_FPRINTF(STD_STDERR, "S58 outcomes failed\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S58:VERR-VERW:OK\n");
    return 0;
}
