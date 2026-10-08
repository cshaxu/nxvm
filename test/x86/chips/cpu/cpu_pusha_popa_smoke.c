#include "support/cpu_stack_probe_fixture.h"
#include "lib/types/file.h"

/* REAL_UD_TERMINAL_CPU_OWNER: terminal-UD assertions stay CPU-owned. */
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
        failed |= cpu_instruction_write(&state, 0x7000u, image, sizeof(image),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||
            cpu_instruction_write(&state, 0x9000u, image, sizeof(image),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        failed |= !cpu_instruction_expect_real_fault(&state, code, bytes, 6u) ||
            cpu_instruction_read(&state, 0x7000u, &observed, sizeof(observed),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
            lib_memory_compare(observed, image, sizeof(image)) != 0 ||
            cpu_instruction_read(&state, 0x9000u, &observed, sizeof(observed),
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
    lib_u16 expected[] = {0xa001u, 0xa002u, 0xa003u, 0xa004u, 0xa005u};
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
        failed |= !core_machine_cpu_is_shutdown(&state.execution) ||
            state.execution.stop_requested;
        after = state.cpu;
        failed |= state.fault.valid || !state.delivered_exception.valid ||
            state.delivered_exception.exception_mask != VCPUINS_EXCEPT_SHUTDOWN ||
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
        failed |= !core_machine_cpu_is_shutdown(&state.execution) ||
            state.execution.stop_requested;
        after = state.cpu;
        failed |= state.fault.valid || !state.delivered_exception.valid ||
            state.delivered_exception.exception_mask != VCPUINS_EXCEPT_SHUTDOWN ||
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


static lib_bool pusha_popa_test_frame_admission(void)
{
    static const core_machine_cpu_bus_provider bus = {
        .read_memory = cpu_stack_probe_read,
        .write_memory = cpu_stack_probe_write,
        .interrupt_pending = cpu_instruction_interrupt_pending
    };
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_u8 profile, operand32, stack32, pop, invalid;
    lib_bool passed = LIB_TRUE;
    for (profile = 0u; profile < 2u; ++profile)
    for (operand32 = 0u; operand32 < (profile ? 2u : 1u); ++operand32)
    for (stack32 = 0u; stack32 < (profile ? 2u : 1u); ++stack32)
    for (pop = 0u; pop < 2u; ++pop)
    for (invalid = 0u; invalid < 2u; ++invalid) {
        cpu_stack_probe_fixture fixture;
        cpu_instruction_fixture *state = &fixture.instruction;
        lib_u8 code[] = {0x66u, pop ? 0x61u : 0x60u};
        const lib_u8 offset = operand32 ? 0u : 1u;
        t_cpu after;

        lib_memory_set(&fixture, 0, sizeof(fixture));
        cpu_instruction_prepare_with_bus(state, profiles[profile], &bus, &fixture);
        fixture.frame_base = 0xe0u;
        fixture.frame_size = 0x30u;
        fixture.reject_at = 1u;
        state->cpu.data.cr0 |= VCPU_CR0_PE;
        state->cpu.data.ss.selector = 0x10u;
        state->cpu.data.ss.seg.data.writable = LIB_TRUE;
        state->cpu.data.ss.seg.data.big = stack32;
        state->cpu.data.ss.seg.data.expdown = !pop;
        state->cpu.data.ss.limit = invalid ? (pop ? 0xfbu : 0xf4u) :
            (pop ? 0x1ffu : 0xbfu);
        state->cpu.data.esp = pop ? 0xf0u : 0x100u;
        /* No deliverable IDT: subsequent fault handling cannot legitimately
         * transfer the watched instruction frame. */
        state->cpu.data.idtr.limit = 0u;
        cpu_instruction_run(state, code + offset, sizeof(code) - offset, &after);
        if (fixture.frame_accesses != (invalid ? 0u : 1u)) {
            lib_c_printf("FRAME admission profile=%u operand32=%u stack32=%u pop=%u invalid=%u accesses=%u\n",
                (unsigned)profile, (unsigned)operand32, (unsigned)stack32,
                (unsigned)pop, (unsigned)invalid, (unsigned)fixture.frame_accesses);
            passed = LIB_FALSE;
        }
    }
    return passed;
}

static lib_bool pusha_popa_test_ordered_failure(void)
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
    lib_u8 profile, operand32, stack32, pop, failure, slot;
    lib_bool passed = LIB_TRUE;
    for (profile = 0u; profile < 3u; ++profile)
    for (operand32 = 0u; operand32 < (profile == 2u ? 2u : 1u); ++operand32)
    for (stack32 = 0u; stack32 < (profile == 2u ? 2u : 1u); ++stack32)
    for (pop = 0u; pop < 2u; ++pop)
    for (failure = 1u; failure <= 8u; ++failure) {
        cpu_stack_probe_fixture fixture;
        cpu_instruction_fixture *state = &fixture.instruction;
        t_cpu before, after;
        const lib_u8 width = operand32 ? 4u : 2u;
        const lib_u32 mask = operand32 ? 0xffffffffu : 0xffffu;
        const lib_u32 sentinel = 0xdeadc0deu;
        lib_u8 code[] = {0x66u, pop ? 0x61u : 0x60u};
        const lib_u8 offset = operand32 ? 0u : 1u;
        lib_u32 expected[8];

        lib_memory_set(&fixture, 0, sizeof(fixture));
        cpu_instruction_prepare_with_bus(state, profiles[profile], &bus, &fixture);
        pusha_popa_seed(state);
        state->cpu.data.esp = stack32 ? 0x4080u : 0x12344080u;
        state->cpu.data.ss.seg.data.big = stack32;
        before = state->cpu;
        expected[0] = before.data.eax; expected[1] = before.data.ecx;
        expected[2] = before.data.edx; expected[3] = before.data.ebx;
        expected[4] = operand32 ? before.data.esp : before.data.sp;
        expected[5] = before.data.ebp; expected[6] = before.data.esi;
        expected[7] = before.data.edi;
        for (slot = 0u; slot < 8u; ++slot) {
            lib_u32 value = pop ? 0x43210000u + slot : sentinel;
            lib_u32 address = pop ? 0x4080u + slot * width : 0x4080u - (slot + 1u) * width;
            cpu_instruction_write(state, address, &value, width,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA);
        }
        fixture.frame_base = 0x4000u;
        fixture.frame_size = 0x100u;
        fixture.reject_at = failure;
        if (cpu_instruction_run(state, code + offset, sizeof(code) - offset,
                &after) != LIB_STATUS_INTERNAL_ERROR ||
            !state->fault.valid || state->fault.exception_mask != VCPUINS_EXCEPT_CE ||
            fixture.frame_accesses != failure || !pusha_popa_cpu_same(&before, &after)) {
            lib_c_printf("ordered stack profile=%u operand32=%u stack32=%u pop=%u failure=%u count=%u\n",
                (unsigned)profile, (unsigned)operand32, (unsigned)stack32,
                (unsigned)pop, (unsigned)failure, (unsigned)fixture.frame_accesses);
            passed = LIB_FALSE;
        }
        for (slot = 0u; slot < 8u; ++slot) {
            lib_u32 observed = 0u;
            const lib_u32 address = pop ? 0x4080u + slot * width :
                0x4080u - (slot + 1u) * width;
            const lib_u32 value = pop ? 0x43210000u + slot :
                (slot + 1u < failure ? expected[slot] : sentinel);
            if (cpu_instruction_read(state, address, &observed, width,
                    CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
                    LIB_STATUS_OK || observed != (value & mask)) passed = LIB_FALSE;
        }
    }
    return passed;
}

static lib_bool pusha_popa_test_initial_sp(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    const lib_u8 code[] = {0x60u};
    const lib_u8 vector[] = {0u, 2u, 0u, 0u};
    lib_u8 profile, stack;
    lib_bool passed = LIB_TRUE;

    for (profile = 0u; profile < 2u; ++profile)
    for (stack = 1u; stack < 16u; stack += 2u) {
        cpu_instruction_fixture state;
        t_cpu after;
        const lib_bool shutdown = stack <= 5u;
        cpu_instruction_prepare(&state, profiles[profile]);
        pusha_popa_seed(&state);
        state.cpu.data.esp = stack;
        cpu_instruction_write(&state, 13u * 4u, vector, sizeof(vector),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA);
        cpu_instruction_run(&state, code, sizeof(code), &after);
        if (!state.delivered_exception.valid ||
            state.delivered_exception.exception_mask != (shutdown ?
                VCPUINS_EXCEPT_SHUTDOWN : VCPUINS_EXCEPT_GP) ||
            core_machine_cpu_is_shutdown(&state.execution) != shutdown ||
            (shutdown && state.execution.stop_requested) ||
            after.data.eax != 0xa1a23344u || after.data.ecx != 0xb1b25566u ||
            (!shutdown && after.data.eip != 0x200u)) {
            lib_c_printf("PUSHA initial profile=%u sp=%u delivered=%x shutdown=%u stop=%u\n",
                (unsigned)profile, (unsigned)stack,
                (unsigned)state.delivered_exception.exception_mask,
                (unsigned)state.execution.shutdown_requested,
                (unsigned)state.execution.stop_requested);
            passed = LIB_FALSE;
        }
    }
    return passed;
}

typedef struct pusha_vm_fixture {
    cpu_instruction_fixture instruction;
    lib_u32 first_idt;
} pusha_vm_fixture;

static lib_status pusha_vm_read(void *opaque, lib_u32 address, void *destination,
    lib_u8 bytes, core_machine_cpu_memory_access_provenance provenance,
    lib_bool observe_only, lib_bool reset_fetch)
{
    pusha_vm_fixture *fixture = opaque;
    if (!observe_only && address >= 0x3000u && address < 0x3800u &&
        fixture->first_idt == 0u) fixture->first_idt = address;
    return cpu_instruction_read(&fixture->instruction, address, destination,
        bytes, provenance, observe_only, reset_fetch);
}

static lib_bool pusha_popa_test_vm_initial_sp(void)
{
    static const core_machine_cpu_bus_provider bus = {
        .read_memory = pusha_vm_read,
        .write_memory = cpu_instruction_write,
        .interrupt_pending = cpu_instruction_interrupt_pending
    };
    lib_u8 operand32, stack;
    lib_bool passed = LIB_TRUE;
    for (operand32 = 0u; operand32 < 2u; ++operand32)
    for (stack = 1u; stack < 16u; stack += 2u) {
        pusha_vm_fixture fixture;
        cpu_instruction_fixture *state = &fixture.instruction;
        lib_u8 code[] = {0x66u, 0x60u};
        const lib_u8 offset = operand32 ? 0u : 1u;
        t_cpu after;
        lib_memory_set(&fixture, 0, sizeof(fixture));
        cpu_instruction_prepare_with_bus(state, CORE_MACHINE_CPU_PROFILE_80386,
            &bus, &fixture);
        state->cpu.data.cr0 |= VCPU_CR0_PE;
        state->cpu.data.eflags |= VCPU_EFLAGS_VM;
        state->cpu.data.esp = stack;
        state->cpu.data.idtr.base = 0x3000u;
        state->cpu.data.idtr.limit = 0x7ffu;
        cpu_instruction_run(state, code + offset, sizeof(code) - offset, &after);
        if (stack <= 5u ? (!state->execution.shutdown_requested ||
                state->delivered_exception.exception_mask != VCPUINS_EXCEPT_SHUTDOWN ||
                fixture.first_idt != 0u) : fixture.first_idt != 0x3068u) {
            lib_c_printf("PUSHA VM operand32=%u sp=%u first-idt=%x shutdown=%u\n",
                (unsigned)operand32, (unsigned)stack, (unsigned)fixture.first_idt,
                (unsigned)state->execution.shutdown_requested);
            passed = LIB_FALSE;
        }
    }
    return passed;
}

lib_i32 main(void)
{
    if (!pusha_popa_test_vm_initial_sp()) return 1;
    if (!pusha_popa_test_ordered_failure()) return 1;
    if (!pusha_popa_test_initial_sp()) return 1;
    if (!pusha_popa_test_frame_admission()) return 1;
    if (!pusha_popa_test_protected_pusha_limit())
    {
        lib_c_printf("CPU stack cache stage=test_protected_pusha_limit\n");
        return 1;
    }
    if (!pusha_popa_test_protected_popa_limit())
    {
        lib_c_printf("CPU stack cache stage=test_protected_popa_limit\n");
        return 1;
    }
    if (!pusha_popa_test_defaults())
    {
        lib_c_printf("PUSHA-POPA stage=defaults\n");
        return 1;
    }
    if (!pusha_popa_test_attributes())
    {
        lib_c_printf("PUSHA-POPA stage=attributes\n");
        return 1;
    }
    if (!pusha_popa_test_rejections())
    {
        lib_c_printf("PUSHA-POPA stage=rejections\n");
        return 1;
    }
    lib_c_printf("PUSHA-POPA:OK\n");
    lib_c_printf("PUSHA-POPA-PROFILES:OK\n");
    return 0;
}
