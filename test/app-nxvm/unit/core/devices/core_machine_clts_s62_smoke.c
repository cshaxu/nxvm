#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"
#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/pic.h"
#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_cpu_fixture.h"

typedef struct clts_s62_machine {
    core_machine *machine;
} clts_s62_machine;

static void clts_s62_reset(void *opaque)
{
    clts_s62_machine *state = (clts_s62_machine *)opaque;

    if (state != LIB_NULL)
        (void)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider clts_s62_provider = {
    clts_s62_reset, LIB_NULL
};

static lib_i32 clts_s62_prepare(clts_s62_machine *state,
    core_machine_cpu_profile profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    lib_memory_set(state, 0, sizeof(*state));
    return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &clts_s62_provider, state, &state->machine) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u);
}

static void clts_s62_seed(clts_s62_machine *state)
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

static lib_i32 clts_s62_data_equal(const t_cpu *before, const t_cpu *after)
{
    return lib_memory_compare(&before->data, &after->data, sizeof(before->data)) == 0;
}

static lib_i32 clts_s62_nonstack_data_equal(const t_cpu *before, const t_cpu *after)
{
    return after->data.eax == before->data.eax &&
        after->data.ecx == before->data.ecx &&
        after->data.edx == before->data.edx &&
        after->data.ebx == before->data.ebx &&
        after->data.ebp == before->data.ebp &&
        after->data.esi == before->data.esi &&
        after->data.edi == before->data.edi &&
        lib_memory_compare(&before->data.cs, &after->data.cs,
            sizeof(before->data.cs)) == 0 &&
        lib_memory_compare(&before->data.ds, &after->data.ds,
            sizeof(before->data.ds)) == 0 &&
        lib_memory_compare(&before->data.es, &after->data.es,
            sizeof(before->data.es)) == 0 &&
        lib_memory_compare(&before->data.ss, &after->data.ss,
            sizeof(before->data.ss)) == 0 &&
        lib_memory_compare(&before->data.fs, &after->data.fs,
            sizeof(before->data.fs)) == 0 &&
        lib_memory_compare(&before->data.gs, &after->data.gs,
            sizeof(before->data.gs)) == 0;
}

static lib_i32 clts_s62_run(clts_s62_machine *state, const lib_u8 *code,
    lib_u8 bytes, lib_u32 cycles, lib_status *status,
    core_machine_run_result *result, core_machine_cpu_diagnostic *diagnostic)
{
    if (core_machine_memory_write(state->machine, 0u, code, bytes) !=
        LIB_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){cycles,0u}, result);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        LIB_STATUS_OK;
}

