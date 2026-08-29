#define main cli_sti_s22_main
#include "core_machine_cli_sti_smoke.c"
#undef main

static C_INT hlt_s49_gprs_preserved(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi;
}

static C_INT hlt_s49_sregs_preserved(const t_cpu *before,
    const t_cpu *after)
{
    return STD_MEMCMP(&before->data.es, &after->data.es,
            sizeof(before->data.es)) == 0 &&
        STD_MEMCMP(&before->data.cs, &after->data.cs,
            sizeof(before->data.cs)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after->data.ss,
            sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after->data.ds,
            sizeof(before->data.ds)) == 0 &&
        STD_MEMCMP(&before->data.fs, &after->data.fs,
            sizeof(before->data.fs)) == 0 &&
        STD_MEMCMP(&before->data.gs, &after->data.gs,
            sizeof(before->data.gs)) == 0;
}

static C_VOID hlt_s49_seed(cli_sti_machine *state)
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
    cpu->data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF |
        VCPU_EFLAGS_DF | VCPU_EFLAGS_OF;
}

static C_INT hlt_s49_run(cli_sti_machine *state, const type_unsigned_8 *code,
    type_unsigned_8 bytes, type_unsigned_32 budget, type_status *status,
    core_machine_run_result *result, t_cpu *after,
    core_machine_cpu_diagnostic *diagnostic)
{
    if (core_machine_memory_write(state->machine, 0u, code, bytes) !=
        TYPE_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){ budget, 0u }, result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        TYPE_STATUS_OK;
}

