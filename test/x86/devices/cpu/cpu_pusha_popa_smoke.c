#include "support/cpu_instruction_fixture.h"
#include <stdio.h>

/* T337_REAL_UD_TERMINAL_CPU_OWNER: terminal-UD assertions stay CPU-owned. */
static void pusha_popa_seed(cpu_instruction_fixture *state)
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

static lib_i32 pusha_popa_sregs_same(const t_cpu *before, const t_cpu *after)
{
    return lib_memory_compare(&before->data.es, &after->data.es,
        sizeof(before->data.es)) == 0 && lib_memory_compare(&before->data.cs,
        &after->data.cs, sizeof(before->data.cs)) == 0 && lib_memory_compare(
        &before->data.ss, &after->data.ss, sizeof(before->data.ss)) == 0 &&
        lib_memory_compare(&before->data.ds, &after->data.ds, sizeof(before->data.ds)) ==
        0 && lib_memory_compare(&before->data.fs, &after->data.fs,
        sizeof(before->data.fs)) == 0 && lib_memory_compare(&before->data.gs,
        &after->data.gs, sizeof(before->data.gs)) == 0;
}

static lib_i32 pusha_popa_cpu_same(const t_cpu *before, const t_cpu *after)
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

static lib_i32 pusha_popa_read_image(cpu_instruction_fixture *state, lib_u32 address,
    lib_u8 width, lib_u32 *value)
{
    *value = 0u;
    return cpu_instruction_read(state, address, &*value, width,
        CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) == LIB_STATUS_OK;
}

static lib_i32 pusha_popa_expect_push_image(cpu_instruction_fixture *state,
    const t_cpu *before, lib_u32 stack, lib_u8 width)
{
    const lib_u32 expected[] = {
        before->data.edi, before->data.esi, before->data.ebp,
        width == 2u ? before->data.sp : before->data.esp,
        before->data.ebx, before->data.edx, before->data.ecx, before->data.eax
    };
    lib_u8 slot;

    for (slot = 0u; slot != sizeof(expected) / sizeof(expected[0]); ++slot)
    {
        lib_u32 image;

        if (!pusha_popa_read_image(state, stack + slot * width, width, &image) ||
            image != (width == 2u ? (expected[slot] & 0xffffu) : expected[slot]))
            return 0;
    }
    return 1;
}

