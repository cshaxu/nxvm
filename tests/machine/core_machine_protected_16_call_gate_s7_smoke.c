#include "type.h"

#include "core/machine/pic.h"

/* Reuse S3's owner-local protected-gate fixture without a support API. */
#define main protected_16_gate_s3_retained_main
#include "core_machine_protected_16_gate_s3_smoke.c"
#undef main

#define S7_TSS_BASE 0x0500u
#define S7_KERNEL_STACK_TOP 0x7000u
#define S7_CALL_GATE_SELECTOR 0x0033u
#define S7_CALL_TARGET 0x0100u
#define S7_PARAMETER_COUNT 2u
#define S7_REJECT_INVALID_TR 0x01u
#define S7_REJECT_NULL_SS 0x02u
#define S7_REJECT_GATE_DPL 0x04u
#define S7_REJECT_TARGET_CODE 0x08u
#define S7_REJECT_STACK 0x10u
#define S7_REJECT_PARAMETER_SOURCE 0x20u

static C_INT s7_prepare_user_stack(s3_gate_machine *state)
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

static C_INT s7_prepare_outer(s3_gate_machine *state,
    core_machine_cpu_profile profile, type_bool tss32)
{
    static const type_unsigned_8 loop[] = { 0xebu,0xfeu };
    type_unsigned_8 tss[16u] = { 0u };
    type_unsigned_8 tss_descriptor[8u] = { 0u };
    t_cpu_data_sreg *tr;

    if (!s3_gate_prepare(state, profile, TYPE_TRUE,
            VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, TYPE_TRUE) ||
        !s7_prepare_user_stack(state) || !s3_gate_write(state,
            S3_CODE_BASE + S7_CALL_TARGET, loop, sizeof(loop))) {
        return 0;
    }
    if (tss32) {
        tss[4u] = S7_KERNEL_STACK_TOP & 0xffu;
        tss[5u] = S7_KERNEL_STACK_TOP >> 8u;
        tss[8u] = 0x10u;
    } else {
        tss[2u] = S7_KERNEL_STACK_TOP & 0xffu;
        tss[3u] = S7_KERNEL_STACK_TOP >> 8u;
        tss[4u] = 0x10u;
    }
    tss_descriptor[0] = (sizeof(tss) - 1u) & 0xffu;
    tss_descriptor[1] = (sizeof(tss) - 1u) >> 8u;
    tss_descriptor[2] = S7_TSS_BASE & 0xffu;
    tss_descriptor[3] = (S7_TSS_BASE >> 8u) & 0xffu;
    tss_descriptor[4] = (S7_TSS_BASE >> 16u) & 0xffu;
    tss_descriptor[5] = (type_unsigned_8)(0x80u | (tss32 ?
        VCPU_DESC_SYS_TYPE_TSS_32_BUSY : VCPU_DESC_SYS_TYPE_TSS_16_BUSY));
    state->machine->executor_cpu.data.gdtr.limit = 55u;
    if (!s3_gate_write(state, S7_TSS_BASE, tss, sizeof(tss)) || !s3_gate_write(
            state, S3_GDT_BASE + 40u, tss_descriptor, sizeof(tss_descriptor))) {
        return 0;
    }
    tr = &state->machine->executor_cpu.data.tr;
    STD_MEMSET(tr, 0, sizeof(*tr));
    tr->flagValid = TYPE_TRUE;
    tr->selector = 0x0028u;
    tr->sregtype = SREG_TR;
    tr->base = S7_TSS_BASE;
    tr->limit = sizeof(tss) - 1u;
    tr->dpl = 0u;
    tr->sys.type = tss32 ? VCPU_DESC_SYS_TYPE_TSS_32_BUSY :
        VCPU_DESC_SYS_TYPE_TSS_16_BUSY;
    return 1;
}

static C_INT s7_install_call_gate(s3_gate_machine *state,
    type_unsigned_8 dpl, type_unsigned_8 parameter_count, type_bool present)
{
    type_unsigned_8 descriptor[8u] = { 0u };

    descriptor[0] = S7_CALL_TARGET & 0xffu;
    descriptor[1] = S7_CALL_TARGET >> 8u;
    descriptor[2] = 0x08u;
    descriptor[4] = parameter_count;
    descriptor[5] = (type_unsigned_8)((present ? 0x80u : 0u) | (dpl << 5u) |
        VCPU_DESC_SYS_TYPE_CALLGATE_16);
    state->machine->executor_cpu.data.gdtr.limit = 55u;
    return s3_gate_write(state, S3_GDT_BASE + 48u, descriptor,
        sizeof(descriptor));
}

