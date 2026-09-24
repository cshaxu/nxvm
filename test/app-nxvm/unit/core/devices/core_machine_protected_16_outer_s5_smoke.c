#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "app-nxvm/devices/pic.h"

/* Reuse S3's owner-local protected-gate fixture without a new support API. */
#define main protected_16_gate_s3_retained_main
#include "core_machine_protected_16_gate_s3_smoke.c"
#undef main

#define S5_TSS_BASE 0x0500u
#define S5_KERNEL_STACK_TOP 0x7000u

static lib_i32 s5_prepare_user_stack(s3_gate_machine *state)
{
    static const lib_u8 user_data[] = {
        0xffu,0xffu,0,0,0,0xf2u,0,0
    };
    t_cpu_data_sreg *ss;

    if (!s3_gate_write(state, S3_GDT_BASE + 32u, user_data, sizeof(user_data)))
        return 0;
    state->machine->executor_cpu.data.gdtr.limit = 39u;
    ss = &state->machine->executor_cpu.data.ss;
    ss->selector = 0x0023u;
    ss->dpl = 3u;
    return 1;
}

static lib_i32 s5_prepare_outer(s3_gate_machine *state,
    core_machine_cpu_profile profile, lib_u8 gate_type,
    lib_u8 nmi, lib_u8 tss32)
{
    static const lib_u8 nop[] = { 0x90u };
    static const lib_u8 loop[] = { 0xebu,0xfeu };
    lib_u8 tss[16u] = { 0u };
    t_cpu_data_sreg *tr;

    if (!s3_gate_prepare(state, profile, LIB_TRUE, gate_type, 0u, LIB_TRUE) ||
        !s5_prepare_user_stack(state) || (nmi && !s3_gate_install(state, 0x02u,
            0x0008u, gate_type, 0u, LIB_TRUE)) || !s3_gate_install(state,
            S3_VECTOR, 0x0008u, gate_type, 0u, LIB_TRUE) || !s3_gate_write(state,
            S3_CODE_BASE, nop, sizeof(nop)) || !s3_gate_write(state,
            S3_CODE_BASE + S3_HANDLER, loop, sizeof(loop))) {
        return 0;
    }
    if (tss32) {
        tss[4u] = S5_KERNEL_STACK_TOP & 0xffu;
        tss[5u] = S5_KERNEL_STACK_TOP >> 8u;
        tss[8u] = 0x10u;
    } else {
        tss[2u] = S5_KERNEL_STACK_TOP & 0xffu;
        tss[3u] = S5_KERNEL_STACK_TOP >> 8u;
        tss[4u] = 0x10u;
    }
    if (!s3_gate_write(state, S5_TSS_BASE, tss, sizeof(tss))) return 0;
    tr = &state->machine->executor_cpu.data.tr;
    lib_memory_set(tr, 0, sizeof(*tr));
    tr->flagValid = LIB_TRUE;
    tr->selector = 0x0028u;
    tr->sregtype = SREG_TR;
    tr->base = S5_TSS_BASE;
    tr->limit = sizeof(tss) - 1u;
    tr->dpl = 0u;
    tr->sys.type = tss32 ? VCPU_DESC_SYS_TYPE_TSS_32_BUSY :
        VCPU_DESC_SYS_TYPE_TSS_16_BUSY;
    state->machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF;
    return 1;
}

static lib_i32 s5_outer_event(core_machine_cpu_profile profile,
    lib_u8 gate_type, lib_u8 nmi, lib_u8 tss32)
{
    s3_gate_machine state;
    core_machine_pic_irq_source source;
    core_machine_run_result result;
    lib_u16 frame[5u] = { 0u,0u,0u,0u,0u };
    lib_u8 expect_if = gate_type == VCPU_DESC_SYS_TYPE_TRAPGATE_16;
    t_cpu before;
    t_cpu after;
    lib_i32 failed = !s5_prepare_outer(&state, profile, gate_type, nmi, tss32);

    lib_memory_set(&source, 0, sizeof(source));
    if (!failed && nmi) state.machine->executor_cpu.data.flagNMI = LIB_TRUE;
    if (!failed && !nmi) {
        state.machine->shared_pic_master.data.icw2 = S3_VECTOR;
        core_machine_pic_irq_source_bind(&source, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
    }
    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine, (core_machine_run_budget){2u,0u},
            &result) != LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
            state.machine->executor_cpu.data.cs.selector != 0x0008u ||
            state.machine->executor_cpu.data.cs.dpl != 0u ||
            state.machine->executor_cpu.data.ss.selector != 0x0010u ||
            state.machine->executor_cpu.data.ss.dpl != 0u ||
            state.machine->executor_cpu.data.eip != S3_HANDLER ||
            state.machine->executor_cpu.data.esp != S5_KERNEL_STACK_TOP - 10u ||
            CORE_MACHINE_BIT_IS_SET(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_TF) ||
            (CORE_MACHINE_BIT_IS_SET(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_IF) !=
                expect_if) || !s3_gate_read(&state, S5_KERNEL_STACK_TOP - 10u,
                frame, sizeof(frame)) || frame[0] != 1u || frame[1] != 0x001bu ||
            frame[2] != (VCPU_EFLAGS_CF | VCPU_EFLAGS_IF) ||
            frame[3] != S3_STACK_TOP || frame[4] != 0x0023u;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !s3_gate_gprs_same(&before, &after) || lib_memory_compare(&before.data.es,
            &after.data.es, sizeof(before.data.es)) != 0 || lib_memory_compare(
            &before.data.ds, &after.data.ds, sizeof(before.data.ds)) != 0 ||
            lib_memory_compare(&before.data.fs, &after.data.fs, sizeof(before.data.fs)) != 0 ||
            lib_memory_compare(&before.data.gs, &after.data.gs, sizeof(before.data.gs)) != 0;
        if (!failed && nmi) {
            failed |= state.machine->executor_cpu.data.flagNMI;
        } else if (!failed) {
            failed |= !CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u)) || CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.irr,
                VPIC_IRR_IRQ(0u));
        }
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 s5_rejected_cpu_same(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx && before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx && before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp && before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi && after->data.eip == before->data.eip + 1u &&
        before->data.eflags == after->data.eflags && lib_memory_compare(&before->data.es,
            &after->data.es, sizeof(before->data.es)) == 0 && lib_memory_compare(
            &before->data.cs, &after->data.cs, sizeof(before->data.cs)) == 0 &&
        lib_memory_compare(&before->data.ss, &after->data.ss, sizeof(before->data.ss)) == 0 &&
        lib_memory_compare(&before->data.ds, &after->data.ds, sizeof(before->data.ds)) == 0 &&
        lib_memory_compare(&before->data.fs, &after->data.fs, sizeof(before->data.fs)) == 0 &&
        lib_memory_compare(&before->data.gs, &after->data.gs, sizeof(before->data.gs)) == 0;
}

