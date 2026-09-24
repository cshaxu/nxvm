#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"
#define main protected_return_atomicity_main
#include "core_machine_protected_return_atomicity_smoke.c"
#undef main

static lib_i32 iret_outer_s52_expect_ud(core_machine_cpu_profile profile,
    const lib_u8 *code, lib_u8 bytes)
{
    atomic_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    lib_u8 stack_before[16] = { 0u };
    lib_u8 stack_after[16] = { 0u };
    lib_i32 failed = !atomic_prepare(&state, profile);

    if (!failed) {
        failed = !test_core_machine_fixture_prepare_real_mode_execution(
            state.machine, 0u);
        failed |= core_machine_memory_write(state.machine, 0u, code, bytes) !=
            LIB_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x7ff8u,
            stack_before, sizeof(stack_before)) != LIB_STATUS_OK;
    }
    if (!failed) {
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

static lib_i32 iret_outer_s52_test_rejections(void)
{
    static const core_machine_cpu_profile legacy[] = {
        CORE_MACHINE_CPU_PROFILE_8086,
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

            if (!iret_outer_s52_expect_ud(legacy[profile],
                    attributes[attribute], bytes))
                return 0;
        }
    }
    for (profile = 0u; profile != sizeof(lock_forms) / sizeof(lock_forms[0]);
        ++profile) {
        lib_u8 bytes = profile == 3u ? 4u : profile == 0u ? 2u : 3u;

        if (!iret_outer_s52_expect_ud(CORE_MACHINE_CPU_PROFILE_80386,
                lock_forms[profile], bytes))
            return 0;
    }
    return 1;
}