static C_INT hlt_s49_test_defaults(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    const type_unsigned_8 code[] = { 0xf4u };
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        cli_sti_machine state;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        t_cpu before = {0};
        t_cpu after = {0};
        type_status status = TYPE_STATUS_INVALID_STATE;
        C_INT failed = !cli_sti_prepare(profiles[profile], &state);

        if (!failed) {
            hlt_s49_seed(&state);
            before = test_core_machine_fixture_capture_cpu_after_run(
                state.machine);
            failed = !hlt_s49_run(&state, code, sizeof(code), 1u, &status,
                &result, &after, &diagnostic);
            failed |= status != TYPE_STATUS_OK;
            failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            failed |= diagnostic.first_fault.valid;
            failed |= after.data.eip != 1u;
            failed |= !after.data.flagHalt;
            failed |= after.data.eflags != before.data.eflags;
            failed |= !hlt_s49_gprs_preserved(&before, &after);
            failed |= !hlt_s49_sregs_preserved(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT hlt_s49_test_attributes_and_rejections(C_VOID)
{
    static const type_unsigned_8 prefixes[][2] = {
        { 0x66u, 0u },
        { 0x67u, 0u },
        { 0x66u, 0x67u }
    };
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    type_unsigned_8 prefix;

    for (prefix = 0u; prefix != sizeof(prefixes) / sizeof(prefixes[0]);
        ++prefix) {
        cli_sti_machine state;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        t_cpu before = {0};
        t_cpu after = {0};
        type_status status = TYPE_STATUS_INVALID_STATE;
        type_unsigned_8 code[] = { prefixes[prefix][0], 0xf4u, 0u };
        type_unsigned_8 bytes = prefix == 2u ? 3u : 2u;
        C_INT failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (prefix == 2u) {
            code[1] = prefixes[prefix][1];
            code[2] = 0xf4u;
        }
        if (!failed) {
            hlt_s49_seed(&state);
            before = test_core_machine_fixture_capture_cpu_after_run(
                state.machine);
            failed = !hlt_s49_run(&state, code, bytes, 1u, &status, &result,
                &after, &diagnostic);
            failed |= status != TYPE_STATUS_OK;
            failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            failed |= diagnostic.first_fault.valid;
            failed |= after.data.eip != bytes;
            failed |= !after.data.flagHalt;
            failed |= after.data.eflags != before.data.eflags;
            failed |= !hlt_s49_gprs_preserved(&before, &after);
            failed |= !hlt_s49_sregs_preserved(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    for (prefix = 0u; prefix != sizeof(prefixes) / sizeof(prefixes[0]);
        ++prefix) {
        type_unsigned_8 profile;

        for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]);
            ++profile) {
            cli_sti_machine state;
            core_machine_cpu_diagnostic diagnostic;
            core_machine_run_result result;
            t_cpu before;
            t_cpu after;
            type_status status;
            type_unsigned_8 code[] = { prefixes[prefix][0], 0xf4u, 0u };
            type_unsigned_8 bytes = prefix == 2u ? 3u : 2u;
            C_INT failed = !cli_sti_prepare(legacy[profile], &state);

            if (prefix == 2u) {
                code[1] = prefixes[prefix][1];
                code[2] = 0xf4u;
            }
            if (!failed) {
                hlt_s49_seed(&state);
                failed = !test_core_machine_fixture_preflight_real_ud_terminal(
                    state.machine);
                before = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed |= !hlt_s49_run(&state, code, bytes, 1u, &status,
                    &result, &after, &diagnostic);
                failed |= status != TYPE_STATUS_FAULT;
                failed |= result.reason != CORE_MACHINE_STOP_FAULT;
                failed |= !diagnostic.first_fault.valid;
                failed |= !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_UD);
                failed |= STD_MEMCMP(&before, &after, sizeof(before)) != 0;
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    for (prefix = 0u; prefix != 4u; ++prefix) {
        cli_sti_machine state;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        t_cpu before;
        t_cpu after;
        type_status status;
        type_unsigned_8 code[] = { 0xf0u, 0xf4u, 0u, 0u };
        type_unsigned_8 bytes = prefix == 0u ? 2u : prefix == 3u ? 4u : 3u;
        C_INT failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80386,
            &state);

        if (prefix != 0u) {
            code[1] = prefixes[prefix - 1u][0];
            code[2] = 0xf4u;
        }
        if (prefix == 3u) {
            code[2] = prefixes[2][1];
            code[3] = 0xf4u;
        }
        if (!failed) {
            hlt_s49_seed(&state);
            failed = !test_core_machine_fixture_preflight_real_ud_terminal(
                state.machine);
            before = test_core_machine_fixture_capture_cpu_after_run(
                state.machine);
            failed |= !hlt_s49_run(&state, code, bytes, 1u, &status, &result,
                &after, &diagnostic);
            failed |= status != TYPE_STATUS_FAULT;
            failed |= result.reason != CORE_MACHINE_STOP_FAULT;
            failed |= !diagnostic.first_fault.valid;
            failed |= !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_UD);
            failed |= STD_MEMCMP(&before, &after, sizeof(before)) != 0;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT hlt_s49_test_protected(C_VOID)
{
    const type_unsigned_8 code[] = { 0xf4u };
    cli_sti_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    type_status status;
    C_INT failed = !cli_sti_prepare_protected(&state);

    if (!failed) {
        hlt_s49_seed(&state);
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed = core_machine_memory_write(state.machine, 0x2000u, code,
            sizeof(code)) != TYPE_STATUS_OK;
        status = core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result);
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK;
        failed |= status != TYPE_STATUS_OK;
        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= diagnostic.first_fault.valid;
        failed |= after.data.eip != 1u;
        failed |= !after.data.flagHalt;
        failed |= after.data.eflags != before.data.eflags;
        failed |= !hlt_s49_gprs_preserved(&before, &after);
        failed |= !hlt_s49_sregs_preserved(&before, &after);
    }
    core_machine_destroy(state.machine);
    if (failed)
        return 0;

    failed = !cli_sti_prepare_protected(&state);
    if (!failed) {
        hlt_s49_seed(&state);
        state.machine->executor_cpu.data.cs.dpl = 3u;
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed = core_machine_memory_write(state.machine, 0x2000u, code,
            sizeof(code)) != TYPE_STATUS_OK;
        status = core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result);
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK;
        failed |= status != TYPE_STATUS_FAULT;
        failed |= result.reason != CORE_MACHINE_STOP_FAULT;
        failed |= !diagnostic.first_fault.valid;
        failed |= !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
            VCPUINS_EXCEPT_DF);
        failed |= STD_MEMCMP(&before, &after, sizeof(before)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT hlt_s49_test_vm86(C_VOID)
{
    const type_unsigned_8 code[] = { 0xf4u };
    cli_sti_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    t_cpu after;
    type_status status;
    const type_unsigned_32 flags = VCPU_EFLAGS_VM | VCPU_EFLAGS_CF |
        VCPU_EFLAGS_IOPL;
    C_INT failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        failed = !test_core_machine_fixture_prepare_real_mode_execution(
            state.machine, 0u);
        failed |= core_machine_memory_write(state.machine, 0u, code,
            sizeof(code)) != TYPE_STATUS_OK;
        failed |= !cli_sti_install_gp_gate(&state);
    }
    if (!failed) {
        hlt_s49_seed(&state);
        state.machine->executor_cpu.data.cr0 |= VCPU_CR0_PE;
        state.machine->executor_cpu.data.eflags = flags;
        state.machine->executor_cpu.data.esp = 0x00008000u;
        state.machine->executor_cpu.data.eip = 0u;
        state.machine->executor_cpu.data.cs.selector = 0u;
        state.machine->executor_cpu.data.cs.base = 0u;
        state.machine->executor_cpu.data.cs.limit = 0xffffu;
        state.machine->executor_cpu.data.cs.dpl = 3u;
        state.machine->executor_cpu.data.cs.flagValid = TYPE_TRUE;
        state.machine->executor_cpu.data.cs.seg.exec.defsize = TYPE_FALSE;
        state.machine->executor_cpu.data.ss.selector = 0u;
        state.machine->executor_cpu.data.ss.base = 0u;
        state.machine->executor_cpu.data.ss.limit = 0xffffu;
        state.machine->executor_cpu.data.ss.dpl = 3u;
        state.machine->executor_cpu.data.ss.flagValid = TYPE_TRUE;
        state.machine->executor_cpu.data.ss.seg.data.big = TYPE_FALSE;
        status = core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result);
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK;
        failed |= status != TYPE_STATUS_OK;
        failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
        failed |= diagnostic.first_fault.valid;
        failed |= !diagnostic.last_delivered_exception.valid;
        failed |= !TYPE_GET_BIT(diagnostic.last_delivered_exception.exception_mask,
            VCPUINS_EXCEPT_GP);
        failed |= after.data.cs.selector != 0x0008u ||
            after.data.ss.selector != 0x0010u || after.data.eip != 0x00000100u ||
            TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_VM);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT hlt_s49_test_irq(C_VOID)
{
    static const type_unsigned_8 hlt = 0xf4u;
    const type_unsigned_32 vector = 0x20u;
    const type_unsigned_16 offset = 0x0100u;
    const type_unsigned_16 segment = 0u;
    cli_sti_machine state;
    core_machine_pic_irq_source source;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    type_unsigned_16 frame_ip = 0u;
    C_INT failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);

    if (!failed) {
        failed = !test_core_machine_fixture_prepare_real_mode_execution(
            state.machine, 0u);
        failed |= core_machine_memory_write(state.machine, vector * 4u,
            &offset, sizeof(offset)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, vector * 4u + 2u,
            &segment, sizeof(segment)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, offset, &hlt,
            sizeof(hlt)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0u, &hlt,
            sizeof(hlt)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        hlt_s49_seed(&state);
        state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        STD_MEMSET(&source, 0, sizeof(source));
        state.machine->shared_pic_master.data.icw2 = (type_unsigned_8)vector;
        core_machine_pic_irq_source_bind(&source,
            &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
        failed = core_machine_run(state.machine,
            (core_machine_run_budget){ 2u, 0u }, &result) != TYPE_STATUS_OK;
        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= after.data.eip != offset + 1u;
        failed |= !after.data.flagHalt;
        failed |= after.data.eflags !=
            (before.data.eflags & ~VCPU_EFLAGS_IF);
        failed |= after.data.eax != before.data.eax;
        failed |= after.data.ecx != before.data.ecx;
        failed |= after.data.edx != before.data.edx;
        failed |= after.data.ebx != before.data.ebx;
        failed |= after.data.ebp != before.data.ebp;
        failed |= after.data.esi != before.data.esi;
        failed |= after.data.edi != before.data.edi;
        failed |= after.data.esp != ((before.data.esp & 0xffff0000u) |
            (type_unsigned_16)(before.data.esp - 6u));
        failed |= !hlt_s49_sregs_preserved(&before, &after);
        failed |= !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
            VPIC_ISR_IRQ(0u));
        failed |= TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
            VPIC_IRR_IRQ(0u));
        failed |= core_machine_memory_read_physical(
            &state.machine->executor_memory,
            after.data.ss.base + (type_unsigned_16)after.data.esp,
            (type_virtual_address)&frame_ip, sizeof(frame_ip)) != TYPE_STATUS_OK;
        failed |= frame_ip != 1u;
    }
    core_machine_destroy(state.machine);
    if (failed)
        return 0;

    failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state);
    if (!failed) {
        failed = !test_core_machine_fixture_prepare_real_mode_execution(
            state.machine, 0u);
        failed |= core_machine_memory_write(state.machine, 0u, &hlt,
            sizeof(hlt)) != TYPE_STATUS_OK;
    }
    if (!failed) {
        hlt_s49_seed(&state);
        state.machine->executor_cpu.data.eflags &= ~VCPU_EFLAGS_IF;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        STD_MEMSET(&source, 0, sizeof(source));
        core_machine_pic_irq_source_bind(&source,
            &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
        failed = core_machine_run(state.machine,
            (core_machine_run_budget){ 2u, 0u }, &result) != TYPE_STATUS_OK;
        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= after.data.eip != 1u;
        failed |= !after.data.flagHalt;
        failed |= after.data.eflags != before.data.eflags;
        failed |= !hlt_s49_gprs_preserved(&before, &after);
        failed |= !hlt_s49_sregs_preserved(&before, &after);
        failed |= !TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
            VPIC_IRR_IRQ(0u));
        failed |= TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
            VPIC_ISR_IRQ(0u));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

C_INT main(C_VOID)
{
    if (!hlt_s49_test_defaults())
        return 1;
    if (!hlt_s49_test_attributes_and_rejections())
        return 1;
    if (!hlt_s49_test_protected())
        return 1;
    if (!hlt_s49_test_vm86())
        return 1;
    if (!hlt_s49_test_irq())
        return 1;
    STD_PRINTF("M5:T316:S49:HLT:OK\n");
    STD_PRINTF("M5:T401:S38:HLT-PROFILES:OK\n");
    return 0;
}
