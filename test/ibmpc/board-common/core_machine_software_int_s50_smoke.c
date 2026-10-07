#include "lib/types/test.h"
#include "lib/types/file.h"
#include "pic_fixture.h"
#include "ibmpc/board-common/machine_board_state.h"
#include "lib/types/types_interface.h"
#include "x86/core/device_support_interface.h"
#include "core_machine_board_fixture.h"
#define main cli_sti_s22_main
#include "machine_cli_sti_interrupt_smoke.c"
#undef main

#define main interrupt_entry_main
#include "machine_interrupt_entry_smoke.c"
#undef main

typedef struct software_int_form {
    lib_u8 vector;
    lib_u8 opcode[2];
    lib_u8 bytes;
    lib_i32 requires_overflow;
} software_int_form;

static lib_bool software_int_s50_is_halted(const core_machine *machine)
{
    core_machine_cpu_state state;
    if (core_machine_get_cpu_state(machine, &state) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
    return state.halted != 0u;
}

static lib_i32 software_int_s50_gprs_same(const core_machine_debug_cpu_snapshot *before,
    const core_machine_debug_cpu_snapshot *after)
{
    return before->eax == after->eax &&
        before->ecx == after->ecx &&
        before->edx == after->edx &&
        before->ebx == after->ebx &&
        before->esp == after->esp &&
        before->ebp == after->ebp &&
        before->esi == after->esi &&
        before->edi == after->edi;
}

static lib_i32 software_int_s50_sregs_same(const core_machine_debug_cpu_snapshot *before,
    const core_machine_debug_cpu_snapshot *after)
{
    return lib_memory_compare(&before->es, &after->es,
            sizeof(before->es)) == 0 &&
        lib_memory_compare(&before->cs, &after->cs,
            sizeof(before->cs)) == 0 &&
        lib_memory_compare(&before->ss, &after->ss,
            sizeof(before->ss)) == 0 &&
        lib_memory_compare(&before->ds, &after->ds,
            sizeof(before->ds)) == 0 &&
        lib_memory_compare(&before->fs, &after->fs,
            sizeof(before->fs)) == 0 &&
        lib_memory_compare(&before->gs, &after->gs,
            sizeof(before->gs)) == 0;
}

static lib_i32 software_int_s50_run_shutdown(interrupt_entry_machine *state,
    core_machine_debug_cpu_snapshot *out_cpu,
    core_machine_cpu_diagnostic *out_diagnostic)
{
    core_machine_run_result result;
    const lib_status status = test_core_machine_fixture_run_after_delivery(
        state->machine, (core_machine_run_budget){32u, 0u}, &result);

    if (core_machine_get_cpu_diagnostic(state->machine, out_diagnostic) !=
        LIB_STATUS_OK) return 0;
    *out_cpu = ie_capture(state->machine);
    return test_core_machine_fixture_shutdown_wait(status, &result);
}

static void software_int_s50_seed(cli_sti_machine *state, lib_u32 flags)
{
    const core_machine_debug_register_patch patch = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ECX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EBP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EDI) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
        .values = {
            [CORE_MACHINE_DEBUG_EAX] = 0xaabbccddu,
            [CORE_MACHINE_DEBUG_ECX] = 0x11223344u,
            [CORE_MACHINE_DEBUG_EDX] = 0x55667788u,
            [CORE_MACHINE_DEBUG_EBX] = 0x99aabbccu,
            [CORE_MACHINE_DEBUG_ESP] = 0x8000u,
            [CORE_MACHINE_DEBUG_EBP] = 0x120u,
            [CORE_MACHINE_DEBUG_ESI] = 0x10u,
            [CORE_MACHINE_DEBUG_EDI] = 0x20u,
            [CORE_MACHINE_DEBUG_EFLAGS] = flags
        }
    };
    if (core_machine_debug_patch_registers(state->machine, &patch) != LIB_STATUS_OK)
        lib_test_assert(LIB_FALSE);
}

