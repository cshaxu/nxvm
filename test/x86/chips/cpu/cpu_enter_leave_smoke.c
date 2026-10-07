#include "support/cpu_stack_probe_fixture.h"
#include "lib/types/file.h"

/* T337_REAL_UD_TERMINAL_CPU_OWNER: terminal-UD assertions stay CPU-owned. */
static void enter_leave_seed(cpu_instruction_fixture *state)
{
    t_cpu *cpu = &state->cpu;

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

static lib_i32 enter_leave_read(cpu_instruction_fixture *state, lib_u32 address,
    lib_u8 width, lib_u32 *value)
{
    *value = 0u;
    return cpu_instruction_read(state, address, value, width,
        CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) == LIB_STATUS_OK;
}

static lib_i32 enter_leave_expect_image(cpu_instruction_fixture *state,
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
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after;
    lib_u32 old_stack;
    lib_u32 frame;
    lib_u32 frame_pointer;
    lib_u32 final_stack;
    lib_u32 display0 = 0x11112222u;
    lib_u32 display1 = 0x33334444u;
    lib_u8 effective_level = level;
    lib_i32 failed = 0;
    cpu_instruction_prepare(&state, profile);

    if (!failed)
    {
        enter_leave_seed(&state);
        if (stack32)
        {
            state.cpu.data.ss.seg.data.big = LIB_TRUE;
            state.cpu.data.esp = 0x00008000u;
        }
        if (width == 4u)
            state.cpu.data.ebp = 0x00009000u;
        old_stack = stack32 ? state.cpu.data.esp :
            state.cpu.data.sp;
        if (effective_level > 1u)
        {
            lib_u32 source = width == 2u ?
                state.cpu.data.bp :
                state.cpu.data.ebp;

            failed |= cpu_instruction_write(&state, source - width, &display0, width,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            if (effective_level > 2u)
                failed |= cpu_instruction_write(&state, source - 2u * width, &display1, width,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        }
        before = state.cpu;
        failed |= cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK || state.fault.valid ||
            after.data.eip != bytes || after.data.eax != before.data.eax ||
            after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx || after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi || after.data.eflags !=
            before.data.eflags || !enter_leave_sregs_same(&before, &after);
        frame = old_stack - width;
        frame_pointer = stack32 ? frame :
            (before.data.esp & 0xffff0000u) | (frame & 0xffffu);
        final_stack = frame - (effective_level ? effective_level * width : 0u) -
            allocation;
        if (width == 2u)
        {
            failed |= after.data.ebp != ((before.data.ebp & 0xffff0000u) |
                (frame & 0xffffu));
        }
        else
            failed |= after.data.ebp != frame_pointer;
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
                effective_level * width, width, frame_pointer);
            if (effective_level > 1u)
                failed |= !enter_leave_expect_image(&state, frame - width,
                    width, display0);
            if (effective_level > 2u)
                failed |= !enter_leave_expect_image(&state, frame - 2u * width,
                    width, display1);
        }
    }
    return !failed;
}

static lib_i32 enter_leave_test_leave(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_u8 width, lib_i32 stack32)
{
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after;
    lib_u32 old_bp = width == 2u ? 0xface4321u : 0xface4321u;
    lib_u32 frame = stack32 ? 0x00008020u : 0x00008020u;
    lib_i32 failed = 0;
    cpu_instruction_prepare(&state, profile);

    if (!failed)
    {
        enter_leave_seed(&state);
        if (stack32)
            state.cpu.data.ss.seg.data.big = LIB_TRUE;
        state.cpu.data.ebp = width == 2u ? 0xe1e28020u :
            frame;
        failed |= cpu_instruction_write(&state, frame, &old_bp, width,
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        before = state.cpu;
        failed |= cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK || state.fault.valid ||
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
    cpu_instruction_fixture state;
    lib_u32 image = 0xdecafbad;
    lib_u32 observed;
    lib_i32 failed = 0;
    cpu_instruction_prepare(&state, profile);

    if (!failed)
    {
        enter_leave_seed(&state);
        failed |= cpu_instruction_write(&state, 0x7000u, &image, sizeof(image),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            cpu_instruction_write(&state, 0x9000u, &image, sizeof(image),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        failed |= !cpu_instruction_expect_real_fault(&state, code, bytes, 6u) ||
            !enter_leave_read(&state, 0x7000u, sizeof(image), &observed) ||
            observed != image || !enter_leave_read(&state, 0x9000u,
            sizeof(image), &observed) || observed != image;
    }
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

static lib_i32 enter_leave_boot_protected(cpu_instruction_fixture *state)
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
    lib_u8 instruction;

    if (cpu_instruction_write(state, 0x0100u, pointer, sizeof(pointer),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
        cpu_instruction_write(state, 0x0300u, gdt, sizeof(gdt),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
        cpu_instruction_write(state, 0u, bootstrap, sizeof(bootstrap),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK)
        return 0;
    for (instruction = 0u; instruction != 10u; ++instruction)
    {
        core_machine_cpu_execution_refresh(&state->execution);
        if (state->execution.stop_requested || state->fault.valid) return 0;
    }
    return state->cpu.data.cs.selector == 8u && state->cpu.data.eip == 0u;
}

static lib_i32 enter_leave_test_protected_stack32(void)
{
    static const lib_u8 enter[] = {0x66u, 0xc8u, 0x08u, 0x00u, 0x02u};
    static const lib_u8 leave[] = {0x66u, 0xc9u};
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after;
    lib_u32 parent = 0x11112222u;
    lib_i32 failed = 0;
    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed)
        failed |= !enter_leave_boot_protected(&state);
    if (!failed)
    {
        enter_leave_seed(&state);
        state.cpu.data.ss.seg.data.big = LIB_TRUE;
        state.cpu.data.esp = 0x00008000u;
        state.cpu.data.ebp = 0x00009000u;
        failed |= cpu_instruction_write(&state, 0xcffcu, &parent, sizeof(parent),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            cpu_instruction_write(&state, 0x2000u, enter, sizeof(enter),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        state.cpu.data.eip = 0u;
        before = state.cpu;
        core_machine_cpu_execution_refresh(&state.execution);
        failed |= state.execution.stop_requested;
        after = state.cpu;
        failed |= state.fault.valid || after.data.eip != sizeof(enter) ||
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
            failed |= cpu_instruction_write(&state, 0x2000u, leave, sizeof(leave),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            before = state.cpu;
            state.cpu.data.eip = 0u;
            core_machine_cpu_execution_refresh(&state.execution);
            failed |= state.execution.stop_requested;
            after = state.cpu;
            failed |= state.fault.valid || after.data.eip != sizeof(leave) ||
                after.data.eax != before.data.eax || after.data.ecx != before.data.ecx ||
                after.data.edx != before.data.edx || after.data.ebx != before.data.ebx ||
                after.data.esi != before.data.esi || after.data.edi != before.data.edi ||
                after.data.eflags != before.data.eflags || after.data.ebp !=
                0x00009000u || after.data.esp != 0x00008000u ||
                !enter_leave_sregs_same(&before, &after);
        }
    }
    return !failed;
}

static lib_i32 enter_leave_test_protected_faults(void)
{
    static const lib_u8 enter[] = {0xc8u, 0x00u, 0x00u, 0x03u};
    static const lib_u8 leave[] = {0xc9u};
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after;
    lib_u16 stack_image[] = {0xaaaau, 0xbbbbu, 0xccccu, 0xddddu,
        0xeeeeu};
    lib_u16 observed_frame[5];
    lib_u32 value;
    lib_i32 failed = 0;
    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed)
        failed |= !enter_leave_boot_protected(&state);
    if (!failed)
    {
        enter_leave_seed(&state);
        state.cpu.data.ss.seg.data.expdown = LIB_TRUE;
        state.cpu.data.ss.limit = 0x18u;
        state.cpu.data.esp = 0x12340020u;
        stack_image[0] = 0x1111u;
        stack_image[1] = 0x2222u;
        failed |= cpu_instruction_write(&state, 0xcffeu, &stack_image[0], sizeof(lib_u16),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            cpu_instruction_write(&state, 0xcffcu, &stack_image[1], sizeof(lib_u16),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            cpu_instruction_write(&state, 0x4018u, stack_image, sizeof(stack_image),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) !=
            LIB_STATUS_OK ||
            cpu_instruction_write(&state, 0x2000u, enter, sizeof(enter),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        state.cpu.data.eip = 0u;
        before = state.cpu;
        core_machine_cpu_execution_refresh(&state.execution);
        failed |= !core_machine_cpu_is_shutdown(&state.execution) ||
            state.execution.stop_requested;
        after = state.cpu;
        failed |= state.fault.valid || !state.delivered_exception.valid ||
            state.delivered_exception.exception_mask != VCPUINS_EXCEPT_SHUTDOWN ||
            !enter_leave_cpu_same(&before, &after) ||
            cpu_instruction_read(&state, 0x4018u, observed_frame,
                sizeof(observed_frame), CORE_MACHINE_CPU_MEMORY_ACCESS_DATA,
                LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
            lib_memory_compare(observed_frame, stack_image, sizeof(stack_image)) != 0;
    }

    if (failed)
        return 0;
    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
    failed = 0;
    if (!failed)
        failed |= !enter_leave_boot_protected(&state);
    if (!failed)
    {
        enter_leave_seed(&state);
        state.cpu.data.ss.limit = 0x1fu;
        state.cpu.data.ebp = 0xe1e20020u;
        failed |= cpu_instruction_write(&state, 0x4020u, stack_image, sizeof(lib_u16),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            cpu_instruction_write(&state, 0x2000u, leave, sizeof(leave),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        state.cpu.data.eip = 0u;
        before = state.cpu;
        core_machine_cpu_execution_refresh(&state.execution);
        failed |= !core_machine_cpu_is_shutdown(&state.execution) ||
            state.execution.stop_requested;
        after = state.cpu;
        failed |= state.fault.valid || !state.delivered_exception.valid ||
            state.delivered_exception.exception_mask != VCPUINS_EXCEPT_SHUTDOWN ||
            !enter_leave_cpu_same(&before, &after) || !enter_leave_read(&state,
            0x4020u, 2u, &value) || value != stack_image[0];
    }
    return !failed;
}

static lib_bool enter_leave_test_chain_attributes(void)
{
    lib_u8 operand32, stack32;
    lib_bool passed = LIB_TRUE;

    for (operand32 = 0u; operand32 < 2u; ++operand32)
    for (stack32 = 0u; stack32 < 2u; ++stack32) {
        const lib_u8 width = operand32 ? 4u : 2u;
        const lib_u32 chain = operand32 ? 0x19000u : 0x9000u;
        const lib_u32 other_chain = operand32 ? 0x9000u : 0x19000u;
        const lib_u32 stack = stack32 ? 0x18000u : 0x8000u;
        const lib_u32 value = operand32 ? 0xcafebabeu : 0xbeefu;
        const lib_u32 decoy = operand32 ? 0x12345678u : 0x1234u;
        lib_u8 code[] = {0x66u, 0xc8u, 0u, 0u, 2u};
        const lib_u8 offset = operand32 ? 0u : 1u;
        cpu_instruction_fixture state;
        t_cpu after;
        lib_u32 observed = 0u;

        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);
        enter_leave_seed(&state);
        state.cpu.data.esp = 0x18000u;
        state.cpu.data.ebp = 0x19000u;
        /* A valid cached wide, expand-up SS permits both candidate addresses.
         * The test observes which address is read, not a bounds side effect. */
        state.cpu.data.ss.limit = 0x3ffffu;
        state.cpu.data.ss.seg.data.big = stack32;
        if (cpu_instruction_write(&state, chain - width, &value, width,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            cpu_instruction_write(&state, other_chain - width, &decoy, width,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            cpu_instruction_run(&state, code + offset, sizeof(code) - offset,
                &after) != LIB_STATUS_OK || state.fault.valid ||
            !enter_leave_read(&state, stack - 2u * width, width, &observed) ||
            observed != value || after.data.eip != sizeof(code) - offset ||
            after.data.eflags != (VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
                VCPU_EFLAGS_ZF | VCPU_EFLAGS_IF)) {
            lib_c_printf("ENTER chain operand32=%u stack32=%u observed=%x expected=%x\n",
                (unsigned)operand32, (unsigned)stack32,
                (unsigned)observed, (unsigned)value);
            passed = LIB_FALSE;
        }
    }
    return passed;
}

static lib_bool enter_leave_test_ordered_failure(void)
{
    static const core_machine_cpu_bus_provider bus = {
        .read_memory = cpu_stack_probe_read,
        .write_memory = cpu_stack_probe_write,
        .interrupt_pending = cpu_instruction_interrupt_pending
    };
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 levels[] = {0u, 1u, 2u, 31u, 32u, 33u, 255u};
    const lib_u32 sentinel = 0xdeadc0deu;
    lib_u8 profile, operand32, stack32, level_index;
    lib_u16 failure, slot;
    lib_bool passed = LIB_TRUE;

    for (profile = 0u; profile < 3u; ++profile)
    for (operand32 = 0u; operand32 < (profile == 2u ? 2u : 1u); ++operand32)
    for (stack32 = 0u; stack32 < (profile == 2u ? 2u : 1u); ++stack32)
    for (level_index = 0u; level_index < sizeof(levels); ++level_index) {
        const lib_u8 level = profile == 0u ? levels[level_index] :
            levels[level_index] & 0x1fu;
        const lib_u8 width = operand32 ? 4u : 2u;
        const lib_u16 transfers = level ? (lib_u16)2u * level : 1u;
        const lib_u16 pushes = (lib_u16)1u + level;
        const lib_u32 mask = operand32 ? 0xffffffffu : 0xffffu;
        for (failure = 0u; failure <= transfers; ++failure) {
            cpu_stack_probe_fixture fixture;
            cpu_instruction_fixture *state = &fixture.instruction;
            lib_u8 code[] = {0x66u, 0xc8u, 8u, 0u, levels[level_index]};
            const lib_u8 offset = operand32 ? 0u : 1u;
            const lib_u32 initial_sp = stack32 ? 0x4800u : 0x12344800u;
            const lib_u32 frame = initial_sp - width;
            t_cpu before, after;

            lib_memory_set(&fixture, 0, sizeof(fixture));
            cpu_instruction_prepare_with_bus(state, profiles[profile], &bus, &fixture);
            enter_leave_seed(state);
            state->cpu.data.esp = initial_sp;
            state->cpu.data.ebp = 0x4c00u;
            state->cpu.data.ss.seg.data.big = stack32;
            for (slot = 1u; slot < level; ++slot) {
                lib_u32 value = 0xabcd0000u + slot;
                cpu_instruction_write(state, 0x4c00u - slot * width, &value,
                    width, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA);
            }
            for (slot = 0u; slot < pushes; ++slot)
                cpu_instruction_write(state, 0x4800u - (slot + 1u) * width,
                    &sentinel, width, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA);
            fixture.frame_base = 0x4000u;
            fixture.frame_size = 0x1000u;
            fixture.reject_at = failure;
            before = state->cpu;
            if (cpu_instruction_run(state, code + offset, sizeof(code) - offset,
                    &after) != (failure ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK) ||
                fixture.frame_accesses != (failure ? failure : transfers) ||
                (failure && (!state->fault.valid ||
                    state->fault.exception_mask != VCPUINS_EXCEPT_CE ||
                    !enter_leave_cpu_same(&before, &after))) ||
                (!failure && (state->fault.valid ||
                    after.data.eip != sizeof(code) - offset ||
                    after.data.eflags != before.data.eflags ||
                    after.data.ebp != (operand32 ? frame : frame & 0xffffu) ||
                    (stack32 ? after.data.esp != frame - level * width - 8u :
                        after.data.esp != (0x12340000u |
                            ((frame - level * width - 8u) & 0xffffu)))))) {
                lib_c_printf("ENTER ordered profile=%u operand32=%u stack32=%u level=%u failure=%u count=%u\n",
                    (unsigned)profile, (unsigned)operand32, (unsigned)stack32,
                    (unsigned)levels[level_index], (unsigned)failure,
                    (unsigned)fixture.frame_accesses);
                passed = LIB_FALSE;
            }
            for (slot = 0u; slot < pushes; ++slot) {
                const lib_u16 event = slot == 0u ? 1u :
                    slot == level ? transfers : (lib_u16)2u * slot + 1u;
                const lib_u32 value = slot == 0u ? before.data.ebp :
                    slot == level ? frame : 0xabcd0000u + slot;
                lib_u32 observed = 0u;
                const lib_u32 expected = !failure || event < failure ? value : sentinel;
                if (cpu_instruction_read(state, 0x4800u - (slot + 1u) * width,
                        &observed, width, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA,
                        LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
                    observed != (expected & mask)) passed = LIB_FALSE;
            }
        }
    }
    return passed;
}

static lib_bool enter_leave_test_allocation_admission(void)
{
    static const core_machine_cpu_bus_provider bus = {
        .read_memory = cpu_stack_probe_read,
        .write_memory = cpu_stack_probe_write,
        .interrupt_pending = cpu_instruction_interrupt_pending
    };
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 levels[] = {0u, 1u, 2u, 31u, 32u, 33u, 255u};
    static const lib_u16 allocations[] = {0u, 8u, 0x80u, 0xffu, 0x100u, 0xffffu};
    lib_u8 profile, operand32, stack32, down, level_index, allocation_index;
    lib_bool passed = LIB_TRUE;
    for (profile = 0u; profile < 2u; ++profile)
    for (operand32 = 0u; operand32 < (profile ? 2u : 1u); ++operand32)
    for (stack32 = 0u; stack32 < (profile ? 2u : 1u); ++stack32)
    for (down = 0u; down < 2u; ++down)
    for (level_index = 0u; level_index < sizeof(levels); ++level_index)
    for (allocation_index = 0u; allocation_index < sizeof(allocations) / sizeof(allocations[0]); ++allocation_index) {
        const lib_u8 level = levels[level_index] & 0x1fu;
        const lib_u8 width = operand32 ? 4u : 2u;
        const lib_u16 allocation = allocations[allocation_index];
        const lib_u32 bottom = 0x100u - ((lib_u32)1u + level) * width;
        const lib_u32 lower = down ? 0x81u : 0u;
        const lib_bool valid = bottom >= lower && allocation <= bottom &&
            bottom - allocation >= lower;
        cpu_stack_probe_fixture fixture;
        cpu_instruction_fixture *state = &fixture.instruction;
        lib_u8 code[] = {0x66u, 0xc8u, (lib_u8)allocation,
            (lib_u8)(allocation >> 8u), levels[level_index]};
        const lib_u8 offset = operand32 ? 0u : 1u;
        t_cpu after;

        lib_memory_set(&fixture, 0, sizeof(fixture));
        cpu_instruction_prepare_with_bus(state, profiles[profile], &bus, &fixture);
        state->cpu.data.cr0 |= VCPU_CR0_PE;
        state->cpu.data.ss.selector = 0x10u;
        state->cpu.data.ss.seg.data.writable = LIB_TRUE;
        state->cpu.data.ss.seg.data.big = stack32;
        state->cpu.data.ss.seg.data.expdown = down;
        state->cpu.data.ss.limit = down ? 0x80u : 0x200u;
        state->cpu.data.esp = 0x100u;
        state->cpu.data.ebp = 0x180u;
        state->cpu.data.idtr.limit = 0u;
        fixture.frame_base = 0x80u;
        fixture.frame_size = 0x100u;
        fixture.reject_at = 1u;
        cpu_instruction_run(state, code + offset, sizeof(code) - offset, &after);
        if (fixture.frame_accesses != (valid ? 1u : 0u)) {
            lib_c_printf("ENTER allocation profile=%u operand32=%u stack32=%u down=%u level=%u allocation=%u valid=%u count=%u\n",
                (unsigned)profile, (unsigned)operand32, (unsigned)stack32,
                (unsigned)down, (unsigned)levels[level_index], (unsigned)allocation,
                (unsigned)valid, (unsigned)fixture.frame_accesses);
            passed = LIB_FALSE;
        }
    }
    return passed;
}

static lib_bool enter_leave_test_overlapping_chain(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 levels[] = {2u, 31u, 32u, 255u};
    const lib_u32 sentinel = 0xdeadc0deu;
    lib_u8 profile, operand32, stack32, level_index;
    lib_u16 slot;
    lib_bool passed = LIB_TRUE;
    for (profile = 0u; profile < 3u; ++profile)
    for (operand32 = 0u; operand32 < (profile == 2u ? 2u : 1u); ++operand32)
    for (stack32 = 0u; stack32 < (profile == 2u ? 2u : 1u); ++stack32)
    for (level_index = 0u; level_index < sizeof(levels); ++level_index) {
        const lib_u8 level = profile == 0u ? levels[level_index] : levels[level_index] & 0x1fu;
        const lib_u8 width = operand32 ? 4u : 2u;
        const lib_u16 pushes = (lib_u16)1u + level;
        cpu_instruction_fixture state;
        t_cpu after;
        lib_u8 code[] = {0x66u, 0xc8u, 0u, 0u, levels[level_index]};
        const lib_u8 offset = operand32 ? 0u : 1u;
        cpu_instruction_prepare(&state, profiles[profile]);
        enter_leave_seed(&state);
        state.cpu.data.esp = state.cpu.data.ebp = 0x4800u;
        state.cpu.data.ss.seg.data.big = stack32;
        for (slot = 0u; slot < pushes; ++slot)
            cpu_instruction_write(&state, 0x4800u - (slot + 1u) * width,
                &sentinel, width, CORE_MACHINE_CPU_MEMORY_ACCESS_DATA);
        if (cpu_instruction_run(&state, code + offset, sizeof(code) - offset,
                &after) != LIB_STATUS_OK || state.fault.valid ||
            after.data.esp != 0x4800u - pushes * width ||
            after.data.ebp != 0x4800u - width) passed = LIB_FALSE;
        for (slot = 0u; slot < pushes; ++slot) {
            lib_u32 observed = 0u;
            /* Each chain read sees the just-pushed prior slot, not its old
             * sentinel. Admission must never snapshot chain values early. */
            const lib_u32 expected = level && slot == level ? 0x4800u - width : 0x4800u;
            if (!enter_leave_read(&state, 0x4800u - (slot + 1u) * width,
                    width, &observed) || observed != expected) passed = LIB_FALSE;
        }
    }
    return passed;
}

lib_i32 main(void)
{
    if (!enter_leave_test_overlapping_chain()) return 1;
    if (!enter_leave_test_allocation_admission()) return 1;
    if (!enter_leave_test_ordered_failure()) return 1;
    if (!enter_leave_test_chain_attributes()) return 1;
    if (!enter_leave_test_defaults())
    {
        lib_c_printf("ENTER-LEAVE stage=defaults\n");
        return 1;
    }
    if (!enter_leave_test_attributes())
    {
        lib_c_printf("ENTER-LEAVE stage=attributes\n");
        return 1;
    }
    if (!enter_leave_test_rejections())
    {
        lib_c_printf("ENTER-LEAVE stage=rejections\n");
        return 1;
    }
    if (!enter_leave_test_protected_stack32())
    {
        lib_c_printf("ENTER-LEAVE stage=protected-stack32\n");
        return 1;
    }
    if (!enter_leave_test_protected_faults())
    {
        lib_c_printf("ENTER-LEAVE stage=protected-faults\n");
        return 1;
    }
    lib_c_printf("CPU:M5:T316:S43:ENTER-LEAVE:OK\n");
    lib_c_printf("CPU:M5:T401:S24:ENTER-LEAVE-PROFILES:OK\n");
    return 0;
}
