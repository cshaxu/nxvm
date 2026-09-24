#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"
#define main protected_iret_s2_main
#include "core_machine_protected_iret_smoke.c"
#undef main

#define main cli_sti_s22_main
#include "core_machine_cli_sti_smoke.c"
#undef main

static lib_i32 iret_s51_sregs_same(const t_cpu *before, const t_cpu *after)
{
    return lib_memory_compare(&before->data.cs, &after->data.cs,
            sizeof(before->data.cs)) == 0 &&
        lib_memory_compare(&before->data.es, &after->data.es,
            sizeof(before->data.es)) == 0 &&
        lib_memory_compare(&before->data.ss, &after->data.ss,
            sizeof(before->data.ss)) == 0 &&
        lib_memory_compare(&before->data.ds, &after->data.ds,
            sizeof(before->data.ds)) == 0 &&
        lib_memory_compare(&before->data.fs, &after->data.fs,
            sizeof(before->data.fs)) == 0 &&
        lib_memory_compare(&before->data.gs, &after->data.gs,
            sizeof(before->data.gs)) == 0;
}

static void iret_s51_seed(cli_sti_machine *state, lib_u32 flags)
{
    t_cpu *cpu = &state->machine->executor_cpu;

    cpu->data.eax = 0xaabbccddu;
    cpu->data.ecx = 0x11223344u;
    cpu->data.edx = 0x55667788u;
    cpu->data.ebx = 0x99aabbccu;
    cpu->data.esp = 0x00018000u;
    cpu->data.ebp = 0x00000120u;
    cpu->data.esi = 0x00000010u;
    cpu->data.edi = 0x00000020u;
    cpu->data.eflags = flags;
}

static lib_u32 iret_s51_real_flags_load(
    core_machine_cpu_profile profile, lib_u32 flags)
{
    const lib_u16 known_mask = profile < CORE_MACHINE_CPU_PROFILE_80286 ?
        0x0fd5u : 0x7fd5u;

    return (flags & known_mask) | 0x02u;
}