static lib_i32 s5_rejected_outer(lib_u8 invalid_tr, lib_u8 invalid_ss,
    lib_u8 nonpresent_ss)
{
    s3_gate_machine state;
    core_machine_pic_irq_source source;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    lib_u16 sentinel_before[5u] = {
        0x1357u,0x2468u,0x369cu,0x48adu,0x5bceu
    };
    lib_u16 sentinel_after[5u] = { 0u,0u,0u,0u,0u };
    lib_u16 null_ss = 0u;
    lib_u8 nonpresent_access = 0x12u;
    t_cpu before;
    t_cpu after;
    lib_i32 failed = !s5_prepare_outer(&state, CORE_MACHINE_CPU_PROFILE_80386,
        VCPU_DESC_SYS_TYPE_INTGATE_16, LIB_FALSE, LIB_TRUE);

    lib_memory_set(&source, 0, sizeof(source));
    if (!failed && invalid_tr) state.machine->executor_cpu.data.tr.flagValid = LIB_FALSE;
    if (!failed && invalid_ss) {
        failed |= !s3_gate_write(&state, S5_TSS_BASE + 8u, &null_ss,
            sizeof(null_ss));
    }
    if (!failed && nonpresent_ss) {
        failed |= !s3_gate_write(&state, S3_GDT_BASE + 16u + 5u,
            &nonpresent_access, sizeof(nonpresent_access));
    }
    if (!failed) {
        failed |= !s3_gate_write(&state, S5_KERNEL_STACK_TOP - sizeof(sentinel_before),
            sentinel_before, sizeof(sentinel_before));
        state.machine->shared_pic_master.data.icw2 = S3_VECTOR;
        core_machine_pic_irq_source_bind(&source, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine, (core_machine_run_budget){8u,0u},
            &result) != LIB_STATUS_INTERNAL_ERROR || result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK ||
            !diagnostic.first_fault.valid;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !s5_rejected_cpu_same(&before, &after) || !s3_gate_read(&state,
            S5_KERNEL_STACK_TOP - sizeof(sentinel_after), sentinel_after,
            sizeof(sentinel_after)) || lib_memory_compare(sentinel_before, sentinel_after,
            sizeof(sentinel_before)) != 0 || !CORE_MACHINE_BIT_IS_SET(
            state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u)) ||
            CORE_MACHINE_BIT_IS_SET(state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u));
    }
    core_machine_destroy(state.machine);
    return !failed;
}

lib_i32 main(void)
{
    const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    const lib_u8 gate_types[] = {
        VCPU_DESC_SYS_TYPE_INTGATE_16, VCPU_DESC_SYS_TYPE_TRAPGATE_16
    };
    lib_size profile;
    lib_size gate;
    lib_i32 failed = 0;

    for (profile = 0u; profile < sizeof(profiles) / sizeof(profiles[0]); ++profile) {
        for (gate = 0u; gate < sizeof(gate_types) / sizeof(gate_types[0]); ++gate) {
            failed |= !s5_outer_event(profiles[profile], gate_types[gate],
                LIB_FALSE, LIB_FALSE) || !s5_outer_event(profiles[profile],
                gate_types[gate], LIB_TRUE, LIB_FALSE);
            if (profiles[profile] == CORE_MACHINE_CPU_PROFILE_80386) {
                failed |= !s5_outer_event(profiles[profile], gate_types[gate],
                    LIB_FALSE, LIB_TRUE) || !s5_outer_event(profiles[profile],
                    gate_types[gate], LIB_TRUE, LIB_TRUE);
            }
        }
    }
    failed |= !s5_rejected_outer(LIB_TRUE, LIB_FALSE, LIB_FALSE) ||
        !s5_rejected_outer(LIB_FALSE, LIB_TRUE, LIB_FALSE) ||
        !s5_rejected_outer(LIB_FALSE, LIB_FALSE, LIB_TRUE);

    if (failed) return 1;
    printf("M5:T323:S5:PROTECTED-16-OUTER:OK\n");
    return 0;
}
