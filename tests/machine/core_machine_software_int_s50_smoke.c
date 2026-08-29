#define main cli_sti_s22_main
#include "core_machine_cli_sti_smoke.c"
#undef main

#define main interrupt_entry_main
#include "core_machine_interrupt_entry_smoke.c"
#undef main

typedef struct software_int_form {
    type_unsigned_8 vector;
    type_unsigned_8 opcode[2];
    type_unsigned_8 bytes;
    C_INT requires_overflow;
} software_int_form;

static C_INT software_int_s50_gprs_same(const t_cpu *before,
    const t_cpu *after)
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

static C_INT software_int_s50_sregs_same(const t_cpu *before,
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

static C_VOID software_int_s50_seed(cli_sti_machine *state, type_unsigned_32 flags)
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
    cpu->data.eflags = flags;
}

static type_unsigned_16 software_int_s50_real_flags_image(
    core_machine_cpu_profile profile, type_unsigned_16 flags)
{
    if (profile < CORE_MACHINE_CPU_PROFILE_80286) flags &= 0x0fffu;
    return (type_unsigned_16)((flags & ~VCPU_EFLAGS_RESERVED) | 0x02u);
}

static type_unsigned_16 software_int_s50_real_flags_known_mask(
    core_machine_cpu_profile profile)
{
    if (profile < CORE_MACHINE_CPU_PROFILE_80286) return 0x0fd5u;
    if (profile == CORE_MACHINE_CPU_PROFILE_80286) return 0x7fd5u;
    return 0xffffu;
}

static C_INT software_int_s50_prepare_real(cli_sti_machine *state,
    core_machine_cpu_profile profile, const software_int_form *form,
    const type_unsigned_8 *prefix, type_unsigned_8 prefix_bytes)
{
    static const type_unsigned_8 hlt = 0xf4u;
    const type_unsigned_16 offset = 0x0100u;
    const type_unsigned_16 segment = 0u;
    type_unsigned_8 code[5] = { 0u };

    STD_MEMCPY(code, prefix, prefix_bytes);
    STD_MEMCPY(code + prefix_bytes, form->opcode, form->bytes);
    return cli_sti_prepare(profile, state) &&
        test_core_machine_fixture_prepare_real_mode_execution(state->machine, 0u) &&
        core_machine_memory_write(state->machine, form->vector * 4u, &offset,
            sizeof(offset)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, form->vector * 4u + 2u,
            &segment, sizeof(segment)) == TYPE_STATUS_OK &&
        core_machine_memory_write(state->machine, offset, &hlt, sizeof(hlt)) ==
            TYPE_STATUS_OK && core_machine_memory_write(state->machine, 0u,
            code, prefix_bytes + form->bytes) == TYPE_STATUS_OK;
}

static C_INT software_int_s50_check_real_frame(cli_sti_machine *state,
    const t_cpu *before, const t_cpu *after, type_unsigned_32 return_ip,
    type_unsigned_8 width, core_machine_cpu_profile profile)
{
    if (width == 2u) {
        type_unsigned_16 frame[3] = { 0u, 0u, 0u };

        return core_machine_memory_read_physical(&state->machine->executor_memory,
                after->data.ss.base + (type_unsigned_16)after->data.esp,
                (type_virtual_address)frame, sizeof(frame)) == TYPE_STATUS_OK &&
            frame[0] == return_ip && frame[1] == before->data.cs.selector &&
            (frame[2] & software_int_s50_real_flags_known_mask(profile)) ==
            (software_int_s50_real_flags_image(profile,
                (type_unsigned_16)before->data.eflags) &
                software_int_s50_real_flags_known_mask(profile));
    }
    {
        type_unsigned_32 frame[3] = { 0u, 0u, 0u };

        return core_machine_memory_read_physical(&state->machine->executor_memory,
                after->data.ss.base + after->data.esp,
                (type_virtual_address)frame, sizeof(frame)) == TYPE_STATUS_OK &&
            frame[0] == return_ip && frame[1] == before->data.cs.selector &&
            (frame[2] & software_int_s50_real_flags_known_mask(profile)) ==
            (software_int_s50_real_flags_image(profile,
                (type_unsigned_16)before->data.eflags) &
                software_int_s50_real_flags_known_mask(profile));
    }
}