static lib_i32 pusha_popa_test_pusha_success(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_u8 width)
{
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after;
    lib_u32 stack = 0x8000u - 8u * width;
    lib_u32 sentinel = 0xdeadbeefu;
    lib_u8 slot;
    lib_i32 failed = 0;
    cpu_instruction_prepare(&state, profile);

    if (!failed)
    {
        pusha_popa_seed(&state);
        for (slot = 0u; slot != 8u; ++slot)
            failed |= cpu_instruction_write(&state, stack + slot * width, &sentinel, width,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        before = state.cpu;
        failed |= cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK || state.fault.valid ||
            after.data.eip != bytes || after.data.eax != before.data.eax ||
            after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx || after.data.ebp != before.data.ebp ||
            after.data.esi != before.data.esi || after.data.edi != before.data.edi ||
            after.data.eflags != before.data.eflags || after.data.esp !=
            ((before.data.esp & 0xffff0000u) | (lib_u16)(before.data.sp -
            8u * width)) || !pusha_popa_sregs_same(&before, &after) ||
            !pusha_popa_expect_push_image(&state, &before, stack, width);
    }
    return !failed;
}

static lib_i32 pusha_popa_test_popa_success(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_u8 width)
{
    static const lib_u32 image[] = {0x0102a5a5u, 0x0304ddefu, 0x0506bbcdu,
        0xdeadbeefu, 0x070899aau, 0x090a7788u, 0x0b0c5566u, 0x0d0e3344u};
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after;
    lib_u8 slot;
    lib_i32 failed = 0;
    cpu_instruction_prepare(&state, profile);

    if (!failed)
    {
        pusha_popa_seed(&state);
        for (slot = 0u; slot != 8u; ++slot)
            failed |= cpu_instruction_write(&state, 0x8000u + slot * width, &image[slot], width,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        before = state.cpu;
        failed |= cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK || state.fault.valid ||
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
            ((before.data.esp & 0xffff0000u) | (lib_u16)(before.data.sp +
            8u * width)) || !pusha_popa_sregs_same(&before, &after);
        for (slot = 0u; !failed && slot != 8u; ++slot)
        {
            lib_u32 observed;

            failed |= !pusha_popa_read_image(&state, 0x8000u + slot * width,
                width, &observed) || observed != (width == 2u ?
                (image[slot] & 0xffffu) : image[slot]);
        }
    }
    return !failed;
}

static lib_i32 pusha_popa_test_defaults(void)
{
    static const core_machine_cpu_profile supported[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386};
    static const lib_u8 pusha[] = {0x60u};
    static const lib_u8 popa[] = {0x61u};
    lib_u8 profile;

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

static lib_i32 pusha_popa_test_attributes(void)
{
    static const lib_u8 pusha32[] = {0x66u, 0x60u};
    static const lib_u8 popa32[] = {0x66u, 0x61u};
    static const lib_u8 pusha67[] = {0x67u, 0x60u};
    static const lib_u8 popa67[] = {0x67u, 0x61u};
    static const lib_u8 pusha32_67[] = {0x66u, 0x67u, 0x60u};
    static const lib_u8 popa32_67[] = {0x66u, 0x67u, 0x61u};

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

static lib_i32 pusha_popa_test_reject_case(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after;
    static const lib_u8 image[32] = {
        0xa5u, 0xb6u, 0xc7u, 0xd8u, 0xe9u, 0xfau, 0x0bu, 0x1cu,
        0x2du, 0x3eu, 0x4fu, 0x50u, 0x61u, 0x72u, 0x83u, 0x94u,
        0x95u, 0x86u, 0x77u, 0x68u, 0x59u, 0x4au, 0x3bu, 0x2cu,
        0x1du, 0x0eu, 0xffu, 0xeeu, 0xddu, 0xccu, 0xbbu, 0xaau
    };
    lib_u8 observed[sizeof(image)];
    lib_i32 failed = 0;
    cpu_instruction_prepare(&state, profile);

    if (!failed)
    {
        pusha_popa_seed(&state);
        failed |= cpu_instruction_write(&state, 0x7fe0u, image, sizeof(image),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||

            cpu_instruction_write(&state, 0x8000u, image, sizeof(image),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        state.cpu.data.idtr.limit = 0x17u;
        before = state.cpu;
        failed |= cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_INTERNAL_ERROR || !state.fault.valid ||
            !X86_CPU_BIT_IS_SET(state.fault.exception_mask,
            VCPUINS_EXCEPT_UD) || !pusha_popa_cpu_same(&before, &after) ||
            cpu_instruction_read(&state, 0x7fe0u, &observed, sizeof(observed),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
            lib_memory_compare(observed, image, sizeof(image)) != 0 ||
            cpu_instruction_read(&state, 0x8000u, &observed, sizeof(observed),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
            lib_memory_compare(observed, image, sizeof(image)) != 0;
    }
    return !failed;
}

static lib_i32 pusha_popa_test_rejections(void)
{
    static const lib_u8 default_codes[][1] = {{0x60u}, {0x61u}};
    static const lib_u8 attribute_codes[][3] = {
        {0x66u, 0x60u, 0u}, {0x66u, 0x61u, 0u},
        {0x67u, 0x60u, 0u}, {0x67u, 0x61u, 0u},
        {0x66u, 0x67u, 0x60u}, {0x66u, 0x67u, 0x61u}
    };
    static const lib_u8 attribute_bytes[] = {2u, 2u, 2u, 2u, 3u, 3u};
    static const lib_u8 lock_codes[][4] = {
        {0xf0u, 0x60u, 0u, 0u}, {0xf0u, 0x61u, 0u, 0u},
        {0xf0u, 0x66u, 0x60u, 0u}, {0xf0u, 0x66u, 0x61u, 0u},
        {0xf0u, 0x67u, 0x60u, 0u}, {0xf0u, 0x67u, 0x61u, 0u},
        {0xf0u, 0x66u, 0x67u, 0x60u}, {0xf0u, 0x66u, 0x67u, 0x61u}
    };
    static const lib_u8 lock_bytes[] = {2u, 2u, 3u, 3u, 3u, 3u, 4u, 4u};
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286};
    lib_u8 profile;
    lib_u8 form;

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

static lib_i32 pusha_popa_boot_protected(cpu_instruction_fixture *state)
{
    static const lib_u8 pointer[] = {0x1fu, 0u, 0u, 0x03u, 0u, 0u};
    static const lib_u8 gdt[] = {
        0u,0u,0u,0u,0u,0u,0u,0u,
        0xffu,0xffu,0u,0x20u,0u,0x9au,0u,0u,
        0xffu,0xffu,0u,0x30u,0u,0x92u,0u,0u,
        0xffu,0xffu,0u,0x40u,0u,0x92u,0u,0u
    };
    static const lib_u8 bootstrap[] = {
        0x0fu,0x01u,0x16u,0x00u,0x01u,0xb8u,0x01u,0x00u,0x0fu,0x01u,0xf0u,
        0xb8u,0x10u,0x00u,0x8eu,0xd8u,0x8eu,0xc0u,0xb8u,0x18u,0x00u,0x8eu,
        0xd0u,0xbcu,0x00u,0x80u,0xeau,0x00u,0x00u,0x08u,0x00u
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

static lib_i32 pusha_popa_test_protected_pusha_limit(void)
{
    static const lib_u8 code[] = {0x60u};
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after;
    lib_u16 expected[] = {0xa5a5u, 0x99aau, 0x7788u, 0x5566u, 0x3344u};
    lib_u8 slot;
    lib_i32 failed = 0;
    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed)
        failed |= !pusha_popa_boot_protected(&state);
    if (!failed)
    {
        pusha_popa_seed(&state);
        state.cpu.data.ss.limit = 0x18u;
        state.cpu.data.ss.seg.data.expdown = LIB_TRUE;
        state.cpu.data.esp = 0x12340022u;
        failed |= cpu_instruction_write(&state, 0x4018u, expected, sizeof(expected),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||

            cpu_instruction_write(&state, 0x2000u, code, sizeof(code),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        before = state.cpu;
        state.cpu.data.eip = 0u;
        core_machine_cpu_execution_refresh(&state.execution);
        failed |= !state.execution.stop_requested;
        after = state.cpu;
        failed |= !state.fault.valid || !X86_CPU_BIT_IS_SET(
            state.fault.exception_mask, VCPUINS_EXCEPT_DF) ||
            after.data.eip != 0u || after.data.eax != before.data.eax ||
            after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx || after.data.ebp != before.data.ebp ||
            after.data.esi != before.data.esi || after.data.edi != before.data.edi ||
            after.data.eflags != before.data.eflags || after.data.esp != before.data.esp ||
            !pusha_popa_sregs_same(&before, &after);
        for (slot = 0u; !failed && slot != sizeof(expected) / sizeof(expected[0]); ++slot)
        {
            lib_u32 value;

            failed |= !pusha_popa_read_image(&state, 0x4018u + slot * 2u, 2u,
                &value) || value != expected[slot];
        }
    }
    return !failed;
}

static lib_i32 pusha_popa_test_protected_popa_limit(void)
{
    static const lib_u8 code[] = {0x61u};
    static const lib_u16 image[] = {0x1111u, 0x2222u, 0x3333u, 0x4444u,
        0x5555u, 0x6666u, 0x7777u, 0x8888u};
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after;
    lib_u8 slot;
    lib_i32 failed = 0;
    cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed)
        failed |= !pusha_popa_boot_protected(&state);
    if (!failed)
    {
        pusha_popa_seed(&state);
        state.cpu.data.ss.limit = 0x1fu;
        state.cpu.data.esp = 0x12340018u;
        failed |= cpu_instruction_write(&state, 0x4018u, image, sizeof(image),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||

            cpu_instruction_write(&state, 0x2000u, code, sizeof(code),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        before = state.cpu;
        state.cpu.data.eip = 0u;
        core_machine_cpu_execution_refresh(&state.execution);
        failed |= !state.execution.stop_requested;
        after = state.cpu;
        failed |= !state.fault.valid || !X86_CPU_BIT_IS_SET(
            state.fault.exception_mask, VCPUINS_EXCEPT_DF) ||
            after.data.eip != 0u || after.data.eax != before.data.eax ||
            after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx || after.data.esp != before.data.esp ||
            after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi ||
            after.data.eflags != before.data.eflags || !pusha_popa_sregs_same(
            &before, &after);
        for (slot = 0u; !failed && slot != sizeof(image) / sizeof(image[0]); ++slot)
        {
            lib_u32 value;

            failed |= !pusha_popa_read_image(&state, 0x4018u + slot * 2u, 2u,
                &value) || value != image[slot];
        }
    }
    return !failed;
}

lib_i32 main(void)
{
    if (!pusha_popa_test_protected_pusha_limit())
    {
        printf("CPU stack cache stage=test_protected_pusha_limit\n");
        return 1;
    }
    if (!pusha_popa_test_protected_popa_limit())
    {
        printf("CPU stack cache stage=test_protected_popa_limit\n");
        return 1;
    }
    if (!pusha_popa_test_defaults())
    {
        printf("PUSHA-POPA stage=defaults\n");
        return 1;
    }
    if (!pusha_popa_test_attributes())
    {
        printf("PUSHA-POPA stage=attributes\n");
        return 1;
    }
    if (!pusha_popa_test_rejections())
    {
        printf("PUSHA-POPA stage=rejections\n");
        return 1;
    }
    printf("CPU:M5:T316:S42:PUSHA-POPA:OK\n");
    printf("CPU:M5:T401:S31:PUSHA-POPA-PROFILES:OK\n");
    return 0;
}
