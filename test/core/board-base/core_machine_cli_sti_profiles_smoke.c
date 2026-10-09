#include "lib/types/file.h"
#include "pic_fixture.h"
#include "core/board-base/machine_board_state.h"
#include "lib/types/types_interface.h"
#include "core/x86/device_support_interface.h"
#define main cli_sti_main
#include "machine_cli_sti_interrupt_smoke.c"
#undef main

static lib_i32 cli_sti_gprs_preserved(const core_machine_debug_cpu_snapshot *before,
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

static lib_i32 cli_sti_sregs_preserved(const core_machine_debug_cpu_snapshot *before,
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

static lib_i32 cli_sti_test_80286_defaults(void)
{
    static const lib_u8 opcodes[] = { 0xfau, 0xfbu };
    const lib_u32 preserved = CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_PF |
        CORE_MACHINE_DEBUG_EFLAGS_AF | CORE_MACHINE_DEBUG_EFLAGS_ZF | CORE_MACHINE_DEBUG_EFLAGS_SF |
        CORE_MACHINE_DEBUG_EFLAGS_DF | CORE_MACHINE_DEBUG_EFLAGS_OF;
    lib_u8 opcode_index;

    for (opcode_index = 0u;
        opcode_index != sizeof(opcodes) / sizeof(opcodes[0]);
        ++opcode_index) {
        cli_sti_machine state;
        core_machine_debug_cpu_snapshot before = {0};
        core_machine_debug_cpu_snapshot after = {0};
        lib_u32 expected;
        lib_i32 failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state, LIB_FALSE);

        if (!failed) {
            test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_EFLAGS, preserved |
                (opcodes[opcode_index] == 0xfau ? CORE_MACHINE_DEBUG_EFLAGS_IF : 0u));
            before = cli_sti_capture(state.machine);
            expected = (before.eflags & ~CORE_MACHINE_DEBUG_EFLAGS_IF) |
                (opcodes[opcode_index] == 0xfbu ? CORE_MACHINE_DEBUG_EFLAGS_IF : 0u);
            failed = !cli_sti_run(&state, &opcodes[opcode_index], 1u, 1u,
                &after);
            failed |= after.eip != 1u;
            failed |= after.eflags != expected;
            failed |= !cli_sti_gprs_preserved(&before, &after);
            failed |= !cli_sti_sregs_preserved(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 cli_sti_test_80286_irq_contracts(void)
{
    static const lib_u8 cli_nop[] = { 0xfau, 0x90u };
    static const lib_u8 sti_nop[] = { 0xfbu, 0x90u };
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
    lib_i32 failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state, LIB_FALSE);

    if (!failed) {
        failed = !cli_sti_real_entry(state.machine, 0u);
        failed |= core_machine_memory_write(state.machine, 0u, cli_nop,
            sizeof(cli_nop)) != LIB_STATUS_OK;
    }
    if (!failed) {
        test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_EFLAGS,
            cli_sti_capture(state.machine).eflags | (CORE_MACHINE_DEBUG_EFLAGS_IF));
        before = cli_sti_capture(state.machine);
        lib_memory_set(&source, 0, sizeof(source));
        test_pic_bind_source(&source,
            state.board->shared_pic_master,
            state.board->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(source);
        core_machine_pic_irq_source_deassert(source);
        failed = core_machine_run(state.machine,
            (core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK;
        failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
        after = cli_sti_capture(state.machine);
        failed |= after.eip != 2u;
        failed |= after.eflags !=
            (before.eflags & ~CORE_MACHINE_DEBUG_EFLAGS_IF);
        failed |= !cli_sti_gprs_preserved(&before, &after);
        failed |= !cli_sti_sregs_preserved(&before, &after);
        failed |= !CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0au),
            VPIC_IRR_IRQ(0u));
        failed |= CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0bu),
            VPIC_ISR_IRQ(0u));
    }
    core_machine_destroy(state.machine);
    if (failed)
        return 0;

    failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state, LIB_FALSE);
    if (!failed) {
        failed = !cli_sti_real_entry(state.machine, 0u);
        failed |= core_machine_memory_write(state.machine, vector * 4u,
            &offset, sizeof(offset)) != LIB_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, vector * 4u + 2u,
            &segment, sizeof(segment)) != LIB_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, offset, &hlt,
            sizeof(hlt)) != LIB_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0u, sti_nop,
            sizeof(sti_nop)) != LIB_STATUS_OK;
    }
    if (!failed) {
        test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_EFLAGS,
            cli_sti_capture(state.machine).eflags & (~CORE_MACHINE_DEBUG_EFLAGS_IF));
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
        failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
        after = cli_sti_capture(state.machine);
        failed |= after.eip != offset;
        failed |= CORE_MACHINE_BIT_IS_SET(after.eflags, CORE_MACHINE_DEBUG_EFLAGS_IF);
        failed |= after.eax != before.eax;
        failed |= after.ecx != before.ecx;
        failed |= after.edx != before.edx;
        failed |= after.ebx != before.ebx;
        failed |= after.ebp != before.ebp;
        failed |= after.esi != before.esi;
        failed |= after.edi != before.edi;
        failed |= after.esp != ((before.esp & 0xffff0000u) |
            (lib_u16)(before.esp - 6u));
        failed |= !cli_sti_sregs_preserved(&before, &after);
        failed |= !CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0bu),
            VPIC_ISR_IRQ(0u));
        failed |= CORE_MACHINE_BIT_IS_SET(test_pic_read(state.board->shared_pic_master, 0x0au),
            VPIC_IRR_IRQ(0u));
        failed |= core_machine_memory_read(state.machine,
            after.ss.base + (lib_u16)after.esp,
            (void *)&frame_ip, sizeof(frame_ip)) != LIB_STATUS_OK;
        failed |= frame_ip != 2u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 cli_sti_test_prefixes(void)
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
    lib_u8 profile;

    for (profile = 0u; profile != sizeof(legacy) / sizeof(legacy[0]);
        ++profile) {
        lib_u8 prefix;

        for (prefix = 0u; prefix != sizeof(prefixes) / sizeof(prefixes[0]);
            ++prefix) {
            lib_u8 opcode;

            for (opcode = 0xfau; opcode <= 0xfbu; ++opcode) {
                cli_sti_machine state;
                core_machine_cpu_diagnostic diagnostic;
                core_machine_run_result result;
                lib_status status;
                core_machine_debug_cpu_snapshot before = {0};
                core_machine_debug_cpu_snapshot after = {0};
                lib_u8 code[] = { prefixes[prefix][0], opcode, 0u };
                lib_u8 bytes = prefix == 2u ? 3u : 2u;
                lib_i32 failed = !cli_sti_prepare(legacy[profile], &state, LIB_TRUE);

                if (prefix == 2u) {
                    code[1] = prefixes[prefix][1];
                    code[2] = opcode;
                }
                if (!failed) {
                    /* REAL_UD_TERMINAL_IVT_REJECT: installed before freeze. */
                    before = cli_sti_capture(state.machine);
                    failed |= core_machine_memory_write(state.machine, 0u, code,
                        bytes) != LIB_STATUS_OK;
                    status = core_machine_run(state.machine,
                        (core_machine_run_budget){ 1u, 0u }, &result);
                    failed |= status != LIB_STATUS_INTERNAL_ERROR;
                    after = cli_sti_capture(state.machine);
                    failed |= core_machine_get_cpu_diagnostic(state.machine,
                        &diagnostic) != LIB_STATUS_OK;
                    failed |= !diagnostic.first_fault.valid;
                    failed |= !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
                        legacy[profile] == CORE_MACHINE_CPU_PROFILE_8086 ?
                            VCPUINS_EXCEPT_UD : VCPUINS_EXCEPT_CE);
                    failed |= lib_memory_compare(&before, &after, sizeof(before)) != 0;
                }
                core_machine_destroy(state.machine);
                if (failed)
                    return 0;
            }
        }
    }
    return 1;
}