static C_INT s7_write_call(s3_gate_machine *state, type_unsigned_16 selector)
{
    type_unsigned_8 call[] = { 0x9au,S7_CALL_TARGET & 0xffu,S7_CALL_TARGET >> 8u,
        selector & 0xffu,selector >> 8u };

    return s3_gate_write(state, S3_CODE_BASE, call, sizeof(call));
}

static C_INT s7_cpu_same(const t_cpu *before, const t_cpu *after)
{
    return before->data.eax == after->data.eax &&
        before->data.ecx == after->data.ecx && before->data.edx == after->data.edx &&
        before->data.ebx == after->data.ebx && before->data.esp == after->data.esp &&
        before->data.ebp == after->data.ebp && before->data.esi == after->data.esi &&
        before->data.edi == after->data.edi && before->data.eip == after->data.eip &&
        before->data.eflags == after->data.eflags && STD_MEMCMP(&before->data.es,
            &after->data.es, sizeof(before->data.es)) == 0 && STD_MEMCMP(
            &before->data.cs, &after->data.cs, sizeof(before->data.cs)) == 0 &&
        STD_MEMCMP(&before->data.ss, &after->data.ss, sizeof(before->data.ss)) == 0 &&
        STD_MEMCMP(&before->data.ds, &after->data.ds, sizeof(before->data.ds)) == 0 &&
        STD_MEMCMP(&before->data.fs, &after->data.fs, sizeof(before->data.fs)) == 0 &&
        STD_MEMCMP(&before->data.gs, &after->data.gs, sizeof(before->data.gs)) == 0;
}