static lib_i32 iret_s51_real_case(core_machine_cpu_profile profile,
    const lib_u8 *prefix, lib_u8 prefix_bytes, lib_u8 wrap_stack)
{
    static const lib_u8 hlt = 0xf4u;
    const lib_u32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_IF | VCPU_EFLAGS_DF | VCPU_EFLAGS_IOPL | VCPU_EFLAGS_NT |
        0x8002u;
    const lib_u32 expected_flags = iret_s51_real_flags_load(profile, flags);
    const lib_i32 wide = prefix_bytes != 0u && prefix[0] == 0x66u;
    const lib_u16 return_ip = wrap_stack ? 0x0200u : 0x0100u;
    const lib_u32 code_offset = wrap_stack ? 0x0100u : 0u;
    cli_sti_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    lib_u8 code[3] = { 0u, 0u, 0u };
    lib_u16 frame16[] = { return_ip, 0x0000u, (lib_u16)flags };
    lib_u32 frame32[] = { return_ip, 0x00000000u, flags };
    lib_u8 unselected_before[12] = {
        0xd1u, 0xd2u, 0xd3u, 0xd4u, 0xd5u, 0xd6u,
        0xd7u, 0xd8u, 0xd9u, 0xdau, 0xdbu, 0xdcu
    };
    lib_u8 unselected_after[12] = { 0u };
    lib_i32 failed = !cli_sti_prepare(profile, &state);

    if (!failed) {
        lib_memory_copy(code, prefix, prefix_bytes);
        code[prefix_bytes] = 0xcfu;
        failed = !test_core_machine_fixture_prepare_real_mode_execution(
            state.machine, code_offset);
        failed |= core_machine_memory_write(state.machine, code_offset, code,
            prefix_bytes + 1u) != LIB_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, return_ip, &hlt,
            sizeof(hlt)) != LIB_STATUS_OK;
        if (wrap_stack) {
            const lib_u32 stack_base = state.machine->executor_cpu.data.ss.base;

            failed |= wide || core_machine_memory_write(state.machine,
                stack_base + 0xfffeu, frame16, sizeof(frame16[0u])) != LIB_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, stack_base,
                frame16 + 1u, sizeof(frame16[0u])) != LIB_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, stack_base + 2u,
                frame16 + 2u, sizeof(frame16[0u])) != LIB_STATUS_OK;
        } else {
            failed |= core_machine_memory_write(state.machine, 0x8000u,
                wide ? (const void *)frame32 : (const void *)frame16,
                wide ? sizeof(frame32) : sizeof(frame16)) != LIB_STATUS_OK;
        }
        failed |= core_machine_memory_write(state.machine, 0x18000u,
            unselected_before, sizeof(unselected_before)) != LIB_STATUS_OK;
    }
    if (!failed) {
        iret_s51_seed(&state, flags);
        if (wrap_stack) state.machine->executor_cpu.data.sp = 0xfffeu;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK;
        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= diagnostic.first_fault.valid;
        failed |= after.data.eip != return_ip + 1u;
        failed |= !after.data.flagHalt;
        failed |= wrap_stack ? after.data.sp != 0x0004u :
            after.data.esp != before.data.esp + (wide ? 12u : 6u);
        failed |= after.data.cs.selector != 0u || after.data.cs.base != 0u;
        failed |= after.data.cs.limit != before.data.cs.limit;
        failed |= after.data.cs.flagValid != before.data.cs.flagValid;
        failed |= after.data.cs.sregtype != before.data.cs.sregtype;
        failed |= (after.data.eflags & iret_s51_real_flags_load(profile,
            0xffffu)) != expected_flags;
        failed |= after.data.eax != before.data.eax;
        failed |= after.data.ecx != before.data.ecx;
        failed |= after.data.edx != before.data.edx;
        failed |= after.data.ebx != before.data.ebx;
        failed |= after.data.ebp != before.data.ebp;
        failed |= after.data.esi != before.data.esi;
        failed |= after.data.edi != before.data.edi;
        failed |= !iret_s51_sregs_same(&before, &after);
        failed |= core_machine_memory_read_physical(
            &state.machine->executor_memory, 0x18000u,
            (lib_uptr)unselected_after,
            sizeof(unselected_after)) != LIB_STATUS_OK;
        failed |= lib_memory_compare(unselected_before, unselected_after,
            sizeof(unselected_before)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 iret_s51_test_real(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const lib_u8 prefixes[][2] = {
        { 0x66u, 0u },
        { 0x67u, 0u },
        { 0x66u, 0x67u }
    };
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        if (!iret_s51_real_case(profiles[profile], (const lib_u8[]){ 0u },
                0u, LIB_FALSE))
            return 0;
    }
    for (profile = 0u; profile != 4u; ++profile) {
        if (!iret_s51_real_case(profiles[profile], (const lib_u8[]){ 0u },
                0u, LIB_TRUE))
            return 0;
    }
    for (profile = 0u; profile != sizeof(prefixes) / sizeof(prefixes[0]);
        ++profile) {
        lib_u8 bytes = profile == 2u ? 2u : 1u;

        if (!iret_s51_real_case(CORE_MACHINE_CPU_PROFILE_80386,
                prefixes[profile], bytes, LIB_FALSE))
            return 0;
    }
    return 1;
}

static lib_i32 iret_s51_test_80286_stack_boundary(void)
{
    static const lib_u8 code[] = { 0xcfu };
    cli_sti_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    lib_i32 failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);

    if (!failed) {
        failed = !test_core_machine_fixture_prepare_real_mode_execution(
            state.machine, 0u);
        failed |= core_machine_memory_write(state.machine, 0u, code,
            sizeof(code)) != LIB_STATUS_OK;
    }
    if (!failed) {
        iret_s51_seed(&state, VCPU_EFLAGS_CF | VCPU_EFLAGS_IF);
        state.machine->executor_cpu.data.sp = 0xffffu;
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result) != LIB_STATUS_INTERNAL_ERROR;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK;
        failed |= result.reason != CORE_MACHINE_STOP_FAULT;
        failed |= !diagnostic.first_fault.valid;
        failed |= !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
            VCPUINS_EXCEPT_GP);
        failed |= lib_memory_compare(&before, &after, sizeof(before)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 iret_s51_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    cli_sti_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    lib_u8 stack_before[16] = { 0u };
    lib_u8 stack_after[16] = { 0u };
    lib_i32 failed = !cli_sti_prepare(profile, &state);

    if (!failed) {
        failed = !test_core_machine_fixture_prepare_real_mode_execution(
            state.machine, 0u);
        failed |= core_machine_memory_write(state.machine, 0u, code, bytes) !=
            LIB_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x7ff8u,
            stack_before, sizeof(stack_before)) != LIB_STATUS_OK;
    }
    if (!failed) {
        iret_s51_seed(&state, VCPU_EFLAGS_CF | VCPU_EFLAGS_IF);
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result) != LIB_STATUS_INTERNAL_ERROR;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK;
        failed |= result.reason != CORE_MACHINE_STOP_FAULT;
        failed |= !diagnostic.first_fault.valid;
        failed |= !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
            VCPUINS_EXCEPT_UD);
        failed |= lib_memory_compare(&before, &after, sizeof(before)) != 0;
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            0x7ff8u, (lib_uptr)stack_after,
            sizeof(stack_after)) != LIB_STATUS_OK;
        failed |= lib_memory_compare(stack_before, stack_after, sizeof(stack_before)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 iret_s51_test_rejections(void)
{
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const lib_u8 attributes[][3] = {
        { 0x66u, 0xcfu, 0u },
        { 0x67u, 0xcfu, 0u },
        { 0x66u, 0x67u, 0xcfu }
    };
    static const lib_u8 lock_forms[][4] = {
        { 0xf0u, 0xcfu, 0u, 0u },
        { 0xf0u, 0x66u, 0xcfu, 0u },
        { 0xf0u, 0x67u, 0xcfu, 0u },
        { 0xf0u, 0x66u, 0x67u, 0xcfu }
    };
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]);
        ++profile) {
        lib_u8 attribute;

        for (attribute = 0u;
            attribute != sizeof(attributes) / sizeof(attributes[0]); ++attribute) {
            lib_u8 bytes = attribute == 2u ? 3u : 2u;

            if (!iret_s51_expect_ud(legacy[profile], attributes[attribute], bytes))
                return 0;
        }
    }
    for (profile = 0u; profile != sizeof(lock_forms) / sizeof(lock_forms[0]);
        ++profile) {
        lib_u8 bytes = profile == 3u ? 4u : profile == 0u ? 2u : 3u;

        if (!iret_s51_expect_ud(CORE_MACHINE_CPU_PROFILE_80386,
                lock_forms[profile], bytes))
            return 0;
    }
    return 1;
}

