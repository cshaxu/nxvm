#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/pic.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct pusha_popa_machine
{
    core_machine *machine;
} pusha_popa_machine;

static C_VOID pusha_popa_reset(C_VOID *opaque)
{
    pusha_popa_machine *state = (pusha_popa_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider pusha_popa_provider = {
    pusha_popa_reset, STD_NULL
};

static C_INT pusha_popa_prepare(core_machine_cpu_profile profile,
    pusha_popa_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    STD_MEMSET(state, 0, sizeof(*state));
return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &pusha_popa_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static C_VOID pusha_popa_seed(pusha_popa_machine *state)
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

static C_INT pusha_popa_run(pusha_popa_machine *state, const type_unsigned_8 *code,
    type_unsigned_8 bytes, core_machine_run_budget budget, t_cpu *after,
    core_machine_cpu_diagnostic *diagnostic, type_status *status,
    core_machine_run_result *result)
{
    if (core_machine_memory_write(state->machine, 0u, code, bytes) !=
        TYPE_STATUS_OK)
        return 0;

    *status = core_machine_run(state->machine, budget, result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        TYPE_STATUS_OK;
}

static C_INT pusha_popa_sregs_same(const t_cpu *before, const t_cpu *after)
{
    return STD_MEMCMP(&before->data.es, &after->data.es,
        sizeof(before->data.es)) == 0 && STD_MEMCMP(&before->data.cs,
        &after->data.cs, sizeof(before->data.cs)) == 0 && STD_MEMCMP(
        &before->data.ss, &after->data.ss, sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after->data.ds, sizeof(before->data.ds)) ==
        0 && STD_MEMCMP(&before->data.fs, &after->data.fs,
        sizeof(before->data.fs)) == 0 && STD_MEMCMP(&before->data.gs,
        &after->data.gs, sizeof(before->data.gs)) == 0;
}

static C_INT pusha_popa_cpu_same(const t_cpu *before, const t_cpu *after)
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
        pusha_popa_sregs_same(before, after);
}

static C_INT pusha_popa_read_image(pusha_popa_machine *state, type_unsigned_32 address,
    type_unsigned_8 width, type_unsigned_32 *value)
{
    *value = 0u;
    return core_machine_memory_read_physical(&state->machine->executor_memory,
        address, TYPE_REFERENCE_OF(*value), width) == TYPE_STATUS_OK;
}

static C_INT pusha_popa_expect_push_image(pusha_popa_machine *state,
    const t_cpu *before, type_unsigned_32 stack, type_unsigned_8 width)
{
    const type_unsigned_32 expected[] = {
        before->data.edi, before->data.esi, before->data.ebp,
        width == 2u ? before->data.sp : before->data.esp,
        before->data.ebx, before->data.edx, before->data.ecx, before->data.eax
    };
    type_unsigned_8 slot;

    for (slot = 0u; slot != sizeof(expected) / sizeof(expected[0]); ++slot)
    {
        type_unsigned_32 image;

        if (!pusha_popa_read_image(state, stack + slot * width, width, &image) ||
            image != (width == 2u ? (expected[slot] & 0xffffu) : expected[slot]))
            return 0;
    }
    return 1;
}

static C_INT pusha_popa_test_pusha_success(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, type_unsigned_8 bytes, type_unsigned_8 width)
{
    pusha_popa_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    type_unsigned_32 stack = 0x8000u - 8u * width;
    type_unsigned_32 sentinel = 0xdeadbeefu;
    type_unsigned_8 slot;
    C_INT failed = !pusha_popa_prepare(profile, &state);

    if (!failed)
    {
        pusha_popa_seed(&state);
        for (slot = 0u; slot != 8u; ++slot)
            failed |= core_machine_memory_write(state.machine, stack + slot * width,
                &sentinel, width) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !pusha_popa_run(&state, code, bytes,
            (core_machine_run_budget){1u, 0u}, &after, &diagnostic, &status,
            &result) || status != TYPE_STATUS_OK || result.reason !=
            CORE_MACHINE_STOP_BUDGET || diagnostic.first_fault.valid ||
            after.data.eip != bytes || after.data.eax != before.data.eax ||
            after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx || after.data.ebp != before.data.ebp ||
            after.data.esi != before.data.esi || after.data.edi != before.data.edi ||
            after.data.eflags != before.data.eflags || after.data.esp !=
            ((before.data.esp & 0xffff0000u) | (type_unsigned_16)(before.data.sp -
            8u * width)) || !pusha_popa_sregs_same(&before, &after) ||
            !pusha_popa_expect_push_image(&state, &before, stack, width);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT pusha_popa_test_popa_success(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, type_unsigned_8 bytes, type_unsigned_8 width)
{
    static const type_unsigned_32 image[] = {0x0102a5a5u, 0x0304ddefu, 0x0506bbcdu,
        0xdeadbeefu, 0x070899aau, 0x090a7788u, 0x0b0c5566u, 0x0d0e3344u};
    pusha_popa_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    type_unsigned_8 slot;
    C_INT failed = !pusha_popa_prepare(profile, &state);

    if (!failed)
    {
        pusha_popa_seed(&state);
        for (slot = 0u; slot != 8u; ++slot)
            failed |= core_machine_memory_write(state.machine, 0x8000u + slot * width,
                &image[slot], width) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !pusha_popa_run(&state, code, bytes,
            (core_machine_run_budget){1u, 0u}, &after, &diagnostic, &status,
            &result) || status != TYPE_STATUS_OK || result.reason !=
            CORE_MACHINE_STOP_BUDGET || diagnostic.first_fault.valid ||
            after.data.eip != bytes || after.data.eflags != before.data.eflags ||
            after.data.edi != (width == 2u ? ((before.data.edi & 0xffff0000u) |
            (image[0] & 0xffffu)) : image[0]) || after.data.esi != (width == 2u ?
            ((before.data.esi & 0xffff0000u) | (image[1] & 0xffffu)) : image[1]) ||
            after.data.ebp != (width == 2u ? ((before.data.ebp & 0xffff0000u) |
            (image[2] & 0xffffu)) : image[2]) || after.data.ebx != (width == 2u ?
            ((before.data.ebx & 0xffff0000u) | (image[4] & 0xffffu)) : image[4]) ||
            after.data.edx != (width == 2u ? ((before.data.edx & 0xffff0000u) |
            (image[5] & 0xffffu)) : image[5]) || after.data.ecx != (width == 2u ?
            ((before.data.ecx & 0xffff0000u) | (image[6] & 0xffffu)) : image[6]) ||
            after.data.eax != (width == 2u ? ((before.data.eax & 0xffff0000u) |
            (image[7] & 0xffffu)) : image[7]) || after.data.esp !=
            ((before.data.esp & 0xffff0000u) | (type_unsigned_16)(before.data.sp +
            8u * width)) || !pusha_popa_sregs_same(&before, &after);
        for (slot = 0u; !failed && slot != 8u; ++slot)
        {
            type_unsigned_32 observed;

            failed |= !pusha_popa_read_image(&state, 0x8000u + slot * width,
                width, &observed) || observed != (width == 2u ?
                (image[slot] & 0xffffu) : image[slot]);
        }
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT pusha_popa_test_defaults(C_VOID)
{
    static const core_machine_cpu_profile supported[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386};
    static const type_unsigned_8 pusha[] = {0x60u};
    static const type_unsigned_8 popa[] = {0x61u};
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(supported) / sizeof(supported[0]);
         ++profile)
    {
        if (!pusha_popa_test_pusha_success(supported[profile], pusha,
            sizeof(pusha), 2u) || !pusha_popa_test_popa_success(supported[profile],
            popa, sizeof(popa), 2u))
            return 0;
    }
    return 1;
}

static C_INT pusha_popa_test_attributes(C_VOID)
{
    static const type_unsigned_8 pusha32[] = {0x66u, 0x60u};
    static const type_unsigned_8 popa32[] = {0x66u, 0x61u};
    static const type_unsigned_8 pusha67[] = {0x67u, 0x60u};
    static const type_unsigned_8 popa67[] = {0x67u, 0x61u};
    static const type_unsigned_8 pusha32_67[] = {0x66u, 0x67u, 0x60u};
    static const type_unsigned_8 popa32_67[] = {0x66u, 0x67u, 0x61u};

    return pusha_popa_test_pusha_success(CORE_MACHINE_CPU_PROFILE_80386,
        pusha32, sizeof(pusha32), 4u) && pusha_popa_test_popa_success(
        CORE_MACHINE_CPU_PROFILE_80386, popa32, sizeof(popa32), 4u) &&
        pusha_popa_test_pusha_success(CORE_MACHINE_CPU_PROFILE_80386, pusha67,
        sizeof(pusha67), 2u) && pusha_popa_test_popa_success(
        CORE_MACHINE_CPU_PROFILE_80386, popa67, sizeof(popa67), 2u) &&
        pusha_popa_test_pusha_success(CORE_MACHINE_CPU_PROFILE_80386,
        pusha32_67, sizeof(pusha32_67), 4u) && pusha_popa_test_popa_success(
        CORE_MACHINE_CPU_PROFILE_80386, popa32_67, sizeof(popa32_67), 4u);
}

static C_INT pusha_popa_test_reject_case(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, type_unsigned_8 bytes)
{
    pusha_popa_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    static const type_unsigned_8 image[32] = {
        0xa5u, 0xb6u, 0xc7u, 0xd8u, 0xe9u, 0xfau, 0x0bu, 0x1cu,
        0x2du, 0x3eu, 0x4fu, 0x50u, 0x61u, 0x72u, 0x83u, 0x94u,
        0x95u, 0x86u, 0x77u, 0x68u, 0x59u, 0x4au, 0x3bu, 0x2cu,
        0x1du, 0x0eu, 0xffu, 0xeeu, 0xddu, 0xccu, 0xbbu, 0xaau
    };
    type_unsigned_8 observed[sizeof(image)];
    C_INT failed = !pusha_popa_prepare(profile, &state);

    if (!failed)
    {
        pusha_popa_seed(&state);
        failed |= core_machine_memory_write(state.machine, 0x7fe0u, image,
            sizeof(image)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x8000u, image, sizeof(image)) != TYPE_STATUS_OK;
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !pusha_popa_run(&state, code, bytes,
            (core_machine_run_budget){1u, 0u}, &after, &diagnostic, &status,
            &result) || status != TYPE_STATUS_FAULT || result.reason !=
            CORE_MACHINE_STOP_FAULT || !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
            VCPUINS_EXCEPT_UD) || !pusha_popa_cpu_same(&before, &after) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
            0x7fe0u, TYPE_REFERENCE_OF(observed), sizeof(observed)) != TYPE_STATUS_OK ||
            STD_MEMCMP(observed, image, sizeof(image)) != 0 ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
            0x8000u, TYPE_REFERENCE_OF(observed), sizeof(observed)) != TYPE_STATUS_OK ||
            STD_MEMCMP(observed, image, sizeof(image)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT pusha_popa_test_rejections(C_VOID)
{
    static const type_unsigned_8 default_codes[][1] = {{0x60u}, {0x61u}};
    static const type_unsigned_8 attribute_codes[][3] = {
        {0x66u, 0x60u, 0u}, {0x66u, 0x61u, 0u},
        {0x67u, 0x60u, 0u}, {0x67u, 0x61u, 0u},
        {0x66u, 0x67u, 0x60u}, {0x66u, 0x67u, 0x61u}
    };
    static const type_unsigned_8 attribute_bytes[] = {2u, 2u, 2u, 2u, 3u, 3u};
    static const type_unsigned_8 lock_codes[][4] = {
        {0xf0u, 0x60u, 0u, 0u}, {0xf0u, 0x61u, 0u, 0u},
        {0xf0u, 0x66u, 0x60u, 0u}, {0xf0u, 0x66u, 0x61u, 0u},
        {0xf0u, 0x67u, 0x60u, 0u}, {0xf0u, 0x67u, 0x61u, 0u},
        {0xf0u, 0x66u, 0x67u, 0x60u}, {0xf0u, 0x66u, 0x67u, 0x61u}
    };
    static const type_unsigned_8 lock_bytes[] = {2u, 2u, 3u, 3u, 3u, 3u, 4u, 4u};
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286};
    type_unsigned_8 profile;
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(default_codes) / sizeof(default_codes[0]);
         ++form)
    {
        if (!pusha_popa_test_reject_case(CORE_MACHINE_CPU_PROFILE_8086,
            default_codes[form], sizeof(default_codes[form])))
            return 0;
    }
    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]); ++profile)
    {
        for (form = 0u; form != sizeof(attribute_codes) /
             sizeof(attribute_codes[0]); ++form)
        {
            if (!pusha_popa_test_reject_case(legacy[profile],
                attribute_codes[form], attribute_bytes[form]))
                return 0;
        }
    }
    for (form = 0u; form != sizeof(lock_codes) / sizeof(lock_codes[0]); ++form)
    {
        if (!pusha_popa_test_reject_case(CORE_MACHINE_CPU_PROFILE_80386,
            lock_codes[form], lock_bytes[form]))
            return 0;
    }
    return 1;
}

static C_INT pusha_popa_boot_protected(pusha_popa_machine *state)
{
    static const type_unsigned_8 pointer[] = {0x1fu, 0u, 0u, 0x03u, 0u, 0u};
    static const type_unsigned_8 gdt[] = {
        0u,0u,0u,0u,0u,0u,0u,0u,
        0xffu,0xffu,0u,0x20u,0u,0x9au,0u,0u,
        0xffu,0xffu,0u,0x30u,0u,0x92u,0u,0u,
        0xffu,0xffu,0u,0x40u,0u,0x92u,0u,0u
    };
    static const type_unsigned_8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,0xb8u,0x18u,0x00u,0x8eu,
        0xd0u,0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
    };
    static const type_unsigned_8 halt[] = {0xf4u};
    core_machine_run_result result;

    return core_machine_memory_write(state->machine, 0x0100u, pointer,
        sizeof(pointer)) == TYPE_STATUS_OK && core_machine_memory_write(
        state->machine, 0x0300u, gdt, sizeof(gdt)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, bootstrap,
        sizeof(bootstrap)) == TYPE_STATUS_OK && core_machine_memory_write(
        state->machine, 0x2000u, halt, sizeof(halt)) == TYPE_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){96u, 0u},
        &result) == TYPE_STATUS_OK && result.reason ==
        CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT pusha_popa_test_protected_pusha_limit(C_VOID)
{
    static const type_unsigned_8 code[] = {0x60u};
    pusha_popa_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_unsigned_16 expected[] = {0xa5a5u, 0x99aau, 0x7788u, 0x5566u, 0x3344u};
    type_unsigned_8 slot;
    C_INT failed = !pusha_popa_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed)
        failed |= !pusha_popa_boot_protected(&state);
    if (!failed)
    {
        pusha_popa_seed(&state);
        state.machine->executor_cpu.data.ss.limit = 0x18u;
        state.machine->executor_cpu.data.ss.seg.data.expdown = TYPE_TRUE;
        state.machine->executor_cpu.data.esp = 0x12340022u;
        failed |= core_machine_memory_write(state.machine, 0x4018u, expected,
            sizeof(expected)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x2000u, code, sizeof(code)) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        failed |= core_machine_run(state.machine, (core_machine_run_budget){1u, 0u},
            &result) != TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
            after.data.eip != 0u || after.data.eax != before.data.eax ||
            after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx || after.data.ebp != before.data.ebp ||
            after.data.esi != before.data.esi || after.data.edi != before.data.edi ||
            after.data.eflags != before.data.eflags || after.data.esp != before.data.esp ||
            !pusha_popa_sregs_same(&before, &after);
        for (slot = 0u; !failed && slot != sizeof(expected) / sizeof(expected[0]); ++slot)
        {
            type_unsigned_32 value;

            failed |= !pusha_popa_read_image(&state, 0x4018u + slot * 2u, 2u,
                &value) || value != expected[slot];
        }
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT pusha_popa_test_protected_popa_limit(C_VOID)
{
    static const type_unsigned_8 code[] = {0x61u};
    static const type_unsigned_16 image[] = {0x1111u, 0x2222u, 0x3333u, 0x4444u,
        0x5555u, 0x6666u, 0x7777u, 0x8888u};
    pusha_popa_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_unsigned_8 slot;
    C_INT failed = !pusha_popa_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed)
        failed |= !pusha_popa_boot_protected(&state);
    if (!failed)
    {
        pusha_popa_seed(&state);
        state.machine->executor_cpu.data.ss.limit = 0x1fu;
        state.machine->executor_cpu.data.esp = 0x12340018u;
        failed |= core_machine_memory_write(state.machine, 0x4018u, image,
            sizeof(image)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x2000u, code, sizeof(code)) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        failed |= core_machine_run(state.machine, (core_machine_run_budget){1u, 0u},
            &result) != TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
            after.data.eip != 0u || after.data.eax != before.data.eax ||
            after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx || after.data.esp != before.data.esp ||
            after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi ||
            after.data.eflags != before.data.eflags || !pusha_popa_sregs_same(
            &before, &after);
        for (slot = 0u; !failed && slot != sizeof(image) / sizeof(image[0]); ++slot)
        {
            type_unsigned_32 value;

            failed |= !pusha_popa_read_image(&state, 0x4018u + slot * 2u, 2u,
                &value) || value != image[slot];
        }
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT pusha_popa_test_irq_no_shadow(C_VOID)
{
    static const type_unsigned_8 codes[][2] = {{0x60u, 0x90u}, {0x61u, 0x90u}};
    static const type_unsigned_8 halt = 0xf4u;
    type_unsigned_8 form;

    for (form = 0u; form != 2u; ++form)
    {
        pusha_popa_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        type_unsigned_16 offset = 0x100u;
        type_unsigned_16 segment = 0u;
        type_unsigned_16 frame_ip = 0u;
        type_unsigned_16 image[] = {0x1111u, 0x2222u, 0x3333u, 0x4444u,
            0x5555u, 0x6666u, 0x7777u, 0x8888u};
        C_INT failed = !pusha_popa_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed)
        {
            pusha_popa_seed(&state);
            if (form != 0u)
                failed |= core_machine_memory_write(state.machine, 0x8000u, image,
                    sizeof(image)) != TYPE_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, 0u, codes[form],
                sizeof(codes[form])) != TYPE_STATUS_OK || core_machine_memory_write(
                state.machine, 0x80u, &offset, sizeof(offset)) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x82u, &segment,
                sizeof(segment)) != TYPE_STATUS_OK || core_machine_memory_write(
                state.machine, 0x100u, &halt, sizeof(halt)) != TYPE_STATUS_OK;
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
                after.data.eip != 0x101u || frame_ip != 1u || !TYPE_GET_BIT(
                state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u)) ||
                TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
                VPIC_IRR_IRQ(0u));
            if (form == 0u)
            {
                failed |= after.data.esp != 0x12347feau || !pusha_popa_expect_push_image(
                    &state, &before, 0x7ff0u, 2u);
            }
            else
            {
                failed |= after.data.eax != 0xa1a28888u || after.data.ecx !=
                    0xb1b27777u || after.data.edx != 0xc1c26666u || after.data.ebx !=
                    0xd1d25555u || after.data.ebp != 0xe1e23333u || after.data.esi !=
                    0xf1f22222u || after.data.edi != 0x11221111u || after.data.esp !=
                    0x1234800au;
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
    if (!pusha_popa_test_defaults())
    {
        STD_PRINTF("PUSHA-POPA stage=defaults\n");
        return 1;
    }
    if (!pusha_popa_test_attributes())
    {
        STD_PRINTF("PUSHA-POPA stage=attributes\n");
        return 1;
    }
    if (!pusha_popa_test_rejections())
    {
        STD_PRINTF("PUSHA-POPA stage=rejections\n");
        return 1;
    }
    if (!pusha_popa_test_protected_pusha_limit())
    {
        STD_PRINTF("PUSHA-POPA stage=protected-pusha\n");
        return 1;
    }
    if (!pusha_popa_test_protected_popa_limit())
    {
        STD_PRINTF("PUSHA-POPA stage=protected-popa\n");
        return 1;
    }
    if (!pusha_popa_test_irq_no_shadow())
    {
        STD_PRINTF("PUSHA-POPA stage=irq\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S42:PUSHA-POPA:OK\n");
    STD_PRINTF("M5:T401:S31:PUSHA-POPA-PROFILES:OK\n");
    return 0;
}