static C_INT s7_outer_success(core_machine_cpu_profile profile, type_bool tss32)
{
    s3_gate_machine state;
    core_machine_run_result result;
    type_unsigned_16 parameters[S7_PARAMETER_COUNT] = { 0x1234u,0xabcdu };
    type_unsigned_16 frame[4u + S7_PARAMETER_COUNT] = { 0u };
    t_cpu before;
    C_INT failed = !s7_prepare_outer(&state, profile, tss32);

    if (!failed) {
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF;
        failed |= !s7_install_call_gate(&state, 3u, S7_PARAMETER_COUNT,
            TYPE_TRUE) || !s7_write_call(&state, S7_CALL_GATE_SELECTOR) || !s3_gate_write(&state,
            S3_STACK_TOP, parameters, sizeof(parameters));
    }
    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){1u,0u}, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            state.machine->executor_cpu.data.eip != S7_CALL_TARGET ||
            state.machine->executor_cpu.data.cs.selector != 0x0008u ||
            state.machine->executor_cpu.data.cs.dpl != 0u ||
            state.machine->executor_cpu.data.ss.selector != 0x0010u ||
            state.machine->executor_cpu.data.ss.dpl != 0u ||
            state.machine->executor_cpu.data.sp != S7_KERNEL_STACK_TOP -
                (type_unsigned_16)sizeof(frame) || state.machine->executor_cpu.data.eflags !=
                before.data.eflags || !s3_gate_gprs_same(&before,
                &state.machine->executor_cpu) || STD_MEMCMP(&before.data.es,
                &state.machine->executor_cpu.data.es, sizeof(before.data.es)) != 0 ||
            STD_MEMCMP(&before.data.ds, &state.machine->executor_cpu.data.ds,
                sizeof(before.data.ds)) != 0 || STD_MEMCMP(&before.data.fs,
                &state.machine->executor_cpu.data.fs, sizeof(before.data.fs)) != 0 ||
            STD_MEMCMP(&before.data.gs, &state.machine->executor_cpu.data.gs,
                sizeof(before.data.gs)) != 0 || !s3_gate_read(&state,
                S7_KERNEL_STACK_TOP - sizeof(frame), frame, sizeof(frame)) ||
            frame[0] != 5u || frame[1] != 0x001bu || frame[2] != parameters[0] ||
            frame[3] != parameters[1] || frame[4] != S3_STACK_TOP ||
            frame[5] != 0x0023u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT s7_same_cpl(C_VOID)
{
    s3_gate_machine state;
    core_machine_run_result result;
    type_unsigned_16 frame[2u] = { 0u,0u };
    t_cpu before;
    C_INT failed = !s3_gate_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386,
        TYPE_FALSE, VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, TYPE_TRUE);

    if (!failed) {
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF;
        failed |= !s7_install_call_gate(&state, 0u, S7_PARAMETER_COUNT,
            TYPE_TRUE) || !s7_write_call(&state, S7_CALL_GATE_SELECTOR & 0xfffcu);
    }
    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){1u,0u}, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            state.machine->executor_cpu.data.eip != S7_CALL_TARGET ||
            state.machine->executor_cpu.data.cs.selector != 0x0008u ||
            state.machine->executor_cpu.data.ss.selector != 0x0010u ||
            state.machine->executor_cpu.data.sp != S3_STACK_TOP - sizeof(frame) ||
            state.machine->executor_cpu.data.eflags != before.data.eflags ||
            !s3_gate_gprs_same(&before, &state.machine->executor_cpu) ||
            !s3_gate_non_target_sregs_same(&before, &state.machine->executor_cpu) ||
            !s3_gate_read(&state, S3_STACK_TOP - sizeof(frame), frame,
                sizeof(frame)) || frame[0] != 5u || frame[1] != 0x0008u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT s7_reject(type_unsigned_8 condition)
{
    s3_gate_machine state;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    type_unsigned_16 zero = 0u;
    type_unsigned_32 bad_stack = 1u;
    type_unsigned_8 data_access = 0x92u;
    C_INT failed = !s7_prepare_outer(&state, CORE_MACHINE_CPU_PROFILE_80386,
        TYPE_TRUE);

    if (!failed) {
        failed |= !s7_install_call_gate(&state,
            (condition & S7_REJECT_GATE_DPL) ? 0u : 3u, S7_PARAMETER_COUNT,
            TYPE_TRUE) || !s7_write_call(&state, S7_CALL_GATE_SELECTOR);
        if (condition & S7_REJECT_INVALID_TR)
            state.machine->executor_cpu.data.tr.flagValid = TYPE_FALSE;
        if (condition & S7_REJECT_NULL_SS)
            failed |= !s3_gate_write(&state, S7_TSS_BASE + 8u, &zero,
                sizeof(zero));
        if (condition & S7_REJECT_TARGET_CODE)
            failed |= !s3_gate_write(&state, S3_GDT_BASE + 13u, &data_access,
                sizeof(data_access));
        if (condition & S7_REJECT_STACK)
            failed |= !s3_gate_write(&state, S7_TSS_BASE + 4u, &bad_stack,
                sizeof(bad_stack));
        if (condition & S7_REJECT_PARAMETER_SOURCE)
            state.machine->executor_cpu.data.ss.limit = 1u;
    }
    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){1u,0u}, &result) != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !s7_cpu_same(&before, &after);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT s7_outer_preflight_priority(C_VOID)
{
    s3_gate_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    type_unsigned_16 zero = 0u;
    C_INT failed = !s7_prepare_outer(&state, CORE_MACHINE_CPU_PROFILE_80386,
        TYPE_TRUE);

    if (!failed) {
        failed |= !s7_install_call_gate(&state, 3u, 1u, TYPE_TRUE) ||
            !s7_write_call(&state, S7_CALL_GATE_SELECTOR) ||
            !s3_gate_install(&state, 10u, 0x001bu,
                VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, TYPE_TRUE) ||
            !s3_gate_write(&state, S7_TSS_BASE + 8u, &zero, sizeof(zero));
        state.machine->executor_cpu.data.sp = 0x0100u;
        state.machine->executor_cpu.data.ss.limit = 0x0100u;
    }
    if (!failed) {
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){1u,0u}, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
                TYPE_STATUS_OK || diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            diagnostic.delivered_exception_count != 1u ||
            diagnostic.last_delivered_exception.exception_mask != VCPUINS_EXCEPT_TS ||
            diagnostic.last_delivered_exception.exception_code != 0u ||
            state.machine->executor_cpu.data.cs.selector != 0x001bu ||
            state.machine->executor_cpu.data.ss.selector != 0x0023u ||
            state.machine->executor_cpu.data.eip != S3_HANDLER ||
            state.machine->executor_cpu.data.sp != 0x00f8u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT s7_irq_no_shadow(C_VOID)
{
    s3_gate_machine state;
    core_machine_pic_irq_source source;
    core_machine_run_result result;
    type_unsigned_16 parameters[S7_PARAMETER_COUNT] = { 0x1234u,0xabcdu };
    type_unsigned_16 call_frame[4u + S7_PARAMETER_COUNT] = { 0u };
    type_unsigned_16 irq_frame[3u] = { 0u,0u,0u };
    static const type_unsigned_8 code[] = {
        0xfbu,0x9au,S7_CALL_TARGET & 0xffu,S7_CALL_TARGET >> 8u,
        S7_CALL_GATE_SELECTOR & 0xffu,S7_CALL_GATE_SELECTOR >> 8u,0x90u
    };
    static const type_unsigned_8 hlt[] = { 0xf4u };
    C_INT failed = !s7_prepare_outer(&state, CORE_MACHINE_CPU_PROFILE_80386,
        TYPE_TRUE);

    STD_MEMSET(&source, 0, sizeof(source));
    if (!failed) {
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IOPL;
        failed |= !s7_install_call_gate(&state, 3u, S7_PARAMETER_COUNT,
            TYPE_TRUE) || !s3_gate_install(&state, S3_VECTOR, 0x0008u,
            VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, TYPE_TRUE) || !s3_gate_write(
            &state, S3_CODE_BASE, code, sizeof(code)) || !s3_gate_write(&state,
            S3_CODE_BASE + S7_CALL_TARGET, hlt, sizeof(hlt)) || !s3_gate_write(
            &state, S3_STACK_TOP, parameters, sizeof(parameters));
    }
    if (!failed) {
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){1u,0u}, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            state.machine->executor_cpu.data.eip != 1u || !TYPE_GET_BIT(
                state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_IF);
        state.machine->shared_pic_master.data.icw2 = S3_VECTOR;
        core_machine_pic_irq_source_bind(&source, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
    }
    if (!failed) {
        failed |= core_machine_run(state.machine,
            (core_machine_run_budget){2u,0u}, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            state.machine->executor_cpu.data.eip != S7_CALL_TARGET + 1u ||
            state.machine->executor_cpu.data.sp != S7_KERNEL_STACK_TOP - 18u ||
            TYPE_GET_BIT(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_IF) ||
            !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u)) ||
            TYPE_GET_BIT(state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u)) ||
            !s3_gate_read(&state, S7_KERNEL_STACK_TOP - sizeof(call_frame),
                call_frame, sizeof(call_frame)) || call_frame[0] != 6u ||
            call_frame[1] != 0x001bu || call_frame[2] != parameters[0] ||
            call_frame[3] != parameters[1] || call_frame[4] != S3_STACK_TOP ||
            call_frame[5] != 0x0023u || !s3_gate_read(&state,
                S7_KERNEL_STACK_TOP - sizeof(call_frame) - sizeof(irq_frame),
                irq_frame, sizeof(irq_frame)) || irq_frame[0] != S7_CALL_TARGET ||
            irq_frame[1] != 0x0008u || irq_frame[2] !=
            (VCPU_EFLAGS_CF | VCPU_EFLAGS_IF | VCPU_EFLAGS_IOPL);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

C_INT main(C_VOID)
{
    C_INT failed = !s7_outer_success(CORE_MACHINE_CPU_PROFILE_80286,
        TYPE_FALSE) || !s7_outer_success(CORE_MACHINE_CPU_PROFILE_80386,
        TYPE_FALSE) || !s7_outer_success(CORE_MACHINE_CPU_PROFILE_80386,
        TYPE_TRUE) || !s7_same_cpl() || !s7_reject(S7_REJECT_INVALID_TR) ||
        !s7_reject(S7_REJECT_NULL_SS) || !s7_reject(S7_REJECT_GATE_DPL) ||
        !s7_reject(S7_REJECT_TARGET_CODE) || !s7_reject(S7_REJECT_STACK) ||
        !s7_reject(S7_REJECT_PARAMETER_SOURCE) ||
        !s7_outer_preflight_priority() ||
        !s7_irq_no_shadow();

    if (failed) return 1;
    STD_PRINTF("M5:T323:S7:PROTECTED-16-CALL-GATE:OK\n");
    return 0;
}
