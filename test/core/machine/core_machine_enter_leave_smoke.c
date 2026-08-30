#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/pic.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct enter_leave_machine
{
    core_machine *machine;
} enter_leave_machine;

static C_VOID enter_leave_reset(C_VOID *opaque)
{
    enter_leave_machine *state = (enter_leave_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider enter_leave_provider = {
    enter_leave_reset, STD_NULL
};

static C_INT enter_leave_prepare(core_machine_cpu_profile profile,
    enter_leave_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    STD_MEMSET(state, 0, sizeof(*state));
    return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &enter_leave_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static C_VOID enter_leave_seed(enter_leave_machine *state)
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

static C_INT enter_leave_sregs_same(const t_cpu *before, const t_cpu *after)
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

static C_INT enter_leave_cpu_same(const t_cpu *before, const t_cpu *after)
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

static C_INT enter_leave_read(enter_leave_machine *state, type_unsigned_32 address,
    type_unsigned_8 width, type_unsigned_32 *value)
{
    *value = 0u;
    return core_machine_memory_read_physical(&state->machine->executor_memory,
        address, TYPE_REFERENCE_OF(*value), width) == TYPE_STATUS_OK;
}

static C_INT enter_leave_run(enter_leave_machine *state, const type_unsigned_8 *code,
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

static C_INT enter_leave_expect_image(enter_leave_machine *state,
    type_unsigned_32 address, type_unsigned_8 width, type_unsigned_32 expected)
{
    type_unsigned_32 observed;

    return enter_leave_read(state, address, width, &observed) && observed ==
        (width == 2u ? (expected & 0xffffu) : expected);
}

static C_INT enter_leave_test_enter(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, type_unsigned_8 bytes, type_unsigned_8 width, type_unsigned_16 allocation,
    type_unsigned_8 level, C_INT stack32)
{
    enter_leave_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    type_unsigned_32 old_stack;
    type_unsigned_32 frame;
    type_unsigned_32 final_stack;
    type_unsigned_32 display0 = 0x11112222u;
    type_unsigned_32 display1 = 0x33334444u;
    type_unsigned_8 effective_level = level;
    C_INT failed = !enter_leave_prepare(profile, &state);

    if (!failed)
    {
        enter_leave_seed(&state);
        if (stack32)
        {
            state.machine->executor_cpu.data.ss.seg.data.big = TYPE_TRUE;
            state.machine->executor_cpu.data.esp = 0x00008000u;
        }
        if (width == 4u)
            state.machine->executor_cpu.data.ebp = 0x00009000u;
        old_stack = stack32 ? state.machine->executor_cpu.data.esp :
            state.machine->executor_cpu.data.sp;
        if (effective_level > 1u)
        {
            type_unsigned_32 source = width == 2u ?
                state.machine->executor_cpu.data.bp :
                state.machine->executor_cpu.data.ebp;

            failed |= core_machine_memory_write(state.machine, source - width,
                &display0, width) != TYPE_STATUS_OK;
            if (effective_level > 2u)
                failed |= core_machine_memory_write(state.machine,
                    source - 2u * width, &display1, width) != TYPE_STATUS_OK;
        }
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !enter_leave_run(&state, code, bytes,
            (core_machine_run_budget){1u, 0u}, &after, &diagnostic, &status,
            &result) || status != TYPE_STATUS_OK || result.reason !=
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

static C_INT enter_leave_test_leave(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, type_unsigned_8 bytes, type_unsigned_8 width, C_INT stack32)
{
    enter_leave_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    type_unsigned_32 old_bp = width == 2u ? 0xface4321u : 0xface4321u;
    type_unsigned_32 frame = stack32 ? 0x00008020u : 0x00008020u;
    C_INT failed = !enter_leave_prepare(profile, &state);

    if (!failed)
    {
        enter_leave_seed(&state);
        if (stack32)
            state.machine->executor_cpu.data.ss.seg.data.big = TYPE_TRUE;
        state.machine->executor_cpu.data.ebp = width == 2u ? 0xe1e28020u :
            frame;
        failed |= core_machine_memory_write(state.machine, frame, &old_bp,
            width) != TYPE_STATUS_OK;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !enter_leave_run(&state, code, bytes,
            (core_machine_run_budget){1u, 0u}, &after, &diagnostic, &status,
            &result) || status != TYPE_STATUS_OK || result.reason !=
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

static C_INT enter_leave_test_defaults(C_VOID)
{
    static const core_machine_cpu_profile supported[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386};
    static const type_unsigned_8 enter0[] = {0xc8u, 0x04u, 0x00u, 0x00u};
    static const type_unsigned_8 enter1[] = {0xc8u, 0x00u, 0x00u, 0x01u};
    static const type_unsigned_8 enter3[] = {0xc8u, 0x04u, 0x00u, 0x03u};
    static const type_unsigned_8 enter33[] = {0xc8u, 0x02u, 0x00u, 0x21u};
    static const type_unsigned_8 enter255[] = {0xc8u, 0x00u, 0x00u, 0xffu};
    static const type_unsigned_8 leave[] = {0xc9u};
    type_unsigned_8 profile;

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

static C_INT enter_leave_test_attributes(C_VOID)
{
    static const type_unsigned_8 enter32[] = {0x66u, 0xc8u, 0x08u, 0x00u, 0x02u};
    static const type_unsigned_8 leave32[] = {0x66u, 0xc9u};
    static const type_unsigned_8 enter67[] = {0x67u, 0xc8u, 0x04u, 0x00u, 0x00u};
    static const type_unsigned_8 leave67[] = {0x67u, 0xc9u};
    static const type_unsigned_8 enter3267[] = {0x66u, 0x67u, 0xc8u, 0x04u, 0x00u,
        0x01u};
    static const type_unsigned_8 leave3267[] = {0x66u, 0x67u, 0xc9u};

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

static C_INT enter_leave_test_reject_case(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, type_unsigned_8 bytes)
{
    enter_leave_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    type_unsigned_32 image = 0xdecafbad;
    type_unsigned_32 observed;
    C_INT failed = !enter_leave_prepare(profile, &state);

    if (!failed)
    {
        enter_leave_seed(&state);
        failed |= core_machine_memory_write(state.machine, 0x7ff0u, &image,
            sizeof(image)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x8000u, &image, sizeof(image)) != TYPE_STATUS_OK;
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !enter_leave_run(&state, code, bytes,
            (core_machine_run_budget){1u, 0u}, &after, &diagnostic, &status,
            &result) || status != TYPE_STATUS_FAULT || result.reason !=
            CORE_MACHINE_STOP_FAULT || !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
            VCPUINS_EXCEPT_UD) || !enter_leave_cpu_same(&before, &after) ||
            !enter_leave_read(&state, 0x7ff0u, sizeof(image), &observed) ||
            observed != image || !enter_leave_read(&state, 0x8000u,
            sizeof(image), &observed) || observed != image;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT enter_leave_test_rejections(C_VOID)
{
    static const type_unsigned_8 enter[] = {0xc8u, 0x04u, 0x00u, 0x00u};
    static const type_unsigned_8 leave[] = {0xc9u};
    static const type_unsigned_8 attributes[][6] = {
        {0x66u, 0xc8u, 0x04u, 0x00u, 0x00u, 0u},
        {0x67u, 0xc8u, 0x04u, 0x00u, 0x00u, 0u},
        {0x66u, 0x67u, 0xc8u, 0x04u, 0x00u, 0x00u},
        {0x66u, 0xc9u, 0u, 0u, 0u, 0u},
        {0x67u, 0xc9u, 0u, 0u, 0u, 0u},
        {0x66u, 0x67u, 0xc9u, 0u, 0u, 0u}
    };
    static const type_unsigned_8 attribute_bytes[] = {5u, 5u, 6u, 2u, 2u, 3u};
    static const type_unsigned_8 lock[][7] = {
        {0xf0u, 0xc8u, 0x04u, 0x00u, 0x00u, 0u, 0u},
        {0xf0u, 0xc9u, 0u, 0u, 0u, 0u, 0u},
        {0xf0u, 0x66u, 0xc8u, 0x04u, 0x00u, 0x00u, 0u},
        {0xf0u, 0x67u, 0xc9u, 0u, 0u, 0u, 0u},
        {0xf0u, 0x66u, 0x67u, 0xc8u, 0x04u, 0x00u, 0x00u}
    };
    static const type_unsigned_8 lock_bytes[] = {5u, 2u, 6u, 3u, 7u};
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286};
    type_unsigned_8 profile;
    type_unsigned_8 form;

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

static C_INT enter_leave_boot_protected(enter_leave_machine *state)
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

static C_INT enter_leave_test_protected_stack32(C_VOID)
{
    static const type_unsigned_8 enter[] = {0x66u, 0xc8u, 0x08u, 0x00u, 0x02u};
    static const type_unsigned_8 leave[] = {0x66u, 0xc9u};
    enter_leave_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_unsigned_32 parent = 0x11112222u;
    C_INT failed = !enter_leave_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);

    if (!failed)
        failed |= !enter_leave_boot_protected(&state);
    if (!failed)
    {
        enter_leave_seed(&state);
        state.machine->executor_cpu.data.ss.seg.data.big = TYPE_TRUE;
        state.machine->executor_cpu.data.esp = 0x00008000u;
        state.machine->executor_cpu.data.ebp = 0x00009000u;
        failed |= core_machine_memory_write(state.machine, 0xcffcu, &parent,
            sizeof(parent)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x2000u, enter, sizeof(enter)) != TYPE_STATUS_OK;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine, (core_machine_run_budget){1u, 0u},
            &result) != TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK;
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
                sizeof(leave)) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){1u, 0u}, &result) != TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK;
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

static C_INT enter_leave_test_protected_faults(C_VOID)
{
    static const type_unsigned_8 enter[] = {0xc8u, 0x00u, 0x00u, 0x03u};
    static const type_unsigned_8 leave[] = {0xc9u};
    enter_leave_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_status status;
    type_unsigned_16 stack_image[] = {0xaaaau, 0xbbbbu, 0xccccu, 0xddddu,
        0xeeeeu};
    type_unsigned_32 value;
    C_INT failed = !enter_leave_prepare(CORE_MACHINE_CPU_PROFILE_80386,
        &state);

    if (!failed)
        failed |= !enter_leave_boot_protected(&state);
    if (!failed)
    {
        enter_leave_seed(&state);
        state.machine->executor_cpu.data.ss.seg.data.expdown = TYPE_TRUE;
        state.machine->executor_cpu.data.ss.limit = 0x18u;
        state.machine->executor_cpu.data.esp = 0x12340020u;
        stack_image[0] = 0x1111u;
        stack_image[1] = 0x2222u;
        failed |= core_machine_memory_write(state.machine, 0xcffeu,
            &stack_image[0], sizeof(type_unsigned_16)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0xcffcu, &stack_image[1],
            sizeof(type_unsigned_16)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x4018u, &stack_image[2], sizeof(type_unsigned_16)) !=
            TYPE_STATUS_OK || core_machine_memory_write(state.machine, 0x2000u,
            enter, sizeof(enter)) != TYPE_STATUS_OK;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        status = core_machine_run(state.machine,
            (core_machine_run_budget){1u, 0u}, &result);
        failed |= status != TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
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
            sizeof(type_unsigned_16)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x2000u, leave, sizeof(leave)) != TYPE_STATUS_OK;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine, (core_machine_run_budget){1u, 0u},
            &result) != TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
            !enter_leave_cpu_same(&before, &after) || !enter_leave_read(&state,
            0x4020u, 2u, &value) || value != stack_image[0];
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT enter_leave_test_irq_no_shadow(C_VOID)
{
    static const type_unsigned_8 codes[][5] = {
        {0xc8u, 0x04u, 0x00u, 0x00u, 0x90u},
        {0xc9u, 0x90u, 0u, 0u, 0u}
    };
    static const type_unsigned_8 bytes[] = {5u, 2u};
    static const type_unsigned_8 halt = 0xf4u;
    type_unsigned_8 form;

    for (form = 0u; form != 2u; ++form)
    {
        enter_leave_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        type_unsigned_16 offset = 0x100u;
        type_unsigned_16 segment = 0u;
        type_unsigned_16 frame_ip = 0u;
        type_unsigned_16 old_bp = 0x4567u;
        C_INT failed = !enter_leave_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed)
        {
            enter_leave_seed(&state);
            if (form != 0u)
            {
                state.machine->executor_cpu.data.ebp = 0xe1e28020u;
                failed |= core_machine_memory_write(state.machine, 0x8020u,
                    &old_bp, sizeof(old_bp)) != TYPE_STATUS_OK;
            }
            failed |= core_machine_memory_write(state.machine, 0u, codes[form],
                bytes[form]) != TYPE_STATUS_OK || core_machine_memory_write(
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
                after.data.eip != 0x101u || frame_ip != (form == 0u ? 4u : 1u) ||
                !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
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

C_INT main(C_VOID)
{
    if (!enter_leave_test_defaults())
    {
        STD_PRINTF("ENTER-LEAVE stage=defaults\n");
        return 1;
    }
    if (!enter_leave_test_attributes())
    {
        STD_PRINTF("ENTER-LEAVE stage=attributes\n");
        return 1;
    }
    if (!enter_leave_test_rejections())
    {
        STD_PRINTF("ENTER-LEAVE stage=rejections\n");
        return 1;
    }
    if (!enter_leave_test_protected_stack32())
    {
        STD_PRINTF("ENTER-LEAVE stage=protected-stack32\n");
        return 1;
    }
    if (!enter_leave_test_protected_faults())
    {
        STD_PRINTF("ENTER-LEAVE stage=protected-faults\n");
        return 1;
    }
    if (!enter_leave_test_irq_no_shadow())
    {
        STD_PRINTF("ENTER-LEAVE stage=irq\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S43:ENTER-LEAVE:OK\n");
    STD_PRINTF("M5:T401:S24:ENTER-LEAVE-PROFILES:OK\n");
    return 0;
}
