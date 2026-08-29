#define main protected_iret_s2_main
#include "core_machine_protected_iret_smoke.c"
#undef main

#define main cli_sti_s22_main
#include "core_machine_cli_sti_smoke.c"
#undef main

static C_INT iret_s51_sregs_same(const t_cpu *before, const t_cpu *after)
{
    return STD_MEMCMP(&before->data.cs, &after->data.cs,
            sizeof(before->data.cs)) == 0 &&
        STD_MEMCMP(&before->data.es, &after->data.es,
            sizeof(before->data.es)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after->data.ss,
            sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after->data.ds,
            sizeof(before->data.ds)) == 0 &&
        STD_MEMCMP(&before->data.fs, &after->data.fs,
            sizeof(before->data.fs)) == 0 &&
        STD_MEMCMP(&before->data.gs, &after->data.gs,
            sizeof(before->data.gs)) == 0;
}

static C_VOID iret_s51_seed(cli_sti_machine *state, type_unsigned_32 flags)
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

static type_unsigned_32 iret_s51_real_flags_load(
    core_machine_cpu_profile profile, type_unsigned_32 flags)
{
    const type_unsigned_16 known_mask = profile < CORE_MACHINE_CPU_PROFILE_80286 ?
        0x0fd5u : (profile == CORE_MACHINE_CPU_PROFILE_80286 ? 0x7fd5u :
            0xffd5u);

    return (flags & known_mask) | 0x02u;
}

static C_INT iret_s51_real_case(core_machine_cpu_profile profile,
    const type_unsigned_8 *prefix, type_unsigned_8 prefix_bytes)
{
    static const type_unsigned_8 hlt = 0xf4u;
    const type_unsigned_32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_IF | VCPU_EFLAGS_DF | VCPU_EFLAGS_IOPL | VCPU_EFLAGS_NT |
        0x8002u;
    const type_unsigned_32 expected_flags = iret_s51_real_flags_load(profile, flags);
    const C_INT wide = prefix_bytes != 0u && prefix[0] == 0x66u;
    cli_sti_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 code[3] = { 0u, 0u, 0u };
    type_unsigned_16 frame16[] = { 0x0100u, 0x0000u, (type_unsigned_16)flags };
    type_unsigned_32 frame32[] = { 0x00000100u, 0x00000000u, flags };
    type_unsigned_8 unselected_before[12] = {
        0xd1u, 0xd2u, 0xd3u, 0xd4u, 0xd5u, 0xd6u,
        0xd7u, 0xd8u, 0xd9u, 0xdau, 0xdbu, 0xdcu
    };
    type_unsigned_8 unselected_after[12] = { 0u };
    C_INT failed = !cli_sti_prepare(profile, &state);

    if (!failed) {
        STD_MEMCPY(code, prefix, prefix_bytes);
        code[prefix_bytes] = 0xcfu;
        failed = !test_core_machine_fixture_prepare_real_mode_execution(
            state.machine, 0u);
        failed |= core_machine_memory_write(state.machine, 0u, code,
            prefix_bytes + 1u) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x0100u, &hlt,
            sizeof(hlt)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x8000u,
            wide ? (const C_VOID *)frame32 : (const C_VOID *)frame16,
            wide ? sizeof(frame32) : sizeof(frame16)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x18000u,
            unselected_before, sizeof(unselected_before)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        iret_s51_seed(&state, flags);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 2u, 0u }, &result) != TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK;
        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= diagnostic.first_fault.valid;
        failed |= after.data.eip != 0x0101u;
        failed |= !after.data.flagHalt;
        failed |= after.data.esp != before.data.esp + (wide ? 12u : 6u);
        failed |= after.data.cs.selector != 0u || after.data.cs.base != 0u;
        failed |= after.data.cs.limit != before.data.cs.limit;
        failed |= after.data.cs.flagValid != before.data.cs.flagValid;
        failed |= after.data.cs.sregtype != before.data.cs.sregtype;
        failed |= after.data.eflags != expected_flags;
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
            (type_virtual_address)unselected_after,
            sizeof(unselected_after)) != TYPE_STATUS_OK;
        failed |= STD_MEMCMP(unselected_before, unselected_after,
            sizeof(unselected_before)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT iret_s51_test_real(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const type_unsigned_8 prefixes[][2] = {
        { 0x66u, 0u },
        { 0x67u, 0u },
        { 0x66u, 0x67u }
    };
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        if (!iret_s51_real_case(profiles[profile], (const type_unsigned_8[]){ 0u },
                0u))
            return 0;
    }
    for (profile = 0u; profile != sizeof(prefixes) / sizeof(prefixes[0]);
        ++profile) {
        type_unsigned_8 bytes = profile == 2u ? 2u : 1u;

        if (!iret_s51_real_case(CORE_MACHINE_CPU_PROFILE_80386,
                prefixes[profile], bytes))
            return 0;
    }
    return 1;
}

