#include "support/cpu_instruction_fixture.h"
#include <stdio.h>

/* T337_REAL_UD_TERMINAL_CPU_OWNER: terminal-UD assertions stay CPU-owned. */
static void gpr_push_pop_seed(cpu_instruction_fixture *state)
{
    t_cpu *cpu = &state->cpu;

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

static lib_i32 gpr_push_pop_sregs_same(const t_cpu *before, const t_cpu *after)
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

static lib_i32 gpr_push_pop_cpu_same(const t_cpu *before, const t_cpu *after)
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

static lib_u32 gpr_push_pop_register(const t_cpu *cpu, lib_u8 index)
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

static lib_i32 gpr_push_pop_test_push_registers(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
         ++profile)
    {
        lib_u8 index;

        for (index = 0u; index != 8u; ++index)
        {
            const lib_u8 code[] = {(lib_u8)(0x50u + index)};
            cpu_instruction_fixture state;
            t_cpu before;
            t_cpu after;
            lib_u32 image = 0u;
            lib_u32 expected;
            lib_i32 failed = 0;
            cpu_instruction_prepare(&state, profiles[profile]);

            if (!failed)
            {
                gpr_push_pop_seed(&state);
                before = state.cpu;
                failed |= cpu_instruction_run(&state, code, sizeof(code), &after) != LIB_STATUS_OK ||
                    state.fault.valid;
                expected = gpr_push_pop_register(&before, index) & 0xffffu;
                if (index == 4u && profiles[profile] < CORE_MACHINE_CPU_PROFILE_80286)
                    expected = 0x7ffeu;
                failed |= cpu_instruction_read(&state, 0x7ffeu, &image, 2u,
                    CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
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
            if (failed)
                return 0;
        }
    }
    return 1;
}

static lib_i32 gpr_push_pop_test_pop_esp_address(void)
{
    static const lib_u8 code[] = {0x67u, 0x8fu, 0x44u, 0x24u, 0x04u};
    cpu_instruction_fixture state;
    t_cpu after;
    lib_u16 stack_value = 0xfaceu;
    lib_u16 old_target = 0xbeefu;
    lib_u16 observed = 0u;
    lib_i32 failed = 0;
    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed)
    {
        gpr_push_pop_seed(&state);
        state.cpu.data.esp = 0x00008000u;
        failed |= cpu_instruction_write(&state, 0x8000u, &stack_value, sizeof(stack_value),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            cpu_instruction_write(&state, 0x8006u, &old_target, sizeof(old_target),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||

                cpu_instruction_run(&state, code, sizeof(code), &after) != LIB_STATUS_OK ||
                state.fault.valid ||
            after.data.eip != sizeof(code) || after.data.esp != 0x00008002u ||
            cpu_instruction_read(&state, 0x8006u, &observed, sizeof(observed),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
            LIB_STATUS_OK || observed != stack_value;
    }
    return !failed;
}

static lib_i32 gpr_push_pop_test_pop_registers(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
         ++profile)
    {
        lib_u8 index;

        for (index = 0u; index != 8u; ++index)
        {
            const lib_u8 code[] = {(lib_u8)(0x58u + index)};
            cpu_instruction_fixture state;
            t_cpu before;
            t_cpu after;
            lib_u16 value = (lib_u16)(0x4100u + index);
            lib_u32 expected;
            lib_i32 failed = 0;
            cpu_instruction_prepare(&state, profiles[profile]);

            if (!failed)
            {
                gpr_push_pop_seed(&state);
                failed |= cpu_instruction_write(&state, 0x8000u, &value, sizeof(value),
                    CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
                before = state.cpu;
                failed |= cpu_instruction_run(&state, code, sizeof(code), &after) != LIB_STATUS_OK ||
                    state.fault.valid || after.data.eip != 1u ||
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
            if (failed)
                return 0;
        }
    }
    return 1;
}

static lib_i32 gpr_push_pop_test_rm_forms(void)
{
    static const lib_u8 push_reg[] = {0xffu, 0xf0u};
    static const lib_u8 pop_reg[] = {0x8fu, 0xc1u};
    static const lib_u8 push_ds[] = {0xffu, 0x36u, 0x20u, 0x00u};
    static const lib_u8 push_ss[] = {0xffu, 0x76u, 0x00u};
    static const lib_u8 pop_ds[] = {0x8fu, 0x06u, 0x20u, 0x00u};
    static const lib_u8 pop_ss[] = {0x8fu, 0x46u, 0x00u};
    static const lib_u8 push_67[] = {0x67u, 0xffu, 0x35u, 0x20u, 0x00u,
        0x00u, 0x00u};
    static const lib_u8 pop_67[] = {0x67u, 0x8fu, 0x05u, 0x20u, 0x00u,
        0x00u, 0x00u};
    const lib_u8 *codes[] = {push_reg, pop_reg, push_ds, push_ss, pop_ds,
        pop_ss, push_67, pop_67};
    const lib_u8 bytes[] = {2u, 2u, 4u, 3u, 4u, 3u, 7u, 7u};
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_u8 profile;
    lib_u8 form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile)
    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form)
    {
        if (form >= 6u && profiles[profile] != CORE_MACHINE_CPU_PROFILE_80386) continue;
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        lib_u32 source = 0xface7788u;
        lib_u32 image = 0xface7788u;
        lib_u32 observed = 0u;
        lib_u32 address = 0x20u;
        lib_i32 push = form == 0u || form == 2u || form == 3u || form == 6u;
        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, profiles[profile]);

        if (!failed)
        {
            gpr_push_pop_seed(&state);
            state.cpu.data.bp = 0x20u;
            if (form == 3u || form == 5u)
                address = 0x20u;
            if (form == 6u || form == 7u)
                state.cpu.data.esp = 0x00008000u;
            if (push && form != 0u)
                failed |= cpu_instruction_write(&state, address, &source, 2u,
                    CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            if (!push)
                failed |= cpu_instruction_write(&state, 0x8000u, &image, 2u,
                    CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            before = state.cpu;
            failed |= cpu_instruction_run(&state, codes[form], bytes[form], &after) != LIB_STATUS_OK ||
                state.fault.valid || after.data.eip != bytes[form] ||
                after.data.eflags != before.data.eflags ||
                !gpr_push_pop_sregs_same(&before, &after);
            if (push)
            {
                failed |= after.data.esp != ((before.data.esp & 0xffff0000u) |
                    ((before.data.sp - 2u) & 0xffffu));
                failed |= cpu_instruction_read(&state, 0x7ffeu, &observed, 2u,
                    CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
                    observed != (form == 0u ? (before.data.eax & 0xffffu) :
                    (source & 0xffffu));
            }
            else
            {
                failed |= after.data.esp != ((before.data.esp & 0xffff0000u) |
                    0x8002u);
                if (form != 1u)
                    failed |= cpu_instruction_read(&state, address, &observed, 2u,
                        CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
                        observed != (image & 0xffffu);
            }
            failed |= after.data.eax != before.data.eax || after.data.edx !=
                before.data.edx || after.data.ebx != before.data.ebx ||
                after.data.ebp != before.data.ebp || after.data.esi !=
                before.data.esi || after.data.edi != before.data.edi ||
                after.data.ecx != (form == 1u ? ((before.data.ecx & 0xffff0000u) |
                (image & 0xffffu)) : before.data.ecx);
        }
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 gpr_push_pop_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after;
    lib_u32 source = 0xface7788u;
    lib_u32 image = 0u;
    lib_i32 failed = 0;
    cpu_instruction_prepare(&state, profile);

    if (!failed)
    {
        gpr_push_pop_seed(&state);
        failed |= cpu_instruction_write(&state, 0x20u, &source, sizeof(source),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||

            cpu_instruction_write(&state, 0x7ffcu, &source, sizeof(source),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        state.cpu.data.idtr.limit = 0x17u;
        before = state.cpu;
        failed |= cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_INTERNAL_ERROR || !state.fault.valid ||
            !X86_CPU_BIT_IS_SET(state.fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            !gpr_push_pop_cpu_same(&before, &after) ||
                cpu_instruction_read(&state, 0x20u, &image, sizeof(image),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
                image != source ||
            cpu_instruction_read(&state, 0x7ffcu, &image, sizeof(image),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
            image != source;
    }
    return !failed;
}

static lib_i32 gpr_push_pop_test_rejections(void)
{
    static const lib_u8 attrs[][3] = {{0x66u, 0x50u}, {0x67u, 0x50u},
        {0x66u, 0x67u, 0x50u}};
    static const lib_u8 locks[][4] = {{0xf0u, 0x50u}, {0xf0u, 0x58u},
        {0xf0u, 0xffu, 0xf0u}, {0xf0u, 0xffu, 0x36u, 0x20u},
        {0xf0u, 0x8fu, 0xc1u}, {0xf0u, 0x8fu, 0x06u, 0x20u}};
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    lib_u8 profile;
    lib_u8 form;

    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]); ++profile)
    {
        for (form = 0u; form != 3u; ++form)
        {
            lib_u8 bytes = form == 2u ? 3u : 2u;

            if (!gpr_push_pop_expect_ud(legacy[profile], attrs[form], bytes))
                return 0;
        }
    }
    for (form = 0u; form != sizeof(locks) / sizeof(locks[0]); ++form)
    {
        lib_u8 bytes = form == 0u || form == 1u ? 2u :
            (form == 3u || form == 5u ? 4u : 3u);

        if (!gpr_push_pop_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, locks[form],
            bytes))
            return 0;
    }
    for (form = 1u; form != 8u; ++form)
    {
        lib_u8 code[] = {0x8fu, (lib_u8)(0xc0u | (form << 3))};

        if (!gpr_push_pop_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, code,
            sizeof(code)))
            return 0;
    }
    return 1;
}

static lib_i32 gpr_push_pop_test_386_attributes(void)
{
    static const lib_u8 push32[] = {0x66u, 0x50u};
    static const lib_u8 pop32[] = {0x66u, 0x59u};
    static const lib_u8 push67[] = {0x67u, 0xffu, 0x35u, 0x20u, 0x00u,
        0x00u, 0x00u};
    static const lib_u8 pop66_67[] = {0x66u, 0x67u, 0x8fu, 0x05u, 0x20u,
        0x00u, 0x00u, 0x00u};
    const lib_u8 *codes[] = {push32, pop32, push67, pop66_67};
    const lib_u8 bytes[] = {2u, 2u, 7u, 8u};
    lib_u8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form)
    {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        lib_u32 value = 0xface7788u;
        lib_u32 observed = 0u;
        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed)
        {
            gpr_push_pop_seed(&state);
            if (form == 2u)
                failed |= cpu_instruction_write(&state, 0x20u, &value, sizeof(value),
                    CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            if (form == 1u || form == 3u)
                failed |= cpu_instruction_write(&state, 0x8000u, &value, sizeof(value),
                    CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            before = state.cpu;
            failed |= cpu_instruction_run(&state, codes[form], bytes[form], &after) != LIB_STATUS_OK ||
                state.fault.valid || after.data.eip != bytes[form] ||
                after.data.eflags != before.data.eflags ||
                !gpr_push_pop_sregs_same(&before, &after);
            if (form == 0u || form == 2u)
            {
                lib_u32 expected = form == 0u ? before.data.eax : value;
                lib_u8 width = form == 0u ? 4u : 2u;
                lib_u32 stack = 0x8000u - width;

                failed |= after.data.esp != ((before.data.esp & 0xffff0000u) |
                    stack) ||
                    cpu_instruction_read(&state, stack, &observed, width,
                        CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                    LIB_STATUS_OK || observed != (width == 2u ?
                    (expected & 0xffffu) : expected);
            }
            else if (form == 1u)
                failed |= after.data.eax != before.data.eax ||
                    after.data.ecx != value || after.data.esp != 0x12348004u;
            else
            {
                failed |= after.data.esp != 0x12348004u ||
                    cpu_instruction_read(&state, 0x20u, &observed, sizeof(observed),
                        CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                    LIB_STATUS_OK || observed != value;
            }
        }
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 gpr_push_pop_boot_protected(cpu_instruction_fixture *state)
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

static lib_i32 gpr_push_pop_protected_fault(const lib_u8 *code, lib_u8 bytes,
    lib_u8 limit_segment, lib_u32 limit, lib_i32 expdown)
{
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after;
    lib_status fault_status;
    lib_u32 sentinel = 0xdeadbeefu;
    lib_u32 observed = 0u;
    lib_i32 failed = 0;
    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed)
        failed |= !gpr_push_pop_boot_protected(&state);
    if (!failed)
    {
        gpr_push_pop_seed(&state);
        state.cpu.data.esp = 0x12348000u;
        if (limit_segment == 0u)
        {
            state.cpu.data.ss.limit = limit;
            state.cpu.data.ss.seg.data.expdown = expdown;
        }
        else
            state.cpu.data.ds.limit = limit;
        failed |= cpu_instruction_write(&state, 0x3010u, &sentinel, sizeof(sentinel),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||

            cpu_instruction_write(&state, 0x4010u, &sentinel, sizeof(sentinel),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            cpu_instruction_write(&state, 0x2000u, code, bytes,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        before = state.cpu;
        state.cpu.data.eip = 0u;
        core_machine_cpu_execution_refresh(&state.execution);
        fault_status = state.execution.stop_requested ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK;
        failed |= fault_status != LIB_STATUS_INTERNAL_ERROR;
        after = state.cpu;
        failed |= !state.fault.valid || !X86_CPU_BIT_IS_SET(
            state.fault.exception_mask, VCPUINS_EXCEPT_DF) ||
            after.data.eip != 0u || after.data.eax != before.data.eax ||
            after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx || after.data.esp != before.data.esp ||
            after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi || after.data.eflags !=
            before.data.eflags || !gpr_push_pop_sregs_same(&before, &after) ||
            cpu_instruction_read(&state, 0x3010u, &observed, sizeof(observed),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
                observed != sentinel ||
            cpu_instruction_read(&state, 0x4010u, &observed, sizeof(observed),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
            LIB_STATUS_OK || observed != sentinel;
    }
    return !failed;
}

static lib_i32 gpr_push_pop_test_protected_faults(void)
{
    static const lib_u8 push[] = {0x50u};
    static const lib_u8 pop[] = {0x59u};
    static const lib_u8 push_source[] = {0xffu, 0x36u, 0x10u, 0x00u};
    static const lib_u8 pop_dest[] = {0x8fu, 0x06u, 0x10u, 0x00u};

    if (!gpr_push_pop_protected_fault(push, sizeof(push), 0u, 0xffffu,
        LIB_TRUE))
    {
        printf("protected push\n");
        return 0;
    }
    if (!gpr_push_pop_protected_fault(pop, sizeof(pop), 0u, 0x7fffu,
        LIB_FALSE))
    {
        printf("protected pop\n");
        return 0;
    }
    if (!gpr_push_pop_protected_fault(push_source, sizeof(push_source), 1u,
        0x0fu, LIB_FALSE))
    {
        printf("protected push-source\n");
        return 0;
    }
    if (!gpr_push_pop_protected_fault(pop_dest, sizeof(pop_dest), 1u, 0x0fu,
        LIB_FALSE))
    {
        printf("protected pop-dest\n");
        return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!gpr_push_pop_test_protected_faults())
    {
        printf("CPU stack cache stage=test_protected_faults\n");
        return 1;
    }
    if (!gpr_push_pop_test_push_registers())
    {
        printf("GPR-PUSH-POP stage=push-registers\n");
        return 1;
    }
    if (!gpr_push_pop_test_pop_esp_address())
    {
        printf("GPR-PUSH-POP stage=pop-esp-address\n");
        return 1;
    }
    if (!gpr_push_pop_test_pop_registers())
    {
        printf("GPR-PUSH-POP stage=pop-registers\n");
        return 1;
    }
    if (!gpr_push_pop_test_rm_forms())
    {
        printf("GPR-PUSH-POP stage=rm\n");
        return 1;
    }
    if (!gpr_push_pop_test_rejections())
    {
        printf("GPR-PUSH-POP stage=rejections\n");
        return 1;
    }
    if (!gpr_push_pop_test_386_attributes())
    {
        printf("GPR-PUSH-POP stage=attributes\n");
        return 1;
    }
    printf("CPU:M5:T316:S44:GPR-PUSH-POP:OK\n");
    printf("CPU:M5:T401:S40:GPR-PUSH-POP-PROFILES:OK\n");
    printf("CPU:M5:T401:S10:GROUP5-PUSH-RM-PROFILES:OK\n");
    return 0;
}
