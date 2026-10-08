#include "support/cpu_instruction_fixture.h"
#include "lib/types/file.h"

/* REAL_UD_TERMINAL_CPU_OWNER: UD and shutdown remain distinct CPU proofs. */
static void push_immediate_seed(cpu_instruction_fixture *state)
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

static lib_i32 push_immediate_sregs_same(const t_cpu *before, const t_cpu *after)
{
    return lib_memory_compare(&before->data.es, &after->data.es, sizeof(before->data.es)) == 0 &&
        lib_memory_compare(&before->data.cs, &after->data.cs, sizeof(before->data.cs)) == 0 &&
        lib_memory_compare(&before->data.ss, &after->data.ss, sizeof(before->data.ss)) == 0 &&
        lib_memory_compare(&before->data.ds, &after->data.ds, sizeof(before->data.ds)) == 0 &&
        lib_memory_compare(&before->data.fs, &after->data.fs, sizeof(before->data.fs)) == 0 &&
        lib_memory_compare(&before->data.gs, &after->data.gs, sizeof(before->data.gs)) == 0;
}

static lib_i32 push_immediate_gprs_same(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax && before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx && before->data.ebx == after->data.ebx &&
        before->data.ebp == after->data.ebp && before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi;
}

static lib_i32 push_immediate_test_success(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_u8 width, lib_u32 expected)
{
    cpu_instruction_fixture state;
    t_cpu before;
    t_cpu after;
    lib_u32 observed = 0u;
    lib_u32 stack = 0x8000u - width;
    lib_i32 failed = 0;
    cpu_instruction_prepare(&state, profile);

    if (!failed)
    {
        push_immediate_seed(&state);
        before = state.cpu;
        failed |= cpu_instruction_run(&state, code, bytes, &after) != LIB_STATUS_OK || state.fault.valid ||
            after.data.eip != bytes || after.data.esp !=
            ((before.data.esp & 0xffff0000u) | stack) ||
            after.data.eflags != before.data.eflags ||
            !push_immediate_gprs_same(&before, &after) ||
            !push_immediate_sregs_same(&before, &after) ||
            cpu_instruction_read(&state, stack, &observed, width,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
            observed != (width == 2u ? (expected & 0xffffu) : expected);
    }
    return !failed;
}

static lib_i32 push_immediate_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    cpu_instruction_fixture state;
    t_cpu before;
    lib_u32 sentinel = 0xdeadbeefu;
    lib_u32 observed = 0u;
    lib_i32 failed = 0;
    cpu_instruction_prepare(&state, profile);

    if (!failed)
    {
        push_immediate_seed(&state);
        failed |= cpu_instruction_write(&state, 0x7ffcu, &sentinel, sizeof(sentinel),
            CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
        before = state.cpu;
        failed |= !cpu_instruction_expect_real_fault(&state, code, bytes, 6u) ||
            cpu_instruction_read(&state, 0x7ffcu, &observed, sizeof(observed),
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) !=
            LIB_STATUS_OK || observed != (profile < CORE_MACHINE_CPU_PROFILE_80186 ? sentinel :
                (lib_u32)(lib_u16)(before.data.flags | 2u |
                    (profile < CORE_MACHINE_CPU_PROFILE_80286 ? 0xf000u : 0u)) << 16u);
    }
    return !failed;
}

static lib_i32 push_immediate_test_defaults(void)
{
    static const core_machine_cpu_profile supported[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 push_iw[] = {0x68u, 0x34u, 0x12u};
    static const lib_u8 push_ib[] = {0x6au, 0x80u};
    static const lib_u8 push_iw_8086[] = {0x68u, 0x34u, 0x12u};
    static const lib_u8 push_ib_8086[] = {0x6au, 0x80u};
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(supported) / sizeof(supported[0]); ++profile)
    {
        if (!push_immediate_test_success(supported[profile], push_iw,
            sizeof(push_iw), 2u, 0x1234u) || !push_immediate_test_success(
            supported[profile], push_ib, sizeof(push_ib), 2u, 0xff80u))
            return 0;
    }
    return push_immediate_expect_ud(CORE_MACHINE_CPU_PROFILE_8086, push_iw_8086,
        sizeof(push_iw_8086)) && push_immediate_expect_ud(
        CORE_MACHINE_CPU_PROFILE_8086, push_ib_8086, sizeof(push_ib_8086));
}

static lib_i32 push_immediate_test_attributes_and_lock(void)
{
    static const lib_u8 iw32[] = {0x66u, 0x68u, 0x78u, 0x56u, 0x34u, 0x12u};
    static const lib_u8 ib32[] = {0x66u, 0x6au, 0x80u};
    static const lib_u8 iw67[] = {0x67u, 0x68u, 0x34u, 0x12u};
    static const lib_u8 ib66_67[] = {0x66u, 0x67u, 0x6au, 0x80u};
    static const lib_u8 locks[][7] = {{0xf0u, 0x68u, 0x34u, 0x12u},
        {0xf0u, 0x6au, 0x80u}, {0xf0u, 0x66u, 0x68u, 0x78u, 0x56u, 0x34u, 0x12u},
        {0xf0u, 0x66u, 0x6au, 0x80u}};
    static const lib_u8 attrs[][6] = {{0x66u, 0x68u, 0x34u, 0x12u},
        {0x67u, 0x6au, 0x80u}, {0x66u, 0x67u, 0x6au, 0x80u}};
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    lib_u8 profile;
    lib_u8 form;

    if (!push_immediate_test_success(CORE_MACHINE_CPU_PROFILE_80386, iw32,
        sizeof(iw32), 4u, 0x12345678u) || !push_immediate_test_success(
        CORE_MACHINE_CPU_PROFILE_80386, ib32, sizeof(ib32), 4u, 0xffffff80u) ||
        !push_immediate_test_success(CORE_MACHINE_CPU_PROFILE_80386, iw67,
        sizeof(iw67), 2u, 0x1234u) || !push_immediate_test_success(
        CORE_MACHINE_CPU_PROFILE_80386, ib66_67, sizeof(ib66_67), 4u,
        0xffffff80u))
        return 0;
    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]); ++profile)
    {
        for (form = 0u; form != sizeof(attrs) / sizeof(attrs[0]); ++form)
        {
            lib_u8 bytes = form == 0u ? 4u : (form == 1u ? 3u : 4u);

            if (!push_immediate_expect_ud(legacy[profile], attrs[form], bytes))
                return 0;
        }
    }
    for (form = 0u; form != sizeof(locks) / sizeof(locks[0]); ++form)
    {
        lib_u8 bytes = form == 0u ? 4u : (form == 1u ? 3u :
            (form == 2u ? 7u : 4u));

        if (!push_immediate_expect_ud(CORE_MACHINE_CPU_PROFILE_80386,
            locks[form], bytes))
            return 0;
    }
    return 1;
}

