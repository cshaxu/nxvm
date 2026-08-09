#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define CG_GDT_BASE 0x0300u
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

static C_VOID cg_reset(C_VOID *opaque)
{
    call_gate_privilege_machine *state = (call_gate_privilege_machine *)opaque;

    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider cg_provider = {
    cg_reset, STD_NULL, STD_NULL
};

static C_INT cg_write(call_gate_privilege_machine *state, uint32_t address,
    const C_VOID *data, STD_SIZE_T bytes)
{
    return state != STD_NULL && state->machine != STD_NULL &&
        core_machine_memory_write(state->machine, address, data, bytes) ==
            TYPE_STATUS_OK;
}

static C_INT cg_read(call_gate_privilege_machine *state, uint32_t address,
    C_VOID *data, STD_SIZE_T bytes)
{
    return state != STD_NULL && state->machine != STD_NULL &&
        core_machine_memory_read_physical(&state->machine->executor_memory,
            address, (type_virtual_address)data, bytes) == TYPE_STATUS_OK;
}

static C_INT cg_prepare(call_gate_privilege_machine *state, uint8_t gate_access,
    uint8_t parameter_count)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    uint8_t gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0x40u,0,
        0xffu,0xffu,0,0,0,0x92u,0xcfu,0,
        0xffu,0xffu,0,0x30u,0,0xfau,0x40u,0,
        0xffu,0xffu,0,0,0,0xf2u,0xcfu,0,
        0x67u,0,0,0x06u,0,0x8bu,0,0,
        0,0x01u,0x08u,0,parameter_count,gate_access,0,0
    };
    uint8_t tss[10] = {0};
    static const uint8_t call[] = {0x9au,0,0x01u,0,0,0x33u,0};
    static const uint8_t handler[] = {0xf4u};
    uint32_t esp0 = 0x00009000u;
    uint16_t ss0 = 0x0010u;
    t_cpu *cpu;

    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    STD_MEMCPY(&tss[4], &esp0, sizeof(esp0));
    STD_MEMCPY(&tss[8], &ss0, sizeof(ss0));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK ||
        core_machine_bind_execution_provider(state->machine, &cg_provider,
            state) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(state->machine) != TYPE_STATUS_OK ||
        core_machine_reset(state->machine) != TYPE_STATUS_OK ||
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

static C_INT cg_fault_is(const core_machine_cpu_diagnostic *diagnostic,
    uint32_t mask, uint32_t code)
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