static lib_i32 iret_outer_s52_test_combined(void)
{
    static const lib_u8 program[] = { 0x66u, 0x67u, 0xcfu };
    static const lib_u16 frame[] = {
        0x0010u, 0x001bu, 0x0203u, 0x1000u, 0x0023u
    };
    const core_machine_run_budget boot_budget = { 128u, 0u };
    const core_machine_run_budget budget = { 1u, 0u };
    atomic_machine state;
    core_machine_run_result result = { 0 };
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    lib_i32 failed = !atomic_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

    if (!failed) {
        failed = !atomic_install(&state);
        failed |= core_machine_run(state.machine, boot_budget, &result) !=
            LIB_STATUS_OK;
        failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
    }
    if (!failed) {
        test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        before.data.cs.seg.exec.defsize = LIB_TRUE;
        before.data.esp = 0x12348000u;
        state.machine->executor_cpu = before;
        failed |= !atomic_write(&state, ATOMIC_KERNEL_BASE, program,
            sizeof(program));
        failed |= !atomic_write(&state, ATOMIC_KERNEL_STACK_BASE + 0x8000u,
            (const lib_u8 *)frame, sizeof(frame));
        failed |= core_machine_run(state.machine, budget, &result) !=
            LIB_STATUS_OK;
        failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= diagnostic.first_fault.valid;
        failed |= after.data.eip != 0x0010u;
        failed |= after.data.cs.selector != 0x001bu;
        failed |= after.data.ss.selector != 0x0023u;
        failed |= after.data.esp != 0x12341000u;
        failed |= after.data.eflags != 0x00000203u;
        failed |= after.data.eax != before.data.eax;
        failed |= after.data.ecx != before.data.ecx;
        failed |= after.data.edx != before.data.edx;
        failed |= after.data.ebx != before.data.ebx;
        failed |= after.data.ebp != before.data.ebp;
        failed |= after.data.esi != before.data.esi;
        failed |= after.data.edi != before.data.edi;
        if (failed) {
            fprintf(stderr,
                "S52 combined reason=%u eip=%08x cs=%04x ss=%04x esp=%08x flags=%08x fault=%u\n",
                result.reason, after.data.eip, after.data.cs.selector,
                after.data.ss.selector, after.data.esp, after.data.eflags,
                diagnostic.first_fault.valid);
        }
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 iret_outer_s52_user_code_cache(const t_cpu_data_sreg *sreg)
{
    return sreg->flagValid && sreg->selector == 0x001bu &&
        sreg->sregtype == SREG_CODE && sreg->base == 0x00004000u &&
        sreg->limit == 0x0000ffffu && sreg->dpl == 3u &&
        sreg->seg.accessed && sreg->seg.executable &&
        !sreg->seg.exec.defsize && !sreg->seg.exec.conform &&
        sreg->seg.exec.readable;
}

static lib_i32 iret_outer_s52_user_stack_cache(const t_cpu_data_sreg *sreg,
    lib_i32 big)
{
    return sreg->flagValid && sreg->selector == 0x0023u &&
        sreg->sregtype == SREG_STACK && sreg->base == 0x00005000u &&
        sreg->limit == 0x0000ffffu && sreg->dpl == 3u &&
        sreg->seg.accessed && !sreg->seg.executable &&
        sreg->seg.data.big == big && !sreg->seg.data.expdown &&
        sreg->seg.data.writable;
}

static lib_i32 iret_outer_s52_gprs_same(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx &&
        before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx &&
        before->data.ebp == after->data.ebp &&
        before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi;
}

static lib_i32 iret_outer_s52_non_target_sregs_same(const t_cpu *before,
    const t_cpu *after)
{
    return lib_memory_compare(&before->data.es, &after->data.es,
            sizeof(before->data.es)) == 0 &&
        lib_memory_compare(&before->data.ds, &after->data.ds,
            sizeof(before->data.ds)) == 0 &&
        lib_memory_compare(&before->data.fs, &after->data.fs,
            sizeof(before->data.fs)) == 0 &&
        lib_memory_compare(&before->data.gs, &after->data.gs,
            sizeof(before->data.gs)) == 0;
}

static lib_i32 iret_outer_s52_test_success(void)
{
    static const lib_u8 programs[][3] = {
        { 0xcfu, 0u, 0u },
        { 0x67u, 0xcfu, 0u },
        { 0x66u, 0xcfu, 0u },
        { 0x66u, 0x67u, 0xcfu }
    };
    static const lib_u8 program_bytes[] = { 1u, 2u, 2u, 3u };
    static const lib_u8 operand16[] = { 0u, 0u, 1u, 1u };
    static const lib_u8 wide_stack[] = { 0u, 1u, 0u, 1u };
    static const lib_u32 frame32[] = {
        0x00000010u, 0x0000001bu, 0x00000203u,
        0x00001000u, 0x00000023u
    };
    static const lib_u16 frame16[] = {
        0x0010u, 0x001bu, 0x0203u, 0x1000u, 0x0023u
    };
    const core_machine_run_budget boot_budget = { 128u, 0u };
    const core_machine_run_budget budget = { 1u, 0u };
    const lib_u8 stack_flags = 0x40u;
    lib_u8 form;

    for (form = 0u; form != sizeof(programs) / sizeof(programs[0]); ++form) {
        atomic_machine state;
        core_machine_run_result result = { 0 };
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        lib_u8 old_before[sizeof(frame32)] = { 0u };
        lib_u8 old_after[sizeof(frame32)] = { 0u };
        lib_u8 outer_target_before[8] = {
            0xa1u, 0xa2u, 0xa3u, 0xa4u, 0xa5u, 0xa6u, 0xa7u, 0xa8u
        };
        lib_u8 outer_target_after[sizeof(outer_target_before)] = { 0u };
        lib_u8 outer_unselected_before[8] = {
            0xb1u, 0xb2u, 0xb3u, 0xb4u, 0xb5u, 0xb6u, 0xb7u, 0xb8u
        };
        lib_u8 outer_unselected_after[sizeof(outer_unselected_before)] = { 0u };
        lib_u32 expected_esp = wide_stack[form] ? 0x00001000u :
            0x12341000u;
        lib_i32 failed = !atomic_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            failed = !atomic_install(&state);
            failed |= core_machine_run(state.machine, boot_budget, &result) !=
                LIB_STATUS_OK;
            failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        }
        if (!failed) {
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            before.data.cs.seg.exec.defsize = LIB_TRUE;
            before.data.esp = 0x12348000u;
            state.machine->executor_cpu = before;
            if (wide_stack[form]) {
                failed |= !atomic_write(&state, ATOMIC_GDT_BASE + 38u,
                    &stack_flags, sizeof(stack_flags));
            }
            failed |= !atomic_write(&state, ATOMIC_KERNEL_BASE, programs[form],
                program_bytes[form]);
            failed |= !atomic_write(&state, ATOMIC_KERNEL_STACK_BASE + 0x8000u,
                operand16[form] ? (const void *)frame16 :
                (const void *)frame32, operand16[form] ? sizeof(frame16) :
                sizeof(frame32));
            failed |= !atomic_write(&state, 0x00006000u, outer_target_before,
                sizeof(outer_target_before));
            failed |= !atomic_write(&state, 0x00005fe0u, outer_unselected_before,
                sizeof(outer_unselected_before));
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                ATOMIC_KERNEL_STACK_BASE + 0x8000u,
                (lib_uptr)old_before, operand16[form] ?
                sizeof(frame16) : sizeof(frame32)) != LIB_STATUS_OK;
        }
        if (!failed) {
            failed |= core_machine_run(state.machine, budget, &result) !=
                LIB_STATUS_OK;
            failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
            failed |= core_machine_get_cpu_diagnostic(state.machine,
                &diagnostic) != LIB_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= diagnostic.first_fault.valid;
            failed |= after.data.eip != 0x00000010u;
            failed |= after.data.eflags != 0x00000203u;
            failed |= after.data.esp != expected_esp;
            failed |= !iret_outer_s52_gprs_same(&before, &after);
            failed |= !iret_outer_s52_user_code_cache(&after.data.cs);
            failed |= !iret_outer_s52_user_stack_cache(&after.data.ss,
                wide_stack[form]);
            failed |= !iret_outer_s52_non_target_sregs_same(&before, &after);
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                ATOMIC_KERNEL_STACK_BASE + 0x8000u,
                (lib_uptr)old_after, operand16[form] ?
                sizeof(frame16) : sizeof(frame32)) != LIB_STATUS_OK;
            failed |= lib_memory_compare(old_before, old_after, operand16[form] ?
                sizeof(frame16) : sizeof(frame32)) != 0;
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                0x00006000u, (lib_uptr)outer_target_after,
                sizeof(outer_target_after)) != LIB_STATUS_OK;
            failed |= lib_memory_compare(outer_target_before, outer_target_after,
                sizeof(outer_target_before)) != 0;
            failed |= core_machine_memory_read_physical(&state.machine->executor_memory,
                0x00005fe0u, (lib_uptr)outer_unselected_after,
                sizeof(outer_unselected_after)) != LIB_STATUS_OK;
            failed |= lib_memory_compare(outer_unselected_before, outer_unselected_after,
                sizeof(outer_unselected_before)) != 0;
        }
        core_machine_destroy(state.machine);
        if (failed)
            return 0;
    }
    return 1;
}

