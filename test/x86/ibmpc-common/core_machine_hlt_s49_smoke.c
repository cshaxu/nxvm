#include "pic_fixture.h"
#include "x86/ibmpc-common/machine_board_state.h"
#include "lib/types/types_interface.h"
#include <stdio.h>
#include "x86/core/device_support_interface.h"
#define main cli_sti_s22_main
#include "machine_cli_sti_interrupt_smoke.c"
#undef main

static lib_i32 hlt_s49_gprs_preserved(const core_machine_debug_cpu_snapshot *before, const core_machine_debug_cpu_snapshot *after)
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

static lib_i32 hlt_s49_sregs_preserved(const core_machine_debug_cpu_snapshot *before,
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

static void hlt_s49_seed(cli_sti_machine *state)
{
    const core_machine_debug_register_patch seed = {
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
            [CORE_MACHINE_DEBUG_ESP] = 0x00008000u,
            [CORE_MACHINE_DEBUG_EBP] = 0x00000120u,
            [CORE_MACHINE_DEBUG_ESI] = 0x00000010u,
            [CORE_MACHINE_DEBUG_EDI] = 0x00000020u,
            [CORE_MACHINE_DEBUG_EFLAGS] = CORE_MACHINE_DEBUG_EFLAGS_CF |
                CORE_MACHINE_DEBUG_EFLAGS_PF | CORE_MACHINE_DEBUG_EFLAGS_AF |
                CORE_MACHINE_DEBUG_EFLAGS_ZF | CORE_MACHINE_DEBUG_EFLAGS_SF |
                CORE_MACHINE_DEBUG_EFLAGS_DF | CORE_MACHINE_DEBUG_EFLAGS_OF
        }
    };
    if (core_machine_debug_patch_registers(state->machine, &seed) != LIB_STATUS_OK)
        exit(EXIT_FAILURE);
}

static lib_bool hlt_s49_is_halted(const core_machine *machine)
{
    core_machine_cpu_state state;
    if (core_machine_get_cpu_state(machine, &state) != LIB_STATUS_OK)
        exit(EXIT_FAILURE);
    return state.halted != 0u;
}

static lib_i32 hlt_s49_run(cli_sti_machine *state, const lib_u8 *code,
    lib_u8 bytes, lib_u32 budget, lib_status *status,
    core_machine_run_result *result, core_machine_debug_cpu_snapshot *after,
    core_machine_cpu_diagnostic *diagnostic)
{
    if (core_machine_memory_write(state->machine, 0u, code, bytes) !=
        LIB_STATUS_OK)
        return 0;
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){ budget, 0u }, result);
    *after = cli_sti_capture(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        LIB_STATUS_OK;
}

