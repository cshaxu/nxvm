#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define CG_GDT_BASE 0x0300u
#define CG_IDT_BASE 0x0400u
#define CG_TSS_BASE 0x0600u
#define CG_KERNEL_CODE_BASE 0x2000u
#define CG_USER_CODE_BASE 0x3000u
#define CG_HANDLER_OFFSET 0x0100u
#define CG_GATE_SELECTOR 0x0033u

typedef struct call_gate_privilege_machine {
    core_machine *machine;
} call_gate_privilege_machine;

typedef enum call_gate_target_failure {
    CALL_GATE_TARGET_CODE_TYPE,
    CALL_GATE_TARGET_CODE_NOT_PRESENT,
    CALL_GATE_TARGET_STACK_TYPE,
    CALL_GATE_TARGET_STACK_NOT_PRESENT,
    CALL_GATE_TARGET_STACK_SELECTOR,
    CALL_GATE_TARGET_STACK_BOUNDARY
} call_gate_target_failure;

typedef enum call_gate_ts_delivery_failure {
    CALL_GATE_TS_DELIVERY_INVALID_GATE,
    CALL_GATE_TS_DELIVERY_NONPRESENT_GATE,
    CALL_GATE_TS_DELIVERY_STACK_LIMIT
} call_gate_ts_delivery_failure;

typedef enum call_gate_outer_delivery_failure {
    CALL_GATE_OUTER_DELIVERY_INVALID_GATE,
    CALL_GATE_OUTER_DELIVERY_NONPRESENT_GATE,
    CALL_GATE_OUTER_DELIVERY_TARGET_NOT_PRESENT,
    CALL_GATE_OUTER_DELIVERY_STACK_LIMIT
} call_gate_outer_delivery_failure;

static C_VOID cg_reset(C_VOID *opaque)
{
    call_gate_privilege_machine *state = (call_gate_privilege_machine *)opaque;

    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider cg_provider = {
    cg_reset, STD_NULL
};

static C_INT cg_write(call_gate_privilege_machine *state, type_unsigned_32 address,
    const C_VOID *data, STD_SIZE_T bytes)
{
    return state != STD_NULL && state->machine != STD_NULL &&
        core_machine_memory_write(state->machine, address, data, bytes) ==
            TYPE_STATUS_OK;
}

static C_INT cg_read(call_gate_privilege_machine *state, type_unsigned_32 address,
    C_VOID *data, STD_SIZE_T bytes)
{
    return state != STD_NULL && state->machine != STD_NULL &&
        core_machine_memory_read_physical(&state->machine->executor_memory,
            address, (type_virtual_address)data, bytes) == TYPE_STATUS_OK;
}

static C_INT cg_prepare(call_gate_privilege_machine *state, type_unsigned_8 gate_access,
    type_unsigned_8 parameter_count)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0x40u,0,
        0xffu,0xffu,0,0,0,0x92u,0xcfu,0,
        0xffu,0xffu,0,0x30u,0,0xfau,0x40u,0,
        0xffu,0xffu,0,0,0,0xf2u,0xcfu,0,
        0x67u,0,0,0x06u,0,0x8bu,0,0,
        0,0x01u,0x08u,0,parameter_count,gate_access,0,0
    };
    type_unsigned_8 tss[10] = {0};
    static const type_unsigned_8 call[] = {0x9au,0,0x01u,0,0,0x33u,0};
    static const type_unsigned_8 handler[] = {0xf4u};
    type_unsigned_32 esp0 = 0x00009000u;
    type_unsigned_16 ss0 = 0x0010u;
    t_cpu *cpu;

    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    STD_MEMCPY(&tss[4], &esp0, sizeof(esp0));
    STD_MEMCPY(&tss[8], &ss0, sizeof(ss0));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK ||
        !test_core_machine_fixture_bind_freeze_reset(state->machine,
            &cg_provider, state) ||
        !cg_write(state, CG_GDT_BASE, gdt, sizeof(gdt)) ||
        !cg_write(state, CG_TSS_BASE, tss, sizeof(tss)) ||
        !cg_write(state, CG_USER_CODE_BASE, call, sizeof(call)) ||
        !cg_write(state, CG_KERNEL_CODE_BASE + CG_HANDLER_OFFSET, handler,
            sizeof(handler))) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    cpu = &state->machine->executor_cpu;
    cpu->data.cr0 = VCPU_CR0_PE;
    cpu->data.gdtr.flagValid = TYPE_TRUE;
    cpu->data.gdtr.sregtype = SREG_GDTR;
    cpu->data.gdtr.base = CG_GDT_BASE;
    cpu->data.gdtr.limit = sizeof(gdt) - 1u;
    cpu->data.cs.flagValid = TYPE_TRUE;
    cpu->data.cs.selector = 0x001bu;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.base = CG_USER_CODE_BASE;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.dpl = 3u;
    cpu->data.cs.seg.executable = TYPE_TRUE;
    cpu->data.cs.seg.exec.defsize = TYPE_TRUE;
    cpu->data.cs.seg.exec.conform = TYPE_FALSE;
    cpu->data.cs.seg.exec.readable = TYPE_TRUE;
    cpu->data.ss.flagValid = TYPE_TRUE;
    cpu->data.ss.selector = 0x0023u;
    cpu->data.ss.sregtype = SREG_STACK;
    cpu->data.ss.base = 0u;
    cpu->data.ss.limit = 0xffffffffu;
    cpu->data.ss.dpl = 3u;
    cpu->data.ss.seg.data.big = TYPE_TRUE;
    cpu->data.ss.seg.data.expdown = TYPE_FALSE;
    cpu->data.ss.seg.data.writable = TYPE_TRUE;
    cpu->data.tr.flagValid = TYPE_TRUE;
    cpu->data.tr.selector = 0x0028u;
    cpu->data.tr.sregtype = SREG_TR;
    cpu->data.tr.base = CG_TSS_BASE;
    cpu->data.tr.limit = 0x67u;
    cpu->data.tr.dpl = 0u;
    cpu->data.tr.sys.type = VCPU_DESC_SYS_TYPE_TSS_32_BUSY;
    cpu->data.eip = 0u;
    cpu->data.esp = 0x00008800u;
    cpu->data.eflags = 0x00000202u;
    cpu->data.flagHalt = TYPE_FALSE;
    return 1;
}

