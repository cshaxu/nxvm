#include "type.h"

#include "core/machine/pic.h"

/* Reuse S3's private protected-machine setup without extending test support. */
#define main protected_16_gate_s3_retained_main
#include "core_machine_protected_16_gate_s3_smoke.c"
#undef main

#define S6_TSS_BASE 0x0500u
#define S6_USER_SP 0x4000u

static C_INT s6_prepare(s3_gate_machine *state, core_machine_cpu_profile profile,
    type_bool address_prefix)
{
    static const type_unsigned_8 user_data[] = {
        0xffu,0xffu,0,0,0,0xf2u,0,0
    };
    static const type_unsigned_8 iret[] = { 0xcfu };
    static const type_unsigned_8 iret_address[] = { 0x67u,0xcfu };
    static const type_unsigned_8 user_nop[] = { 0x90u };
    t_cpu *cpu;

    if (!s3_gate_prepare(state, profile, TYPE_FALSE,
            VCPU_DESC_SYS_TYPE_INTGATE_16, 0u, TYPE_TRUE) ||
        !s3_gate_write(state, S3_GDT_BASE + 32u, user_data, sizeof(user_data)) ||
        !s3_gate_write(state, S3_CODE_BASE, address_prefix ? iret_address : iret,
            address_prefix ? sizeof(iret_address) : sizeof(iret)) ||
        !s3_gate_write(state, S3_CODE_BASE + 0x10u, user_nop, sizeof(user_nop))) {
        return 0;
    }
    cpu = &state->machine->executor_cpu;
    cpu->data.gdtr.limit = 39u;
    cpu->data.esp = 0x12348000u;
    cpu->data.eflags = VCPU_EFLAGS_CF;
    return 1;
}

static C_INT s6_write_frame(s3_gate_machine *state, type_unsigned_16 cs,
    type_unsigned_16 ss, type_unsigned_16 flags)
{
    const type_unsigned_16 frame[] = { 0x0010u,cs,flags,S6_USER_SP,ss };

    return s3_gate_write(state, S3_STACK_TOP, frame, sizeof(frame));
}