static lib_i32 hlt_s49_test_defaults(void)
{
    static const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286,
        CORE_MACHINE_CPU_PROFILE_80386
    };
    const lib_u8 code[] = { 0xf4u };
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(profiles) / sizeof(profiles[0]);
        ++profile) {
        cli_sti_machine state;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        core_machine_debug_cpu_snapshot before = {0};
        core_machine_debug_cpu_snapshot after = {0};
        lib_status status = LIB_STATUS_INVALID_STATE;
        lib_i32 failed = !cli_sti_prepare(profiles[profile], &state, LIB_FALSE);

        if (!failed) {
            hlt_s49_seed(&state);
            before = cli_sti_capture(state.machine);
            failed = !hlt_s49_run(&state, code, sizeof(code), 1u, &status,
                &result, &after, &diagnostic);
            failed |= status != LIB_STATUS_OK;
            failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            failed |= diagnostic.first_fault.valid;
            failed |= after.eip != 1u;
            failed |= !hlt_s49_is_halted(state.machine);
            failed |= after.eflags != before.eflags;
            failed |= !hlt_s49_gprs_preserved(&before, &after);
            failed |= !hlt_s49_sregs_preserved(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 hlt_s49_test_attributes_and_rejections(void)
{
    static const lib_u8 prefixes[][2] = {
        { 0x66u, 0u },
        { 0x67u, 0u },
        { 0x66u, 0x67u }
    };
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286
    };
    lib_u8 prefix;

    for (prefix = 0u; prefix != sizeof(prefixes) / sizeof(prefixes[0]);
        ++prefix) {
        cli_sti_machine state;
        core_machine_cpu_diagnostic diagnostic;
        core_machine_run_result result;
        core_machine_debug_cpu_snapshot before = {0};
        core_machine_debug_cpu_snapshot after = {0};
        lib_status status = LIB_STATUS_INVALID_STATE;
        lib_u8 code[] = { prefixes[prefix][0], 0xf4u, 0u };
        lib_u8 bytes = prefix == 2u ? 3u : 2u;
        lib_i32 failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state, LIB_FALSE);

        if (prefix == 2u) {
            code[1] = prefixes[prefix][1];
            code[2] = 0xf4u;
        }
        if (!failed) {
            hlt_s49_seed(&state);
            before = cli_sti_capture(state.machine);
            failed = !hlt_s49_run(&state, code, bytes, 1u, &status, &result,
                &after, &diagnostic);
            failed |= status != LIB_STATUS_OK;
            failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
            failed |= diagnostic.first_fault.valid;
            failed |= after.eip != bytes;
            failed |= !hlt_s49_is_halted(state.machine);
            failed |= after.eflags != before.eflags;
            failed |= !hlt_s49_gprs_preserved(&before, &after);
            failed |= !hlt_s49_sregs_preserved(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    for (prefix = 0u; prefix != sizeof(prefixes) / sizeof(prefixes[0]);
        ++prefix) {
        lib_u8 profile;

        for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]);
            ++profile) {
            cli_sti_machine state;
            core_machine_cpu_diagnostic diagnostic;
            core_machine_run_result result;
            core_machine_debug_cpu_snapshot before = {0};
            core_machine_debug_cpu_snapshot after = {0};
            lib_status status;
            lib_u8 code[] = { prefixes[prefix][0], 0xf4u, 0u };
            lib_u8 bytes = prefix == 2u ? 3u : 2u;
            lib_i32 failed = !cli_sti_prepare(legacy[profile], &state, LIB_TRUE);

            if (prefix == 2u) {
                code[1] = prefixes[prefix][1];
                code[2] = 0xf4u;
            }
            if (!failed) {
                hlt_s49_seed(&state);
                /* T337_REAL_UD_TERMINAL_IVT_REJECT: installed before freeze. */
                before = cli_sti_capture(state.machine);
                failed |= !hlt_s49_run(&state, code, bytes, 1u, &status,
                    &result, &after, &diagnostic);
                failed |= status != LIB_STATUS_INTERNAL_ERROR;
                failed |= result.reason != CORE_MACHINE_STOP_FAULT;
                failed |= !diagnostic.first_fault.valid;
                failed |= !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_UD);
                failed |= lib_memory_compare(&before, &after, sizeof(before)) != 0;
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
        core_machine_debug_cpu_snapshot before = {0};
        core_machine_debug_cpu_snapshot after = {0};
        lib_status status;
        lib_u8 code[] = { 0xf0u, 0xf4u, 0u, 0u };
        lib_u8 bytes = prefix == 0u ? 2u : prefix == 3u ? 4u : 3u;
        lib_i32 failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state, LIB_TRUE);

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
            /* T337_REAL_UD_TERMINAL_IVT_REJECT: installed before freeze. */
            before = cli_sti_capture(state.machine);
            failed |= !hlt_s49_run(&state, code, bytes, 1u, &status, &result,
                &after, &diagnostic);
            failed |= status != LIB_STATUS_INTERNAL_ERROR;
            failed |= result.reason != CORE_MACHINE_STOP_FAULT;
            failed |= !diagnostic.first_fault.valid;
            failed |= !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
                VCPUINS_EXCEPT_UD);
            failed |= lib_memory_compare(&before, &after, sizeof(before)) != 0;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 hlt_s49_test_protected(void)
{
    const lib_u8 code[] = { 0xf4u };
    cli_sti_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    core_machine_debug_cpu_snapshot before = {0};
    core_machine_debug_cpu_snapshot after = {0};
    lib_status status;
    lib_i32 failed = !cli_sti_prepare_mode(&state, 0u, 2u);

    if (!failed) {
        hlt_s49_seed(&state);
        before = cli_sti_capture(state.machine);
        failed |= core_machine_memory_write(state.machine, 0x2000u, code,
            sizeof(code)) != LIB_STATUS_OK;
        status = core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result);
        after = cli_sti_capture(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK;
        failed |= status != LIB_STATUS_OK;
        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= diagnostic.first_fault.valid;
        failed |= after.eip != 1u;
        failed |= !hlt_s49_is_halted(state.machine);
        failed |= after.eflags != before.eflags;
        failed |= !hlt_s49_gprs_preserved(&before, &after);
        failed |= !hlt_s49_sregs_preserved(&before, &after);
    }
    core_machine_destroy(state.machine);
    if (failed)
        return 0;

    failed = !cli_sti_prepare_mode(&state, 1u, 2u);
    if (!failed) {
        hlt_s49_seed(&state);
        const lib_u8 absent_gate[8] = {0};
        failed |= core_machine_memory_write(state.machine, 0x0400u + 13u * 8u,
            absent_gate, sizeof(absent_gate)) != LIB_STATUS_OK;
        before = cli_sti_capture(state.machine);
        failed |= core_machine_memory_write(state.machine, 0x2000u, code,
            sizeof(code)) != LIB_STATUS_OK;
        status = core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result);
        after = cli_sti_capture(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK;
        failed |= status != LIB_STATUS_INTERNAL_ERROR;
        failed |= result.reason != CORE_MACHINE_STOP_FAULT;
        failed |= !diagnostic.first_fault.valid;
        failed |= !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
            VCPUINS_EXCEPT_DF);
        failed |= lib_memory_compare(&before, &after, sizeof(before)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 hlt_s49_test_vm86(void)
{
    const lib_u8 code[] = { 0xf4u };
    cli_sti_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    core_machine_debug_cpu_snapshot after = {0};
    lib_status status;
    const lib_u32 flags = CORE_MACHINE_DEBUG_EFLAGS_VM | CORE_MACHINE_DEBUG_EFLAGS_CF |
        (3u << 12u);
    lib_i32 failed = !cli_sti_prepare_mode(&state, 2u, flags | 2u);

    if (!failed) {
        failed = core_machine_memory_write(state.machine, 0u, code,
            sizeof(code)) != LIB_STATUS_OK;
    }
    if (!failed) {
        hlt_s49_seed(&state);
        test_core_machine_fixture_write_register(state.machine,
            CORE_MACHINE_DEBUG_EFLAGS, flags | 2u);
        status = core_machine_run(state.machine,
            (core_machine_run_budget){ 1u, 0u }, &result);
        after = cli_sti_capture(state.machine);
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK;
        failed |= status != LIB_STATUS_OK;
        failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
        failed |= diagnostic.first_fault.valid;
        failed |= !diagnostic.last_delivered_exception.valid;
        failed |= !CORE_MACHINE_BIT_IS_SET(diagnostic.last_delivered_exception.exception_mask,
            VCPUINS_EXCEPT_GP);
        failed |= after.cs.selector != 0x0008u ||
            after.ss.selector != 0x0010u || after.eip != 0x00000100u ||
            CORE_MACHINE_BIT_IS_SET(after.eflags, CORE_MACHINE_DEBUG_EFLAGS_VM);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 hlt_s49_test_irq(void)
{
    static const lib_u8 hlt = 0xf4u;
    const lib_u32 vector = 0x20u;
    const lib_u16 offset = 0x0100u;
    const lib_u16 segment = 0u;
    cli_sti_machine state;
    core_machine_pic_irq_source *source = LIB_NULL;
    core_machine_run_result result;
    core_machine_debug_cpu_snapshot before = {0};
    core_machine_debug_cpu_snapshot after = {0};
    lib_u16 frame_ip = 0u;
    lib_i32 failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state, LIB_FALSE);

    if (!failed) {
        failed = !cli_sti_real_entry(state.machine, 0u);
        failed |= core_machine_memory_write(state.machine, vector * 4u,
            &offset, sizeof(offset)) != LIB_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, vector * 4u + 2u,
            &segment, sizeof(segment)) != LIB_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, offset, &hlt,
            sizeof(hlt)) != LIB_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0u, &hlt,
            sizeof(hlt)) != LIB_STATUS_OK;
    }
    if (!failed) {
        hlt_s49_seed(&state);
        test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_EFLAGS,
            cli_sti_capture(state.machine).eflags | (CORE_MACHINE_DEBUG_EFLAGS_IF));
        before = cli_sti_capture(state.machine);
        lib_memory_set(&source, 0, sizeof(source));
        test_pic_program_vector(state.board->shared_pic_master, (lib_u8)vector);
        test_pic_bind_source(&source,
            state.board->shared_pic_master,
            state.board->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(source);
        core_machine_pic_irq_source_deassert(source);
        failed = core_machine_run(state.machine,
            (core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK;
        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = cli_sti_capture(state.machine);
        failed |= after.eip != offset + 1u;
        failed |= !hlt_s49_is_halted(state.machine);
        failed |= after.eflags !=
            (before.eflags & ~CORE_MACHINE_DEBUG_EFLAGS_IF);
        failed |= after.eax != before.eax;
        failed |= after.ecx != before.ecx;
        failed |= after.edx != before.edx;
        failed |= after.ebx != before.ebx;
        failed |= after.ebp != before.ebp;
        failed |= after.esi != before.esi;
        failed |= after.edi != before.edi;
        failed |= after.esp != ((before.esp & 0xffff0000u) |
            (lib_u16)(before.esp - 6u));
        failed |= !hlt_s49_sregs_preserved(&before, &after);
        failed |= !CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0bu),
            VPIC_ISR_IRQ(0u));
        failed |= CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0au),
            VPIC_IRR_IRQ(0u));
        failed |= core_machine_memory_read(state.machine,
            after.ss.base + (lib_u16)after.esp,
            (void *)&frame_ip, sizeof(frame_ip)) != LIB_STATUS_OK;
        failed |= frame_ip != 1u;
    }
    core_machine_destroy(state.machine);
    if (failed)
        return 0;

    failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state, LIB_FALSE);
    if (!failed) {
        failed = !cli_sti_real_entry(state.machine, 0u);
        failed |= core_machine_memory_write(state.machine, 0u, &hlt,
            sizeof(hlt)) != LIB_STATUS_OK;
    }
    if (!failed) {
        hlt_s49_seed(&state);
        test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_EFLAGS,
            cli_sti_capture(state.machine).eflags & (~CORE_MACHINE_DEBUG_EFLAGS_IF));
        before = cli_sti_capture(state.machine);
        lib_memory_set(&source, 0, sizeof(source));
        test_pic_bind_source(&source,
            state.board->shared_pic_master,
            state.board->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(source);
        core_machine_pic_irq_source_deassert(source);
        failed = core_machine_run(state.machine,
            (core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK;
        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = cli_sti_capture(state.machine);
        failed |= after.eip != 1u;
        failed |= !hlt_s49_is_halted(state.machine);
        failed |= after.eflags != before.eflags;
        failed |= !hlt_s49_gprs_preserved(&before, &after);
        failed |= !hlt_s49_sregs_preserved(&before, &after);
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
    printf("M5:T316:S49:HLT:OK\n");
    printf("M5:T401:S38:HLT-PROFILES:OK\n");
    return 0;
}
