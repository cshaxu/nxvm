#include "type.h"

#include "core/machine/pic.h"

/* Reuse S3's owner-local protected gate fixture without creating a public test API. */
#define main protected_16_gate_s3_retained_main
#include "core_machine_protected_16_gate_s3_smoke.c"
#undef main

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
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF |
            VCPU_EFLAGS_TF;
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
            frame[2] != (VCPU_EFLAGS_CF | VCPU_EFLAGS_IF | VCPU_EFLAGS_TF)) {
            core_machine_cpu_diagnostic diagnostic;

            (C_VOID)core_machine_get_cpu_diagnostic(state.machine, &diagnostic);
            STD_PRINTF("S4 p=%u g=%u n=%u reason=%u eip=%x esp=%x flags=%x first=%u/%x last=%u/%x\\n",
                (type_unsigned_32)profile, (type_unsigned_32)gate_type,
                (type_unsigned_32)nmi, (type_unsigned_32)result.reason,
                state.machine->executor_cpu.data.eip,
                state.machine->executor_cpu.data.esp,
                state.machine->executor_cpu.data.eflags,
                (type_unsigned_32)diagnostic.first_fault.valid,
                diagnostic.first_fault.exception_mask,
                (type_unsigned_32)diagnostic.last_delivered_exception.valid,
                diagnostic.last_delivered_exception.exception_mask);
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
            failed |= !TYPE_GET_BIT(state.machine->shared_pic_master.data.irr,
                VPIC_IRR_IRQ(0u)) || TYPE_GET_BIT(state.machine->shared_pic_master.data.isr,
                VPIC_ISR_IRQ(0u));
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
        !s4_rejected_event(0u, TYPE_TRUE, TYPE_FALSE) ||
        !s4_rejected_event(VCPU_DESC_SYS_TYPE_INTGATE_16, TYPE_FALSE,
            TYPE_TRUE);

    if (failed) return 1;
    STD_PRINTF("M5:T323:S4:PROTECTED-16-EXTERNAL:OK\n");
    return 0;
}