static lib_u16 software_int_s50_real_flags_image(
    core_machine_cpu_profile profile, lib_u16 flags)
{
    if (profile < CORE_MACHINE_CPU_PROFILE_80286) flags &= 0x0fffu;
    return (lib_u16)((flags & ~0x802au) | 0x02u);
}

static lib_u16 software_int_s50_real_flags_known_mask(
    core_machine_cpu_profile profile)
{
    if (profile < CORE_MACHINE_CPU_PROFILE_80286) return 0x0fd5u;
    return 0x7fd5u;
}

static lib_i32 software_int_s50_prepare_real(cli_sti_machine *state,
    core_machine_cpu_profile profile, const software_int_form *form,
    const lib_u8 *prefix, lib_u8 prefix_bytes)
{
    static const lib_u8 hlt = 0xf4u;
    const lib_u16 offset = 0x0100u;
    const lib_u16 segment = 0u;
    lib_u8 code[5] = { 0u };

    lib_memory_copy(code, prefix, prefix_bytes);
    lib_memory_copy(code + prefix_bytes, form->opcode, form->bytes);
    return cli_sti_prepare(profile, state, LIB_FALSE) &&
        cli_sti_real_entry(state->machine, 0u) &&
        core_machine_memory_write(state->machine, form->vector * 4u, &offset,
            sizeof(offset)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, form->vector * 4u + 2u,
            &segment, sizeof(segment)) == LIB_STATUS_OK &&
        core_machine_memory_write(state->machine, offset, &hlt, sizeof(hlt)) ==
            LIB_STATUS_OK && core_machine_memory_write(state->machine, 0u,
            code, prefix_bytes + form->bytes) == LIB_STATUS_OK;
}

static lib_i32 software_int_s50_check_real_frame(cli_sti_machine *state,
    const core_machine_debug_cpu_snapshot *before, const core_machine_debug_cpu_snapshot *after, lib_u32 return_ip,
    lib_u8 width, core_machine_cpu_profile profile)
{
    if (width == 2u) {
        lib_u16 frame[3] = { 0u, 0u, 0u };

        return core_machine_memory_inspect(state->machine,
                after->ss.base + (lib_u16)after->esp,
                (void *)frame, sizeof(frame)) == LIB_STATUS_OK &&
            frame[0] == return_ip && frame[1] == before->cs.selector &&
            (frame[2] & software_int_s50_real_flags_known_mask(profile)) ==
            (software_int_s50_real_flags_image(profile,
                (lib_u16)before->eflags) &
                software_int_s50_real_flags_known_mask(profile)) &&
            (profile != CORE_MACHINE_CPU_PROFILE_80386 ||
                (frame[2] & 0x8000u) == 0u);
    }
    {
        lib_u32 frame[3] = { 0u, 0u, 0u };

        return core_machine_memory_inspect(state->machine,
                after->ss.base + after->esp,
                (void *)frame, sizeof(frame)) == LIB_STATUS_OK &&
            frame[0] == return_ip && frame[1] == before->cs.selector &&
            (frame[2] & software_int_s50_real_flags_known_mask(profile)) ==
            (software_int_s50_real_flags_image(profile,
                (lib_u16)before->eflags) &
                software_int_s50_real_flags_known_mask(profile)) &&
            (profile != CORE_MACHINE_CPU_PROFILE_80386 ||
                (frame[2] & 0x8000u) == 0u);
    }
}

