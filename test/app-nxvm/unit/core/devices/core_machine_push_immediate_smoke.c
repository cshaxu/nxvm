#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"
#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/pic.h"
#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_cpu_fixture.h"

typedef struct push_immediate_machine
{
    core_machine *machine;
} push_immediate_machine;

static void push_immediate_reset(void *opaque)
{
    push_immediate_machine *state = (push_immediate_machine *)opaque;

    if (state != LIB_NULL)
        (void)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider push_immediate_provider = {
    push_immediate_reset, LIB_NULL
};

static lib_i32 push_immediate_prepare(core_machine_cpu_profile profile,
    push_immediate_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    lib_memory_set(state, 0, sizeof(*state));
return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &push_immediate_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static void push_immediate_seed(push_immediate_machine *state)
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

static lib_i32 push_immediate_run(push_immediate_machine *state, const lib_u8 *code,
    lib_u8 bytes, core_machine_run_budget budget, t_cpu *after,
    core_machine_cpu_diagnostic *diagnostic, lib_status *status,
    core_machine_run_result *result)
{
    if (core_machine_memory_write(state->machine, 0u, code, bytes) != LIB_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine, budget, result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) == LIB_STATUS_OK;
}

static lib_i32 push_immediate_test_success(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes, lib_u8 width, lib_u32 expected)
{
    push_immediate_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    lib_status status;
    lib_u32 observed = 0u;
    lib_u32 stack = 0x8000u - width;
    lib_i32 failed = !push_immediate_prepare(profile, &state);

    if (!failed)
    {
        push_immediate_seed(&state);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !push_immediate_run(&state, code, bytes,
            (core_machine_run_budget){1u, 0u}, &after, &diagnostic, &status,
            &result) || status != LIB_STATUS_OK || result.reason !=
            CORE_MACHINE_STOP_BUDGET || diagnostic.first_fault.valid ||
            after.data.eip != bytes || after.data.esp !=
            ((before.data.esp & 0xffff0000u) | stack) ||
            after.data.eflags != before.data.eflags ||
            !push_immediate_gprs_same(&before, &after) ||
            !push_immediate_sregs_same(&before, &after) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
            stack, CORE_MACHINE_REFERENCE_OF(observed), width) != LIB_STATUS_OK ||
            observed != (width == 2u ? (expected & 0xffffu) : expected);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 push_immediate_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    push_immediate_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    lib_status status;
    lib_u32 sentinel = 0xdeadbeefu;
    lib_u32 observed = 0u;
    lib_i32 failed = !push_immediate_prepare(profile, &state);

    if (!failed)
    {
        push_immediate_seed(&state);
        failed |= core_machine_memory_write(state.machine, 0x7ffcu, &sentinel,
            sizeof(sentinel)) != LIB_STATUS_OK;
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !push_immediate_run(&state, code, bytes,
            (core_machine_run_budget){1u, 0u}, &after, &diagnostic, &status,
            &result) || status != LIB_STATUS_INTERNAL_ERROR || result.reason !=
            CORE_MACHINE_STOP_FAULT || !diagnostic.first_fault.valid ||
            !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eip != before.data.eip || after.data.esp != before.data.esp ||
            after.data.eflags != before.data.eflags ||
            !push_immediate_gprs_same(&before, &after) ||
            !push_immediate_sregs_same(&before, &after) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
            0x7ffcu, CORE_MACHINE_REFERENCE_OF(observed), sizeof(observed)) !=
            LIB_STATUS_OK || observed != sentinel;
    }
    core_machine_destroy(state.machine);
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

static lib_i32 push_immediate_boot_protected(push_immediate_machine *state)
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
    static const lib_u8 halt = 0xf4u;
    core_machine_run_result result;

    return core_machine_memory_write(state->machine, 0x100u, pointer,
        sizeof(pointer)) == LIB_STATUS_OK && core_machine_memory_write(
        state->machine, 0x300u, gdt, sizeof(gdt)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, bootstrap,
        sizeof(bootstrap)) == LIB_STATUS_OK && core_machine_memory_write(
        state->machine, 0x2000u, &halt, sizeof(halt)) == LIB_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){96u,0u},
        &result) == LIB_STATUS_OK && result.reason ==
        CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static lib_i32 push_immediate_test_protected(void)
{
    static const lib_u8 codes[][6] = {{0x68u,0x34u,0x12u},
        {0x66u,0x6au,0x80u}};
    static const lib_u8 bytes[] = {3u,3u};
    lib_u8 form;

    for (form = 0u; form != 2u; ++form)
    {
        push_immediate_machine state;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        lib_u32 sentinel = 0xdeadbeefu;
        lib_u32 observed = 0u;
        lib_i32 failed = !push_immediate_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed)
            failed |= !push_immediate_boot_protected(&state);
        if (!failed)
        {
            lib_u32 stack = form == 0u ? 0xbffeu : 0xbffcu;
            lib_u8 width = form == 0u ? 2u : 4u;

            push_immediate_seed(&state);
            state.machine->executor_cpu.data.ss.limit = 0xffffu;
            state.machine->executor_cpu.data.ss.seg.data.expdown = LIB_TRUE;
            failed |= core_machine_memory_write(state.machine, stack, &sentinel,
                width) != LIB_STATUS_OK || core_machine_memory_write(
                state.machine, 0x2000u, codes[form], bytes[form]) != LIB_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){1u,0u}, &result) != LIB_STATUS_INTERNAL_ERROR ||
                result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                LIB_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                after.data.eip != 0u || after.data.eax != before.data.eax ||
                after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
                after.data.ebx != before.data.ebx || after.data.esp != before.data.esp ||
                after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
                after.data.edi != before.data.edi || after.data.eflags !=
            before.data.eflags || !push_immediate_sregs_same(&before, &after) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
            stack, CORE_MACHINE_REFERENCE_OF(observed), width) != LIB_STATUS_OK ||
            observed != (width == 2u ? (sentinel & 0xffffu) : sentinel);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 push_immediate_test_irq(void)
{
    static const lib_u8 codes[][5] = {{0x68u,0x34u,0x12u,0x90u},
        {0x6au,0x80u,0x90u}};
    static const lib_u8 length[] = {3u,2u};
    static const lib_u8 halt = 0xf4u;
    lib_u8 form;

    for (form = 0u; form != 2u; ++form)
    {
        push_immediate_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        t_cpu after;
        lib_u16 offset = 0x100u;
        lib_u16 segment = 0u;
        lib_u16 frame_ip = 0u;
        lib_u16 value = 0u;
        lib_i32 failed = !push_immediate_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed)
        {
            push_immediate_seed(&state);
            failed |= core_machine_memory_write(state.machine, 0u, codes[form],
                length[form] + 1u) != LIB_STATUS_OK || core_machine_memory_write(
                state.machine, 0x80u, &offset, 2u) != LIB_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x82u, &segment, 2u) !=
                LIB_STATUS_OK || core_machine_memory_write(state.machine, 0x100u,
                &halt, 1u) != LIB_STATUS_OK;
        }
        if (!failed)
        {
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
            lib_memory_set(&source, 0, sizeof(source));
            state.machine->shared_pic_master.data.icw2 = 0x20u;
            core_machine_pic_irq_source_bind(&source, &state.machine->shared_pic_master,
                &state.machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){2u,0u}, &result) != LIB_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (lib_u16)after.data.esp,
                CORE_MACHINE_REFERENCE_OF(frame_ip), 2u) != LIB_STATUS_OK ||
                after.data.eip != 0x101u || frame_ip != length[form] ||
                !CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u)) || CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.irr,
                VPIC_IRR_IRQ(0u)) || after.data.esp != 0x12347ff8u ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                0x7ffeu, CORE_MACHINE_REFERENCE_OF(value), 2u) != LIB_STATUS_OK ||
                value != (form == 0u ? 0x1234u : 0xff80u);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

lib_i32 main(void)
{
    if (!push_immediate_test_defaults())
    {
        printf("PUSH-IMMEDIATE stage=defaults\n");
        return 1;
    }
    if (!push_immediate_test_attributes_and_lock())
    {
        printf("PUSH-IMMEDIATE stage=attributes-lock\n");
        return 1;
    }
    if (!push_immediate_test_protected())
    {
        printf("PUSH-IMMEDIATE stage=protected\n");
        return 1;
    }
    if (!push_immediate_test_irq())
    {
        printf("PUSH-IMMEDIATE stage=irq\n");
        return 1;
    }
    printf("M5:T316:S45:PUSH-IMMEDIATE:OK\n");
    return 0;
}