static C_INT s6_success(core_machine_cpu_profile profile, type_bool address_prefix)
{
    s3_gate_machine state;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    C_INT failed = !s6_prepare(&state, profile, address_prefix);

    if (!failed) {
        failed |= !s6_write_frame(&state, 0x001bu, 0x0023u,
            VCPU_EFLAGS_CF | VCPU_EFLAGS_IF | VCPU_EFLAGS_IOPL);
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine, (core_machine_run_budget){1u,0u},
            &result) != TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= after.data.eip != 0x0010u || after.data.cs.selector != 0x001bu ||
            after.data.cs.dpl != 3u || after.data.ss.selector != 0x0023u ||
            after.data.ss.dpl != 3u || after.data.esp != 0x12344000u ||
            after.data.eflags != (VCPU_EFLAGS_CF | VCPU_EFLAGS_IF |
                VCPU_EFLAGS_IOPL) || !s3_gate_gprs_same(&before, &after) ||
            STD_MEMCMP(&before.data.es, &after.data.es, sizeof(before.data.es)) != 0 ||
            STD_MEMCMP(&before.data.ds, &after.data.ds, sizeof(before.data.ds)) != 0 ||
            STD_MEMCMP(&before.data.fs, &after.data.fs, sizeof(before.data.fs)) != 0 ||
            STD_MEMCMP(&before.data.gs, &after.data.gs, sizeof(before.data.gs)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT s6_failure(type_unsigned_16 cs, type_unsigned_16 ss,
    type_bool short_frame)
{
    s3_gate_machine state;
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    type_unsigned_16 frame_before[5u] = {
        0x0010u,cs,VCPU_EFLAGS_CF | VCPU_EFLAGS_IF,S6_USER_SP,ss
    };
    type_unsigned_16 frame_after[5u] = { 0u,0u,0u,0u,0u };
    t_cpu before;
    t_cpu after;
    C_INT failed = !s6_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386,
        TYPE_FALSE);

    if (!failed && short_frame) state.machine->executor_cpu.data.ss.limit = 8u;
    if (!failed) {
        failed |= !s3_gate_write(&state, S3_STACK_TOP, frame_before,
            sizeof(frame_before));
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= core_machine_run(state.machine, (core_machine_run_budget){8u,0u},
            &result) != TYPE_STATUS_FAULT || result.reason != CORE_MACHINE_STOP_FAULT ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK ||
            !diagnostic.first_fault.valid;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !s3_gate_cpu_same(&before, &after) || !s3_gate_read(&state,
            S3_STACK_TOP, frame_after, sizeof(frame_after)) || STD_MEMCMP(
            frame_before, frame_after, sizeof(frame_before)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT s6_iret_irq(C_VOID)
{
    static const type_unsigned_8 loop[] = { 0xebu,0xfeu };
    type_unsigned_8 tss[16u] = { 0u };
    s3_gate_machine state;
    core_machine_pic_irq_source source;
    core_machine_run_result result;
    type_unsigned_16 frame[5u] = { 0u,0u,0u,0u,0u };
    C_INT failed = !s6_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386,
        TYPE_FALSE);

    STD_MEMSET(&source, 0, sizeof(source));
    if (!failed) {
        tss[4u] = 0x00u;
        tss[5u] = 0x70u;
        tss[8u] = 0x10u;
        failed |= !s3_gate_write(&state, S6_TSS_BASE, tss, sizeof(tss)) ||
            !s3_gate_write(&state, S3_CODE_BASE + S3_HANDLER, loop,
                sizeof(loop));
        state.machine->executor_cpu.data.tr.flagValid = TYPE_TRUE;
        state.machine->executor_cpu.data.tr.selector = 0x0028u;
        state.machine->executor_cpu.data.tr.sregtype = SREG_TR;
        state.machine->executor_cpu.data.tr.base = S6_TSS_BASE;
        state.machine->executor_cpu.data.tr.limit = sizeof(tss) - 1u;
        state.machine->executor_cpu.data.tr.dpl = 0u;
        state.machine->executor_cpu.data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
        state.machine->shared_pic_master.data.icw2 = S3_VECTOR;
        core_machine_pic_irq_source_bind(&source, &state.machine->shared_pic_master,
            &state.machine->shared_pic_slave, 0u);
        core_machine_pic_irq_source_assert(&source);
        core_machine_pic_irq_source_deassert(&source);
        failed |= !s6_write_frame(&state, 0x001bu, 0x0023u,
            VCPU_EFLAGS_CF | VCPU_EFLAGS_IF | VCPU_EFLAGS_IOPL) ||
            core_machine_run(state.machine, (core_machine_run_budget){1u,0u},
                &result) != TYPE_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
            state.machine->executor_cpu.data.cs.selector != 0x0008u ||
            state.machine->executor_cpu.data.ss.selector != 0x0010u ||
            state.machine->executor_cpu.data.eip != S3_HANDLER ||
            state.machine->executor_cpu.data.esp != 0x12346ff6u ||
            TYPE_GET_BIT(state.machine->executor_cpu.data.eflags, VCPU_EFLAGS_IF) ||
            !TYPE_GET_BIT(state.machine->shared_pic_master.data.isr, VPIC_ISR_IRQ(0u)) ||
            TYPE_GET_BIT(state.machine->shared_pic_master.data.irr, VPIC_IRR_IRQ(0u)) ||
            !s3_gate_read(&state, 0x00006ff6u, frame, sizeof(frame)) ||
            frame[0] != 0x0010u || frame[1] != 0x001bu ||
            frame[2] != (VCPU_EFLAGS_CF | VCPU_EFLAGS_IF | VCPU_EFLAGS_IOPL) ||
            frame[3] != S6_USER_SP || frame[4] != 0x0023u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

C_INT main(C_VOID)
{
    C_INT failed = !s6_success(CORE_MACHINE_CPU_PROFILE_80286, TYPE_FALSE) ||
        !s6_success(CORE_MACHINE_CPU_PROFILE_80386, TYPE_FALSE) ||
        !s6_success(CORE_MACHINE_CPU_PROFILE_80386, TYPE_TRUE) ||
        !s6_failure(0u, 0x0023u, TYPE_FALSE) ||
        !s6_failure(0x001bu, 0u, TYPE_FALSE) ||
        !s6_failure(0x001bu, 0x0023u, TYPE_TRUE) || !s6_iret_irq();

    if (failed) return 1;
    STD_PRINTF("M5:T323:S6:PROTECTED-16-OUTER-IRET:OK\n");
    return 0;
}