static lib_i32 software_int_s50_real_transfer(core_machine_cpu_profile profile,
    const software_int_form *form, const lib_u8 *prefix, lib_u8 prefix_bytes)
{
    const lib_u32 flags = CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_PF | CORE_MACHINE_DEBUG_EFLAGS_IF |
        CORE_MACHINE_DEBUG_EFLAGS_TF | CORE_MACHINE_DEBUG_EFLAGS_DF | CORE_MACHINE_DEBUG_EFLAGS_OF | 0x8000u;
    const lib_u8 width = prefix_bytes != 0u && prefix[0] == 0x66u ? 4u : 2u;
    cli_sti_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    core_machine_debug_cpu_snapshot before;
    core_machine_debug_cpu_snapshot after;
    lib_status status;
    lib_u32 return_ip = prefix_bytes + form->bytes;
    lib_i32 failed = !software_int_s50_prepare_real(&state, profile, form, prefix,
        prefix_bytes);

    if (!failed) {
        software_int_s50_seed(&state, flags);
        before = cli_sti_capture(state.machine);
        status = core_machine_run(state.machine,
            (core_machine_run_budget){ 2u, 0u }, &result);
        after = cli_sti_capture(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK;
        failed |= status != LIB_STATUS_OK;
        if (!failed) failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        if (!failed) failed |= diagnostic.first_fault.valid;
        failed |= after.eip != 0x0101u;
        failed |= !software_int_s50_is_halted(state.machine);
        failed |= after.esp != before.esp - 3u * width;
        failed |= after.eflags != (before.eflags &
            ~(CORE_MACHINE_DEBUG_EFLAGS_IF | CORE_MACHINE_DEBUG_EFLAGS_TF));
        failed |= after.eax != before.eax;
        failed |= after.ecx != before.ecx;
        failed |= after.edx != before.edx;
        failed |= after.ebx != before.ebx;
        failed |= after.ebp != before.ebp;
        failed |= after.esi != before.esi;
        failed |= after.edi != before.edi;
        failed |= !software_int_s50_sregs_same(&before, &after);
        failed |= !software_int_s50_check_real_frame(&state, &before, &after,
            return_ip, width, profile);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 software_int_s50_real_into_clear(core_machine_cpu_profile profile,
    const lib_u8 *prefix, lib_u8 prefix_bytes)
{
    const software_int_form into = { 0x04u, { 0xceu, 0u }, 1u, 1 };
    const lib_u32 flags = CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_PF | CORE_MACHINE_DEBUG_EFLAGS_IF |
        CORE_MACHINE_DEBUG_EFLAGS_DF;
    static const lib_u8 hlt = 0xf4u;
    cli_sti_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    core_machine_debug_cpu_snapshot before;
    core_machine_debug_cpu_snapshot after;
    lib_status status;
    lib_u8 code[5] = { 0u };
    lib_i32 failed = !cli_sti_prepare(profile, &state, LIB_FALSE);

    if (!failed) {
        lib_memory_copy(code, prefix, prefix_bytes);
        code[prefix_bytes] = into.opcode[0];
        code[prefix_bytes + 1u] = hlt;
        failed = !cli_sti_real_entry(
            state.machine, 0u);
        failed |= core_machine_memory_write(state.machine, 0u, code,
            prefix_bytes + 2u) != LIB_STATUS_OK;
    }
    if (!failed) {
        software_int_s50_seed(&state, flags);
        before = cli_sti_capture(state.machine);
        status = core_machine_run(state.machine,
            (core_machine_run_budget){ 2u, 0u }, &result);
        after = cli_sti_capture(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK;
        failed |= status != LIB_STATUS_OK;
        if (!failed) failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        if (!failed) failed |= diagnostic.first_fault.valid;
        failed |= after.eip != prefix_bytes + 2u;
        failed |= !software_int_s50_is_halted(state.machine);
        failed |= !software_int_s50_gprs_same(&before, &after);
        failed |= after.eflags != before.eflags;
        failed |= !software_int_s50_sregs_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 software_int_s50_test_real_forms(void)
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
    static const lib_u8 no_prefix[] = { 0u };
    static const lib_u8 prefixes[][2] = {
        { 0x66u, 0u },
        { 0x67u, 0u },
        { 0x66u, 0x67u }
    };
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        lib_u8 form;

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
        lib_u8 form;
        lib_u8 prefix_bytes = profile == 2u ? 2u : 1u;

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


static lib_i32 software_int_s50_test_protected(void)
{
    static const software_int_form forms[] = {
        { 0x03u, { 0xccu, 0u }, 1u, 0 },
        { IE_VECTOR, { 0xcdu, IE_VECTOR }, 2u, 0 },
        { 0x04u, { 0xceu, 0u }, 1u, 1 }
    };
    lib_u8 form;

    for (form = 0u; form != sizeof(forms) / sizeof(forms[0]); ++form) {
        interrupt_entry_machine state;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_debug_cpu_snapshot after;
        lib_u32 frame[3] = { 0u, 0u, 0u };
        lib_u8 code[2] = { forms[form].opcode[0], forms[form].opcode[1] };
        lib_u32 flags = 0x00000302u |
            (forms[form].requires_overflow ? CORE_MACHINE_DEBUG_EFLAGS_OF : 0u);
        lib_i32 failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE,
            IE_INTGATE_32);

        if (!failed) {
            failed = !ie_install_gate(&state, forms[form].vector, 0x0008u,
                0xeeu);
            failed |= !ie_write(&state, IE_CODE_BASE, code,
                forms[form].bytes);
            test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_EFLAGS, flags);
            failed |= !ie_run(&state, 0, &after, &diagnostic);
        }
        if (!failed) {
            failed |= diagnostic.first_fault.valid;
            failed |= after.cs.selector != 0x0008u;
            failed |= after.eip != IE_HANDLER_OFFSET + 1u;
            failed |= after.esp != IE_STACK_BASE - 12u;
            failed |= CORE_MACHINE_BIT_IS_SET(after.eflags, CORE_MACHINE_DEBUG_EFLAGS_IF);
            failed |= CORE_MACHINE_BIT_IS_SET(after.eflags, CORE_MACHINE_DEBUG_EFLAGS_TF);
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
        core_machine_debug_cpu_snapshot before;
        core_machine_debug_cpu_snapshot after;
        lib_u8 code[2] = { forms[form].opcode[0], forms[form].opcode[1] };
        lib_i32 failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE,
            IE_INTGATE_32);

        if (!failed) {
            failed = !ie_prepare_user_code(&state);
            failed |= !ie_install_gate(&state, forms[form].vector, 0x000bu,
                0x8eu);
            failed |= !ie_write(&state, IE_CODE_BASE, code,
                forms[form].bytes);
            test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_EFLAGS, 0x00000302u |
                (forms[form].requires_overflow ? CORE_MACHINE_DEBUG_EFLAGS_OF : 0u));
            before = cli_sti_capture(
                state.machine);
            failed |= !software_int_s50_run_shutdown(&state, &after,
                &diagnostic);
            if (!failed) failed |= diagnostic.first_fault.valid ||
                !diagnostic.last_delivered_exception.valid ||
                !CORE_MACHINE_BIT_IS_SET(
                    diagnostic.last_delivered_exception.exception_mask,
                    VCPUINS_EXCEPT_SHUTDOWN) ||
                lib_memory_compare(&before, &after, sizeof(before)) != 0;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 software_int_s50_test_protected_faults_and_vm86(void)
{
    interrupt_entry_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_debug_cpu_snapshot before;
    core_machine_debug_cpu_snapshot after;
    lib_u8 target_access = 0x92u;
    lib_i32 failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_IDT_LIMIT,
        IE_INTGATE_32);

    if (!failed) {
        before = cli_sti_capture(state.machine);
        failed = !software_int_s50_run_shutdown(&state, &after, &diagnostic);
        if (!failed) failed |= diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            !CORE_MACHINE_BIT_IS_SET(
                diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_SHUTDOWN) ||
            lib_memory_compare(&before, &after, sizeof(before)) != 0;
    }
    core_machine_destroy(state.machine);
    if (failed)
        return 0;

    failed = !ie_prepare(&state, INTERRUPT_ENTRY_NEGATIVE_NONE,
        IE_INTGATE_32);
    if (!failed) {
        failed = !ie_write(&state, IE_GDT_BASE + 13u, &target_access,
            sizeof(target_access));
        before = cli_sti_capture(state.machine);
        failed |= !software_int_s50_run_shutdown(&state, &after, &diagnostic);
        if (!failed) failed |= diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            !CORE_MACHINE_BIT_IS_SET(
                diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_SHUTDOWN) ||
            lib_memory_compare(&before, &after, sizeof(before)) != 0;
    }
    core_machine_destroy(state.machine);
    if (failed)
        return 0;

    cli_sti_machine vm86;
    const lib_u8 code[] = {0xcdu,IE_VECTOR};
    const lib_u8 empty_gate[8] = {0};
    failed = !cli_sti_prepare_mode(&vm86, 2u,
        CORE_MACHINE_DEBUG_EFLAGS_VM | CORE_MACHINE_DEBUG_EFLAGS_CF | 2u);
    if (!failed) {
        failed = core_machine_memory_write(vm86.machine, 0u, code, sizeof(code)) != LIB_STATUS_OK ||
            core_machine_memory_write(vm86.machine, 0x400u + 13u * 8u,
                empty_gate, sizeof(empty_gate)) != LIB_STATUS_OK;
        before = cli_sti_capture(vm86.machine);
        core_machine_run_result result;
        const lib_status status = core_machine_run(vm86.machine,
            (core_machine_run_budget){1u,0u}, &result);
        after = cli_sti_capture(vm86.machine);
        failed |= !test_core_machine_fixture_shutdown_wait(status, &result) ||
            core_machine_get_cpu_diagnostic(vm86.machine, &diagnostic) != LIB_STATUS_OK ||
            diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            !CORE_MACHINE_BIT_IS_SET(diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_SHUTDOWN) ||
            lib_memory_compare(&before, &after, sizeof(before)) != 0;
    }
    core_machine_destroy(vm86.machine);
    return !failed;
}

static lib_i32 software_int_s50_test_pic_boundary(void)
{
    const software_int_form form = { 0x31u, { 0xcdu, 0x31u }, 2u, 0 };
    const lib_u32 flags = CORE_MACHINE_DEBUG_EFLAGS_IF | CORE_MACHINE_DEBUG_EFLAGS_CF;
    cli_sti_machine state;
    core_machine_pic_irq_source *source = LIB_NULL;
    core_machine_run_result result;
    core_machine_debug_cpu_snapshot after;
    lib_i32 failed = !software_int_s50_prepare_real(&state,
        CORE_MACHINE_CPU_PROFILE_80386, &form, (const lib_u8[]){ 0u }, 0u);

    if (!failed) {
        software_int_s50_seed(&state, flags);
        lib_memory_set(&source, 0, sizeof(source));
        test_pic_program_vector(state.board->shared_pic_master, 0x20u);
        test_pic_bind_source(&source,
            state.board->shared_pic_master,
            state.board->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(source);
        core_machine_pic_irq_source_deassert(source);
        failed = core_machine_run(state.machine,
            (core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK;
        if (!failed) failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = cli_sti_capture(state.machine);
        failed |= after.eip != 0x0101u;
        failed |= !software_int_s50_is_halted(state.machine);
        failed |= CORE_MACHINE_BIT_IS_SET(after.eflags, CORE_MACHINE_DEBUG_EFLAGS_IF);
        failed |= !CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0au),
            VPIC_IRR_IRQ(0u));
        failed |= CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0bu),
            VPIC_ISR_IRQ(0u));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!software_int_s50_test_real_forms())
        return 1;
    if (!software_int_s50_test_protected())
        return 1;
    if (!software_int_s50_test_protected_faults_and_vm86())
        return 1;
    if (!software_int_s50_test_pic_boundary())
        return 1;
    lib_c_printf("M5:T316:S50:SOFTWARE-INT:OK\n");
    lib_c_printf("M5:T401:S25:INT-IMMEDIATE-PROFILES:OK\n");
    lib_c_printf("M5:T401:S26:INT3-INTO-PROFILES:OK\n");
    return 0;
}
