#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define IRET_GDT_BASE 0x0300u
#define IRET_CODE_BASE 0x2000u
#define IRET_STACK 0x8000u
#define IRET_CODE_ACCESS (IRET_GDT_BASE + 13u)

typedef enum iret_negative {
    IRET_NEGATIVE_NONE,
    IRET_NEGATIVE_NONPRESENT,
    IRET_NEGATIVE_LIMIT,
    IRET_NEGATIVE_CODE_TYPE,
    IRET_NEGATIVE_CODE_DPL,
    IRET_NEGATIVE_STACK_LIMIT
} iret_negative;

typedef struct iret_machine {
    core_machine *machine;
} iret_machine;

static C_VOID iret_reset(C_VOID *opaque)
{
    iret_machine *state = (iret_machine *)opaque;

    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider iret_provider = {
    iret_reset, STD_NULL
};

static C_INT iret_write(iret_machine *state, type_unsigned_32 address,
    const C_VOID *data, STD_SIZE_T bytes)
{
    return state != STD_NULL && state->machine != STD_NULL &&
        core_machine_memory_write(state->machine, address, data, bytes) ==
            TYPE_STATUS_OK;
}

static C_INT iret_prepare(iret_machine *state, iret_negative negative,
    C_INT small_stack, C_INT conforming, C_INT user_cpl)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    type_unsigned_8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0x40u,0,
        0xffu,0xffu,0,0,0,0x92u,0xcfu,0
    };
    t_cpu *cpu;

    if (state == STD_NULL) return 0;
    STD_MEMSET(state, 0, sizeof(*state));
    if (negative == IRET_NEGATIVE_NONPRESENT) gdt[13u] &= 0x7fu;
    if (conforming) gdt[13u] = 0x9eu;
    if (negative == IRET_NEGATIVE_CODE_TYPE) gdt[13u] = 0x92u;
    if (negative == IRET_NEGATIVE_CODE_DPL) gdt[13u] = 0xbau;
    if (user_cpl) {
        gdt[13u] = 0xfau;
        gdt[21u] = 0xf2u;
    }
    if (negative == IRET_NEGATIVE_LIMIT) {
        gdt[8u] = 0u;
        gdt[9u] = 0u;
    }
    if (!test_core_machine_fixture_create_bind_freeze_reset(&config,
            &iret_provider, state, &state->machine) ||
        !iret_write(state, IRET_GDT_BASE, gdt, sizeof(gdt))) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    cpu = &state->machine->executor_cpu;
    cpu->data.cr0 = VCPU_CR0_PE;
    cpu->data.gdtr.flagValid = TYPE_TRUE;
    cpu->data.gdtr.sregtype = SREG_GDTR;
    cpu->data.gdtr.base = IRET_GDT_BASE;
    cpu->data.gdtr.limit = sizeof(gdt) - 1u;
    cpu->data.cs.flagValid = TYPE_TRUE;
    cpu->data.cs.selector = user_cpl ? 0x000bu : 0x0008u;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.base = IRET_CODE_BASE;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.dpl = user_cpl ? 3u : 0u;
    cpu->data.cs.seg.accessed = TYPE_FALSE;
    cpu->data.cs.seg.executable = TYPE_TRUE;
    cpu->data.cs.seg.exec.defsize = TYPE_TRUE;
    cpu->data.cs.seg.exec.conform = TYPE_FALSE;
    cpu->data.cs.seg.exec.readable = TYPE_TRUE;
    cpu->data.ss.flagValid = TYPE_TRUE;
    cpu->data.ss.selector = user_cpl ? 0x0013u : 0x0010u;
    cpu->data.ss.sregtype = SREG_STACK;
    cpu->data.ss.base = 0u;
    cpu->data.ss.limit = negative == IRET_NEGATIVE_STACK_LIMIT ? 1u :
        0xffffffffu;
    cpu->data.ss.dpl = user_cpl ? 3u : 0u;
    cpu->data.ss.seg.accessed = TYPE_FALSE;
    cpu->data.ss.seg.executable = TYPE_FALSE;
    cpu->data.ss.seg.data.big = small_stack ? TYPE_FALSE : TYPE_TRUE;
    cpu->data.ss.seg.data.expdown = TYPE_FALSE;
    cpu->data.ss.seg.data.writable = TYPE_TRUE;
    cpu->data.eip = 0u;
    cpu->data.esp = small_stack ? 0x00018000u : IRET_STACK;
    cpu->data.eflags = user_cpl ? 0x00000002u : 0x00000202u;
    cpu->data.flagHalt = TYPE_FALSE;
    return 1;
}

static C_INT iret_fault_is(const core_machine_cpu_diagnostic *diagnostic,
    type_unsigned_32 mask, type_unsigned_32 code)
{
    return diagnostic->first_fault.valid && TYPE_GET_BIT(
        diagnostic->first_fault.exception_mask, mask) &&
        diagnostic->first_fault.exception_code == code;
}

static C_INT iret_run(iret_machine *state, C_INT expect_fault, t_cpu *after,
    core_machine_cpu_diagnostic *diagnostic)
{
    const core_machine_run_budget budget = {16u, 0u};
    core_machine_run_result result;
    type_status status = core_machine_run(state->machine, budget, &result);

    if (core_machine_get_cpu_diagnostic(state->machine, diagnostic) !=
        TYPE_STATUS_OK) return 0;
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return status == (expect_fault ? TYPE_STATUS_FAULT : TYPE_STATUS_OK) &&
        result.reason == (expect_fault ? CORE_MACHINE_STOP_FAULT :
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT);
}

