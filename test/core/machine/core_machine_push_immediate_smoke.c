#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/pic.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct push_immediate_machine
{
    core_machine *machine;
} push_immediate_machine;

static C_VOID push_immediate_reset(C_VOID *opaque)
{
    push_immediate_machine *state = (push_immediate_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider push_immediate_provider = {
    push_immediate_reset, STD_NULL
};

static C_INT push_immediate_prepare(core_machine_cpu_profile profile,
    push_immediate_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    STD_MEMSET(state, 0, sizeof(*state));
return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &push_immediate_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static C_VOID push_immediate_seed(push_immediate_machine *state)
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

static C_INT push_immediate_sregs_same(const t_cpu *before, const t_cpu *after)
{
    return STD_MEMCMP(&before->data.es, &after->data.es, sizeof(before->data.es)) == 0 &&
        STD_MEMCMP(&before->data.cs, &after->data.cs, sizeof(before->data.cs)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after->data.ss, sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after->data.ds, sizeof(before->data.ds)) == 0 &&
        STD_MEMCMP(&before->data.fs, &after->data.fs, sizeof(before->data.fs)) == 0 &&
        STD_MEMCMP(&before->data.gs, &after->data.gs, sizeof(before->data.gs)) == 0;
}

static C_INT push_immediate_gprs_same(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax && before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx && before->data.ebx == after->data.ebx &&
        before->data.ebp == after->data.ebp && before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi;
}

static C_INT push_immediate_run(push_immediate_machine *state, const type_unsigned_8 *code,
    type_unsigned_8 bytes, core_machine_run_budget budget, t_cpu *after,
    core_machine_cpu_diagnostic *diagnostic, type_status *status,
    core_machine_run_result *result)
{
    if (core_machine_memory_write(state->machine, 0u, code, bytes) != TYPE_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine, budget, result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) == TYPE_STATUS_OK;
}

static C_INT push_immediate_test_success(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, type_unsigned_8 bytes, type_unsigned_8 width, type_unsigned_32 expected)
{
    push_immediate_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    type_status status;
    type_unsigned_32 observed = 0u;
    type_unsigned_32 stack = 0x8000u - width;
    C_INT failed = !push_immediate_prepare(profile, &state);

    if (!failed)
    {
        push_immediate_seed(&state);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !push_immediate_run(&state, code, bytes,
            (core_machine_run_budget){1u, 0u}, &after, &diagnostic, &status,
            &result) || status != TYPE_STATUS_OK || result.reason !=
            CORE_MACHINE_STOP_BUDGET || diagnostic.first_fault.valid ||
            after.data.eip != bytes || after.data.esp !=
            ((before.data.esp & 0xffff0000u) | stack) ||
            after.data.eflags != before.data.eflags ||
            !push_immediate_gprs_same(&before, &after) ||
            !push_immediate_sregs_same(&before, &after) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
            stack, TYPE_REFERENCE_OF(observed), width) != TYPE_STATUS_OK ||
            observed != (width == 2u ? (expected & 0xffffu) : expected);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT push_immediate_expect_ud(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, type_unsigned_8 bytes)
{
    push_immediate_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    type_status status;
    type_unsigned_32 sentinel = 0xdeadbeefu;
    type_unsigned_32 observed = 0u;
    C_INT failed = !push_immediate_prepare(profile, &state);

    if (!failed)
    {
        push_immediate_seed(&state);
        failed |= core_machine_memory_write(state.machine, 0x7ffcu, &sentinel,
            sizeof(sentinel)) != TYPE_STATUS_OK;
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !push_immediate_run(&state, code, bytes,
            (core_machine_run_budget){1u, 0u}, &after, &diagnostic, &status,
            &result) || status != TYPE_STATUS_FAULT || result.reason !=
            CORE_MACHINE_STOP_FAULT || !diagnostic.first_fault.valid ||
            !TYPE_GET_BIT(diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eip != before.data.eip || after.data.esp != before.data.esp ||
            after.data.eflags != before.data.eflags ||
            !push_immediate_gprs_same(&before, &after) ||
            !push_immediate_sregs_same(&before, &after) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
            0x7ffcu, TYPE_REFERENCE_OF(observed), sizeof(observed)) !=
            TYPE_STATUS_OK || observed != sentinel;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT push_immediate_test_defaults(C_VOID)
{
    static const core_machine_cpu_profile supported[] = {
        CORE_MACHINE_CPU_PROFILE_80186, CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 push_iw[] = {0x68u, 0x34u, 0x12u};
    static const type_unsigned_8 push_ib[] = {0x6au, 0x80u};
    static const type_unsigned_8 push_iw_8086[] = {0x68u, 0x34u, 0x12u};
    static const type_unsigned_8 push_ib_8086[] = {0x6au, 0x80u};
    type_unsigned_8 profile;

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

static C_INT push_immediate_test_attributes_and_lock(C_VOID)
{
    static const type_unsigned_8 iw32[] = {0x66u, 0x68u, 0x78u, 0x56u, 0x34u, 0x12u};
    static const type_unsigned_8 ib32[] = {0x66u, 0x6au, 0x80u};
    static const type_unsigned_8 iw67[] = {0x67u, 0x68u, 0x34u, 0x12u};
    static const type_unsigned_8 ib66_67[] = {0x66u, 0x67u, 0x6au, 0x80u};
    static const type_unsigned_8 locks[][7] = {{0xf0u, 0x68u, 0x34u, 0x12u},
        {0xf0u, 0x6au, 0x80u}, {0xf0u, 0x66u, 0x68u, 0x78u, 0x56u, 0x34u, 0x12u},
        {0xf0u, 0x66u, 0x6au, 0x80u}};
    static const type_unsigned_8 attrs[][6] = {{0x66u, 0x68u, 0x34u, 0x12u},
        {0x67u, 0x6au, 0x80u}, {0x66u, 0x67u, 0x6au, 0x80u}};
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    type_unsigned_8 profile;
    type_unsigned_8 form;

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
            type_unsigned_8 bytes = form == 0u ? 4u : (form == 1u ? 3u : 4u);

            if (!push_immediate_expect_ud(legacy[profile], attrs[form], bytes))
                return 0;
        }
    }
    for (form = 0u; form != sizeof(locks) / sizeof(locks[0]); ++form)
    {
        type_unsigned_8 bytes = form == 0u ? 4u : (form == 1u ? 3u :
            (form == 2u ? 7u : 4u));

        if (!push_immediate_expect_ud(CORE_MACHINE_CPU_PROFILE_80386,
            locks[form], bytes))
            return 0;
    }
    return 1;
}

static C_INT push_immediate_boot_protected(push_immediate_machine *state)
{
    static const type_unsigned_8 pointer[] = {0x1fu,0u,0u,0x03u,0u,0u};
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
    static const type_unsigned_8 halt = 0xf4u;
    core_machine_run_result result;

    return core_machine_memory_write(state->machine, 0x100u, pointer,
        sizeof(pointer)) == TYPE_STATUS_OK && core_machine_memory_write(
        state->machine, 0x300u, gdt, sizeof(gdt)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, bootstrap,
        sizeof(bootstrap)) == TYPE_STATUS_OK && core_machine_memory_write(
        state->machine, 0x2000u, &halt, sizeof(halt)) == TYPE_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){96u,0u},
        &result) == TYPE_STATUS_OK && result.reason ==
        CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static C_INT push_immediate_test_protected(C_VOID)
{
    static const type_unsigned_8 codes[][6] = {{0x68u,0x34u,0x12u},
        {0x66u,0x6au,0x80u}};
    static const type_unsigned_8 bytes[] = {3u,3u};
    type_unsigned_8 form;

    for (form = 0u; form != 2u; ++form)
    {
        push_immediate_machine state;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        type_unsigned_32 sentinel = 0xdeadbeefu;
        type_unsigned_32 observed = 0u;
        C_INT failed = !push_immediate_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed)
            failed |= !push_immediate_boot_protected(&state);
        if (!failed)
        {
            type_unsigned_32 stack = form == 0u ? 0xbffeu : 0xbffcu;
            type_unsigned_8 width = form == 0u ? 2u : 4u;

            push_immediate_seed(&state);
            state.machine->executor_cpu.data.ss.limit = 0xffffu;
            state.machine->executor_cpu.data.ss.seg.data.expdown = TYPE_TRUE;
            failed |= core_machine_memory_write(state.machine, stack, &sentinel,
                width) != TYPE_STATUS_OK || core_machine_memory_write(
                state.machine, 0x2000u, codes[form], bytes[form]) != TYPE_STATUS_OK;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){1u,0u}, &result) != TYPE_STATUS_FAULT ||
                result.reason != CORE_MACHINE_STOP_FAULT ||
                core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_DF) ||
                after.data.eip != 0u || after.data.eax != before.data.eax ||
                after.data.ecx != before.data.ecx || after.data.edx != before.data.edx ||
                after.data.ebx != before.data.ebx || after.data.esp != before.data.esp ||
                after.data.ebp != before.data.ebp || after.data.esi != before.data.esi ||
                after.data.edi != before.data.edi || after.data.eflags !=
            before.data.eflags || !push_immediate_sregs_same(&before, &after) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
            stack, TYPE_REFERENCE_OF(observed), width) != TYPE_STATUS_OK ||
            observed != (width == 2u ? (sentinel & 0xffffu) : sentinel);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT push_immediate_test_irq(C_VOID)
{
    static const type_unsigned_8 codes[][5] = {{0x68u,0x34u,0x12u,0x90u},
        {0x6au,0x80u,0x90u}};
    static const type_unsigned_8 length[] = {3u,2u};
    static const type_unsigned_8 halt = 0xf4u;
    type_unsigned_8 form;

    for (form = 0u; form != 2u; ++form)
    {
        push_immediate_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        t_cpu after;
        type_unsigned_16 offset = 0x100u;
        type_unsigned_16 segment = 0u;
        type_unsigned_16 frame_ip = 0u;
        type_unsigned_16 value = 0u;
        C_INT failed = !push_immediate_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed)
        {
            push_immediate_seed(&state);
            failed |= core_machine_memory_write(state.machine, 0u, codes[form],
                length[form] + 1u) != TYPE_STATUS_OK || core_machine_memory_write(
                state.machine, 0x80u, &offset, 2u) != TYPE_STATUS_OK ||
                core_machine_memory_write(state.machine, 0x82u, &segment, 2u) !=
                TYPE_STATUS_OK || core_machine_memory_write(state.machine, 0x100u,
                &halt, 1u) != TYPE_STATUS_OK;
        }
        if (!failed)
        {
            state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
            STD_MEMSET(&source, 0, sizeof(source));
            state.machine->shared_pic_master.data.icw2 = 0x20u;
            core_machine_pic_irq_source_bind(&source, &state.machine->shared_pic_master,
                &state.machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){2u,0u}, &result) != TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (type_unsigned_16)after.data.esp,
                TYPE_REFERENCE_OF(frame_ip), 2u) != TYPE_STATUS_OK ||
                after.data.eip != 0x101u || frame_ip != length[form] ||
                !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
                VPIC_IRR_IRQ(0u)) || after.data.esp != 0x12347ff8u ||
                core_machine_memory_read_physical(&state.machine->executor_memory,
                0x7ffeu, TYPE_REFERENCE_OF(value), 2u) != TYPE_STATUS_OK ||
                value != (form == 0u ? 0x1234u : 0xff80u);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

C_INT main(C_VOID)
{
    if (!push_immediate_test_defaults())
    {
        STD_PRINTF("PUSH-IMMEDIATE stage=defaults\n");
        return 1;
    }
    if (!push_immediate_test_attributes_and_lock())
    {
        STD_PRINTF("PUSH-IMMEDIATE stage=attributes-lock\n");
        return 1;
    }
    if (!push_immediate_test_protected())
    {
        STD_PRINTF("PUSH-IMMEDIATE stage=protected\n");
        return 1;
    }
    if (!push_immediate_test_irq())
    {
        STD_PRINTF("PUSH-IMMEDIATE stage=irq\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S45:PUSH-IMMEDIATE:OK\n");
    return 0;
}
