#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"
#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/pic.h"
#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_cpu_fixture.h"

typedef struct lea_machine {
    core_machine *machine;
} lea_machine;

static void lea_reset(void *opaque)
{
    lea_machine *state = (lea_machine *)opaque;

    if (state != LIB_NULL)
        (void)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider lea_provider = {
    lea_reset, LIB_NULL
};

static lib_i32 lea_prepare(core_machine_cpu_profile profile, lea_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };

    lib_memory_set(state, 0, sizeof(*state));
return test_core_machine_fixture_create_bind_freeze_reset(&config,
        &lea_provider, state, &state->machine);
}

static lib_i32 lea_run_prepared(lea_machine *state, const lib_u8 *code,
    lib_u8 bytes, t_cpu *after, core_machine_cpu_diagnostic *diagnostic,
    lib_status *status)
{
    core_machine_run_result result;

    if (core_machine_memory_write(state->machine, 0u, code, bytes) !=
            LIB_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){ 1u, 0u }, &result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        LIB_STATUS_OK;
}

static void lea_set_registers(lea_machine *state)
{
    state->machine->executor_cpu.data.eax = 0xaabb0000u;
    state->machine->executor_cpu.data.ebx = 0x11112000u;
    state->machine->executor_cpu.data.esi = 0x12345000u;
    state->machine->executor_cpu.data.ebp = 0x56780000u;
    state->machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_ZF;
}

static lib_i32 lea_test_real_forms(void)
{
    static const lib_u8 code16[] = { 0x8du, 0x40u, 0x10u };
    static const lib_u8 code66[] = { 0x66u, 0x8du, 0x40u, 0x10u };
    static const lib_u8 code67[] = { 0x67u, 0x8du, 0x46u, 0x10u };
    static const lib_u8 code6766[] = {
        0x67u, 0x66u, 0x8du, 0x46u, 0x10u
    };
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    const lib_u8 *codes[] = { code16, code66, code67, code6766 };
    const lib_u8 code_bytes[] = { 3u, 4u, 4u, 5u };
    const lib_u32 expected_eax[] = {
        0xaabb7010u, 0x00007010u, 0xaabb5010u, 0x12345010u
    };
    lib_u8 profile;
    lib_u8 form;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
            ++profile) {
        for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form) {
            lea_machine state;
            t_cpu before = {0};
            t_cpu after = {0};
            core_machine_cpu_diagnostic diagnostic = {0};
            lib_status status = LIB_STATUS_INVALID_STATE;
            lib_i32 failed = !lea_prepare(profiles[profile], &state);

            if (!failed) {
                failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                    state.machine, 0u);
                lea_set_registers(&state);
                if (profile != 3u && form != 0u)
                    failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                        state.machine);
                before = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed |= !lea_run_prepared(&state, codes[form],
                        code_bytes[form], &after, &diagnostic, &status);
                if (profile == 3u || form == 0u) {
                    failed |= status != LIB_STATUS_OK ||
                        diagnostic.first_fault.valid ||
                        after.data.eip != code_bytes[form] ||
                        after.data.eax != expected_eax[form] ||
                        after.data.ebx != before.data.ebx ||
                        after.data.esi != before.data.esi ||
                        after.data.eflags != before.data.eflags;
                }
                else {
                    failed |= status != LIB_STATUS_INTERNAL_ERROR ||
                        !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
                            diagnostic.first_fault.exception_mask,
                            VCPUINS_EXCEPT_UD) || after.data.eip != before.data.eip ||
                        after.data.eax != before.data.eax ||
                        after.data.eflags != before.data.eflags;
                }
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

