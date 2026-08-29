#include "type.h"

#include "core/machine/pic.h"

/* Reuse S3's owner-local protected gate fixture without creating a public test API. */
#define main protected_16_gate_s3_retained_main
#include "core_machine_protected_16_gate_s3_smoke.c"
#undef main

#define S4_TSS_BASE 0x0500u
#define S4_KERNEL_STACK_TOP 0x7000u

static C_INT s4_prepare_user_stack(s3_gate_machine *state)
{
    static const type_unsigned_8 user_data[] = {
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

static C_INT s4_prepare_outer_entry(s3_gate_machine *state,
    core_machine_cpu_profile profile, type_unsigned_8 gate_type,
    type_unsigned_8 gate_dpl)
{
    type_unsigned_8 tss[8u] = { 0u };
    type_unsigned_8 descriptor[8u] = { 0u };
    t_cpu_data_sreg *tr;

    if (!s3_gate_prepare(state, profile, TYPE_TRUE, gate_type, gate_dpl,
            TYPE_TRUE) || !s4_prepare_user_stack(state) || !s3_gate_install(
            state, S3_VECTOR, 0x0008u, gate_type, gate_dpl, TYPE_TRUE)) {
        return 0;
    }
    tss[2u] = S4_KERNEL_STACK_TOP & 0xffu;
    tss[3u] = S4_KERNEL_STACK_TOP >> 8u;
    tss[4u] = 0x10u;
    descriptor[0u] = (sizeof(tss) - 1u) & 0xffu;
    descriptor[1u] = (sizeof(tss) - 1u) >> 8u;
    descriptor[2u] = S4_TSS_BASE & 0xffu;
    descriptor[3u] = (S4_TSS_BASE >> 8u) & 0xffu;
    descriptor[4u] = (S4_TSS_BASE >> 16u) & 0xffu;
    descriptor[5u] = 0x80u | VCPU_DESC_SYS_TYPE_TSS_16_BUSY;
    if (!s3_gate_write(state, S4_TSS_BASE, tss, sizeof(tss)) || !s3_gate_write(
            state, S3_GDT_BASE + 40u, descriptor, sizeof(descriptor))) {
        return 0;
    }
    state->machine->executor_cpu.data.gdtr.limit = 47u;
    tr = &state->machine->executor_cpu.data.tr;
    STD_MEMSET(tr, 0, sizeof(*tr));
    tr->flagValid = TYPE_TRUE;
    tr->selector = 0x0028u;
    tr->sregtype = SREG_TR;
    tr->base = S4_TSS_BASE;
    tr->limit = sizeof(tss) - 1u;
    tr->dpl = 0u;
    tr->sys.type = VCPU_DESC_SYS_TYPE_TSS_16_BUSY;
    return 1;
}

static C_INT s4_outer_entry(core_machine_cpu_profile profile,
    type_unsigned_8 gate_type, type_bool software_origin, type_bool nmi)
{
    static const type_unsigned_8 nop[] = { 0x90u };
    static const type_unsigned_8 software[] = { 0xcdu,S3_VECTOR };
    s3_gate_machine state;
    core_machine_pic_irq_source source;
    core_machine_run_result result;
    type_unsigned_16 frame[5u] = { 0u,0u,0u,0u,0u };
    type_unsigned_16 expected_ip = software_origin ? 2u : 1u;
    type_bool expect_if = gate_type == VCPU_DESC_SYS_TYPE_TRAPGATE_16;
    type_bool frame_read;
    type_status run_status;
    C_INT failed = !s4_prepare_outer_entry(&state, profile, gate_type,
        software_origin ? 3u : 0u);

    STD_MEMSET(&source, 0, sizeof(source));
    if (!failed) {
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
            VCPU_EFLAGS_IF | (software_origin ? VCPU_EFLAGS_TF : 0u);
        failed |= !s3_gate_write(&state, S3_CODE_BASE,
            software_origin ? software : nop,
            software_origin ? sizeof(software) : sizeof(nop));
        if (!failed && nmi) {
            failed |= !s3_gate_install(&state, 0x02u, 0x0008u, gate_type,
                0u, TYPE_TRUE);
            state.machine->executor_cpu.data.flagNMI = TYPE_TRUE;
        }
        if (!failed && !software_origin && !nmi) {
            state.machine->shared_pic_master.data.icw2 = S3_VECTOR;
            core_machine_pic_irq_source_bind(&source,
                &state.machine->shared_pic_master,
                &state.machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
        }
        run_status = core_machine_run(state.machine, (core_machine_run_budget){1u,0u},
            &result);
        frame_read = s3_gate_read(&state, S4_KERNEL_STACK_TOP - sizeof(frame),
            frame, sizeof(frame));
        failed |= run_status != TYPE_STATUS_OK || result.reason !=
                CORE_MACHINE_STOP_BUDGET ||
            state.machine->executor_cpu.data.eip != S3_HANDLER ||
            state.machine->executor_cpu.data.cs.selector != 0x0008u ||
            state.machine->executor_cpu.data.cs.dpl != 0u ||
            state.machine->executor_cpu.data.ss.selector != 0x0010u ||
            state.machine->executor_cpu.data.ss.dpl != 0u ||
            state.machine->executor_cpu.data.sp != S4_KERNEL_STACK_TOP -
                sizeof(frame) || !TYPE_GET_BIT(state.machine->executor_cpu.data.eflags,
                VCPU_EFLAGS_CF) || TYPE_GET_BIT(
                state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_TF) ||
            (TYPE_GET_BIT(state.machine->executor_cpu.data.eflags,
                VCPU_EFLAGS_IF) != expect_if) ||
            !frame_read ||
            frame[0u] != expected_ip || frame[1u] != 0x001bu || frame[2u] !=
                (VCPU_EFLAGS_CF | VCPU_EFLAGS_IF |
                (software_origin ? VCPU_EFLAGS_TF : 0u)) ||
            frame[3u] != S3_STACK_TOP || frame[4u] != 0x0023u;
        if (!failed && nmi) {
            failed |= state.machine->executor_cpu.data.flagNMI;
        } else if (!failed && !software_origin) {
            failed |= !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(
                state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u));
        }
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT s4_outer_software_dpl_error(C_VOID)
{
    static const type_unsigned_8 software[] = { 0xcdu,S3_VECTOR };
    s3_gate_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    type_unsigned_16 frame[6u] = { 0u,0u,0u,0u,0u,0u };
    C_INT failed = !s4_prepare_outer_entry(&state,
        CORE_MACHINE_CPU_PROFILE_80286, VCPU_DESC_SYS_TYPE_INTGATE_16, 0u);

    if (!failed) {
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
            VCPU_EFLAGS_IF | VCPU_EFLAGS_TF;
        failed |= !s3_gate_install(&state, 0x0du, 0x0008u,
            VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, TYPE_TRUE) || !s3_gate_write(
            &state, S3_CODE_BASE, software, sizeof(software)) ||
            core_machine_run(state.machine, (core_machine_run_budget){1u,0u},
                &result) != TYPE_STATUS_OK || result.reason !=
                CORE_MACHINE_STOP_BUDGET || core_machine_get_cpu_diagnostic(
                state.machine, &diagnostic) != TYPE_STATUS_OK ||
            !diagnostic.last_delivered_exception.valid ||
            diagnostic.last_delivered_exception.exception_mask !=
                VCPUINS_EXCEPT_GP || state.machine->executor_cpu.data.eip !=
                S3_HANDLER || state.machine->executor_cpu.data.cs.selector !=
                0x0008u || state.machine->executor_cpu.data.ss.selector !=
                0x0010u || state.machine->executor_cpu.data.sp !=
                S4_KERNEL_STACK_TOP - sizeof(frame) || !s3_gate_read(&state,
                S4_KERNEL_STACK_TOP - sizeof(frame), frame, sizeof(frame)) ||
            frame[0u] != (type_unsigned_16)(S3_VECTOR * 8u + 2u) ||
            frame[1u] != 0u || frame[2u] != 0x001bu || frame[3u] !=
                (VCPU_EFLAGS_CF | VCPU_EFLAGS_IF | VCPU_EFLAGS_TF) ||
            frame[4u] != S3_STACK_TOP || frame[5u] != 0x0023u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT s4_rejected_cpu_same(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx && before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx && before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp && before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi && after->data.eip == before->data.eip + 1u &&
        before->data.eflags == after->data.eflags && STD_MEMCMP(&before->data.es,
            &after->data.es, sizeof(before->data.es)) == 0 && STD_MEMCMP(
            &before->data.cs, &after->data.cs, sizeof(before->data.cs)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after->data.ss, sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after->data.ds, sizeof(before->data.ds)) == 0 &&
        STD_MEMCMP(&before->data.fs, &after->data.fs, sizeof(before->data.fs)) == 0 &&
        STD_MEMCMP(&before->data.gs, &after->data.gs, sizeof(before->data.gs)) == 0;
}

static C_INT s4_external_event(core_machine_cpu_profile profile,
    type_unsigned_8 gate_type, type_bool nmi)
{
    static const type_unsigned_8 nop[] = { 0x90u };
    static const type_unsigned_8 loop[] = { 0xebu,0xfeu };
    s3_gate_machine state;
    core_machine_pic_irq_source source;
    core_machine_run_result result;
    type_unsigned_16 frame[3u] = { 0u,0u,0u };
    type_bool expect_if = gate_type == VCPU_DESC_SYS_TYPE_TRAPGATE_16;
    C_INT failed = !s3_gate_prepare(&state, profile, TYPE_TRUE, gate_type, 0u,
        TYPE_TRUE);

    STD_MEMSET(&source, 0, sizeof(source));
    if (!failed) {
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF;
        failed |= !s4_prepare_user_stack(&state) ||
            (nmi && !s3_gate_install(&state, 0x02u, 0x001bu, gate_type, 0u,
                TYPE_TRUE)) ||
            !s3_gate_write(&state, S3_CODE_BASE, nop, sizeof(nop)) ||
            !s3_gate_write(&state, S3_CODE_BASE + S3_HANDLER, loop, sizeof(loop));
        if (!failed && nmi) state.machine->executor_cpu.data.flagNMI = TYPE_TRUE;
        if (!failed && !nmi) {
            state.machine->shared_pic_master.data.icw2 = S3_VECTOR;
            core_machine_pic_irq_source_bind(&source,
                &state.machine->shared_pic_master, &state.machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
        }
        if (core_machine_run(state.machine, (core_machine_run_budget){2u,0u},
            &result) != TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
            state.machine->executor_cpu.data.eip != S3_HANDLER ||
            state.machine->executor_cpu.data.esp != S3_STACK_TOP - 6u ||
            TYPE_GET_BIT(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_TF) ||
            (TYPE_GET_BIT(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_IF) !=
                expect_if) || !s3_gate_read(&state, S3_STACK_TOP - 6u, frame,
                sizeof(frame)) || frame[0] != 1u || frame[1] != 0x001bu ||
            frame[2] != (VCPU_EFLAGS_CF | VCPU_EFLAGS_IF)) {
            failed = 1;
        }
        if (!failed && nmi) {
            failed |= state.machine->executor_cpu.data.flagNMI;
        } else if (!failed) {
            failed |= !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
                VPIC_IRR_IRQ(0u));
        }
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT s4_rejected_event(type_unsigned_8 gate_type, type_bool present,
    type_bool nmi)
{
    static const type_unsigned_8 nop[] = { 0x90u };
    s3_gate_machine state;
    core_machine_pic_irq_source source;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_16 stack_before[3u] = { 0x1357u,0x2468u,0x369cu };
    type_unsigned_16 stack_after[3u] = { 0u,0u,0u };
    C_INT failed = !s3_gate_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386,
        TYPE_TRUE, gate_type, 0u, present);

    STD_MEMSET(&source, 0, sizeof(source));
    if (!failed) {
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_IF;
        failed |= !s4_prepare_user_stack(&state) ||
            (nmi && !s3_gate_install(&state, 0x02u, 0x001bu, gate_type, 0u,
                present)) ||
            !s3_gate_write(&state, S3_CODE_BASE, nop, sizeof(nop)) ||
            !s3_gate_write(&state, S3_STACK_TOP - sizeof(stack_before),
                stack_before, sizeof(stack_before));
        if (!failed && nmi) state.machine->executor_cpu.data.flagNMI = TYPE_TRUE;
        if (!failed && !nmi) {
            state.machine->shared_pic_master.data.icw2 = S3_VECTOR;
            core_machine_pic_irq_source_bind(&source,
                &state.machine->shared_pic_master, &state.machine->shared_pic_slave, 0u);
            core_machine_pic_irq_source_assert(&source);
            core_machine_pic_irq_source_deassert(&source);
        }
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine, (core_machine_run_budget){8u,0u},
            &result) != TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK ||
            !diagnostic.first_fault.valid;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !s4_rejected_cpu_same(&before, &after) ||
            !s3_gate_read(&state, S3_STACK_TOP - sizeof(stack_after), stack_after,
                sizeof(stack_after)) || STD_MEMCMP(stack_before, stack_after,
                sizeof(stack_before)) != 0;
        if (!failed && nmi) {
            failed |= !state.machine->executor_cpu.data.flagNMI;
        } else if (!failed) {
            failed |= !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u)) || TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
                VPIC_IRR_IRQ(0u));
        }
    }
    core_machine_destroy(state.machine);
    return !failed;
}

C_INT main(C_VOID)
{
    C_INT failed = !s4_external_event(CORE_MACHINE_CPU_PROFILE_80286,
        VCPU_DESC_SYS_TYPE_INTGATE_16, TYPE_FALSE) ||
        !s4_external_event(CORE_MACHINE_CPU_PROFILE_80286,
            VCPU_DESC_SYS_TYPE_TRAPGATE_16, TYPE_TRUE) ||
        !s4_external_event(CORE_MACHINE_CPU_PROFILE_80386,
            VCPU_DESC_SYS_TYPE_INTGATE_16, TYPE_TRUE) ||
        !s4_external_event(CORE_MACHINE_CPU_PROFILE_80386,
            VCPU_DESC_SYS_TYPE_TRAPGATE_16, TYPE_FALSE) ||
        !s4_outer_entry(CORE_MACHINE_CPU_PROFILE_80286,
            VCPU_DESC_SYS_TYPE_INTGATE_16, TYPE_TRUE, TYPE_FALSE) ||
        !s4_outer_entry(CORE_MACHINE_CPU_PROFILE_80286,
            VCPU_DESC_SYS_TYPE_INTGATE_16, TYPE_FALSE, TYPE_FALSE) ||
        !s4_outer_entry(CORE_MACHINE_CPU_PROFILE_80286,
            VCPU_DESC_SYS_TYPE_TRAPGATE_16, TYPE_FALSE, TYPE_TRUE) ||
        !s4_outer_software_dpl_error() ||
        !s4_rejected_event(0u, TYPE_TRUE, TYPE_FALSE) ||
        !s4_rejected_event(VCPU_DESC_SYS_TYPE_INTGATE_16, TYPE_FALSE,
            TYPE_TRUE);

    if (failed) return 1;
    STD_PRINTF("M5:T323:S4:PROTECTED-16-EXTERNAL:OK\n");
    return 0;
}
