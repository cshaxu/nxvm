#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct gpr_push_pop_machine
{
    core_machine *machine;
} gpr_push_pop_machine;

static C_VOID gpr_push_pop_reset(C_VOID *opaque)
{
    gpr_push_pop_machine *state = (gpr_push_pop_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider gpr_push_pop_provider = {
    gpr_push_pop_reset, STD_NULL
};

static C_INT gpr_push_pop_prepare(core_machine_cpu_profile profile,
    gpr_push_pop_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    STD_MEMSET(state, 0, sizeof(*state));
return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &gpr_push_pop_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static C_VOID gpr_push_pop_seed(gpr_push_pop_machine *state)
{
    t_cpu *cpu = &state->machine->executor_cpu;

    cpu->data.eax = 0xa1a23344u;
    cpu->data.ecx = 0xb1b25566u;
    cpu->data.edx = 0xc1c27788u;
    cpu->data.ebx = 0xd1d299aau;
    cpu->data.esp = 0x12348000u;
    cpu->data.ebp = 0xe1e2bbcdu;
    cpu->data.esi = 0xf1f2ddefu;
    cpu->data.edi = 0x1122a5a5u;
    cpu->data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF |
        VCPU_EFLAGS_IF;
}

static C_INT gpr_push_pop_sregs_same(const t_cpu *before, const t_cpu *after)
{
    return STD_MEMCMP(&before->data.es, &after->data.es,
        sizeof(before->data.es)) == 0 && STD_MEMCMP(&before->data.cs,
        &after->data.cs, sizeof(before->data.cs)) == 0 && STD_MEMCMP(
        &before->data.ss, &after->data.ss, sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after->data.ds,
        sizeof(before->data.ds)) == 0 && STD_MEMCMP(&before->data.fs,
        &after->data.fs, sizeof(before->data.fs)) == 0 && STD_MEMCMP(
        &before->data.gs, &after->data.gs, sizeof(before->data.gs)) == 0;
}

static C_INT gpr_push_pop_cpu_same(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi &&
        before->data.eip == after->data.eip &&
        before->data.eflags == after->data.eflags &&
        gpr_push_pop_sregs_same(before, after);
}

static C_INT gpr_push_pop_run(gpr_push_pop_machine *state,
    const type_unsigned_8 *code, type_unsigned_8 bytes, t_cpu *after,
    core_machine_cpu_diagnostic *diagnostic, type_status *status)
{
    core_machine_run_result result;

    if (core_machine_memory_write(state->machine, 0u, code, bytes) !=
        TYPE_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine, (core_machine_run_budget){1u, 0u},
        &result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return result.reason == CORE_MACHINE_STOP_BUDGET &&
        core_machine_get_cpu_diagnostic(state->machine, diagnostic) == TYPE_STATUS_OK;
}

static type_unsigned_32 gpr_push_pop_register(const t_cpu *cpu, type_unsigned_8 index)
{
    switch (index)
    {
    case 0:
        return cpu->data.eax;
    case 1:
        return cpu->data.ecx;
    case 2:
        return cpu->data.edx;
    case 3:
        return cpu->data.ebx;
    case 4:
        return cpu->data.esp;
    case 5:
        return cpu->data.ebp;
    case 6:
        return cpu->data.esi;
    case 7:
        return cpu->data.edi;
    default:
        return 0u;
    }
}

static C_INT gpr_push_pop_test_push_registers(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
         ++profile)
    {
        type_unsigned_8 index;

        for (index = 0u; index != 8u; ++index)
        {
            const type_unsigned_8 code[] = {(type_unsigned_8)(0x50u + index)};
            gpr_push_pop_machine state;
            core_machine_cpu_diagnostic diagnostic;
            t_cpu before;
            t_cpu after;
            type_status status;
            type_unsigned_32 image = 0u;
            type_unsigned_32 expected;
            C_INT failed = !gpr_push_pop_prepare(profiles[profile], &state);

            if (!failed)
            {
                gpr_push_pop_seed(&state);
                before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                failed |= !gpr_push_pop_run(&state, code, sizeof(code), &after,
                    &diagnostic, &status) || status != TYPE_STATUS_OK ||
                    diagnostic.first_fault.valid;
                expected = gpr_push_pop_register(&before, index) & 0xffffu;
                if (index == 4u && profiles[profile] < CORE_MACHINE_CPU_PROFILE_80286)
                    expected = 0x7ffeu;
                failed |= core_machine_memory_read_physical(
                    &state.machine->executor_memory, 0x7ffeu,
                    TYPE_REFERENCE_OF(image), 2u) != TYPE_STATUS_OK ||
                    image != expected || after.data.eip != 1u ||
                    after.data.eflags != before.data.eflags ||
                    after.data.eax != before.data.eax ||
                    after.data.ecx != before.data.ecx ||
                    after.data.edx != before.data.edx ||
                    after.data.ebx != before.data.ebx ||
                    after.data.ebp != before.data.ebp ||
                    after.data.esi != before.data.esi ||
                    after.data.edi != before.data.edi || !gpr_push_pop_sregs_same(
                    &before, &after) || after.data.esp !=
                    ((before.data.esp & 0xffff0000u) | 0x7ffeu);
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

static C_INT gpr_push_pop_test_pop_esp_address(C_VOID)
{
    static const type_unsigned_8 code[] = {0x67u, 0x8fu, 0x44u, 0x24u, 0x04u};
    gpr_push_pop_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu after;
    type_status status;
    type_unsigned_16 stack_value = 0xfaceu;
    type_unsigned_16 old_target = 0xbeefu;
    type_unsigned_16 observed = 0u;
    C_INT failed = !gpr_push_pop_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed)
    {
        gpr_push_pop_seed(&state);
        state.machine->executor_cpu.data.esp = 0x00008000u;
        failed |= core_machine_memory_write(state.machine, 0x8000u, &stack_value,
            sizeof(stack_value)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x8006u, &old_target,
            sizeof(old_target)) != TYPE_STATUS_OK || !gpr_push_pop_run(&state,
            code, sizeof(code), &after, &diagnostic, &status) ||
            status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            after.data.eip != sizeof(code) || after.data.esp != 0x00008002u ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                0x8006u, TYPE_REFERENCE_OF(observed), sizeof(observed)) !=
            TYPE_STATUS_OK || observed != stack_value;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT gpr_push_pop_test_pop_registers(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
         ++profile)
    {
        type_unsigned_8 index;

        for (index = 0u; index != 8u; ++index)
        {
            const type_unsigned_8 code[] = {(type_unsigned_8)(0x58u + index)};
            gpr_push_pop_machine state;
            core_machine_cpu_diagnostic diagnostic;
            t_cpu before;
            t_cpu after;
            type_status status;
            type_unsigned_16 value = (type_unsigned_16)(0x4100u + index);
            type_unsigned_32 expected;
            C_INT failed = !gpr_push_pop_prepare(profiles[profile], &state);

            if (!failed)
            {
                gpr_push_pop_seed(&state);
                failed |= core_machine_memory_write(state.machine, 0x8000u,
                    &value, sizeof(value)) != TYPE_STATUS_OK;
                before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
                failed |= !gpr_push_pop_run(&state, code, sizeof(code), &after,
                    &diagnostic, &status) || status != TYPE_STATUS_OK ||
                    diagnostic.first_fault.valid || after.data.eip != 1u ||
                    after.data.eflags != before.data.eflags ||
                    !gpr_push_pop_sregs_same(&before, &after);
                expected = (gpr_push_pop_register(&before, index) & 0xffff0000u) |
                    value;
                if (index == 4u)
                    expected = (before.data.esp & 0xffff0000u) | value;
                failed |= gpr_push_pop_register(&after, index) != expected ||
                    after.data.eax != (index == 0u ? expected : before.data.eax) ||
                    after.data.ecx != (index == 1u ? expected : before.data.ecx) ||
                    after.data.edx != (index == 2u ? expected : before.data.edx) ||
                    after.data.ebx != (index == 3u ? expected : before.data.ebx) ||
                    after.data.ebp != (index == 5u ? expected : before.data.ebp) ||
                    after.data.esi != (index == 6u ? expected : before.data.esi) ||
                    after.data.edi != (index == 7u ? expected : before.data.edi) ||
                    after.data.esp != (index == 4u ? expected :
                    ((before.data.esp & 0xffff0000u) | 0x8002u));
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

static C_INT gpr_push_pop_test_rm_forms(C_VOID)
{
    static const type_unsigned_8 push_reg[] = {0xffu, 0xf0u};
    static const type_unsigned_8 pop_reg[] = {0x8fu, 0xc1u};
    static const type_unsigned_8 push_ds[] = {0xffu, 0x36u, 0x20u, 0x00u};
    static const type_unsigned_8 push_ss[] = {0xffu, 0x76u, 0x00u};
    static const type_unsigned_8 pop_ds[] = {0x8fu, 0x06u, 0x20u, 0x00u};
    static const type_unsigned_8 pop_ss[] = {0x8fu, 0x46u, 0x00u};
    static const type_unsigned_8 push_67[] = {0x67u, 0xffu, 0x35u, 0x20u, 0x00u,
        0x00u, 0x00u};
    static const type_unsigned_8 pop_67[] = {0x67u, 0x8fu, 0x05u, 0x20u, 0x00u,
        0x00u, 0x00u};
    const type_unsigned_8 *codes[] = {push_reg, pop_reg, push_ds, push_ss, pop_ds,
        pop_ss, push_67, pop_67};
    const type_unsigned_8 bytes[] = {2u, 2u, 4u, 3u, 4u, 3u, 7u, 7u};
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;
    type_unsigned_8 form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form)
    {
        if (form >= 6u && profiles[profile] != CORE_MACHINE_CPU_PROFILE_80386) continue;
        gpr_push_pop_machine state;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        type_status status;
        type_unsigned_32 source = 0xface7788u;
        type_unsigned_32 image = 0xface7788u;
        type_unsigned_32 observed = 0u;
        type_unsigned_32 address = 0x20u;
        C_INT push = form == 0u || form == 2u || form == 3u || form == 6u;
        C_INT failed = !gpr_push_pop_prepare(profiles[profile], &state);

        if (!failed)
        {
            gpr_push_pop_seed(&state);
            state.machine->executor_cpu.data.bp = 0x20u;
            if (form == 3u || form == 5u)
                address = 0x20u;
            if (form == 6u || form == 7u)
                state.machine->executor_cpu.data.esp = 0x00008000u;
            if (push && form != 0u)
                failed |= core_machine_memory_write(state.machine, address, &source,
                    2u) != TYPE_STATUS_OK;
            if (!push)
                failed |= core_machine_memory_write(state.machine, 0x8000u,
                    &image, 2u) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !gpr_push_pop_run(&state, codes[form], bytes[form], &after,
                &diagnostic, &status) || status != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid || after.data.eip != bytes[form] ||
                after.data.eflags != before.data.eflags ||
                !gpr_push_pop_sregs_same(&before, &after);
            if (push)
            {
                failed |= after.data.esp != ((before.data.esp & 0xffff0000u) |
                    ((before.data.sp - 2u) & 0xffffu));
                failed |= core_machine_memory_read_physical(
                    &state.machine->executor_memory, 0x7ffeu,
                    TYPE_REFERENCE_OF(observed), 2u) != TYPE_STATUS_OK ||
                    observed != (form == 0u ? (before.data.eax & 0xffffu) :
                    (source & 0xffffu));
            }
            else
            {
                failed |= after.data.esp != ((before.data.esp & 0xffff0000u) |
                    0x8002u);
                if (form != 1u)
                    failed |= core_machine_memory_read_physical(
                        &state.machine->executor_memory, address,
                        TYPE_REFERENCE_OF(observed), 2u) != TYPE_STATUS_OK ||
                        observed != (image & 0xffffu);
            }
            failed |= after.data.eax != before.data.eax || after.data.edx !=
                before.data.edx || after.data.ebx != before.data.ebx ||
                after.data.ebp != before.data.ebp || after.data.esi !=
                before.data.esi || after.data.edi != before.data.edi ||
                after.data.ecx != (form == 1u ? ((before.data.ecx & 0xffff0000u) |
                (image & 0xffffu)) : before.data.ecx);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT gpr_push_pop_expect_ud(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, type_unsigned_8 bytes)
{
    gpr_push_pop_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    type_status status;
    type_unsigned_32 source = 0xface7788u;
    type_unsigned_32 image = 0u;
    C_INT failed = !gpr_push_pop_prepare(profile, &state);

    if (!failed)
    {
        gpr_push_pop_seed(&state);
        failed |= core_machine_memory_write(state.machine, 0x20u, &source,
            sizeof(source)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x7ffcu, &source, sizeof(source)) != TYPE_STATUS_OK;
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_write(state.machine, 0u, code, bytes) !=
            TYPE_STATUS_OK;
        status = core_machine_run(state.machine, (core_machine_run_budget){1u, 0u},
            &result);
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK || status != TYPE_STATUS_FAULT || result.reason !=
            CORE_MACHINE_STOP_FAULT || !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            !gpr_push_pop_cpu_same(&before, &after) || core_machine_memory_read_physical(
            &state.machine->executor_memory, 0x20u, TYPE_REFERENCE_OF(image),
            sizeof(image)) != TYPE_STATUS_OK || image != source ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
            0x7ffcu, TYPE_REFERENCE_OF(image), sizeof(image)) != TYPE_STATUS_OK ||
            image != source;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT gpr_push_pop_test_rejections(C_VOID)
{
    static const type_unsigned_8 attrs[][3] = {{0x66u, 0x50u}, {0x67u, 0x50u},
        {0x66u, 0x67u, 0x50u}};
    static const type_unsigned_8 locks[][4] = {{0xf0u, 0x50u}, {0xf0u, 0x58u},
        {0xf0u, 0xffu, 0xf0u}, {0xf0u, 0xffu, 0x36u, 0x20u},
        {0xf0u, 0x8fu, 0xc1u}, {0xf0u, 0x8fu, 0x06u, 0x20u}};
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    type_unsigned_8 profile;
    type_unsigned_8 form;

    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]); ++profile)
    {
        for (form = 0u; form != 3u; ++form)
        {
            type_unsigned_8 bytes = form == 2u ? 3u : 2u;

            if (!gpr_push_pop_expect_ud(legacy[profile], attrs[form], bytes))
                return 0;
        }
    }
    for (form = 0u; form != sizeof(locks) / sizeof(locks[0]); ++form)
    {
        type_unsigned_8 bytes = form == 0u || form == 1u ? 2u :
            (form == 3u || form == 5u ? 4u : 3u);

        if (!gpr_push_pop_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, locks[form],
            bytes))
            return 0;
    }
    for (form = 1u; form != 8u; ++form)
    {
        type_unsigned_8 code[] = {0x8fu, (type_unsigned_8)(0xc0u | (form << 3))};

        if (!gpr_push_pop_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, code,
            sizeof(code)))
            return 0;
    }
    return 1;
}

static C_INT gpr_push_pop_boot_protected(gpr_push_pop_machine *state)
{
    static const type_unsigned_8 pointer[] = {0x1fu, 0u, 0u, 0x03u, 0u, 0u};
    static const type_unsigned_8 gdt[] = {
        0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
        0xffu, 0xffu, 0u, 0x20u, 0u, 0x9au, 0u, 0u,
        0xffu, 0xffu, 0u, 0x30u, 0u, 0x92u, 0u, 0u,
        0xffu, 0xffu, 0u, 0x40u, 0u, 0x92u, 0u, 0u
    };
    static const type_unsigned_8 bootstrap[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u, 0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u, 0xb8u, 0x10u, 0x00u, 0x8eu, 0xd8u,
        0x8eu, 0xc0u, 0xb8u, 0x18u, 0x00u, 0x8eu, 0xd0u, 0xbcu,
        0x00u, 0x80u, 0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    static const type_unsigned_8 halt = 0xf4u;
    core_machine_run_result result;

    return core_machine_memory_write(state->machine, 0x0100u, pointer,
        sizeof(pointer)) == TYPE_STATUS_OK && core_machine_memory_write(
        state->machine, 0x0300u, gdt, sizeof(gdt)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, bootstrap,
        sizeof(bootstrap)) == TYPE_STATUS_OK && core_machine_memory_write(
        state->machine, 0x2000u, &halt, sizeof(halt)) == TYPE_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){96u, 0u},
        &result) == TYPE_STATUS_OK && result.reason ==
        CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT gpr_push_pop_protected_fault(const type_unsigned_8 *code, type_unsigned_8 bytes,
    type_unsigned_8 limit_segment, type_unsigned_32 limit, C_INT expdown)
{
    gpr_push_pop_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    type_status fault_status;
    type_unsigned_32 sentinel = 0xdeadbeefu;
    type_unsigned_32 observed = 0u;
    C_INT failed = !gpr_push_pop_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);

    if (!failed)
        failed |= !gpr_push_pop_boot_protected(&state);
    if (!failed)
    {
        gpr_push_pop_seed(&state);
        state.machine->executor_cpu.data.esp = 0x12348000u;
        if (limit_segment == 0u)
        {
            state.machine->executor_cpu.data.ss.limit = limit;
            state.machine->executor_cpu.data.ss.seg.data.expdown = expdown;
        }
        else
            state.machine->executor_cpu.data.ds.limit = limit;
        failed |= core_machine_memory_write(state.machine, 0x3010u, &sentinel,
            sizeof(sentinel)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x4010u, &sentinel, sizeof(sentinel)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x47ffeu, &sentinel,
            sizeof(sentinel)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x2000u, code, bytes) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        fault_status = core_machine_run(state.machine,
            (core_machine_run_budget){1u, 0u}, &result);
        failed |= fault_status != TYPE_STATUS_FAULT || result.reason !=
            CORE_MACHINE_STOP_FAULT || core_machine_get_cpu_diagnostic(
            state.machine, &diagnostic) != TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
            after.data.eip != 0u || after.data.eax != before.data.eax ||
            after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx || after.data.esp != before.data.esp ||
            after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi || after.data.eflags !=
            before.data.eflags || !gpr_push_pop_sregs_same(&before, &after) ||
            core_machine_memory_read_physical(
            &state.machine->executor_memory, 0x3010u, TYPE_REFERENCE_OF(observed),
            sizeof(observed)) != TYPE_STATUS_OK || observed != sentinel ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
            0x4010u, TYPE_REFERENCE_OF(observed), sizeof(observed)) !=
            TYPE_STATUS_OK || observed != sentinel ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
            0x47ffeu, TYPE_REFERENCE_OF(observed), sizeof(observed)) !=
            TYPE_STATUS_OK || observed != sentinel;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT gpr_push_pop_test_protected_faults(C_VOID)
{
    static const type_unsigned_8 push[] = {0x50u};
    static const type_unsigned_8 pop[] = {0x59u};
    static const type_unsigned_8 push_source[] = {0xffu, 0x36u, 0x10u, 0x00u};
    static const type_unsigned_8 pop_dest[] = {0x8fu, 0x06u, 0x10u, 0x00u};

    if (!gpr_push_pop_protected_fault(push, sizeof(push), 0u, 0xffffu,
        TYPE_TRUE))
    {
        STD_PRINTF("protected push\n");
        return 0;
    }
    if (!gpr_push_pop_protected_fault(pop, sizeof(pop), 0u, 0x7fffu,
        TYPE_FALSE))
    {
        STD_PRINTF("protected pop\n");
        return 0;
    }
    if (!gpr_push_pop_protected_fault(push_source, sizeof(push_source), 1u,
        0x0fu, TYPE_FALSE))
    {
        STD_PRINTF("protected push-source\n");
        return 0;
    }
    if (!gpr_push_pop_protected_fault(pop_dest, sizeof(pop_dest), 1u, 0x0fu,
        TYPE_FALSE))
    {
        STD_PRINTF("protected pop-dest\n");
        return 0;
    }
    return 1;
}

static C_INT gpr_push_pop_test_386_attributes(C_VOID)
{
    static const type_unsigned_8 push32[] = {0x66u, 0x50u};
    static const type_unsigned_8 pop32[] = {0x66u, 0x59u};
    static const type_unsigned_8 push67[] = {0x67u, 0xffu, 0x35u, 0x20u, 0x00u,
        0x00u, 0x00u};
    static const type_unsigned_8 pop66_67[] = {0x66u, 0x67u, 0x8fu, 0x05u, 0x20u,
        0x00u, 0x00u, 0x00u};
    const type_unsigned_8 *codes[] = {push32, pop32, push67, pop66_67};
    const type_unsigned_8 bytes[] = {2u, 2u, 7u, 8u};
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form)
    {
        gpr_push_pop_machine state;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        type_status status;
        type_unsigned_32 value = 0xface7788u;
        type_unsigned_32 observed = 0u;
        C_INT failed = !gpr_push_pop_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed)
        {
            gpr_push_pop_seed(&state);
            if (form == 2u)
                failed |= core_machine_memory_write(state.machine, 0x20u, &value,
                    sizeof(value)) != TYPE_STATUS_OK;
            if (form == 1u || form == 3u)
                failed |= core_machine_memory_write(state.machine, 0x8000u,
                    &value, sizeof(value)) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !gpr_push_pop_run(&state, codes[form], bytes[form], &after,
                &diagnostic, &status) || status != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid || after.data.eip != bytes[form] ||
                after.data.eflags != before.data.eflags ||
                !gpr_push_pop_sregs_same(&before, &after);
            if (form == 0u || form == 2u)
            {
                type_unsigned_32 expected = form == 0u ? before.data.eax : value;
                type_unsigned_8 width = form == 0u ? 4u : 2u;
                type_unsigned_32 stack = 0x8000u - width;

                failed |= after.data.esp != ((before.data.esp & 0xffff0000u) |
                    stack) ||
                    core_machine_memory_read_physical(&state.machine->executor_memory,
                    stack, TYPE_REFERENCE_OF(observed), width) !=
                    TYPE_STATUS_OK || observed != (width == 2u ?
                    (expected & 0xffffu) : expected);
            }
            else if (form == 1u)
                failed |= after.data.eax != before.data.eax ||
                    after.data.ecx != value || after.data.esp != 0x12348004u;
            else
            {
                failed |= after.data.esp != 0x12348004u ||
                    core_machine_memory_read_physical(&state.machine->executor_memory,
                    0x20u, TYPE_REFERENCE_OF(observed), sizeof(observed)) !=
                    TYPE_STATUS_OK || observed != value;
            }
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT gpr_push_pop_test_irq_no_shadow(C_VOID)
{
    static const type_unsigned_8 push_reg[] = {0x50u, 0x90u};
    static const type_unsigned_8 pop_reg[] = {0x59u, 0x90u};
    static const type_unsigned_8 push_rm[] = {0xffu, 0xf0u, 0x90u};
    static const type_unsigned_8 pop_rm[] = {0x8fu, 0x06u, 0x20u, 0x00u, 0x90u};
    static const type_unsigned_8 halt = 0xf4u;
    const type_unsigned_8 *codes[] = {push_reg, pop_reg, push_rm, pop_rm};
    const type_unsigned_8 lengths[] = {1u, 1u, 2u, 4u};
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form)
    {
        gpr_push_pop_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        type_unsigned_16 vector_offset = 0x100u;
        type_unsigned_16 vector_segment = 0u;
        type_unsigned_16 frame_ip = 0u;
        type_unsigned_16 image = 0xfaceu;
        type_unsigned_16 source_word = 0x7788u;
        type_unsigned_16 observed = 0u;
        C_INT failed = !gpr_push_pop_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed)
        {
            gpr_push_pop_seed(&state);
            if (form == 1u || form == 3u)
                failed |= core_machine_memory_write(state.machine, 0x8000u,
                    &image, sizeof(image)) != TYPE_STATUS_OK;
            if (form == 2u)
                state.machine->executor_cpu.data.eax = 0xa1a27788u;
            failed |= core_machine_memory_write(state.machine, 0x20u,
                &source_word, sizeof(source_word)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0u, codes[form],
                lengths[form] + 1u) != TYPE_STATUS_OK || core_machine_memory_write(
                state.machine, 0x80u, &vector_offset, sizeof(vector_offset)) !=
                TYPE_STATUS_OK || core_machine_memory_write(state.machine, 0x82u,
                &vector_segment, sizeof(vector_segment)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x100u, &halt,
                sizeof(halt)) != TYPE_STATUS_OK;
        }
        if (!failed)
        {
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
            STD_MEMSET(&source, 0, sizeof(source));
            state.machine->shared_pic_master.data.icw2 = 0x20u;
            core_machine_pic_irq_source_bind(&source,
                &state.machine->shared_pic_master, &state.machine->shared_pic_slave,
                0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){2u, 0u}, &result) != TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (type_unsigned_16)after.data.esp,
                TYPE_REFERENCE_OF(frame_ip), sizeof(frame_ip)) != TYPE_STATUS_OK ||
                after.data.eip != 0x101u || frame_ip != lengths[form] ||
                !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(
                state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u)) ||
                after.data.eflags != (before.data.eflags & ~VCPU_EFLAGS_IF);
            if (form == 0u || form == 2u)
            {
                type_unsigned_16 expected = form == 0u ?
                    (type_unsigned_16)before.data.eax : source_word;

                failed |= after.data.esp != 0x12347ff8u ||
                    core_machine_memory_read_physical(&state.machine->executor_memory,
                    0x7ffeu, TYPE_REFERENCE_OF(observed), sizeof(observed)) !=
                    TYPE_STATUS_OK || observed != expected;
            }
            else if (form == 1u)
                failed |= after.data.ecx != 0xb1b2faceu ||
                    after.data.esp != 0x12347ffcu;
            else
            {
                failed |= after.data.esp != 0x12347ffcu ||
                    core_machine_memory_read_physical(&state.machine->executor_memory,
                    0x20u, TYPE_REFERENCE_OF(observed), sizeof(observed)) !=
                    TYPE_STATUS_OK || observed != image;
            }
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

C_INT main(C_VOID)
{
    if (!gpr_push_pop_test_push_registers())
    {
        STD_PRINTF("GPR-PUSH-POP stage=push-registers\n");
        return 1;
    }
    if (!gpr_push_pop_test_pop_esp_address())
    {
        STD_PRINTF("GPR-PUSH-POP stage=pop-esp-address\n");
        return 1;
    }
    if (!gpr_push_pop_test_pop_registers())
    {
        STD_PRINTF("GPR-PUSH-POP stage=pop-registers\n");
        return 1;
    }
    if (!gpr_push_pop_test_rm_forms())
    {
        STD_PRINTF("GPR-PUSH-POP stage=rm\n");
        return 1;
    }
    if (!gpr_push_pop_test_rejections())
    {
        STD_PRINTF("GPR-PUSH-POP stage=rejections\n");
        return 1;
    }
    if (!gpr_push_pop_test_386_attributes())
    {
        STD_PRINTF("GPR-PUSH-POP stage=attributes\n");
        return 1;
    }
    if (!gpr_push_pop_test_protected_faults())
    {
        STD_PRINTF("GPR-PUSH-POP stage=protected\n");
        return 1;
    }
    if (!gpr_push_pop_test_irq_no_shadow())
    {
        STD_PRINTF("GPR-PUSH-POP stage=irq\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S44:GPR-PUSH-POP:OK\n");
    STD_PRINTF("M5:T401:S40:GPR-PUSH-POP-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S10:GROUP5-PUSH-RM-PROFILES:OK\n");
    return 0;
}