static C_INT cg_run(call_gate_privilege_machine *state, C_INT expect_fault,
    t_cpu *out_cpu, core_machine_cpu_diagnostic *out_diagnostic)
{
    const core_machine_run_budget budget = {32u, 0u};
    core_machine_run_result result;
    type_status status = core_machine_run(state->machine, budget, &result);

    if (core_machine_get_cpu_diagnostic(state->machine, out_diagnostic) !=
        TYPE_STATUS_OK) return 0;
    *out_cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return status == (expect_fault ? TYPE_STATUS_FAULT : TYPE_STATUS_OK) &&
        result.reason == (expect_fault ? CORE_MACHINE_STOP_FAULT :
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT);
}

static C_INT cg_run_budget(call_gate_privilege_machine *state, t_cpu *out_cpu,
    core_machine_cpu_diagnostic *out_diagnostic)
{
    const core_machine_run_budget budget = {32u, 0u};
    core_machine_run_result result;
    type_status status = core_machine_run(state->machine, budget, &result);

    if (core_machine_get_cpu_diagnostic(state->machine, out_diagnostic) !=
        TYPE_STATUS_OK) return 0;
    *out_cpu = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return status == TYPE_STATUS_OK && result.reason == CORE_MACHINE_STOP_BUDGET;
}

static C_INT cg_fault_is(const core_machine_cpu_diagnostic *diagnostic,
    type_unsigned_32 mask, type_unsigned_32 code)
{
    return diagnostic->first_fault.valid && TYPE_GET_BIT(
        diagnostic->first_fault.exception_mask, mask) &&
        diagnostic->first_fault.exception_code == code;
}

static C_INT cg_entry_state_equal(const t_cpu *before, const t_cpu *after)
{
    return before->data.eip == after->data.eip &&
        before->data.esp == after->data.esp &&
        before->data.eflags == after->data.eflags &&
        STD_MEMCMP(&before->data.cs, &after->data.cs,
            sizeof(before->data.cs)) == 0 && STD_MEMCMP(&before->data.ss,
            &after->data.ss, sizeof(before->data.ss)) == 0;
}

static C_INT cg_install_ts_delivery_gate(call_gate_privilege_machine *state,
    type_unsigned_8 access)
{
    type_unsigned_8 gate[8] = {0};
    t_cpu *cpu;

    if (state == STD_NULL || state->machine == STD_NULL) return 0;
    gate[0] = CG_HANDLER_OFFSET & 0xffu;
    gate[1] = CG_HANDLER_OFFSET >> 8u;
    gate[2] = 0x1bu;
    gate[5] = access;
    cpu = &state->machine->executor_cpu;
    cpu->data.idtr.flagValid = TYPE_TRUE;
    cpu->data.idtr.sregtype = SREG_IDTR;
    cpu->data.idtr.base = CG_IDT_BASE;
    cpu->data.idtr.limit = 10u * 8u + 7u;
    return cg_write(state, CG_IDT_BASE + 10u * 8u, gate, sizeof(gate)) &&
        cg_write(state, CG_USER_CODE_BASE + CG_HANDLER_OFFSET,
            (const type_unsigned_8[]){0xebu,0xfeu}, 2u);
}

