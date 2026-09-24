#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"
#define main cli_sti_s22_main
#include "core_machine_cli_sti_smoke.c"
#undef main

static lib_i32 cli_sti_s48_gprs_preserved(const t_cpu *before,
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

static lib_i32 cli_sti_s48_sregs_preserved(const t_cpu *before,
    const t_cpu *after)
{
    return lib_memory_compare(&before->data.es, &after->data.es,
            sizeof(before->data.es)) == 0 &&
        lib_memory_compare(&before->data.cs, &after->data.cs,
            sizeof(before->data.cs)) == 0 &&
        lib_memory_compare(&before->data.ss, &after->data.ss,
            sizeof(before->data.ss)) == 0 &&
        lib_memory_compare(&before->data.ds, &after->data.ds,
            sizeof(before->data.ds)) == 0 &&
        lib_memory_compare(&before->data.fs, &after->data.fs,
            sizeof(before->data.fs)) == 0 &&
        lib_memory_compare(&before->data.gs, &after->data.gs,
            sizeof(before->data.gs)) == 0;
}

static lib_i32 cli_sti_s48_test_80286_defaults(void)
{
    static const lib_u8 opcodes[] = { 0xfau, 0xfbu };
    const lib_u32 preserved = VCPU_EFLAGS_CF | VCPU_EFLAGS_PF |
        VCPU_EFLAGS_AF | VCPU_EFLAGS_ZF | VCPU_EFLAGS_SF |
        VCPU_EFLAGS_DF | VCPU_EFLAGS_OF;
    lib_u8 opcode_index;

    for (opcode_index = 0u;
        opcode_index != sizeof(opcodes) / sizeof(opcodes[0]);
        ++opcode_index) {
        cli_sti_machine state;
        t_cpu before;
        t_cpu after;
        lib_u32 expected;
        lib_i32 failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80286,
            &state);

        if (!failed) {
            state.machine->executor_cpu.data.eflags = preserved |
                (opcodes[opcode_index] == 0xfau ? VCPU_EFLAGS_IF : 0u);
            before = test_core_machine_fixture_capture_cpu_after_run(
                state.machine);
            expected = (before.data.eflags & ~VCPU_EFLAGS_IF) |
                (opcodes[opcode_index] == 0xfbu ? VCPU_EFLAGS_IF : 0u);
            failed = !cli_sti_run(&state, &opcodes[opcode_index], 1u, 1u,
                &after);
            failed |= after.data.eip != 1u;
            failed |= after.data.eflags != expected;
            failed |= !cli_sti_s48_gprs_preserved(&before, &after);
            failed |= !cli_sti_s48_sregs_preserved(&before, &after);
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 cli_sti_s48_test_80286_irq_contracts(void)
{
    static const lib_u8 cli_nop[] = { 0xfau, 0x90u };
    static const lib_u8 sti_nop[] = { 0xfbu, 0x90u };
    static const lib_u8 hlt = 0xf4u;
    const lib_u32 vector = 0x20u;
    const lib_u16 offset = 0x0100u;
    const lib_u16 segment = 0u;
    cli_sti_machine state;
    core_machine_pic_irq_source source;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    lib_u16 frame_ip = 0u;
    lib_i32 failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);

    if (!failed) {
        failed = !test_core_machine_fixture_prepare_real_mode_execution(
            state.machine, 0u);
        failed |= core_machine_memory_write(state.machine, 0u, cli_nop,
            sizeof(cli_nop)) != LIB_STATUS_OK;
    }
    if (!failed) {
        state.machine->executor_cpu.data.eflags |= VCPU_EFLAGS_IF;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        lib_memory_set(&source, 0, sizeof(source));
        core_machine_pic_irq_source_bind(&source,
            &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
        failed = core_machine_run(state.machine,
            (core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK;
        failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= after.data.eip != 2u;
        failed |= after.data.eflags !=
            (before.data.eflags & ~VCPU_EFLAGS_IF);
        failed |= !cli_sti_s48_gprs_preserved(&before, &after);
        failed |= !cli_sti_s48_sregs_preserved(&before, &after);
        failed |= !CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.irr,
            VPIC_IRR_IRQ(0u));
        failed |= CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.isr,
            VPIC_ISR_IRQ(0u));
    }
    core_machine_destroy(state.machine);
    if (failed)
        return 0;

    failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80286, &state);
    if (!failed) {
        failed = !test_core_machine_fixture_prepare_real_mode_execution(
            state.machine, 0u);
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
        state.machine->executor_cpu.data.eflags &= ~VCPU_EFLAGS_IF;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        lib_memory_set(&source, 0, sizeof(source));
        state.machine->shared_pic_master.data.icw2 = (lib_u8)vector;
        core_machine_pic_irq_source_bind(&source,
            &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
        failed = core_machine_run(state.machine,
            (core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK;
        failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= after.data.eip != offset;
        failed |= CORE_MACHINE_BIT_IS_SET(after.data.eflags, VCPU_EFLAGS_IF);
        failed |= after.data.eax != before.data.eax;
        failed |= after.data.ecx != before.data.ecx;
        failed |= after.data.edx != before.data.edx;
        failed |= after.data.ebx != before.data.ebx;
        failed |= after.data.ebp != before.data.ebp;
        failed |= after.data.esi != before.data.esi;
        failed |= after.data.edi != before.data.edi;
        failed |= after.data.esp != ((before.data.esp & 0xffff0000u) |
            (lib_u16)(before.data.esp - 6u));
        failed |= !cli_sti_s48_sregs_preserved(&before, &after);
        failed |= !CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.isr,
            VPIC_ISR_IRQ(0u));
        failed |= CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.irr,
            VPIC_IRR_IRQ(0u));
        failed |= core_machine_memory_read_physical(
            &state.machine->executor_memory,
            after.data.ss.base + (lib_u16)after.data.esp,
            (lib_uptr)&frame_ip, sizeof(frame_ip)) != LIB_STATUS_OK;
        failed |= frame_ip != 2u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 cli_sti_s48_test_prefixes(void)
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
                t_cpu before;
                t_cpu after;
                lib_u8 code[] = { prefixes[prefix][0], opcode, 0u };
                lib_u8 bytes = prefix == 2u ? 3u : 2u;
                lib_i32 failed = !cli_sti_prepare(legacy[profile], &state);

                if (prefix == 2u) {
                    code[1] = prefixes[prefix][1];
                    code[2] = opcode;
                }
                if (!failed) {
                    failed = !test_core_machine_fixture_preflight_real_ud_terminal(
                        state.machine);
                    before = test_core_machine_fixture_capture_cpu_after_run(
                        state.machine);
                    failed |= core_machine_memory_write(state.machine, 0u, code,
                        bytes) != LIB_STATUS_OK;
                    status = core_machine_run(state.machine,
                        (core_machine_run_budget){ 1u, 0u }, &result);
                    failed |= status != LIB_STATUS_INTERNAL_ERROR;
                    after = test_core_machine_fixture_capture_cpu_after_run(
                        state.machine);
                    failed |= core_machine_get_cpu_diagnostic(state.machine,
                        &diagnostic) != LIB_STATUS_OK;
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
    }
    return 1;
}

static lib_i32 cli_sti_s48_test_386_prefix_and_lock(void)
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
            t_cpu before;
            t_cpu after;
            lib_u8 code[] = { prefixes[attribute][0], opcode, 0u };
            lib_u8 bytes = attribute == 2u ? 3u : 2u;
            lib_u32 expected_flags;
            lib_i32 failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80386,
                &state);

            if (attribute == 2u) {
                code[1] = prefixes[attribute][1];
                code[2] = opcode;
            }
            if (!failed) {
                state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
                    VCPU_EFLAGS_DF | VCPU_EFLAGS_OF |
                    (opcode == 0xfau ? VCPU_EFLAGS_IF : 0u);
                before = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                expected_flags = (before.data.eflags & ~VCPU_EFLAGS_IF) |
                    (opcode == 0xfbu ? VCPU_EFLAGS_IF : 0u);
                failed = !cli_sti_run(&state, code, bytes, 1u, &after);
                failed |= after.data.eip != bytes;
                failed |= after.data.eflags != expected_flags;
                failed |= !cli_sti_s48_gprs_preserved(&before, &after);
                failed |= !cli_sti_s48_sregs_preserved(&before, &after);
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
            t_cpu before;
            t_cpu after;
            lib_u8 code[] = { 0xf0u, opcode, 0u, 0u };
            lib_u8 bytes = attribute == 0u ? 2u :
                attribute == 3u ? 4u : 3u;
            lib_i32 failed = !cli_sti_prepare(CORE_MACHINE_CPU_PROFILE_80386,
                &state);

            if (attribute != 0u) {
                code[1] = prefixes[attribute - 1u][0];
                code[2] = opcode;
            }
            if (attribute == 3u) {
                code[2] = prefixes[2][1];
                code[3] = opcode;
            }
            if (!failed) {
                failed = !test_core_machine_fixture_preflight_real_ud_terminal(
                    state.machine);
                before = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed |= core_machine_memory_write(state.machine, 0u, code,
                    bytes) != LIB_STATUS_OK;
                status = core_machine_run(state.machine,
                    (core_machine_run_budget){ 1u, 0u }, &result);
                failed |= status != LIB_STATUS_INTERNAL_ERROR;
                after = test_core_machine_fixture_capture_cpu_after_run(
                    state.machine);
                failed |= core_machine_get_cpu_diagnostic(state.machine,
                    &diagnostic) != LIB_STATUS_OK;
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
    return 1;
}

lib_i32 main(void)
{
    if (cli_sti_s22_main() != 0)
        return 1;
    if (!cli_sti_s48_test_80286_defaults())
        return 1;
    if (!cli_sti_s48_test_80286_irq_contracts())
        return 1;
    if (!cli_sti_s48_test_prefixes())
        return 1;
    if (!cli_sti_s48_test_386_prefix_and_lock())
        return 1;
    printf("M5:T316:S48:CLI-STI:OK\n");
    printf("M5:T401:S37:CLI-STI-PROFILES:OK\n");
    return 0;
}