static C_INT cg_test_success(uint8_t count)
{
    call_gate_privilege_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu after;
    uint32_t frame[6] = {0u,0u,0u,0u,0u,0u};
    uint32_t parameters[2] = {0x11223344u,0x55667788u};
    C_INT failed = !cg_prepare(&state, 0xecu, count);

    if (!failed && count) failed |= !cg_write(&state, 0x00008800u,
        parameters, (STD_SIZE_T)count * sizeof(parameters[0]));
    if (!failed) {
        failed |= !cg_run(&state, 0, &after, &diagnostic) ||
            diagnostic.first_fault.valid || after.data.cs.selector != 0x0008u ||
            after.data.cs.dpl != 0u || after.data.eip != CG_HANDLER_OFFSET + 1u ||
            after.data.ss.selector != 0x0010u || after.data.esp !=
                0x00009000u - (uint32_t)(4u + count) * 4u ||
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
    uint8_t cs_before = 0u, cs_after = 0u;
    uint8_t ss_before = 0u, ss_after = 0u;
    C_INT failed = !cg_prepare(&state, 0x8cu, 0u);

    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !cg_read(&state, CG_GDT_BASE + 13u, &cs_before,
                sizeof(cs_before)) || !cg_read(&state, CG_GDT_BASE + 21u,
                &ss_before, sizeof(ss_before)) || !cg_run(&state, 1, &after,
                &diagnostic) || !cg_fault_is(&diagnostic, VCPUINS_EXCEPT_GP,
                CG_GATE_SELECTOR & 0xfffcu) || !cg_read(&state, CG_GDT_BASE + 13u,
                &cs_after, sizeof(cs_after)) || !cg_read(&state, CG_GDT_BASE + 21u,
                &ss_after, sizeof(ss_after)) || after.data.eip != before.data.eip ||
            after.data.esp != before.data.esp || after.data.cs.selector !=
                before.data.cs.selector || after.data.ss.selector !=
                before.data.ss.selector || cs_after != cs_before || ss_after != ss_before;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT cg_test_gate_failure_atomic(uint8_t gate_access, uint32_t mask,
    uint32_t code)
{
    call_gate_privilege_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    uint8_t cs_before = 0u, cs_after = 0u;
    uint8_t ss_before = 0u, ss_after = 0u;
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
    uint8_t cs_before = 0u, cs_after = 0u;
    uint8_t ss_before = 0u, ss_after = 0u;
    C_INT failed = !cg_prepare(&state, 0xecu, 1u);

    if (!failed) {
        state.machine->executor_cpu.data.ss.limit = 0x00000100u;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !cg_read(&state, CG_GDT_BASE + 13u, &cs_before,
                sizeof(cs_before)) || !cg_read(&state, CG_GDT_BASE + 21u,
                &ss_before, sizeof(ss_before)) || !cg_run(&state, 1, &after,
                &diagnostic) || !cg_fault_is(&diagnostic, VCPUINS_EXCEPT_SS, 0u) ||
            !cg_read(&state, CG_GDT_BASE + 13u, &cs_after, sizeof(cs_after)) ||
            !cg_read(&state, CG_GDT_BASE + 21u, &ss_after, sizeof(ss_after)) ||
            after.data.eip != before.data.eip || after.data.esp != before.data.esp ||
            after.data.eflags != before.data.eflags || after.data.cs.selector !=
                before.data.cs.selector || after.data.ss.selector !=
                before.data.ss.selector || cs_after != cs_before || ss_after != ss_before;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT cg_test_target_failure_atomic(call_gate_target_failure failure,
    uint32_t mask, uint32_t code)
{
    call_gate_privilege_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    uint8_t cs_before = 0u, cs_after = 0u;
    uint8_t ss_before = 0u, ss_after = 0u;
    uint8_t access = 0u;
    uint16_t selector = 0u;
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

int main(void)
{
    C_INT failed = !cg_test_success(0u) || !cg_test_success(2u) ||
        !cg_test_dpl_failure_atomic() || !cg_test_gate_failure_atomic(0x6cu,
            VCPUINS_EXCEPT_NP, CG_GATE_SELECTOR & 0xfffcu) ||
        !cg_test_gate_failure_atomic(0xeeu, VCPUINS_EXCEPT_GP,
            CG_GATE_SELECTOR & 0xfffcu) || !cg_test_parameter_source_failure_atomic() ||
        !cg_test_target_failure_atomic(CALL_GATE_TARGET_CODE_TYPE,
            VCPUINS_EXCEPT_GP, 0x0008u) ||
        !cg_test_target_failure_atomic(CALL_GATE_TARGET_CODE_NOT_PRESENT,
            VCPUINS_EXCEPT_NP, 0x0008u) ||
        !cg_test_target_failure_atomic(CALL_GATE_TARGET_STACK_TYPE,
            VCPUINS_EXCEPT_GP, 0x0010u) ||
        !cg_test_target_failure_atomic(CALL_GATE_TARGET_STACK_NOT_PRESENT,
            VCPUINS_EXCEPT_SS, 0x0010u) ||
        !cg_test_target_failure_atomic(CALL_GATE_TARGET_STACK_BOUNDARY,
            VCPUINS_EXCEPT_SS, 0u);

    if (failed) return 1;
    STD_PRINTF("M5:T307:CALL-GATE-PRIVILEGE-ENTRY:OK\n");
    return 0;
}