static lib_i32 iret_outer_s52_test_outer(void)
{
    static const atomic_return_case cases[] = {
        { "nonpresent-cs", 0x002bu, 0x0023u, 11u, 0x0028u,
            VCPUINS_EXCEPT_NP, 1 },
        { "nonpresent-ss", 0x001bu, 0x0033u, 12u, 0x0030u,
            VCPUINS_EXCEPT_SS, 1 },
        { "inaccessible-entry", 0x003bu, 0x0023u, 13u, 0x0038u,
            VCPUINS_EXCEPT_GP, 1 },
        { "conforming-cs", 0x0043u, 0x0023u, 13u, 0x0040u,
            VCPUINS_EXCEPT_GP, 1 }
    };
    lib_size index;

    for (index = 0u; index != sizeof(cases) / sizeof(cases[0]); ++index) {
        if (atomic_test_outer_return(&cases[index], 1))
            return 0;
    }
    return !atomic_test_outer_iret_frame(1, 0, 0, 0) &&
        !atomic_test_outer_iret_frame(0, 0, 1, 0) &&
        !atomic_test_outer_iret_frame(0, 1, 1, 0) &&
        !atomic_test_outer_iret_frame(0, 0, 0, 1) &&
        !atomic_test_outer_iret32_failure(0) &&
        !atomic_test_outer_iret32_failure(1);
}

