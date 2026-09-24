#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"
#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/pic.h"
#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_cpu_fixture.h"

typedef struct enter_leave_machine
{
    core_machine *machine;
} enter_leave_machine;

static void enter_leave_reset(void *opaque)
{
    enter_leave_machine *state = (enter_leave_machine *)opaque;

    if (state != LIB_NULL)
        (void)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider enter_leave_provider = {
    enter_leave_reset, LIB_NULL
};

static lib_i32 enter_leave_prepare(core_machine_cpu_profile profile,
    enter_leave_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    lib_memory_set(state, 0, sizeof(*state));
    return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &enter_leave_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static void enter_leave_seed(enter_leave_machine *state)
{
    t_cpu *cpu = &state->machine->executor_cpu;

    cpu->data.eax = 0xa1a23344u;
    cpu->data.ecx = 0xb1b25566u;
    cpu->data.edx = 0xc1c27788u;
    cpu->data.ebx = 0xd1d299aau;
    cpu->data.esp = 0x12348000u;
    cpu->data.ebp = 0xe1e29000u;
    cpu->data.esi = 0xf1f2ddefu;
    cpu->data.edi = 0x1122a5a5u;
    cpu->data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_ZF |
        VCPU_EFLAGS_IF;
}

static lib_i32 enter_leave_sregs_same(const t_cpu *before, const t_cpu *after)
{
    return lib_memory_compare(&before->data.es, &after->data.es,
        sizeof(before->data.es)) == 0 && lib_memory_compare(&before->data.cs,
        &after->data.cs, sizeof(before->data.cs)) == 0 && lib_memory_compare(
        &before->data.ss, &after->data.ss, sizeof(before->data.ss)) == 0 &&
        lib_memory_compare(&before->data.ds, &after->data.ds,
        sizeof(before->data.ds)) == 0 && lib_memory_compare(&before->data.fs,
        &after->data.fs, sizeof(before->data.fs)) == 0 && lib_memory_compare(
        &before->data.gs, &after->data.gs, sizeof(before->data.gs)) == 0;
}

static lib_i32 enter_leave_cpu_same(const t_cpu *before, const t_cpu *after)
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
        enter_leave_sregs_same(before, after);
}

static lib_i32 enter_leave_read(enter_leave_machine *state, lib_u32 address,
    lib_u8 width, lib_u32 *value)
{
    *value = 0u;
    return core_machine_memory_read_physical(&state->machine->executor_memory,
        address, CORE_MACHINE_REFERENCE_OF(*value), width) == LIB_STATUS_OK;
}

static lib_i32 enter_leave_run(enter_leave_machine *state, const lib_u8 *code,
    lib_u8 bytes, core_machine_run_budget budget, t_cpu *after,
    core_machine_cpu_diagnostic *diagnostic, lib_status *status,
    core_machine_run_result *result)
{
    if (core_machine_memory_write(state->machine, 0u, code, bytes) !=
        LIB_STATUS_OK)
        return 0;

    *status = core_machine_run(state->machine, budget, result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        LIB_STATUS_OK;
}

static lib_i32 enter_leave_expect_image(enter_leave_machine *state,
    lib_u32 address, lib_u8 width, lib_u32 expected)
{
    lib_u32 observed;

    return enter_leave_read(state, address, width, &observed) && observed ==
        (width == 2u ? (expected & 0xffffu) : expected);
}

static lib_i32 enter_leave_test_enter(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_u8 width, lib_u16 allocation,
    lib_u8 level, lib_i32 stack32)
{
    enter_leave_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    lib_status status;
    lib_u32 old_stack;
    lib_u32 frame;
    lib_u32 final_stack;
    lib_u32 display0 = 0x11112222u;
    lib_u32 display1 = 0x33334444u;
    lib_u8 effective_level = level;
    lib_i32 failed = !enter_leave_prepare(profile, &state);

    if (!failed)
    {
        enter_leave_seed(&state);
        if (stack32)
        {
            state.machine->executor_cpu.data.ss.seg.data.big = LIB_TRUE;
            state.machine->executor_cpu.data.esp = 0x00008000u;
        }
        if (width == 4u)
            state.machine->executor_cpu.data.ebp = 0x00009000u;
        old_stack = stack32 ? state.machine->executor_cpu.data.esp :
            state.machine->executor_cpu.data.sp;
        if (effective_level > 1u)
        {
            lib_u32 source = width == 2u ?
                state.machine->executor_cpu.data.bp :
                state.machine->executor_cpu.data.ebp;

            failed |= core_machine_memory_write(state.machine, source - width,
                &display0, width) != LIB_STATUS_OK;
            if (effective_level > 2u)
                failed |= core_machine_memory_write(state.machine,
                    source - 2u * width, &display1, width) != LIB_STATUS_OK;
        }
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !enter_leave_run(&state, code, bytes,
            (core_machine_run_budget){1u, 0u}, &after, &diagnostic, &status,
            &result) || status != LIB_STATUS_OK || result.reason !=
            CORE_MACHINE_STOP_BUDGET || diagnostic.first_fault.valid ||
            after.data.eip != bytes || after.data.eax != before.data.eax ||
            after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx || after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi || after.data.eflags !=
            before.data.eflags || !enter_leave_sregs_same(&before, &after);
        frame = old_stack - width;
        final_stack = frame - (effective_level ? effective_level * width : 0u) -
            allocation;
        if (width == 2u)
        {
            failed |= after.data.ebp != ((before.data.ebp & 0xffff0000u) |
                (frame & 0xffffu));
        }
        else
            failed |= after.data.ebp != frame;
        if (stack32)
            failed |= after.data.esp != final_stack;
        else
            failed |= after.data.esp != ((before.data.esp & 0xffff0000u) |
                (final_stack & 0xffffu));
        failed |= !enter_leave_expect_image(&state, frame, width,
            before.data.ebp);
        if (effective_level)
        {
            failed |= !enter_leave_expect_image(&state, frame -
                effective_level * width, width, frame);
            if (effective_level > 1u)
                failed |= !enter_leave_expect_image(&state, frame - width,
                    width, display0);
            if (effective_level > 2u)
                failed |= !enter_leave_expect_image(&state, frame - 2u * width,
                    width, display1);
        }
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 enter_leave_test_leave(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_u8 width, lib_i32 stack32)
{
    enter_leave_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    lib_status status;
    lib_u32 old_bp = width == 2u ? 0xface4321u : 0xface4321u;
    lib_u32 frame = stack32 ? 0x00008020u : 0x00008020u;
    lib_i32 failed = !enter_leave_prepare(profile, &state);

    if (!failed)
    {
        enter_leave_seed(&state);
        if (stack32)
            state.machine->executor_cpu.data.ss.seg.data.big = LIB_TRUE;
        state.machine->executor_cpu.data.ebp = width == 2u ? 0xe1e28020u :
            frame;
        failed |= core_machine_memory_write(state.machine, frame, &old_bp,
            width) != LIB_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !enter_leave_run(&state, code, bytes,
            (core_machine_run_budget){1u, 0u}, &after, &diagnostic, &status,
            &result) || status != LIB_STATUS_OK || result.reason !=
            CORE_MACHINE_STOP_BUDGET || diagnostic.first_fault.valid ||
            after.data.eip != bytes || after.data.eax != before.data.eax ||
            after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx || after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi || after.data.eflags !=
            before.data.eflags || !enter_leave_sregs_same(&before, &after) ||
            !enter_leave_expect_image(&state, frame, width, old_bp);
        if (width == 2u)
            failed |= after.data.ebp != ((before.data.ebp & 0xffff0000u) |
                (old_bp & 0xffffu));
        else
            failed |= after.data.ebp != old_bp;
        if (stack32)
            failed |= after.data.esp != frame + width;
        else
            failed |= after.data.esp != ((before.data.esp & 0xffff0000u) |
                ((frame + width) & 0xffffu));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 enter_leave_test_defaults(void)
{
    static const core_machine_cpu_profile supported[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386};
    static const lib_u8 enter0[] = {0xc8u, 0x04u, 0x00u, 0x00u};
    static const lib_u8 enter1[] = {0xc8u, 0x00u, 0x00u, 0x01u};
    static const lib_u8 enter3[] = {0xc8u, 0x04u, 0x00u, 0x03u};
    static const lib_u8 enter33[] = {0xc8u, 0x02u, 0x00u, 0x21u};
    static const lib_u8 enter255[] = {0xc8u, 0x00u, 0x00u, 0xffu};
    static const lib_u8 leave[] = {0xc9u};
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(supported) / sizeof(supported[0]);
         ++profile)
    {
        if (!enter_leave_test_enter(supported[profile], enter0, sizeof(enter0),
            2u, 4u, 0u, 0))
            return 0;
        if (!enter_leave_test_enter(supported[profile], enter1, sizeof(enter1),
            2u, 0u, 1u, 0))
            return 0;
        if (!enter_leave_test_enter(supported[profile], enter3, sizeof(enter3),
            2u, 4u, 3u, 0))
            return 0;
        if (!enter_leave_test_enter(supported[profile], enter33, sizeof(enter33),
            2u, 2u, supported[profile] == CORE_MACHINE_CPU_PROFILE_80186 ?
            33u : 1u, 0))
            return 0;
        if (supported[profile] == CORE_MACHINE_CPU_PROFILE_80186 &&
            !enter_leave_test_enter(supported[profile], enter255,
                sizeof(enter255), 2u, 0u, 255u, 0))
            return 0;
        if (!enter_leave_test_leave(supported[profile], leave, sizeof(leave),
            2u, 0))
            return 0;
    }
    return 1;
}

static lib_i32 enter_leave_test_attributes(void)
{
    static const lib_u8 enter32[] = {0x66u, 0xc8u, 0x08u, 0x00u, 0x02u};
    static const lib_u8 leave32[] = {0x66u, 0xc9u};
    static const lib_u8 enter67[] = {0x67u, 0xc8u, 0x04u, 0x00u, 0x00u};
    static const lib_u8 leave67[] = {0x67u, 0xc9u};
    static const lib_u8 enter3267[] = {0x66u, 0x67u, 0xc8u, 0x04u, 0x00u,
        0x01u};
    static const lib_u8 leave3267[] = {0x66u, 0x67u, 0xc9u};

    if (!enter_leave_test_enter(CORE_MACHINE_CPU_PROFILE_80386, enter32,
        sizeof(enter32), 4u, 8u, 2u, 0))
        return 0;
    if (!enter_leave_test_leave(CORE_MACHINE_CPU_PROFILE_80386, leave32,
        sizeof(leave32), 4u, 0))
        return 0;
    if (!enter_leave_test_enter(CORE_MACHINE_CPU_PROFILE_80386, enter67,
        sizeof(enter67), 2u, 4u, 0u, 0))
        return 0;
    if (!enter_leave_test_leave(CORE_MACHINE_CPU_PROFILE_80386, leave67,
        sizeof(leave67), 2u, 0))
        return 0;
    if (!enter_leave_test_enter(CORE_MACHINE_CPU_PROFILE_80386, enter3267,
        sizeof(enter3267), 4u, 4u, 1u, 0))
        return 0;
    if (!enter_leave_test_leave(CORE_MACHINE_CPU_PROFILE_80386, leave3267,
        sizeof(leave3267), 4u, 0))
        return 0;
    return 1;
}

static lib_i32 enter_leave_test_reject_case(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    enter_leave_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    lib_status status;
    lib_u32 image = 0xdecafbad;
    lib_u32 observed;
    lib_i32 failed = !enter_leave_prepare(profile, &state);

    if (!failed)
    {
        enter_leave_seed(&state);
        failed |= core_machine_memory_write(state.machine, 0x7ff0u, &image,
            sizeof(image)) != LIB_STATUS_OK || core_machine_memory_write(
            state.machine, 0x8000u, &image, sizeof(image)) != LIB_STATUS_OK;
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !enter_leave_run(&state, code, bytes,
            (core_machine_run_budget){1u, 0u}, &after, &diagnostic, &status,
            &result) || status != LIB_STATUS_INTERNAL_ERROR || result.reason !=
            CORE_MACHINE_STOP_FAULT || !diagnostic.first_fault.valid ||
            !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
            VCPUINS_EXCEPT_UD) || !enter_leave_cpu_same(&before, &after) ||
            !enter_leave_read(&state, 0x7ff0u, sizeof(image), &observed) ||
            observed != image || !enter_leave_read(&state, 0x8000u,
            sizeof(image), &observed) || observed != image;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 enter_leave_test_rejections(void)
{
    static const lib_u8 enter[] = {0xc8u, 0x04u, 0x00u, 0x00u};
    static const lib_u8 leave[] = {0xc9u};
    static const lib_u8 attributes[][6] = {
        {0x66u, 0xc8u, 0x04u, 0x00u, 0x00u, 0u},
        {0x67u, 0xc8u, 0x04u, 0x00u, 0x00u, 0u},
        {0x66u, 0x67u, 0xc8u, 0x04u, 0x00u, 0x00u},
        {0x66u, 0xc9u, 0u, 0u, 0u, 0u},
        {0x67u, 0xc9u, 0u, 0u, 0u, 0u},
        {0x66u, 0x67u, 0xc9u, 0u, 0u, 0u}
    };
    static const lib_u8 attribute_bytes[] = {5u, 5u, 6u, 2u, 2u, 3u};
    static const lib_u8 lock[][7] = {
        {0xf0u, 0xc8u, 0x04u, 0x00u, 0x00u, 0u, 0u},
        {0xf0u, 0xc9u, 0u, 0u, 0u, 0u, 0u},
        {0xf0u, 0x66u, 0xc8u, 0x04u, 0x00u, 0x00u, 0u},
        {0xf0u, 0x67u, 0xc9u, 0u, 0u, 0u, 0u},
        {0xf0u, 0x66u, 0x67u, 0xc8u, 0x04u, 0x00u, 0x00u}
    };
    static const lib_u8 lock_bytes[] = {5u, 2u, 6u, 3u, 7u};
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286};
    lib_u8 profile;
    lib_u8 form;

    if (!enter_leave_test_reject_case(CORE_MACHINE_CPU_PROFILE_8086, enter,
        sizeof(enter)) || !enter_leave_test_reject_case(
        CORE_MACHINE_CPU_PROFILE_8086, leave, sizeof(leave)))
        return 0;
    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]);
         ++profile)
    {
        for (form = 0u; form != sizeof(attributes) / sizeof(attributes[0]);
             ++form)
        {
            if (!enter_leave_test_reject_case(legacy[profile], attributes[form],
                attribute_bytes[form]))
                return 0;
        }
    }
    for (form = 0u; form != sizeof(lock) / sizeof(lock[0]); ++form)
    {
        if (!enter_leave_test_reject_case(CORE_MACHINE_CPU_PROFILE_80386,
            lock[form], lock_bytes[form]))
            return 0;
    }
    return 1;
}

static lib_i32 enter_leave_boot_protected(enter_leave_machine *state)
{
    static const lib_u8 pointer[] = {0x1fu, 0u, 0u, 0x03u, 0u, 0u};
    static const lib_u8 gdt[] = {
        0u, 0u, 0u, 0u, 0u, 0u, 0u, 0u,
        0xffu, 0xffu, 0u, 0x20u, 0u, 0x9au, 0u, 0u,
        0xffu, 0xffu, 0u, 0x30u, 0u, 0x92u, 0u, 0u,
        0xffu, 0xffu, 0u, 0x40u, 0u, 0x92u, 0u, 0u
    };
    static const lib_u8 bootstrap[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u, 0xb8u, 0x01u, 0x00u,
        0x0fu, 0x01u, 0xf0u, 0xb8u, 0x10u, 0x00u, 0x8eu, 0xd8u,
        0x8eu, 0xc0u, 0xb8u, 0x18u, 0x00u, 0x8eu, 0xd0u, 0xbcu,
        0x00u, 0x80u, 0xeau, 0x00u, 0x00u, 0x08u, 0x00u
    };
    static const lib_u8 halt[] = {0xf4u};
    core_machine_run_result result;

    return core_machine_memory_write(state->machine, 0x0100u, pointer,
        sizeof(pointer)) == LIB_STATUS_OK && core_machine_memory_write(
        state->machine, 0x0300u, gdt, sizeof(gdt)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, bootstrap,
        sizeof(bootstrap)) == LIB_STATUS_OK && core_machine_memory_write(
        state->machine, 0x2000u, halt, sizeof(halt)) == LIB_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){96u, 0u},
        &result) == LIB_STATUS_OK && result.reason ==
        CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static lib_i32 enter_leave_test_protected_stack32(void)
{
    static const lib_u8 enter[] = {0x66u, 0xc8u, 0x08u, 0x00u, 0x02u};
    static const lib_u8 leave[] = {0x66u, 0xc9u};
    enter_leave_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    lib_u32 parent = 0x11112222u;
    lib_i32 failed = !enter_leave_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);

    if (!failed)
        failed |= !enter_leave_boot_protected(&state);
    if (!failed)
    {
        enter_leave_seed(&state);
        state.machine->executor_cpu.data.ss.seg.data.big = LIB_TRUE;
        state.machine->executor_cpu.data.esp = 0x00008000u;
        state.machine->executor_cpu.data.ebp = 0x00009000u;
        failed |= core_machine_memory_write(state.machine, 0xcffcu, &parent,
            sizeof(parent)) != LIB_STATUS_OK || core_machine_memory_write(
            state.machine, 0x2000u, enter, sizeof(enter)) != LIB_STATUS_OK;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine, (core_machine_run_budget){1u, 0u},
            &result) != LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= diagnostic.first_fault.valid || after.data.eip != sizeof(enter) ||
            after.data.eax != before.data.eax || after.data.ecx != before.data.ecx ||
            after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
            after.data.esi != before.data.esi || after.data.edi != before.data.edi ||
            after.data.eflags != before.data.eflags || after.data.ebp != 0x7ffcu ||
            after.data.esp != 0x7fecu || !enter_leave_expect_image(&state,
            0x7ffcu + 0x4000u, 4u, before.data.ebp) ||
            !enter_leave_expect_image(&state, 0x7ff8u + 0x4000u, 4u, parent) ||
            !enter_leave_expect_image(&state, 0x7ff4u + 0x4000u, 4u, 0x7ffcu);
        if (!failed)
        {
            failed |= core_machine_memory_write(state.machine, 0x2000u, leave,
                sizeof(leave)) != LIB_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){1u, 0u}, &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                LIB_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= diagnostic.first_fault.valid || after.data.eip != sizeof(leave) ||
                after.data.eax != before.data.eax || after.data.ecx != before.data.ecx ||
                after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
                after.data.esi != before.data.esi || after.data.edi != before.data.edi ||
                after.data.eflags != before.data.eflags || after.data.ebp !=
                0x00009000u || after.data.esp != 0x00008000u ||
                !enter_leave_sregs_same(&before, &after);
        }
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 enter_leave_test_protected_faults(void)
{
    static const lib_u8 enter[] = {0xc8u, 0x00u, 0x00u, 0x03u};
    static const lib_u8 leave[] = {0xc9u};
    enter_leave_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    lib_status status;
    lib_u16 stack_image[] = {0xaaaau, 0xbbbbu, 0xccccu, 0xddddu,
        0xeeeeu};
    lib_u32 value;
    lib_i32 failed = !enter_leave_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);

    if (!failed)
        failed |= !enter_leave_boot_protected(&state);
    if (!failed)
    {
        enter_leave_seed(&state);
        state.machine->executor_cpu.data.ss.seg.data.expdown = LIB_TRUE;
        state.machine->executor_cpu.data.ss.limit = 0x18u;
        state.machine->executor_cpu.data.esp = 0x12340020u;
        stack_image[0] = 0x1111u;
        stack_image[1] = 0x2222u;
        failed |= core_machine_memory_write(state.machine, 0xcffeu,
            &stack_image[0], sizeof(lib_u16)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, 0xcffcu, &stack_image[1],
            sizeof(lib_u16)) != LIB_STATUS_OK || core_machine_memory_write(
            state.machine, 0x4018u, &stack_image[2], sizeof(lib_u16)) !=
            LIB_STATUS_OK || core_machine_memory_write(state.machine, 0x2000u,
            enter, sizeof(enter)) != LIB_STATUS_OK;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        status = core_machine_run(state.machine,
            (core_machine_run_budget){1u, 0u}, &result);
        failed |= status != LIB_STATUS_INTERNAL_ERROR || result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
            !enter_leave_cpu_same(&before, &after) ||
            !enter_leave_expect_image(&state, 0x401eu, 2u,
            before.data.ebp) || !enter_leave_expect_image(&state, 0x401cu,
            2u, stack_image[0]) || !enter_leave_expect_image(&state, 0x401au,
            2u, stack_image[1]) || !enter_leave_read(&state, 0x4018u, 2u,
            &value) || value != stack_image[2];
    }
    core_machine_destroy(state.machine);

    if (failed)
        return 0;
    failed = !enter_leave_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
    if (!failed)
        failed |= !enter_leave_boot_protected(&state);
    if (!failed)
    {
        enter_leave_seed(&state);
        state.machine->executor_cpu.data.ss.limit = 0x1fu;
        state.machine->executor_cpu.data.ebp = 0xe1e20020u;
        failed |= core_machine_memory_write(state.machine, 0x4020u, stack_image,
            sizeof(lib_u16)) != LIB_STATUS_OK || core_machine_memory_write(
            state.machine, 0x2000u, leave, sizeof(leave)) != LIB_STATUS_OK;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine, (core_machine_run_budget){1u, 0u},
            &result) != LIB_STATUS_INTERNAL_ERROR || result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
            !enter_leave_cpu_same(&before, &after) || !enter_leave_read(&state,
            0x4020u, 2u, &value) || value != stack_image[0];
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 enter_leave_test_irq_no_shadow(void)
{
    static const lib_u8 codes[][5] = {
        {0xc8u, 0x04u, 0x00u, 0x00u, 0x90u},
        {0xc9u, 0x90u, 0u, 0u, 0u}
    };
    static const lib_u8 bytes[] = {5u, 2u};
    static const lib_u8 halt = 0xf4u;
    lib_u8 form;

    for (form = 0u; form != 2u; ++form)
    {
        enter_leave_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        lib_u16 offset = 0x100u;
        lib_u16 segment = 0u;
        lib_u16 frame_ip = 0u;
        lib_u16 old_bp = 0x4567u;
        lib_i32 failed = !enter_leave_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed)
        {
            enter_leave_seed(&state);
            if (form != 0u)
            {
                state.machine->executor_cpu.data.ebp = 0xe1e28020u;
                failed |= core_machine_memory_write(state.machine, 0x8020u,
                    &old_bp, sizeof(old_bp)) != LIB_STATUS_OK;
            }
            failed |= core_machine_memory_write(state.machine, 0u, codes[form],
                bytes[form]) != LIB_STATUS_OK || core_machine_memory_write(
                state.machine, 0x80u, &offset, sizeof(offset)) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x82u, &segment,
                sizeof(segment)) != LIB_STATUS_OK || core_machine_memory_write(
                state.machine, 0x100u, &halt, sizeof(halt)) != LIB_STATUS_OK;
        }
        if (!failed)
        {
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
            lib_memory_set(&source, 0, sizeof(source));
            state.machine->shared_pic_master.data.icw2 = 0x20u;
            core_machine_pic_irq_source_bind(&source,
                &state.machine->shared_pic_master, &state.machine->shared_pic_slave,
                0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){2u, 0u}, &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (lib_u16)after.data.esp,
                CORE_MACHINE_REFERENCE_OF(frame_ip), sizeof(frame_ip)) != LIB_STATUS_OK ||
                after.data.eip != 0x101u || frame_ip != (form == 0u ? 4u : 1u) ||
                !CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u)) || CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.irr,
                VPIC_IRR_IRQ(0u));
            if (form == 0u)
            {
                failed |= after.data.bp != 0x7ffeu || !enter_leave_expect_image(
                    &state, 0x7ffeu, 2u, before.data.bp);
            }
            else
            {
                failed |= after.data.bp != old_bp || after.data.esp !=
                    0x1234801cu;
            }
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!enter_leave_test_defaults())
    {
        printf("ENTER-LEAVE stage=defaults\n");
        return 1;
    }
    if (!enter_leave_test_attributes())
    {
        printf("ENTER-LEAVE stage=attributes\n");
        return 1;
    }
    if (!enter_leave_test_rejections())
    {
        printf("ENTER-LEAVE stage=rejections\n");
        return 1;
    }
    if (!enter_leave_test_protected_stack32())
    {
        printf("ENTER-LEAVE stage=protected-stack32\n");
        return 1;
    }
    if (!enter_leave_test_protected_faults())
    {
        printf("ENTER-LEAVE stage=protected-faults\n");
        return 1;
    }
    if (!enter_leave_test_irq_no_shadow())
    {
        printf("ENTER-LEAVE stage=irq\n");
        return 1;
    }
    printf("M5:T316:S43:ENTER-LEAVE:OK\n");
    printf("M5:T401:S24:ENTER-LEAVE-PROFILES:OK\n");
    return 0;
}