static C_INT cg_prepare_ts_delivery(call_gate_privilege_machine *state,
    type_unsigned_8 gate_access, type_unsigned_8 parameter_count)
{
    type_unsigned_16 invalid_ss0 = 0x0013u;

    return cg_prepare(state, 0xecu, parameter_count) &&
        cg_write(state, CG_TSS_BASE + 8u, &invalid_ss0, sizeof(invalid_ss0)) &&
        cg_install_ts_delivery_gate(state, gate_access);
}

static C_INT cg_install_outer_error_gate(call_gate_privilege_machine *state,
    type_unsigned_8 access)
{
    type_unsigned_8 gate[8] = {0};
    t_cpu *cpu;

    if (state == STD_NULL || state->machine == STD_NULL) return 0;
    gate[0] = CG_HANDLER_OFFSET & 0xffu;
    gate[1] = CG_HANDLER_OFFSET >> 8u;
    gate[2] = 0x08u;
    gate[5] = access;
    cpu = &state->machine->executor_cpu;
    cpu->data.idtr.flagValid = TYPE_TRUE;
    cpu->data.idtr.sregtype = SREG_IDTR;
    cpu->data.idtr.base = CG_IDT_BASE;
    cpu->data.idtr.limit = 13u * 8u + 7u;
    return cg_write(state, CG_IDT_BASE + 13u * 8u, gate, sizeof(gate));
}

static C_INT cg_install_double_fault_gate(call_gate_privilege_machine *state,
    type_unsigned_8 access)
{
    type_unsigned_8 gate[8] = {0};

    if (state == STD_NULL || state->machine == STD_NULL) return 0;
    gate[0] = CG_HANDLER_OFFSET & 0xffu;
    gate[1] = CG_HANDLER_OFFSET >> 8u;
    gate[2] = 0x08u;
    gate[5] = access;
    return cg_write(state, CG_IDT_BASE + 8u * 8u, gate, sizeof(gate));
}