static lib_i32 iret_outer_s52_test_pic(void)
{
    static const lib_u8 iret[] = { 0xcfu };
    static const lib_u8 nop = 0x90u;
    static const lib_u8 hlt = 0xf4u;
    static const lib_u8 tss_descriptor[] = {
        0x67u, 0x00u, 0x00u, 0x06u,
        0x00u, 0x8bu, 0x00u, 0x00u
    };
    const lib_u8 vector = 0x20u;
    lib_u8 restore_if;

    for (restore_if = 0u; restore_if != 2u; ++restore_if) {
        atomic_machine state;
        core_machine_pic_irq_source source;
        core_machine_run_result result = { 0 };
        core_machine_cpu_diagnostic diagnostic;
        t_cpu before;
        t_cpu after;
        lib_u8 gate[8] = { 0u };
        lib_u32 frame[] = {
            0x00000010u, 0x0000001bu,
            restore_if ? VCPU_EFLAGS_IF | 0x02u : 0x02u,
            0x00001000u, 0x00000023u
        };
        lib_u32 irq_frame[3] = { 0u, 0u, 0u };
        lib_u8 tss[10] = { 0u };
        lib_u32 esp0 = 0x00009000u;
        lib_u16 ss0 = 0x0010u;
        lib_i32 failed = !atomic_prepare(&state,
            CORE_MACHINE_CPU_PROFILE_80386);

        if (!failed) {
            failed = !atomic_install(&state);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){ 128u, 0u }, &result) != LIB_STATUS_OK;
            failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        }
        if (!failed) {
            test_core_machine_fixture_resume_after_halt_at(state.machine, 0u);
            before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            before.data.cs.seg.exec.defsize = LIB_TRUE;
            before.data.esp = 0x12348000u;
            state.machine->executor_cpu = before;
            lib_memory_copy(&tss[4], &esp0, sizeof(esp0));
            lib_memory_copy(&tss[8], &ss0, sizeof(ss0));
            gate[0] = 0x00u;
            gate[1] = 0x01u;
            gate[2] = 0x08u;
            gate[5] = 0x8eu;
            state.machine->executor_cpu.data.gdtr.limit = 0x004fu;
            state.machine->executor_cpu.data.idtr.limit = 0x0107u;
            state.machine->executor_cpu.data.tr.flagValid = LIB_TRUE;
            state.machine->executor_cpu.data.tr.selector = 0x0048u;
            state.machine->executor_cpu.data.tr.sregtype = SREG_TR;
            state.machine->executor_cpu.data.tr.base = 0x0600u;
            state.machine->executor_cpu.data.tr.limit = 0x0067u;
            state.machine->executor_cpu.data.tr.dpl = 0u;
            state.machine->executor_cpu.data.tr.sys.type =
                VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
            failed |= !atomic_write(&state, ATOMIC_KERNEL_BASE, iret,
                sizeof(iret));
            failed |= !atomic_write(&state,
                ATOMIC_KERNEL_STACK_BASE + 0x8000u, (const lib_u8 *)frame,
                sizeof(frame));
            failed |= !atomic_write(&state, 0x4010u, &nop, sizeof(nop));
            failed |= !atomic_write(&state, 0x4100u, &hlt, sizeof(hlt));
            failed |= !atomic_write(&state, ATOMIC_GDT_BASE + 72u,
                tss_descriptor, sizeof(tss_descriptor));
            failed |= !atomic_write(&state, 0x0600u, tss, sizeof(tss));
            failed |= !atomic_write(&state, ATOMIC_IDT_BASE + vector * 8u,
                gate, sizeof(gate));
        }
        if (!failed) {
            lib_memory_set(&source, 0, sizeof(source));
            state.machine->shared_pic_master.data.icw2 = vector;
            core_machine_pic_irq_source_bind(&source,
                &state.machine->shared_pic_master,
                &state.machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
            failed |= core_machine_run(state.machine,
                (core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK;
            failed |= core_machine_get_cpu_diagnostic(state.machine,
                &diagnostic) != LIB_STATUS_OK;
            after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
            failed |= diagnostic.first_fault.valid;
            if (restore_if) {
                failed |= result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
                failed |= after.data.eip != 0x00000101u;
                failed |= after.data.cs.selector != 0x0008u;
                failed |= !after.data.flagHalt;
                failed |= !CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.isr,
                    VPIC_ISR_IRQ(0u));
                failed |= CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.irr,
                    VPIC_IRR_IRQ(0u));
                failed |= core_machine_memory_read_physical(
                    &state.machine->executor_memory, after.data.ss.base +
                    CORE_MACHINE_MASK_U16(after.data.esp),
                    (lib_uptr)irq_frame,
                    sizeof(irq_frame)) != LIB_STATUS_OK;
                failed |= irq_frame[0] != 0x00000010u;
            } else {
                failed |= result.reason != CORE_MACHINE_STOP_BUDGET;
                failed |= after.data.eip != 0x00000011u;
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
    if (!iret_outer_s52_test_rejections()) {
        fprintf(stderr, "S52 reject failed\n");
        return 1;
    }
    if (!iret_outer_s52_test_outer()) {
        fprintf(stderr, "S52 outer failed\n");
        return 1;
    }
    if (!iret_outer_s52_test_combined()) {
        fprintf(stderr, "S52 combined failed\n");
        return 1;
    }
    if (!iret_outer_s52_test_success()) {
        fprintf(stderr, "S52 success failed\n");
        return 1;
    }
    if (!iret_outer_s52_test_pic())
        return 1;
    printf("M5:T316:S52:IRET-OUTER:OK\n");
    return 0;
}