static C_INT iret_test_success(type_unsigned_8 prefix, C_INT operand16,
    C_INT small_stack, C_INT conforming)
{
    iret_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu after;
    type_unsigned_8 code[] = { prefix, 0xcfu, 0xf4u };
    type_unsigned_32 frame32[] = { prefix ? 2u : 1u, 0x0008u, 0x00000203u };
    type_unsigned_16 frame16[] = { 2u, 0x0008u, 0x0203u };
    type_unsigned_32 expected_esp = small_stack ?
        (operand16 ? 0x00018006u : 0x0001800cu) :
        (operand16 ? IRET_STACK + 6u : IRET_STACK + 12u);
    C_INT failed = !iret_prepare(&state, IRET_NEGATIVE_NONE, small_stack,
        conforming, 0);

    if (!failed) {
        failed |= !iret_write(&state, IRET_CODE_BASE, prefix ? code : code + 1u,
                prefix ? sizeof(code) : sizeof(code) - 1u) ||
            !iret_write(&state, IRET_STACK, operand16 ? (const C_VOID *)frame16 :
                (const C_VOID *)frame32, operand16 ? sizeof(frame16) : sizeof(frame32)) ||
            !iret_run(&state, 0, &after, &diagnostic) || diagnostic.first_fault.valid ||
            after.data.eip != (prefix ? 3u : 2u) || after.data.esp != expected_esp ||
            after.data.cs.selector != 0x0008u || after.data.eflags != 0x00000203u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT iret_test_failure(iret_negative negative, type_unsigned_32 mask,
    type_unsigned_32 code)
{
    iret_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    type_unsigned_8 program[] = {0xcfu,0xf4u};
    type_unsigned_32 frame[] = { negative == IRET_NEGATIVE_LIMIT ? 1u : 1u,
        0x0008u, 0x00000203u };
    type_unsigned_8 access_before = 0u;
    type_unsigned_8 access_after = 0u;
    C_INT failed = !iret_prepare(&state, negative, 0, 0, 0);

    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !iret_write(&state, IRET_CODE_BASE, program, sizeof(program)) ||
            !iret_write(&state, IRET_STACK, frame, sizeof(frame)) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                IRET_CODE_ACCESS, TYPE_REFERENCE_OF(access_before), 1u) != TYPE_STATUS_OK ||
            !iret_run(&state, 1, &after, &diagnostic) ||
            !iret_fault_is(&diagnostic, mask, code) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                IRET_CODE_ACCESS, TYPE_REFERENCE_OF(access_after), 1u) != TYPE_STATUS_OK ||
            after.data.eip != before.data.eip || after.data.esp != before.data.esp ||
            after.data.eflags != before.data.eflags ||
            STD_MEMCMP(&after.data.cs, &before.data.cs, sizeof(before.data.cs)) != 0 ||
            STD_MEMCMP(&after.data.ss, &before.data.ss, sizeof(before.data.ss)) != 0 ||
            access_after != access_before;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT iret_test_user_flags(C_VOID)
{
    const core_machine_run_budget budget = {1u, 0u};
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    iret_machine state;
    t_cpu after;
    type_unsigned_8 program[] = {0xcfu,0x90u};
    type_unsigned_32 frame[] = {1u,0x000bu,0x00013203u};
    C_INT failed = !iret_prepare(&state, IRET_NEGATIVE_NONE, 0, 0, 1);

    if (!failed) {
        failed |= !iret_write(&state, IRET_CODE_BASE, program, sizeof(program)) ||
            !iret_write(&state, IRET_STACK, frame, sizeof(frame)) ||
            core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= diagnostic.first_fault.valid || after.data.eip != 1u ||
            after.data.esp != IRET_STACK + 12u || after.data.cs.selector != 0x000bu ||
            after.data.eflags != 0x00010003u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

C_INT main(C_VOID)
{
    if (!iret_test_success(0u, 0, 0, 0) || !iret_test_success(0x66u, 1, 0, 0) ||
        !iret_test_success(0x67u, 0, 0, 0) || !iret_test_success(0x66u, 1, 1, 0) ||
        !iret_test_success(0u, 0, 0, 1) ||
        !iret_test_user_flags() ||
        !iret_test_failure(IRET_NEGATIVE_NONPRESENT, VCPUINS_EXCEPT_DF, 0u) ||
        !iret_test_failure(IRET_NEGATIVE_LIMIT, VCPUINS_EXCEPT_DF, 0u) ||
        !iret_test_failure(IRET_NEGATIVE_CODE_TYPE, VCPUINS_EXCEPT_DF, 0u) ||
        !iret_test_failure(IRET_NEGATIVE_CODE_DPL, VCPUINS_EXCEPT_DF, 0u) ||
        !iret_test_failure(IRET_NEGATIVE_STACK_LIMIT, VCPUINS_EXCEPT_DF, 0u)) return 1;
    STD_PRINTF("M5:T306:S2:SAME-CPL-IRET:OK\n");
    return 0;
}