static C_INT iret_s51_expect_ud(core_machine_cpu_profile profile,
    const type_unsigned_8 *code, type_unsigned_8 bytes)
{
    cli_sti_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 stack_before[16] = { 0u };
    type_unsigned_8 stack_after[16] = { 0u };
    C_INT failed = !cli_sti_prepare(profile, &state);

    if (!failed) {
        failed = !test_core_machine_fixture_prepare_real_mode_execution(
            state.machine, 0u);
        failed |= core_machine_memory_write(state.machine, 0u, code, bytes) !=
            TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x7ff8u,
            stack_before, sizeof(stack_before)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        iret_s51_seed(&state, VCPU_EFLAGS_CF | VCPU_EFLAGS_IF);
        failed |= !test_core_machine_fixture_preflight_real_ud_terminal(
            state.machine);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result) != TYPE_STATUS_FAULT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK;
        failed |= result.reason != CORE_MACHINE_STOP_FAULT;
        failed |= !diagnostic.first_fault.valid;
        failed |= !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
            VCPUINS_EXCEPT_UD);
        failed |= STD_MEMCMP(&before, &after, sizeof(before)) != 0;
        failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
            0x7ff8u, (type_virtual_address)stack_after,
            sizeof(stack_after)) != TYPE_STATUS_OK;
        failed |= STD_MEMCMP(stack_before, stack_after, sizeof(stack_before)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT iret_s51_test_rejections(C_VOID)
{
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    static const type_unsigned_8 attributes[][3] = {
        { 0x66u, 0xcfu, 0u },
        { 0x67u, 0xcfu, 0u },
        { 0x66u, 0x67u, 0xcfu }
    };
    static const type_unsigned_8 lock_forms[][4] = {
        { 0xf0u, 0xcfu, 0u, 0u },
        { 0xf0u, 0x66u, 0xcfu, 0u },
        { 0xf0u, 0x67u, 0xcfu, 0u },
        { 0xf0u, 0x66u, 0x67u, 0xcfu }
    };
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]);
        ++profile) {
        type_unsigned_8 attribute;

        for (attribute = 0u;
            attribute != sizeof(attributes) / sizeof(attributes[0]); ++attribute) {
            type_unsigned_8 bytes = attribute == 2u ? 3u : 2u;

            if (!iret_s51_expect_ud(legacy[profile], attributes[attribute], bytes))
                return 0;
        }
    }
    for (profile = 0u; profile != sizeof(lock_forms) / sizeof(lock_forms[0]);
        ++profile) {
        type_unsigned_8 bytes = profile == 3u ? 4u : profile == 0u ? 2u : 3u;

        if (!iret_s51_expect_ud(CORE_MACHINE_CPU_PROFILE_80386,
                lock_forms[profile], bytes))
            return 0;
    }
    return 1;
}

static C_INT iret_s51_test_protected(C_VOID)
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

static C_INT iret_s51_test_pic(C_VOID)
{
    static const type_unsigned_8 code[] = { 0xcfu, 0x90u };
    static const type_unsigned_8 hlt = 0xf4u;
    const type_unsigned_16 offset = 0x0100u;
    const type_unsigned_16 segment = 0u;
    const type_unsigned_32 vector = 0x20u;
    type_unsigned_8 restore_if;

    for (restore_if = 0u; restore_if != 2u; ++restore_if) {
        cli_sti_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result;
        t_cpu after;
        type_unsigned_16 frame[] = { 0x0001u, 0x0000u,
            (type_unsigned_16)(restore_if ? VCPU_EFLAGS_IF : 0u) };
        type_unsigned_16 frame_ip = 0u;
        C_INT failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (!failed) {
            failed = !test_core_machine_fixture_prepare_real_mode_execution(
                state.machine, 0u);
            failed |= core_machine_memory_write(state.machine, 0u, code,
                sizeof(code)) != TYPE_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, 0x8000u, frame,
                sizeof(frame)) != TYPE_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, vector * 4u,
                &offset, sizeof(offset)) != TYPE_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, vector * 4u + 2u,
                &segment, sizeof(segment)) != TYPE_STATUS_OK;
            failed |= core_machine_memory_write(state.machine, offset, &hlt,
                sizeof(hlt)) != TYPE_STATUS_OK;
        }
        if (!failed) {
            iret_s51_seed(&state, VCPU_EFLAGS_CF);
            STD_MEMSET(&source, 0, sizeof(source));
            state.machine->shared_pic_master.data.icw2 = (type_unsigned_8)vector;
            core_machine_pic_irq_source_bind(&source,
                &state.machine->shared_pic_master,
                &state.machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){ 2u, 0u }, &result) != TYPE_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            if (restore_if) {
                failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
                failed |= after.data.eip != offset + 1u;
                failed |= !after.data.flagHalt;
                failed |= !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                    VPIC_ISR_IRQ(0u));
                failed |= TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
                    VPIC_IRR_IRQ(0u));
                failed |= core_machine_memory_read_physical(
                    &state.machine->executor_memory,
                    after.data.ss.base + (type_unsigned_16)after.data.esp,
                    TYPE_REFERENCE_OF(frame_ip), sizeof(frame_ip)) != TYPE_STATUS_OK;
                failed |= frame_ip != 1u;
            } else {
                failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
                failed |= after.data.eip != 2u;
                failed |= TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                    VPIC_ISR_IRQ(0u));
                failed |= !TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
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

C_INT main(C_VOID)
{
    if (!iret_s51_test_real() || !iret_s51_test_rejections() ||
        !iret_s51_test_protected() || !iret_s51_test_pic())
        return 1;
    STD_PRINTF("M5:T316:S51:IRET:OK\n");
    STD_PRINTF("M5:T401:S27:IRET-PROFILES:OK\n");
    return 0;
}
