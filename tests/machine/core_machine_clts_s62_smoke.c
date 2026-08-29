#include "type.h"
#include "core/machine/cpu.h"
#include "core/machine/pic.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct clts_s62_machine {
    core_machine *machine;
} clts_s62_machine;

static C_VOID clts_s62_reset(C_VOID *opaque)
{
    clts_s62_machine *state = (clts_s62_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider clts_s62_provider = {
    clts_s62_reset, STD_NULL
};

static C_INT clts_s62_prepare(clts_s62_machine *state,
    core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    STD_MEMSET(state, 0, sizeof(*state));
    return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &clts_s62_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static C_VOID clts_s62_seed(clts_s62_machine *state)
{
    t_cpu *cpu = &state->machine->executor_cpu;

    cpu->data.eax = 0xaabbccddu;
    cpu->data.ecx = 0x11223344u;
    cpu->data.edx = 0x55667788u;
    cpu->data.ebx = 0x99aabbccu;
    cpu->data.esp = 0x00008000u;
    cpu->data.ebp = 0x00000120u;
    cpu->data.esi = 0x00000010u;
    cpu->data.edi = 0x00000020u;
    cpu->data.eflags = VCPU_EFLAGS_IF | VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF | VCPU_EFLAGS_OF;
}

static C_INT clts_s62_data_equal(const t_cpu *before, const t_cpu *after)
{
    return STD_MEMCMP(&before->data, &after->data, sizeof(before->data)) == 0;
}

static C_INT clts_s62_nonstack_data_equal(const t_cpu *before, const t_cpu *after)
{
    return after->data.eax == before->data.eax &&
        after->data.ecx == before->data.ecx &&
        after->data.edx == before->data.edx &&
        after->data.ebx == before->data.ebx &&
        after->data.ebp == before->data.ebp &&
        after->data.esi == before->data.esi &&
        after->data.edi == before->data.edi &&
        STD_MEMCMP(&before->data.cs, &after->data.cs,
            sizeof(before->data.cs)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after->data.ds,
            sizeof(before->data.ds)) == 0 &&
        STD_MEMCMP(&before->data.es, &after->data.es,
            sizeof(before->data.es)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after->data.ss,
            sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.fs, &after->data.fs,
            sizeof(before->data.fs)) == 0 &&
        STD_MEMCMP(&before->data.gs, &after->data.gs,
            sizeof(before->data.gs)) == 0;
}

static C_INT clts_s62_run(clts_s62_machine *state, const type_unsigned_8 *code,
    type_unsigned_8 bytes, type_unsigned_32 cycles, type_status *status,
    core_machine_run_result *result, core_machine_cpu_diagnostic *diagnostic)
{
    if (core_machine_memory_write(state->machine, 0u, code, bytes) !=
        TYPE_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){cycles,0u}, result);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        TYPE_STATUS_OK;
}

static C_INT clts_s62_expect_ud(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, type_unsigned_8 bytes)
{
    clts_s62_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    type_status status;
    C_INT failed = !clts_s62_prepare(&state, profile);

    if (!failed) {
        clts_s62_seed(&state);
        state.machine->executor_cpu.data.cr0 |= VCPU_CR0_TS;
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !clts_s62_run(&state, code, bytes, 1u, &status, &result,
            &diagnostic) || status != TYPE_STATUS_FAULT ||
            !diagnostic.first_fault.valid || !TYPE_GET_BIT(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD);
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !clts_s62_data_equal(&before, &after);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT clts_s62_test_real_and_attributes(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 attributes[][4] = {
        {0x66u,0x0fu,0x06u,0u}, {0x67u,0x0fu,0x06u,0u},
        {0x66u,0x67u,0x0fu,0x06u}
    };
    static const type_unsigned_8 plain[] = {0x0fu,0x06u};
    type_unsigned_8 profile;
    type_unsigned_8 attribute;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        clts_s62_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        type_status status = TYPE_STATUS_INVALID_STATE;
        C_INT failed = !clts_s62_prepare(&state, profiles[profile]);

        if (!failed) {
            clts_s62_seed(&state);
            state.machine->executor_cpu.data.cr0 = VCPU_CR0_TS | 0x0000000du;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !clts_s62_run(&state, plain, sizeof(plain), 1u, &status,
                &result, &diagnostic) || status != TYPE_STATUS_OK ||
                result.reason != CORE_MACHINE_STOP_BUDGET || diagnostic.first_fault.valid;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= after.data.eip != 2u || !clts_s62_nonstack_data_equal(
                &before, &after) || after.data.esp != before.data.esp ||
                after.data.eflags != before.data.eflags || after.data.cr0 !=
                (before.data.cr0 & ~VCPU_CR0_TS);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    for (attribute = 0u; attribute != sizeof(attributes) / sizeof(attributes[0]); ++attribute) {
        clts_s62_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        type_status status = TYPE_STATUS_INVALID_STATE;
        const type_unsigned_8 bytes = attribute == 2u ? 4u : 3u;
        C_INT failed = !clts_s62_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            clts_s62_seed(&state);
            state.machine->executor_cpu.data.cr0 |= VCPU_CR0_TS;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !clts_s62_run(&state, attributes[attribute], bytes, 1u,
                &status, &result, &diagnostic) || status != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= after.data.eip != bytes || !clts_s62_nonstack_data_equal(
                &before, &after) || after.data.esp != before.data.esp ||
                after.data.eflags != before.data.eflags || after.data.cr0 !=
                (before.data.cr0 & ~VCPU_CR0_TS);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    for (profile = 0u; profile != 3u; ++profile) {
        const core_machine_cpu_profile legacy[] = {
            CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
            CORE_MACHINE_CPU_PROFILE_80286
        };

        if (!clts_s62_expect_ud(legacy[profile], attributes[0], 3u) ||
            !clts_s62_expect_ud(legacy[profile], attributes[1], 3u) ||
            !clts_s62_expect_ud(legacy[profile], attributes[2], 4u))
            return 0;
    }
    return clts_s62_expect_ud(CORE_MACHINE_CPU_PROFILE_80186, plain,
        sizeof(plain));
}

static C_VOID clts_s62_enter_protected(clts_s62_machine *state, type_unsigned_8 cpl,
    C_INT vm86)
{
    t_cpu *cpu = &state->machine->executor_cpu;

    TYPE_SET_BIT(cpu->data.cr0, VCPU_CR0_PE);
    cpu->data.cs.selector = (type_unsigned_16)(0x0008u | cpl);
    cpu->data.cs.base = 0u;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.dpl = cpl;
    cpu->data.cs.flagValid = TYPE_TRUE;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.seg.executable = TYPE_TRUE;
    if (vm86) {
        TYPE_SET_BIT(cpu->data.eflags, VCPU_EFLAGS_VM);
        cpu->data.cs.dpl = 3u;
    }
}

static C_INT clts_s62_test_privilege(C_VOID)
{
    static const type_unsigned_8 code[] = {0x0fu,0x06u};
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    type_unsigned_8 profile;
    type_unsigned_8 cpl;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        clts_s62_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        type_status status = TYPE_STATUS_INVALID_STATE;
        C_INT failed = !clts_s62_prepare(&state, profiles[profile]);

        if (!failed) {
            clts_s62_seed(&state);
            clts_s62_enter_protected(&state, 0u, 0);
            state.machine->executor_cpu.data.cr0 |= VCPU_CR0_TS;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !clts_s62_run(&state, code, sizeof(code), 1u, &status,
                &result, &diagnostic) || status != TYPE_STATUS_OK ||
                diagnostic.first_fault.valid;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= after.data.eip != 2u || !clts_s62_nonstack_data_equal(
                &before, &after) || after.data.esp != before.data.esp ||
                after.data.eflags != before.data.eflags || after.data.cr0 !=
                (before.data.cr0 & ~VCPU_CR0_TS);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    for (cpl = 1u; cpl != 3u; ++cpl) {
        clts_s62_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        type_status status = TYPE_STATUS_INVALID_STATE;
        C_INT failed = !clts_s62_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            clts_s62_seed(&state);
            clts_s62_enter_protected(&state, cpl == 0u ? 0u : 3u, cpl == 2u);
            state.machine->executor_cpu.data.cr0 |= VCPU_CR0_TS;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !clts_s62_run(&state, code, sizeof(code), 1u, &status,
                &result, &diagnostic);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            if (cpl == 0u) {
                failed |= status != TYPE_STATUS_OK || diagnostic.first_fault.valid ||
                    after.data.eip != 2u || !clts_s62_nonstack_data_equal(&before,
                    &after) || after.data.esp != before.data.esp ||
                    after.data.eflags != before.data.eflags || after.data.cr0 !=
                    (before.data.cr0 & ~VCPU_CR0_TS);
            } else {
                failed |= status != TYPE_STATUS_FAULT || !diagnostic.first_fault.valid ||
                    !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_DF) || after.data.eip != 0u ||
                    after.data.cr0 != before.data.cr0 || after.data.eflags !=
                    before.data.eflags || !clts_s62_nonstack_data_equal(&before,
                    &after) || after.data.esp != before.data.esp;
            }
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT clts_s62_test_lock(C_VOID)
{
    static const type_unsigned_8 forms[][5] = {
        {0xf0u,0x0fu,0x06u,0u,0u}, {0xf0u,0x66u,0x0fu,0x06u,0u},
        {0xf0u,0x67u,0x0fu,0x06u,0u}, {0xf0u,0x66u,0x67u,0x0fu,0x06u}
    };
    static const type_unsigned_8 bytes[] = {3u,4u,4u,5u};
    type_unsigned_8 index;

    for (index = 0u; index != sizeof(forms) / sizeof(forms[0]); ++index)
        if (!clts_s62_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, forms[index],
            bytes[index]))
            return 0;
    return 1;
}

static C_INT clts_s62_test_irq(C_VOID)
{
    static const type_unsigned_8 code[] = {0x0fu,0x06u,0x90u};
    static const type_unsigned_8 hlt = 0xf4u;
    clts_s62_machine state;
    core_machine_pic_irq_source irq;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    type_unsigned_16 offset = 0x0100u;
    type_unsigned_16 segment = 0u;
    type_unsigned_16 frame_ip = 0u;
    C_INT failed = !clts_s62_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0u, code,
            sizeof(code)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x80u, &offset, sizeof(offset)) != TYPE_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x82u, &segment,
            sizeof(segment)) != TYPE_STATUS_OK || core_machine_memory_write(
            state.machine, 0x100u, &hlt, sizeof(hlt)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        clts_s62_seed(&state);
        state.machine->executor_cpu.data.cr0 |= VCPU_CR0_TS;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        STD_MEMSET(&irq, 0, sizeof(irq));
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&irq, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){2u,0u}, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            after.data.ss.base + (type_unsigned_16)after.data.esp, TYPE_REFERENCE_OF(frame_ip),
            sizeof(frame_ip)) != TYPE_STATUS_OK || after.data.eip != 0x101u ||
            frame_ip != 2u || !clts_s62_nonstack_data_equal(&before, &after) ||
            after.data.cr0 != (before.data.cr0 & ~VCPU_CR0_TS) ||
            after.data.eflags != (before.data.eflags & ~VCPU_EFLAGS_IF) ||
            !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u)) ||
            TYPE_GET_BIT(state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

C_INT main(C_VOID)
{
    if (!clts_s62_test_real_and_attributes()) {
        STD_PRINTF("CLTS stage=real-attributes\n");
        return 1;
    }
    if (!clts_s62_test_privilege()) {
        STD_PRINTF("CLTS stage=privilege\n");
        return 1;
    }
    if (!clts_s62_test_lock()) {
        STD_PRINTF("CLTS stage=lock\n");
        return 1;
    }
    if (!clts_s62_test_irq()) {
        STD_PRINTF("CLTS stage=irq\n");
        return 1;
    }
    STD_PRINTF("M5:T316:S62:CLTS:OK\n");
    return 0;
}