static lib_i32 lea_test_register_direct(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 code[] = { 0x8du, 0xc0u };
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
            ++profile) {
        lea_machine state;
        t_cpu before;
        t_cpu after;
        core_machine_cpu_diagnostic diagnostic;
        lib_status status;
        lib_i32 failed = !lea_prepare(profiles[profile], &state);

        if (!failed) {
            failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                state.machine, 0u);
            lea_set_registers(&state);
            failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
                state.machine);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= !lea_run_prepared(&state, code, sizeof(code), &after,
                    &diagnostic, &status) || status != LIB_STATUS_INTERNAL_ERROR ||
                !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
                    diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
                after.data.eip != before.data.eip ||
                after.data.eax != before.data.eax ||
                after.data.eflags != before.data.eflags;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 lea_test_lock_ud(void)
{
    static const lib_u8 code[] = { 0xf0u, 0x8du, 0x40u, 0x10u };
    lea_machine state;
    t_cpu before;
    t_cpu after;
    core_machine_cpu_diagnostic diagnostic;
    lib_status status;
    lib_i32 failed = !lea_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        failed |= !test_core_machine_fixture_prepare_real_mode_execution(
            state.machine, 0u);
        lea_set_registers(&state);
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !lea_run_prepared(&state, code, sizeof(code), &after,
                &diagnostic, &status) || status != LIB_STATUS_INTERNAL_ERROR ||
            !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            after.data.eip != before.data.eip || after.data.eax != before.data.eax ||
            after.data.eflags != before.data.eflags;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 lea_prepare_protected(lea_machine *state)
{
    static const lib_u8 pointer[] = { 0x1fu, 0, 0, 0x03u, 0, 0 };
    static const lib_u8 gdt[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0xffu, 0xffu, 0, 0x20u, 0, 0x9au, 0, 0,
        0xffu, 0xffu, 0, 0, 0, 0x92u, 0, 0, 0xffu, 0xffu, 0, 0x40u, 0, 0x92u,
        0, 0
    };
    static const lib_u8 bootstrap[] = {
        0x0fu, 0x01u, 0x16u, 0x00u, 0x01u, 0xb8u, 0x01u, 0x00u, 0x0fu, 0x01u,
        0xf0u, 0xb8u, 0x10u, 0x00u, 0x8eu, 0xd8u, 0x8eu, 0xc0u, 0xb8u, 0x18u,
        0x00u, 0x8eu, 0xd0u, 0xbcu, 0x00u, 0x80u, 0xeau, 0x00u, 0x00u, 0x08u,
        0x00u
    };
    static const lib_u8 hlt[] = { 0xf4u };
    core_machine_run_result result;

    return lea_prepare(CORE_MACHINE_CPU_PROFILE_80386, state) &&
        core_machine_memory_write(state->machine, 0x0100u, pointer,
            sizeof(pointer)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x0300u, gdt,
            sizeof(gdt)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0u, bootstrap,
            sizeof(bootstrap)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, 0x2000u, hlt,
            sizeof(hlt)) == LIB_STATUS_OK &&
        core_machine_run(state->machine, (core_machine_run_budget){ 96u, 0u },
            &result) == LIB_STATUS_OK &&
        result.reason == CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
}

static lib_i32 lea_test_protected(void)
{
    static const lib_u8 code16[] = { 0x8du, 0x40u, 0x10u };
    static const lib_u8 code66[] = { 0x66u, 0x8du, 0x40u, 0x10u };
    static const lib_u8 code67[] = { 0x67u, 0x8du, 0x46u, 0x10u };
    static const lib_u8 code6766[] = {
        0x67u, 0x66u, 0x8du, 0x46u, 0x10u
    };
    const lib_u8 *codes[] = { code16, code66, code67, code6766 };
    const lib_u8 code_bytes[] = { 3u, 4u, 4u, 5u };
    const lib_u32 expected_eax[] = {
        0xaabb7010u, 0x00007010u, 0xaabb5010u, 0x12345010u
    };
    lib_u8 form;

    for (form = 0u; form != sizeof(codes) / sizeof(codes[0]); ++form) {
        lea_machine state;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        lib_i32 failed = !lea_prepare_protected(&state);

        if (!failed) {
            lea_set_registers(&state);
            failed |= core_machine_memory_write(state.machine, 0x2000u,
                    codes[form], code_bytes[form]) != LIB_STATUS_OK;
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= core_machine_run(state.machine,
                    (core_machine_run_budget){ 1u, 0u }, &result) !=
                        LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= after.data.eip != code_bytes[form] ||
                after.data.eax != expected_eax[form] ||
                after.data.ebx != before.data.ebx ||
                after.data.esi != before.data.esi ||
                after.data.eflags != before.data.eflags;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 lea_test_null_ds_no_read(void)
{
    static const lib_u8 code[] = { 0x8du, 0x40u, 0x10u };
    lea_machine state;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    lib_i32 failed = !lea_prepare_protected(&state);

    if (!failed) {
        lea_set_registers(&state);
        state.machine->executor_cpu.data.ds.selector = 0u;
        state.machine->executor_cpu.data.ds.flagValid = LIB_FALSE;
        failed |= core_machine_memory_write(state.machine, 0x2000u, code,
                sizeof(code)) != LIB_STATUS_OK;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine,
                (core_machine_run_budget){ 1u, 0u }, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= after.data.eip != sizeof(code) ||
            after.data.eax != 0xaabb7010u ||
            after.data.ebx != before.data.ebx ||
            after.data.esi != before.data.esi ||
            after.data.eflags != before.data.eflags ||
            after.data.ds.selector != 0u || after.data.ds.flagValid;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 lea_test_irq_no_shadow(void)
{
    static const lib_u8 code[] = { 0x8du, 0x40u, 0x10u, 0x90u };
    static const lib_u8 hlt = 0xf4u;
    lea_machine state;
    core_machine_pic_irq_source source;
    core_machine_run_result result;
    t_cpu after;
    lib_u16 vector_offset = 0x0100u;
    lib_u16 vector_segment = 0u;
    lib_u16 frame_ip = 0u;
    lib_i32 failed = !lea_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        failed |= !test_core_machine_fixture_prepare_real_mode_execution(
                state.machine, 0u) ||
            core_machine_memory_write(state.machine, 0u, code,
                sizeof(code)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x20u * 4u,
                &vector_offset, sizeof(vector_offset)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x20u * 4u + 2u,
                &vector_segment, sizeof(vector_segment)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, 0x0100u, &hlt,
                sizeof(hlt)) != LIB_STATUS_OK;
    }
    if (!failed) {
        lea_set_registers(&state);
        state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
        lib_memory_set(&source, 0, sizeof(source));
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&source,
            &state.machine->shared_pic_master, &state.machine->shared_pic_slave,
            0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
        failed |= core_machine_run(state.machine,
                (core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (lib_u16)after.data.esp,
                (lib_uptr)&frame_ip, sizeof(frame_ip)) != LIB_STATUS_OK ||
            after.data.eip != 0x0101u || !CORE_MACHINE_BIT_IS_SET(
                state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u)) ||
            CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.irr,
                VPIC_IRR_IRQ(0u)) || frame_ip != 3u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!lea_test_real_forms() || !lea_test_register_direct() ||
            !lea_test_lock_ud() || !lea_test_protected() ||
            !lea_test_null_ds_no_read() || !lea_test_irq_no_shadow())
        return 1;
    printf("M5:T316:S26:LEA:OK\n");
    printf("M5:T401:S44:LEA-PROFILES:OK\n");
    return 0;
}