static lib_i32 clts_s62_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    clts_s62_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    lib_status status;
    lib_i32 failed = !clts_s62_prepare(&state, profile);

    if (!failed) {
        clts_s62_seed(&state);
        state.machine->executor_cpu.data.cr0 |= VCPU_CR0_TS;
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !clts_s62_run(&state, code, bytes, 1u, &status, &result,
            &diagnostic) || status != LIB_STATUS_INTERNAL_ERROR ||
            !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
            diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD);
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !clts_s62_data_equal(&before, &after);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 clts_s62_test_real_and_attributes(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 attributes[][4] = {
        {0x66u,0x0fu,0x06u,0u}, {0x67u,0x0fu,0x06u,0u},
        {0x66u,0x67u,0x0fu,0x06u}
    };
    static const lib_u8 plain[] = {0x0fu,0x06u};
    lib_u8 profile;
    lib_u8 attribute;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        clts_s62_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        lib_status status = LIB_STATUS_INVALID_STATE;
        lib_i32 failed = !clts_s62_prepare(&state, profiles[profile]);

        if (!failed) {
            clts_s62_seed(&state);
            state.machine->executor_cpu.data.cr0 = VCPU_CR0_TS | 0x0000000du;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !clts_s62_run(&state, plain, sizeof(plain), 1u, &status,
                &result, &diagnostic) || status != LIB_STATUS_OK ||
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
        lib_status status = LIB_STATUS_INVALID_STATE;
        const lib_u8 bytes = attribute == 2u ? 4u : 3u;
        lib_i32 failed = !clts_s62_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            clts_s62_seed(&state);
            state.machine->executor_cpu.data.cr0 |= VCPU_CR0_TS;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !clts_s62_run(&state, attributes[attribute], bytes, 1u,
                &status, &result, &diagnostic) || status != LIB_STATUS_OK ||
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

static void clts_s62_enter_protected(clts_s62_machine *state, lib_u8 cpl,
    lib_i32 vm86)
{
    t_cpu *cpu = &state->machine->executor_cpu;

    CORE_MACHINE_BIT_SET(cpu->data.cr0, VCPU_CR0_PE);
    cpu->data.cs.selector = (lib_u16)(0x0008u | cpl);
    cpu->data.cs.base = 0u;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.dpl = cpl;
    cpu->data.cs.flagValid = LIB_TRUE;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.seg.executable = LIB_TRUE;
    if (vm86) {
        CORE_MACHINE_BIT_SET(cpu->data.eflags, VCPU_EFLAGS_VM);
        cpu->data.cs.dpl = 3u;
    }
}

static lib_i32 clts_s62_test_privilege(void)
{
    static const lib_u8 code[] = {0x0fu,0x06u};
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    lib_u8 profile;
    lib_u8 cpl;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        clts_s62_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_run_result result;
        core_machine_cpu_diagnostic diagnostic;
        lib_status status = LIB_STATUS_INVALID_STATE;
        lib_i32 failed = !clts_s62_prepare(&state, profiles[profile]);

        if (!failed) {
            clts_s62_seed(&state);
            clts_s62_enter_protected(&state, 0u, 0);
            state.machine->executor_cpu.data.cr0 |= VCPU_CR0_TS;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !clts_s62_run(&state, code, sizeof(code), 1u, &status,
                &result, &diagnostic) || status != LIB_STATUS_OK ||
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
        lib_status status = LIB_STATUS_INVALID_STATE;
        lib_i32 failed = !clts_s62_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            clts_s62_seed(&state);
            clts_s62_enter_protected(&state, cpl == 0u ? 0u : 3u, cpl == 2u);
            state.machine->executor_cpu.data.cr0 |= VCPU_CR0_TS;
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !clts_s62_run(&state, code, sizeof(code), 1u, &status,
                &result, &diagnostic);
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            if (cpl == 0u) {
                failed |= status != LIB_STATUS_OK || diagnostic.first_fault.valid ||
                    after.data.eip != 2u || !clts_s62_nonstack_data_equal(&before,
                    &after) || after.data.esp != before.data.esp ||
                    after.data.eflags != before.data.eflags || after.data.cr0 !=
                    (before.data.cr0 & ~VCPU_CR0_TS);
            } else {
                failed |= status != LIB_STATUS_INTERNAL_ERROR || !diagnostic.first_fault.valid ||
                    !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
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

static lib_i32 clts_s62_test_lock(void)
{
    static const lib_u8 forms[][5] = {
        {0xf0u,0x0fu,0x06u,0u,0u}, {0xf0u,0x66u,0x0fu,0x06u,0u},
        {0xf0u,0x67u,0x0fu,0x06u,0u}, {0xf0u,0x66u,0x67u,0x0fu,0x06u}
    };
    static const lib_u8 bytes[] = {3u,4u,4u,5u};
    lib_u8 index;

    for (index = 0u; index != sizeof(forms) / sizeof(forms[0]); ++index)
        if (!clts_s62_expect_ud(CORE_MACHINE_CPU_PROFILE_80386, forms[index],
            bytes[index]))
            return 0;
    return 1;
}

static lib_i32 clts_s62_test_irq(void)
{
    static const lib_u8 code[] = {0x0fu,0x06u,0x90u};
    static const lib_u8 hlt = 0xf4u;
    clts_s62_machine state;
    core_machine_pic_irq_source irq;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    lib_u16 offset = 0x0100u;
    lib_u16 segment = 0u;
    lib_u16 frame_ip = 0u;
    lib_i32 failed = !clts_s62_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0u, code,
            sizeof(code)) != LIB_STATUS_OK || core_machine_memory_write(
            state.machine, 0x80u, &offset, sizeof(offset)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x82u, &segment,
            sizeof(segment)) != LIB_STATUS_OK || core_machine_memory_write(
            state.machine, 0x100u, &hlt, sizeof(hlt)) != LIB_STATUS_OK;
    }
    if (!failed) {
        clts_s62_seed(&state);
        state.machine->executor_cpu.data.cr0 |= VCPU_CR0_TS;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        lib_memory_set(&irq, 0, sizeof(irq));
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&irq, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&irq);
        core_machine_pic_irq_source_deassert(&irq);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){2u,0u}, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            after.data.ss.base + (lib_u16)after.data.esp, CORE_MACHINE_REFERENCE_OF(frame_ip),
            sizeof(frame_ip)) != LIB_STATUS_OK || after.data.eip != 0x101u ||
            frame_ip != 2u || !clts_s62_nonstack_data_equal(&before, &after) ||
            after.data.cr0 != (before.data.cr0 & ~VCPU_CR0_TS) ||
            after.data.eflags != (before.data.eflags & ~VCPU_EFLAGS_IF) ||
            !CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u)) ||
            CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!clts_s62_test_real_and_attributes()) {
        printf("CLTS stage=real-attributes\n");
        return 1;
    }
    if (!clts_s62_test_privilege()) {
        printf("CLTS stage=privilege\n");
        return 1;
    }
    if (!clts_s62_test_lock()) {
        printf("CLTS stage=lock\n");
        return 1;
    }
    if (!clts_s62_test_irq()) {
        printf("CLTS stage=irq\n");
        return 1;
    }
    printf("M5:T316:S62:CLTS:OK\n");
    return 0;
}