static lib_i32 cli_sti_test_386_prefix_and_lock(void)
{
    static const lib_u8 prefixes[][2] = {
        { 0x66u, 0u },
        { 0x67u, 0u },
        { 0x66u, 0x67u }
    };
    lib_u8 attribute;

    for (attribute = 0u; attribute != sizeof(prefixes) / sizeof(prefixes[0]);
        ++attribute) {
        lib_u8 opcode;

        for (opcode = 0xfau; opcode <= 0xfbu; ++opcode) {
            cli_sti_machine state;
            core_machine_cpu_diagnostic diagnostic;
            core_machine_debug_cpu_snapshot before = {0};
            core_machine_debug_cpu_snapshot after = {0};
            lib_u8 code[] = { prefixes[attribute][0], opcode, 0u };
            lib_u8 bytes = attribute == 2u ? 3u : 2u;
            lib_u32 expected_flags;
            lib_i32 failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state, LIB_FALSE);

            if (attribute == 2u) {
                code[1] = prefixes[attribute][1];
                code[2] = opcode;
            }
            if (!failed) {
                test_core_machine_fixture_write_register(state.machine, CORE_MACHINE_DEBUG_EFLAGS, CORE_MACHINE_DEBUG_EFLAGS_CF |
                    CORE_MACHINE_DEBUG_EFLAGS_DF | CORE_MACHINE_DEBUG_EFLAGS_OF |
                    (opcode == 0xfau ? CORE_MACHINE_DEBUG_EFLAGS_IF : 0u));
                before = cli_sti_capture(state.machine);
                expected_flags = (before.eflags & ~CORE_MACHINE_DEBUG_EFLAGS_IF) |
                    (opcode == 0xfbu ? CORE_MACHINE_DEBUG_EFLAGS_IF : 0u);
                failed = !cli_sti_run(&state, code, bytes, 1u, &after);
                failed |= after.eip != bytes;
                failed |= after.eflags != expected_flags;
                failed |= !cli_sti_gprs_preserved(&before, &after);
                failed |= !cli_sti_sregs_preserved(&before, &after);
                failed |= core_machine_get_cpu_diagnostic(state.machine,
                    &diagnostic) != LIB_STATUS_OK;
                failed |= diagnostic.first_fault.valid;
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    for (attribute = 0u; attribute != 4u; ++attribute) {
        lib_u8 opcode;

        for (opcode = 0xfau; opcode <= 0xfbu; ++opcode) {
            cli_sti_machine state;
            core_machine_cpu_diagnostic diagnostic;
            core_machine_run_result result;
            lib_status status;
            core_machine_debug_cpu_snapshot before = {0};
            core_machine_debug_cpu_snapshot after = {0};
            lib_u8 code[] = { 0xf0u, opcode, 0u, 0u };
            lib_u8 bytes = attribute == 0u ? 2u :
                attribute == 3u ? 4u : 3u;
            lib_i32 failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80386, &state, LIB_TRUE);

            if (attribute != 0u) {
                code[1] = prefixes[attribute - 1u][0];
                code[2] = opcode;
            }
            if (attribute == 3u) {
                code[2] = prefixes[2][1];
                code[3] = opcode;
            }
            if (!failed) {
                /* REAL_UD_TERMINAL_IVT_REJECT: installed before freeze. */
                before = cli_sti_capture(state.machine);
                failed |= core_machine_memory_write(state.machine, 0u, code,
                    bytes) != LIB_STATUS_OK;
                status = core_machine_run(state.machine,
                    (core_machine_run_budget){ 1u, 0u }, &result);
                failed |= status != LIB_STATUS_INTERNAL_ERROR;
                after = cli_sti_capture(state.machine);
                failed |= core_machine_get_cpu_diagnostic(state.machine,
                    &diagnostic) != LIB_STATUS_OK;
                failed |= !diagnostic.first_fault.valid;
                failed |= !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_CE);
                failed |= lib_memory_compare(&before, &after, sizeof(before)) != 0;
            }
            core_machine_destroy(state.machine);
            if (failed)
                return 0;
        }
    }
    return 1;
}

lib_i32 main(void)
{
    if (cli_sti_main() != 0)
        return 1;
    if (!cli_sti_test_80286_defaults())
        return 1;
    if (!cli_sti_test_80286_irq_contracts())
        return 1;
    if (!cli_sti_test_prefixes())
        return 1;
    if (!cli_sti_test_386_prefix_and_lock())
        return 1;
    lib_c_printf("CLI-STI:OK\n");
    lib_c_printf("CLI-STI-PROFILES:OK\n");
    return 0;
}