static lib_i32 push_immediate_boot_protected(cpu_instruction_fixture *state)
{
    static const lib_u8 pointer[] = {0x1fu,0u,0u,0x03u,0u,0u};
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

static lib_i32 push_immediate_test_protected(void)
{
    static const lib_u8 codes[][6] = {{0x68u,0x34u,0x12u},
        {0x66u,0x6au,0x80u}};
    static const lib_u8 bytes[] = {3u,3u};
    lib_u8 form;

    for (form = 0u; form != 2u; ++form)
    {
        cpu_instruction_fixture state;
        t_cpu before;
        t_cpu after;
        lib_u32 sentinel = 0xdeadbeefu;
        lib_u32 observed = 0u;
        lib_i32 failed = 0;
        cpu_instruction_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed)
            failed |= !push_immediate_boot_protected(&state);
        if (!failed)
        {
            lib_u32 stack = form == 0u ? 0xbffeu : 0xbffcu;
            lib_u8 width = form == 0u ? 2u : 4u;

            push_immediate_seed(&state);
            state.cpu.data.ss.limit = 0xffffu;
            state.cpu.data.ss.seg.data.expdown = LIB_TRUE;
            failed |= cpu_instruction_write(&state, stack, &sentinel, width,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK ||

                cpu_instruction_write(&state, 0x2000u, codes[form], bytes[form],
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA) != LIB_STATUS_OK;
            before = state.cpu;
            state.cpu.data.eip = 0u;
            core_machine_cpu_execution_refresh(&state.execution);
            failed |= !core_machine_cpu_is_shutdown(&state.execution) || state.execution.stop_requested;
            after = state.cpu;
            failed |= state.fault.valid || !state.delivered_exception.valid ||
                state.delivered_exception.exception_mask != VCPUINS_EXCEPT_SHUTDOWN ||
                after.data.eip != 0u || after.data.eax != before.data.eax ||
                after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
                after.data.ebx != before.data.ebx || after.data.esp != before.data.esp ||
                after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
                after.data.edi != before.data.edi || after.data.eflags !=
            before.data.eflags || !push_immediate_sregs_same(&before, &after) ||
            cpu_instruction_read(&state, stack, &observed, width,
                CORE_MACHINE_CPU_MEMORY_ACCESS_DATA, LIB_FALSE, LIB_FALSE) != LIB_STATUS_OK ||
            observed != (width == 2u ? (sentinel & 0xffffu) : sentinel);
        }
        if (failed)
            return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!push_immediate_test_protected())
    {
        lib_c_printf("CPU stack cache stage=test_protected\n");
        return 1;
    }
    if (!push_immediate_test_defaults())
    {
        lib_c_printf("PUSH-IMMEDIATE stage=defaults\n");
        return 1;
    }
    if (!push_immediate_test_attributes_and_lock())
    {
        lib_c_printf("PUSH-IMMEDIATE stage=attributes-lock\n");
        return 1;
    }
    lib_c_printf("PUSH-IMMEDIATE:OK\n");
    return 0;
}