static C_INT software_int_s50_real_transfer(core_machine_cpu_profile profile,
    const software_int_form *form, const type_unsigned_8 *prefix, type_unsigned_8 prefix_bytes)
{
    const type_unsigned_32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_IF |
        VCPU_EFLAGS_TF | VCPU_EFLAGS_DF | VCPU_EFLAGS_OF;
    const type_unsigned_8 width = prefix_bytes != 0u && prefix[0] == 0x66u ? 4u : 2u;
    cli_sti_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    type_status status;
    type_unsigned_32 return_ip = prefix_bytes + form->bytes;
    C_INT failed = !software_int_s50_prepare_real(&state, profile, form, prefix,
        prefix_bytes);

    if (!failed) {
        software_int_s50_seed(&state, flags);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        status = core_machine_run(state.machine,
            (core_machine_run_budget){ 2u, 0u }, &result);
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK;
        failed |= status != TYPE_STATUS_OK;
        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= diagnostic.first_fault.valid;
        failed |= after.data.eip != 0x0101u;
        failed |= !after.data.flagHalt;
        failed |= after.data.esp != before.data.esp - 3u * width;
        failed |= after.data.eflags != (before.data.eflags &
            ~(VCPU_EFLAGS_IF | VCPU_EFLAGS_TF));
        failed |= after.data.eax != before.data.eax;
        failed |= after.data.ecx != before.data.ecx;
        failed |= after.data.edx != before.data.edx;
        failed |= after.data.ebx != before.data.ebx;
        failed |= after.data.ebp != before.data.ebp;
        failed |= after.data.esi != before.data.esi;
        failed |= after.data.edi != before.data.edi;
        failed |= !software_int_s50_sregs_same(&before, &after);
        failed |= !software_int_s50_check_real_frame(&state, &before, &after,
            return_ip, width, profile);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT software_int_s50_real_into_clear(core_machine_cpu_profile profile,
    const type_unsigned_8 *prefix, type_unsigned_8 prefix_bytes)
{
    const software_int_form into = { 0x04u, { 0xceu, 0u }, 1u, 1 };
    const type_unsigned_32 flags = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF | VCPU_EFLAGS_IF |
        VCPU_EFLAGS_DF;
    static const type_unsigned_8 hlt = 0xf4u;
    cli_sti_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    type_status status;
    type_unsigned_8 code[5] = { 0u };
    C_INT failed = !cli_sti_prepare(profile, &state);

    if (!failed) {
        STD_MEMCPY(code, prefix, prefix_bytes);
        code[prefix_bytes] = into.opcode[0];
        code[prefix_bytes + 1u] = hlt;
        failed = !test_core_machine_fixture_prepare_real_mode_execution(
            state.machine, 0u);
        failed |= core_machine_memory_write(state.machine, 0u, code,
            prefix_bytes + 2u) != TYPE_STATUS_OK;
    }
    if (!failed) {
        software_int_s50_seed(&state, flags);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        status = core_machine_run(state.machine,
            (core_machine_run_budget){ 2u, 0u }, &result);
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK;
        failed |= status != TYPE_STATUS_OK;
        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= diagnostic.first_fault.valid;
        failed |= after.data.eip != prefix_bytes + 2u;
        failed |= !after.data.flagHalt;
        failed |= !software_int_s50_gprs_same(&before, &after);
        failed |= after.data.eflags != before.data.eflags;
        failed |= !software_int_s50_sregs_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT software_int_s50_test_real_forms(C_VOID)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    static const software_int_form forms[] = {
        { 0x03u, { 0xccu, 0u }, 1u, 0 },
        { 0x31u, { 0xcdu, 0x31u }, 2u, 0 },
        { 0x04u, { 0xceu, 0u }, 1u, 1 }
    };
    static const type_unsigned_8 no_prefix[] = { 0u };
    static const type_unsigned_8 prefixes[][2] = {
        { 0x66u, 0u },
        { 0x67u, 0u },
        { 0x66u, 0x67u }
    };
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        type_unsigned_8 form;

        for (form = 0u; form != sizeof(forms) / sizeof(forms[0]); ++form) {
            if (!software_int_s50_real_transfer(profiles[profile], &forms[form],
                    no_prefix, 0u))
                return 0;
        }
        if (!software_int_s50_real_into_clear(profiles[profile], no_prefix, 0u))
            return 0;
    }
    for (profile = 0u; profile != sizeof(prefixes) / sizeof(prefixes[0]);
        ++profile) {
        type_unsigned_8 form;
        type_unsigned_8 prefix_bytes = profile == 2u ? 2u : 1u;

        for (form = 0u; form != sizeof(forms) / sizeof(forms[0]); ++form) {
            if (!software_int_s50_real_transfer(CORE_MACHINE_CPU_PROFILE_80386,
                    &forms[form], prefixes[profile], prefix_bytes))
                return 0;
        }
        if (!software_int_s50_real_into_clear(CORE_MACHINE_CPU_PROFILE_80386,
                prefixes[profile], prefix_bytes))
            return 0;
    }
    return 1;
}

static C_INT software_int_s50_test_rejections(C_VOID)
{
    static const software_int_form forms[] = {
        { 0x03u, { 0xccu, 0u }, 1u, 0 },
        { 0x31u, { 0xcdu, 0x31u }, 2u, 0 },
        { 0x04u, { 0xceu, 0u }, 1u, 1 }
    };
    static const type_unsigned_8 prefixes[][2] = {
        { 0x66u, 0u },
        { 0x67u, 0u },
        { 0x66u, 0x67u }
    };
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_8088,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    type_unsigned_8 profile;

    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]);
        ++profile) {
        type_unsigned_8 prefix;

        for (prefix = 0u; prefix != sizeof(prefixes) / sizeof(prefixes[0]);
            ++prefix) {
            type_unsigned_8 form;

            for (form = 0u; form != sizeof(forms) / sizeof(forms[0]); ++form) {
                cli_sti_machine state;
                core_machine_cpu_diagnostic diagnostic;
                core_machine_run_result result;
                t_cpu before;
                t_cpu after;
                type_status status;
                type_unsigned_8 code[5] = { 0u };
                type_unsigned_8 prefix_bytes = prefix == 2u ? 2u : 1u;
                type_unsigned_8 stack_before[16] = { 0u };
                type_unsigned_8 stack_after[16] = { 0u };
                C_INT failed = !cli_sti_prepare(legacy[profile], &state);

                STD_MEMCPY(code, prefixes[prefix], prefix_bytes);
                STD_MEMCPY(code + prefix_bytes, forms[form].opcode,
                    forms[form].bytes);
                if (!failed) {
                    failed = !test_core_machine_fixture_prepare_real_mode_execution(
                        state.machine, 0u);
                    failed |= core_machine_memory_write(state.machine, 0u, code,
                        prefix_bytes + forms[form].bytes) != TYPE_STATUS_OK;
                    failed |= core_machine_memory_write(state.machine, 0x7ff0u,
                        stack_before, sizeof(stack_before)) != TYPE_STATUS_OK;
                }
                if (!failed) {
                    software_int_s50_seed(&state, VCPU_EFLAGS_OF |
                        VCPU_EFLAGS_IF | VCPU_EFLAGS_CF);
                    failed = !test_core_machine_fixture_preflight_real_ud_terminal(
                        state.machine);
                    before = test_core_machine_fixture_capture_cpu_after_run(
                        state.machine);
                    status = core_machine_run(state.machine,
                        (core_machine_run_budget){ 1u, 0u }, &result);
                    after = test_core_machine_fixture_capture_cpu_after_run(
                        state.machine);
                    failed |= core_machine_get_cpu_diagnostic(state.machine,
                        &diagnostic) != TYPE_STATUS_OK;
                    failed |= status != TYPE_STATUS_FAULT;
                    failed |= result.reason != CORE_MACHINE_STOP_FAULT;
                    failed |= !diagnostic.first_fault.valid;
                    failed |= !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                        VCPUINS_EXCEPT_UD);
                    failed |= STD_MEMCMP(&before, &after, sizeof(before)) != 0;
                    failed |= core_machine_memory_read_physical(
                        &state.machine->executor_memory, 0x7ff0u,
                        (type_virtual_address)stack_after,
                        sizeof(stack_after)) != TYPE_STATUS_OK;
                    failed |= STD_MEMCMP(stack_before, stack_after,
                        sizeof(stack_before)) != 0;
                }
                core_machine_destroy(state.machine);
                if (failed)
                    return 0;
            }
        }
    }
    for (profile = 0u; profile != 4u; ++profile) {
        type_unsigned_8 form;

        for (form = 0u; form != sizeof(forms) / sizeof(forms[0]); ++form) {
            cli_sti_machine state;
            core_machine_cpu_diagnostic diagnostic;
            core_machine_run_result result;
            t_cpu before;
            t_cpu after;
            type_status status;
            type_unsigned_8 code[] = { 0xf0u, 0u, 0u, 0u, 0u };
            type_unsigned_8 prefix_bytes = profile == 0u ? 0u :
                profile == 3u ? 2u : 1u;
            type_unsigned_8 bytes;
            type_unsigned_8 stack_before[16] = { 0u };
            type_unsigned_8 stack_after[16] = { 0u };
            C_INT failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80386,
                &state);

            if (profile != 0u)
                STD_MEMCPY(code + 1u, prefixes[profile - 1u], prefix_bytes);
            STD_MEMCPY(code + 1u + prefix_bytes, forms[form].opcode,
                forms[form].bytes);
            bytes = 1u + prefix_bytes + forms[form].bytes;
            if (!failed) {
                failed = !test_core_machine_fixture_prepare_real_mode_execution(
                    state.machine, 0u);
                failed |= core_machine_memory_write(state.machine, 0u, code,
                    bytes) != TYPE_STATUS_OK;
                failed |= core_machine_memory_write(state.machine, 0x7ff0u,
                    stack_before, sizeof(stack_before)) != TYPE_STATUS_OK;
            }
            if (!failed) {
                software_int_s50_seed(&state, VCPU_EFLAGS_OF |
                    VCPU_EFLAGS_IF | VCPU_EFLAGS_CF);
                failed = !test_core_machine_fixture_preflight_real_ud_terminal(
                    state.machine);
                before = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                status = core_machine_run(state.machine,
                    (core_machine_run_budget){ 1u, 0u }, &result);
                after = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed |= core_machine_get_cpu_diagnostic(state.machine,
                    &diagnostic) != TYPE_STATUS_OK;
                failed |= status != TYPE_STATUS_FAULT;
                failed |= result.reason != CORE_MACHINE_STOP_FAULT;
                failed |= !diagnostic.first_fault.valid;
                failed |= !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_UD);
                failed |= STD_MEMCMP(&before, &after, sizeof(before)) != 0;
                failed |= core_machine_memory_read_physical(
                    &state.machine->executor_memory, 0x7ff0u,
                    (type_virtual_address)stack_after,
                    sizeof(stack_after)) != TYPE_STATUS_OK;
                failed |= STD_MEMCMP(stack_before, stack_after,
                    sizeof(stack_before)) != 0;
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

static C_INT software_int_s50_test_protected(C_VOID)
{
    static const software_int_form forms[] = {
        { 0x03u, { 0xccu, 0u }, 1u, 0 },
        { IE_VECTOR, { 0xcdu, IE_VECTOR }, 2u, 0 },
        { 0x04u, { 0xceu, 0u }, 1u, 1 }
    };
    type_unsigned_8 form;

    for (form = 0u; form != sizeof(forms) / sizeof(forms[0]); ++form) {
        interrupt_entry_machine state;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu after;
        type_unsigned_32 frame[3] = { 0u, 0u, 0u };
        type_unsigned_8 code[2] = { forms[form].opcode[0], forms[form].opcode[1] };
        type_unsigned_32 flags = 0x00000302u |
            (forms[form].requires_overflow ? VCPU_EFLAGS_OF : 0u);
        C_INT failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE,
            VCPU_DESC_SYS_TYPE_INTGATE_32);

        if (!failed) {
            failed = !ie_install_gate(&state, forms[form].vector, 0x0008u,
                0xeeu);
            failed |= !ie_write(&state, IE_CODE_BASE, code,
                forms[form].bytes);
            state.machine->executor_cpu.data.eflags = flags;
            failed |= !ie_run(&state, 0, &after, &diagnostic);
            failed |= diagnostic.first_fault.valid;
            failed |= after.data.cs.selector != 0x0008u;
            failed |= after.data.eip != IE_HANDLER_OFFSET + 1u;
            failed |= after.data.esp != IE_STACK_BASE - 12u;
            failed |= TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_IF);
            failed |= TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_TF);
            failed |= !ie_read(&state, IE_STACK_BASE - 12u, frame,
                sizeof(frame));
            failed |= frame[0] != forms[form].bytes;
            failed |= frame[1] != 0x0008u;
            failed |= frame[2] != flags;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    for (form = 0u; form != sizeof(forms) / sizeof(forms[0]); ++form) {
        interrupt_entry_machine state;
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        type_unsigned_8 code[2] = { forms[form].opcode[0], forms[form].opcode[1] };
        C_INT failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE,
            VCPU_DESC_SYS_TYPE_INTGATE_32);

        if (!failed) {
            failed = !ie_prepare_user_code(&state);
            failed |= !ie_install_gate(&state, forms[form].vector, 0x000bu,
                0x8eu);
            failed |= !ie_write(&state, IE_CODE_BASE, code,
                forms[form].bytes);
            state.machine->executor_cpu.data.eflags = 0x00000302u |
                (forms[form].requires_overflow ? VCPU_EFLAGS_OF : 0u);
            before = test_core_machine_fixture_capture_cpu_after_run(
                state.machine);
            failed |= !ie_run(&state, 1, &after, &diagnostic);
            failed |= !ie_fault_is(&diagnostic, VCPUINS_EXCEPT_DF, 0u);
            failed |= STD_MEMCMP(&before, &after, sizeof(before)) != 0;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static C_INT software_int_s50_test_protected_faults_and_vm86(C_VOID)
{
    interrupt_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 target_access = 0x92u;
    C_INT failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_IDT_LIMIT,
        VCPU_DESC_SYS_TYPE_INTGATE_32);

    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed = !ie_run(&state, 1, &after, &diagnostic);
        failed |= !ie_fault_is(&diagnostic, VCPUINS_EXCEPT_DF, 0u);
        failed |= STD_MEMCMP(&before, &after, sizeof(before)) != 0;
    }
    core_machine_destroy(state.machine);
    if (failed)
        return 0;

    failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE,
        VCPU_DESC_SYS_TYPE_INTGATE_32);
    if (!failed) {
        failed = !ie_write(&state, IE_GDT_BASE + 13u, &target_access,
            sizeof(target_access));
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !ie_run(&state, 1, &after, &diagnostic);
        failed |= !ie_fault_is(&diagnostic, VCPUINS_EXCEPT_DF, 0u);
        failed |= STD_MEMCMP(&before, &after, sizeof(before)) != 0;
    }
    core_machine_destroy(state.machine);
    if (failed)
        return 0;

    failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE,
        VCPU_DESC_SYS_TYPE_INTGATE_32);
    if (!failed) {
        state.machine->executor_cpu.data.cs.dpl = 3u;
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_VM |
            VCPU_EFLAGS_CF;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed = !ie_run(&state, 1, &after, &diagnostic);
        failed |= !ie_fault_is(&diagnostic, VCPUINS_EXCEPT_DF, 0u);
        failed |= STD_MEMCMP(&before, &after, sizeof(before)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT software_int_s50_test_pic_boundary(C_VOID)
{
    const software_int_form form = { 0x31u, { 0xcdu, 0x31u }, 2u, 0 };
    const type_unsigned_32 flags = VCPU_EFLAGS_IF | VCPU_EFLAGS_CF;
    cli_sti_machine state;
    core_machine_pic_irq_source source;
    core_machine_run_result result;
    t_cpu after;
    C_INT failed = !software_int_s50_prepare_real(&state,
        CORE_MACHINE_CPU_PROFILE_80386, &form, (const type_unsigned_8[]){ 0u }, 0u);

    if (!failed) {
        software_int_s50_seed(&state, flags);
        STD_MEMSET(&source, 0, sizeof(source));
        state.machine->shared_pic_master.data.icw2 = 0x20u;
        core_machine_pic_irq_source_bind(&source,
            &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
        failed = core_machine_run(state.machine,
            (core_machine_run_budget){ 2u, 0u }, &result) != TYPE_STATUS_OK;
        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= after.data.eip != 0x0101u;
        failed |= !after.data.flagHalt;
        failed |= TYPE_GET_BIT(after.data.eflags, VCPU_EFLAGS_IF);
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
    if (!software_int_s50_test_real_forms())
        return 1;
    if (!software_int_s50_test_rejections())
        return 1;
    if (!software_int_s50_test_protected())
        return 1;
    if (!software_int_s50_test_protected_faults_and_vm86())
        return 1;
    if (!software_int_s50_test_pic_boundary())
        return 1;
    STD_PRINTF("M5:T316:S50:SOFTWARE-INT:OK\n");
    STD_PRINTF("M5:T401:S25:INT-IMMEDIATE-PROFILES:OK\n");
    STD_PRINTF("M5:T401:S26:INT3-INTO-PROFILES:OK\n");
    return 0;
}