static C_INT cg_test_success(type_unsigned_8 count)
{
    call_gate_privilege_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu after;
    type_unsigned_32 frame[6] = {0u,0u,0u,0u,0u,0u};
    type_unsigned_32 parameters[2] = {0x11223344u,0x55667788u};
    C_INT failed = !cg_prepare(&state, 0xecu, count);

    if (!failed && count) failed |= !cg_write(&state, 0x00008800u,
        parameters, (STD_SIZE_T)count * sizeof(parameters[0]));
    if (!failed) {
        failed |= !cg_run(&state, 0, &after, &diagnostic) ||
            diagnostic.first_fault.valid || after.data.cs.selector != 0x0008u ||
            after.data.cs.dpl != 0u || after.data.eip != CG_HANDLER_OFFSET + 1u ||
            after.data.ss.selector != 0x0010u || after.data.esp !=
                0x00009000u - (type_unsigned_32)(4u + count) * 4u ||
            !cg_read(&state, after.data.esp, frame,
                (STD_SIZE_T)(4u + count) * sizeof(frame[0])) ||
            frame[0] != 7u || frame[1] != 0x0000001bu ||
            frame[2 + count] != 0x00008800u ||
            frame[3 + count] != 0x00000023u ||
            (count > 0u && frame[2] != parameters[0]) ||
            (count > 1u && frame[3] != parameters[1]);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT cg_test_dpl_failure_atomic(C_VOID)
{
    call_gate_privilege_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 cs_before = 0u, cs_after = 0u;
    type_unsigned_8 ss_before = 0u, ss_after = 0u;
    C_INT failed = !cg_prepare(&state, 0x8cu, 0u);

    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !cg_read(&state, CG_GDT_BASE + 13u, &cs_before,
                sizeof(cs_before)) || !cg_read(&state, CG_GDT_BASE + 21u,
                &ss_before, sizeof(ss_before)) || !cg_run(&state, 1, &after,
                &diagnostic) || !cg_fault_is(&diagnostic, VCPUINS_EXCEPT_DF,
                0u) || !cg_read(&state, CG_GDT_BASE + 13u,
                &cs_after, sizeof(cs_after)) || !cg_read(&state, CG_GDT_BASE + 21u,
                &ss_after, sizeof(ss_after)) || after.data.eip != before.data.eip ||
            after.data.esp != before.data.esp || after.data.cs.selector !=
                before.data.cs.selector || after.data.ss.selector !=
                before.data.ss.selector || cs_after != cs_before || ss_after != ss_before ||
                !state.machine->executor_cpu_execution.shutdown_requested;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT cg_test_gate_failure_atomic(type_unsigned_8 gate_access, type_unsigned_32 mask,
    type_unsigned_32 code)
{
    call_gate_privilege_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 cs_before = 0u, cs_after = 0u;
    type_unsigned_8 ss_before = 0u, ss_after = 0u;
    C_INT failed = !cg_prepare(&state, gate_access, 0u);

    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !cg_read(&state, CG_GDT_BASE + 13u, &cs_before,
                sizeof(cs_before)) || !cg_read(&state, CG_GDT_BASE + 21u,
                &ss_before, sizeof(ss_before)) || !cg_run(&state, 1, &after,
                &diagnostic) || !cg_fault_is(&diagnostic, mask,
                code) || !cg_read(&state, CG_GDT_BASE + 13u,
                &cs_after, sizeof(cs_after)) || !cg_read(&state, CG_GDT_BASE + 21u,
                &ss_after, sizeof(ss_after)) || after.data.eip != before.data.eip ||
            after.data.esp != before.data.esp || after.data.eflags !=
                before.data.eflags || after.data.cs.selector != before.data.cs.selector ||
            after.data.ss.selector != before.data.ss.selector || cs_after != cs_before ||
            ss_after != ss_before;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT cg_test_parameter_source_failure_atomic(C_VOID)
{
    call_gate_privilege_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 cs_before = 0u, cs_after = 0u;
    type_unsigned_8 ss_before = 0u, ss_after = 0u;
    C_INT failed = !cg_prepare(&state, 0xecu, 1u);

    if (!failed) {
        state.machine->executor_cpu.data.ss.limit = 0x00000100u;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !cg_read(&state, CG_GDT_BASE + 13u, &cs_before,
                sizeof(cs_before)) || !cg_read(&state, CG_GDT_BASE + 21u,
                &ss_before, sizeof(ss_before)) || !cg_run(&state, 1, &after,
            &diagnostic) || !cg_fault_is(&diagnostic, VCPUINS_EXCEPT_DF, 0u) ||
            !cg_read(&state, CG_GDT_BASE + 13u, &cs_after, sizeof(cs_after)) ||
            !cg_read(&state, CG_GDT_BASE + 21u, &ss_after, sizeof(ss_after)) ||
            after.data.eip != before.data.eip || after.data.esp != before.data.esp ||
            after.data.eflags != before.data.eflags || after.data.cs.selector !=
                before.data.cs.selector || after.data.ss.selector !=
                before.data.ss.selector || cs_after != cs_before || ss_after != ss_before ||
                !state.machine->executor_cpu_execution.shutdown_requested;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT cg_test_target_failure_atomic(call_gate_target_failure failure,
    type_unsigned_32 mask, type_unsigned_32 code)
{
    call_gate_privilege_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 cs_before = 0u, cs_after = 0u;
    type_unsigned_8 ss_before = 0u, ss_after = 0u;
    type_unsigned_8 access = 0u;
    type_unsigned_16 selector = 0u;
    const core_machine_run_budget budget = {1u, 0u};
    core_machine_run_result result;
    type_status status;
    C_INT failed = !cg_prepare(&state, 0xecu, 0u);

    if (!failed) {
        switch (failure) {
        case CALL_GATE_TARGET_CODE_TYPE:
            access = 0x92u;
            failed |= !cg_write(&state, CG_GDT_BASE + 13u, &access,
                sizeof(access));
            break;
        case CALL_GATE_TARGET_CODE_NOT_PRESENT:
            access = 0x1au;
            failed |= !cg_write(&state, CG_GDT_BASE + 13u, &access,
                sizeof(access));
            break;
        case CALL_GATE_TARGET_STACK_TYPE:
            access = 0x9au;
            failed |= !cg_write(&state, CG_GDT_BASE + 21u, &access,
                sizeof(access));
            break;
        case CALL_GATE_TARGET_STACK_NOT_PRESENT:
            access = 0x12u;
            failed |= !cg_write(&state, CG_GDT_BASE + 21u, &access,
                sizeof(access));
            break;
        case CALL_GATE_TARGET_STACK_SELECTOR:
            selector = 0x0013u;
            failed |= !cg_write(&state, CG_TSS_BASE + 8u, &selector,
                sizeof(selector));
            break;
        case CALL_GATE_TARGET_STACK_BOUNDARY:
            selector = 0x000fu;
            failed |= !cg_write(&state, CG_GDT_BASE + 16u, &selector,
                sizeof(selector));
            access = 0x40u;
            failed |= !cg_write(&state, CG_GDT_BASE + 22u, &access,
                sizeof(access));
            break;
        default:
            return 0;
        }
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !cg_read(&state, CG_GDT_BASE + 13u, &cs_before,
            sizeof(cs_before)) || !cg_read(&state, CG_GDT_BASE + 21u,
            &ss_before, sizeof(ss_before));
        status = core_machine_run(state.machine, budget, &result);
        if (core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK) failed = 1;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= status != TYPE_STATUS_FAULT || result.reason !=
            CORE_MACHINE_STOP_FAULT || !cg_fault_is(&diagnostic,
                mask, code) ||
            !cg_read(&state, CG_GDT_BASE + 13u, &cs_after, sizeof(cs_after)) ||
            !cg_read(&state, CG_GDT_BASE + 21u, &ss_after, sizeof(ss_after)) ||
            !cg_entry_state_equal(&before, &after) || cs_after != cs_before ||
            ss_after != ss_before;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT cg_test_ts_delivery(C_VOID)
{
    call_gate_privilege_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu after;
    type_unsigned_32 frame[4] = {0u,0u,0u,0u};
    C_INT failed = !cg_prepare_ts_delivery(&state, 0xeeu, 0u);

    if (!failed) {
        failed |= !cg_run_budget(&state, &after, &diagnostic) ||
            diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            diagnostic.delivered_exception_count != 1u ||
            !TYPE_GET_BIT(diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_TS) ||
            diagnostic.last_delivered_exception.exception_code != 0x0010u ||
            after.data.cs.selector != 0x001bu ||
            after.data.ss.selector != 0x0023u ||
            after.data.eip != CG_HANDLER_OFFSET ||
            after.data.esp != 0x000087f0u ||
            !cg_read(&state, after.data.esp, frame, sizeof(frame)) ||
            frame[0] != 0x0010u || frame[1] != 0u ||
            frame[2] != 0x0000001bu || frame[3] != 0x00000202u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT cg_test_outer_preflight_priority(C_VOID)
{
    call_gate_privilege_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu after;
    C_INT failed = !cg_prepare_ts_delivery(&state, 0xeeu, 1u);

    if (!failed) {
        state.machine->executor_cpu.data.esp = 0x00000100u;
        state.machine->executor_cpu.data.ss.limit = 0x00000100u;
        failed |= !cg_run_budget(&state, &after, &diagnostic) ||
            diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            diagnostic.delivered_exception_count != 1u ||
            !TYPE_GET_BIT(diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_TS) ||
            diagnostic.last_delivered_exception.exception_code != 0x0010u ||
            after.data.cs.selector != 0x001bu ||
            after.data.ss.selector != 0x0023u ||
            after.data.eip != CG_HANDLER_OFFSET || after.data.esp != 0x000000f0u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT cg_test_ts_delivery_failure(call_gate_ts_delivery_failure failure)
{
    call_gate_privilege_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 access_before = 0u;
    type_unsigned_8 access_after = 0u;
    type_unsigned_32 stack_before[4] = {0u,0u,0u,0u};
    type_unsigned_32 stack_after[4] = {0u,0u,0u,0u};
    type_unsigned_8 gate_access = 0xeeu;
    C_INT failed;

    if (failure == CALL_GATE_TS_DELIVERY_INVALID_GATE) gate_access = 0x80u;
    if (failure == CALL_GATE_TS_DELIVERY_NONPRESENT_GATE) gate_access = 0x6eu;
    failed = !cg_prepare_ts_delivery(&state, gate_access, 0u);
    if (!failed && failure == CALL_GATE_TS_DELIVERY_STACK_LIMIT)
        state.machine->executor_cpu.data.ss.limit = 0x000087feu;
    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !cg_read(&state, CG_GDT_BASE + 29u, &access_before,
            sizeof(access_before)) || !cg_read(&state, 0x000087f0u,
            stack_before, sizeof(stack_before)) || !cg_run(&state, 1, &after,
            &diagnostic) || !cg_fault_is(&diagnostic, VCPUINS_EXCEPT_DF,
            0u) || diagnostic.last_delivered_exception.valid ||
            diagnostic.delivered_exception_count != 0u ||
            !cg_read(&state, CG_GDT_BASE + 29u, &access_after,
                sizeof(access_after)) || !cg_read(&state, 0x000087f0u,
                stack_after, sizeof(stack_after)) ||
            !cg_entry_state_equal(&before, &after) ||
            access_after != access_before || STD_MEMCMP(stack_before, stack_after,
                sizeof(stack_before)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT cg_test_outer_gp_delivery(C_VOID)
{
    call_gate_privilege_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu after;
    type_unsigned_32 frame[6] = {0u,0u,0u,0u,0u,0u};
    type_unsigned_8 cs_access = 0u;
    type_unsigned_8 ss_access = 0u;
    C_INT failed = !cg_prepare(&state, 0x8cu, 0u);

    if (!failed) {
        failed |= !cg_install_outer_error_gate(&state,
                (type_unsigned_8)(0x80u | VCPU_DESC_SYS_TYPE_INTGATE_32)) ||
            !cg_run(&state, 0, &after, &diagnostic) ||
            diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            diagnostic.delivered_exception_count != 1u ||
            !TYPE_GET_BIT(diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_GP) ||
            diagnostic.last_delivered_exception.exception_code != 0x0030u ||
            after.data.cs.selector != 0x0008u ||
            after.data.ss.selector != 0x0010u ||
            after.data.eip != CG_HANDLER_OFFSET + 1u ||
            after.data.esp != 0x00008fe8u ||
            !cg_read(&state, after.data.esp, frame, sizeof(frame)) ||
            frame[0] != 0x0030u || frame[1] != 0u ||
            frame[2] != 0x0000001bu || frame[3] != 0x00000202u ||
            frame[4] != 0x00008800u || frame[5] != 0x00000023u ||
            !cg_read(&state, CG_GDT_BASE + 13u, &cs_access,
                sizeof(cs_access)) || !cg_read(&state, CG_GDT_BASE + 21u,
                &ss_access, sizeof(ss_access)) || cs_access != 0x9bu ||
            ss_access != 0x93u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT cg_test_outer_gp_delivery_failure(
    call_gate_outer_delivery_failure failure)
{
    call_gate_privilege_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 cs_before = 0u;
    type_unsigned_8 cs_after = 0u;
    type_unsigned_8 ss_before = 0u;
    type_unsigned_8 ss_after = 0u;
    type_unsigned_32 stack_before[6] = {0u,0u,0u,0u,0u,0u};
    type_unsigned_32 stack_after[6] = {0u,0u,0u,0u,0u,0u};
    type_unsigned_8 gate_access = (type_unsigned_8)(0x80u | VCPU_DESC_SYS_TYPE_INTGATE_32);
    C_INT failed = !cg_prepare(&state, 0x8cu, 0u);

    if (failure == CALL_GATE_OUTER_DELIVERY_INVALID_GATE) gate_access = 0x80u;
    if (failure == CALL_GATE_OUTER_DELIVERY_NONPRESENT_GATE)
        gate_access = VCPU_DESC_SYS_TYPE_INTGATE_32;
    if (!failed) {
        failed |= !cg_install_outer_error_gate(&state, gate_access);
        if (!failed && failure == CALL_GATE_OUTER_DELIVERY_TARGET_NOT_PRESENT) {
            type_unsigned_8 access = 0x1au;
            failed |= !cg_write(&state, CG_GDT_BASE + 13u, &access,
                sizeof(access));
        }
        if (!failed && failure == CALL_GATE_OUTER_DELIVERY_STACK_LIMIT) {
            type_unsigned_8 limit[] = {0u,0u};
            type_unsigned_8 granularity = 0x40u;
            failed |= !cg_write(&state, CG_GDT_BASE + 16u, limit,
                sizeof(limit)) || !cg_write(&state, CG_GDT_BASE + 22u,
                &granularity, sizeof(granularity));
        }
    }
    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !cg_read(&state, CG_GDT_BASE + 13u, &cs_before,
            sizeof(cs_before)) || !cg_read(&state, CG_GDT_BASE + 21u,
            &ss_before, sizeof(ss_before)) || !cg_read(&state, 0x00008fe8u,
            stack_before, sizeof(stack_before)) || !cg_run(&state, 1, &after,
            &diagnostic) || !cg_fault_is(&diagnostic, VCPUINS_EXCEPT_DF,
            0u) || diagnostic.last_delivered_exception.valid ||
            diagnostic.delivered_exception_count != 0u || !cg_read(&state,
                CG_GDT_BASE + 13u, &cs_after, sizeof(cs_after)) || !cg_read(&state,
                CG_GDT_BASE + 21u, &ss_after, sizeof(ss_after)) || !cg_read(&state,
                0x00008fe8u, stack_after, sizeof(stack_after)) ||
            !cg_entry_state_equal(&before, &after) || cs_after != cs_before ||
            ss_after != ss_before || STD_MEMCMP(stack_before, stack_after,
                sizeof(stack_before)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT cg_test_outer_gp_double_fault(C_INT double_fault_gate_valid)
{
    call_gate_privilege_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_32 frame[6] = {0u,0u,0u,0u,0u,0u};
    type_unsigned_32 frame_before[6] = {0u,0u,0u,0u,0u,0u};
    type_unsigned_32 frame_after[6] = {0u,0u,0u,0u,0u,0u};
    type_unsigned_8 cs_before = 0u;
    type_unsigned_8 cs_after = 0u;
    type_unsigned_8 ss_before = 0u;
    type_unsigned_8 ss_after = 0u;
    C_INT failed = !cg_prepare(&state, 0x8cu, 0u);

    if (!failed) {
        failed |= !cg_install_outer_error_gate(&state, 0x80u) ||
            !cg_install_double_fault_gate(&state, double_fault_gate_valid ?
                (type_unsigned_8)(0x80u | VCPU_DESC_SYS_TYPE_INTGATE_32) : 0x80u);
    }
    if (!failed && double_fault_gate_valid) {
        failed |= !cg_run(&state, 0, &after, &diagnostic) ||
            diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid ||
            diagnostic.delivered_exception_count != 1u ||
            !TYPE_GET_BIT(diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_DF) ||
            diagnostic.last_delivered_exception.exception_code != 0u ||
            after.data.cs.selector != 0x0008u ||
            after.data.ss.selector != 0x0010u ||
            after.data.eip != CG_HANDLER_OFFSET + 1u ||
            after.data.esp != 0x00008fe8u ||
            !cg_read(&state, after.data.esp, frame, sizeof(frame)) ||
            frame[0] != 0u || frame[1] != 0u || frame[2] != 0x0000001bu ||
            frame[3] != 0x00000202u || frame[4] != 0x00008800u ||
            frame[5] != 0x00000023u;
    }
    if (!failed && !double_fault_gate_valid) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !cg_read(&state, CG_GDT_BASE + 13u, &cs_before,
            sizeof(cs_before)) || !cg_read(&state, CG_GDT_BASE + 21u,
            &ss_before, sizeof(ss_before)) || !cg_read(&state, 0x00008fe8u,
            frame_before, sizeof(frame_before)) || !cg_run(&state, 1, &after,
            &diagnostic) || !cg_fault_is(&diagnostic, VCPUINS_EXCEPT_DF, 0u) ||
            diagnostic.last_delivered_exception.valid ||
            diagnostic.delivered_exception_count != 0u || !cg_read(&state,
                CG_GDT_BASE + 13u, &cs_after, sizeof(cs_after)) || !cg_read(&state,
                CG_GDT_BASE + 21u, &ss_after, sizeof(ss_after)) || !cg_read(&state,
                0x00008fe8u, frame_after, sizeof(frame_after)) ||
            !cg_entry_state_equal(&before, &after) || cs_after != cs_before ||
            ss_after != ss_before || STD_MEMCMP(frame_before, frame_after,
                sizeof(frame_before)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT cg_test_32_same_cpl_without_tss(C_VOID)
{
    call_gate_privilege_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_32 frame[3] = {0u, 0u, 0u};
    const type_unsigned_32 sentinel[3] = {0x11223344u, 0x55667788u,
        0x99aabbccu};
    static const type_unsigned_8 call[] = {0x9au,0u,0u,0u,0u,0x33u,0u};
    t_cpu *cpu;
    C_INT failed = !cg_prepare(&state, 0xecu, 3u);

    if (!failed) {
        cpu = &state.machine->executor_cpu;
        cpu->data.cs.selector = 0x0008u;
        cpu->data.cs.base = CG_KERNEL_CODE_BASE;
        cpu->data.cs.dpl = 0u;
        cpu->data.ss.selector = 0x0010u;
        cpu->data.ss.dpl = 0u;
        cpu->data.tr.flagValid = TYPE_FALSE;
        cpu->data.eip = 0u;
        cpu->data.esp = 0x00008800u;
        failed |= !cg_write(&state, CG_KERNEL_CODE_BASE, call, sizeof(call)) ||
            !cg_write(&state, 0x000087f8u, sentinel, sizeof(sentinel));
    }
    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !cg_run(&state, 0, &after, &diagnostic) ||
            diagnostic.first_fault.valid || diagnostic.last_delivered_exception.valid ||
            after.data.cs.selector != 0x0008u ||
            after.data.ss.selector != 0x0010u ||
            after.data.eip != CG_HANDLER_OFFSET + 1u ||
            after.data.esp != 0x000087f8u ||
            after.data.eflags != before.data.eflags ||
            after.data.eax != before.data.eax ||
            after.data.ecx != before.data.ecx ||
            after.data.edx != before.data.edx ||
            after.data.ebx != before.data.ebx ||
            after.data.ebp != before.data.ebp ||
            after.data.esi != before.data.esi ||
            after.data.edi != before.data.edi ||
            !cg_read(&state, after.data.esp, frame, sizeof(frame)) ||
            frame[0] != 7u || frame[1] != 8u || frame[2] != sentinel[2];
    }
    core_machine_destroy(state.machine);
    return !failed;
}

int main(void)
{
    C_INT failed = !cg_test_success(0u) || !cg_test_success(2u) ||
        !cg_test_dpl_failure_atomic() || !cg_test_gate_failure_atomic(0x6cu,
            VCPUINS_EXCEPT_DF, 0u) ||
        !cg_test_gate_failure_atomic(0xeeu, VCPUINS_EXCEPT_DF,
            0u) || !cg_test_parameter_source_failure_atomic() ||
        !cg_test_target_failure_atomic(CALL_GATE_TARGET_CODE_TYPE,
            VCPUINS_EXCEPT_DF, 0u) ||
        !cg_test_target_failure_atomic(CALL_GATE_TARGET_CODE_NOT_PRESENT,
            VCPUINS_EXCEPT_DF, 0u) ||
        !cg_test_target_failure_atomic(CALL_GATE_TARGET_STACK_TYPE,
            VCPUINS_EXCEPT_DF, 0u) ||
        !cg_test_target_failure_atomic(CALL_GATE_TARGET_STACK_NOT_PRESENT,
            VCPUINS_EXCEPT_DF, 0u) ||
        !cg_test_target_failure_atomic(CALL_GATE_TARGET_STACK_SELECTOR,
            VCPUINS_EXCEPT_DF, 0u) ||
        !cg_test_target_failure_atomic(CALL_GATE_TARGET_STACK_BOUNDARY,
            VCPUINS_EXCEPT_DF, 0u) || !cg_test_ts_delivery() ||
        !cg_test_outer_preflight_priority() ||
        !cg_test_ts_delivery_failure(CALL_GATE_TS_DELIVERY_INVALID_GATE) ||
        !cg_test_ts_delivery_failure(CALL_GATE_TS_DELIVERY_NONPRESENT_GATE) ||
        !cg_test_ts_delivery_failure(CALL_GATE_TS_DELIVERY_STACK_LIMIT) ||
        !cg_test_outer_gp_delivery() || !cg_test_outer_gp_delivery_failure(
            CALL_GATE_OUTER_DELIVERY_INVALID_GATE) ||
        !cg_test_outer_gp_delivery_failure(
            CALL_GATE_OUTER_DELIVERY_NONPRESENT_GATE) ||
        !cg_test_outer_gp_delivery_failure(
            CALL_GATE_OUTER_DELIVERY_TARGET_NOT_PRESENT) ||
        !cg_test_outer_gp_delivery_failure(
            CALL_GATE_OUTER_DELIVERY_STACK_LIMIT) ||
        !cg_test_outer_gp_double_fault(1) || !cg_test_outer_gp_double_fault(0) ||
        !cg_test_32_same_cpl_without_tss();

    if (failed) return 1;
    STD_PRINTF("M5:T307:CALL-GATE-PRIVILEGE-ENTRY:OK\n");
    STD_PRINTF("M5:T308:S3:SAME-CPL-TS-DELIVERY:OK\n");
    STD_PRINTF("M5:T308:S5:OUTER-CPL-ERROR-DELIVERY:OK\n");
    STD_PRINTF("M5:T308:S6:DOUBLE-FAULT-CONTAINMENT:OK\n");
    STD_PRINTF("M5:T330:S2:CALL-GATE-SAME-CPL:OK\n");
    return 0;
}