static lib_i32 iret_s51_test_protected(void)
{
    return iret_test_success(0u, 0, 0, 0) &&
        iret_test_success(0x66u, 1, 0, 0) &&
        iret_test_success(0x67u, 0, 0, 0) &&
        iret_test_success(0x66u, 1, 1, 0) &&
        iret_test_user_flags() &&
        iret_test_failure(IRET_NEGATIVE_NONPRESENT, VCPUINS_EXCEPT_DF, 0u) &&
        iret_test_failure(IRET_NEGATIVE_LIMIT, VCPUINS_EXCEPT_DF, 0u) &&
        iret_test_failure(IRET_NEGATIVE_CODE_TYPE, VCPUINS_EXCEPT_DF, 0u) &&
        iret_test_failure(IRET_NEGATIVE_CODE_DPL, VCPUINS_EXCEPT_DF, 0u) &&
        iret_test_failure(IRET_NEGATIVE_STACK_LIMIT, VCPUINS_EXCEPT_DF, 0u);
}

static lib_i32 iret_s51_test_pic(void)
{
    static const lib_u8 code[] = { 0xcfu, 0x90u };
    static const lib_u8 hlt = 0xf4u;
    const lib_u16 offset = 0x0100u;
    const lib_u16 segment = 0u;
    const lib_u32 vector = 0x20u;
    lib_u8 restore_if;

    for (restore_if = 0u; restore_if != 2u; ++restore_if) {
        cli_sti_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        t_cpu after;
        lib_u16 frame[] = { 0x0001u, 0x0000u,
            (lib_u16)(restore_if ? VCPU_EFLAGS_IF : 0u) };
        lib_u16 frame_ip = 0u;
        lib_i32 failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed) {
            failed = !test_core_machine_fixture_prepare_real_mode_execution(
                state.machine, 0u);
            failed |= core_machine_memory_write(state.machine, 0u, code,
                sizeof(code)) != LIB_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, 0x8000u, frame,
                sizeof(frame)) != LIB_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, vector * 4u,
                &offset, sizeof(offset)) != LIB_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, vector * 4u + 2u,
                &segment, sizeof(segment)) != LIB_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, offset, &hlt,
                sizeof(hlt)) != LIB_STATUS_OK;
        }
        if (!failed) {
            iret_s51_seed(&state, VCPU_EFLAGS_CF);
            lib_memory_set(&source, 0, sizeof(source));
            state.machine->shared_pic_master.data.icw2 = (lib_u8)vector;
            core_machine_pic_irq_source_bind(&source,
                &state.machine->shared_pic_master,
                &state.machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            if (restore_if) {
                failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
                failed |= after.data.eip != offset + 1u;
                failed |= !after.data.flagHalt;
                failed |= !CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.isr,
                    VPIC_ISR_IRQ(0u));
                failed |= CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.irr,
                    VPIC_IRR_IRQ(0u));
                failed |= core_machine_memory_read_physical(
                    &state.machine->executor_memory,
                    after.data.ss.base + (lib_u16)after.data.esp,
                    CORE_MACHINE_REFERENCE_OF(frame_ip), sizeof(frame_ip)) != LIB_STATUS_OK;
                failed |= frame_ip != 1u;
            } else {
                failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
                failed |= after.data.eip != 2u;
                failed |= CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.isr,
                    VPIC_ISR_IRQ(0u));
                failed |= !CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.irr,
                    VPIC_IRR_IRQ(0u));
            }
        }
        if (failed) {
            core_machine_destroy(state.machine);
            return 0;
        }
        core_machine_destroy(state.machine);
    }
    return 1;
}

lib_i32 main(void)
{
    if (!iret_s51_test_real() || !iret_s51_test_80286_stack_boundary() ||
        !iret_s51_test_rejections() ||
        !iret_s51_test_protected() || !iret_s51_test_pic())
        return 1;
    printf("M5:T316:S51:IRET:OK\n");
    printf("M5:T401:S27:IRET-PROFILES:OK\n");
    return 0;
}
