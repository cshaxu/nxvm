#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_cpu_fixture.h"

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

static void iret_reset(void *opaque)
{
    iret_machine *state = (iret_machine *)opaque;

    if (state != LIB_NULL) (void)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider iret_provider = {
    iret_reset, LIB_NULL
};

static lib_i32 iret_write(iret_machine *state, lib_u32 address,
    const void *data, lib_size bytes)
{
    return state != LIB_NULL && state->machine != LIB_NULL &&
        core_machine_memory_write(state->machine, address, data, bytes) ==
            LIB_STATUS_OK;
}

static lib_i32 iret_prepare(iret_machine *state, iret_negative negative,
    lib_i32 small_stack, lib_i32 conforming, lib_i32 user_cpl)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    lib_u8 gdt[] = {
        0,0,0,0,0,0,0,0,
        0xffu,0xffu,0,0x20u,0,0x9au,0x40u,0,
        0xffu,0xffu,0,0,0,0x92u,0xcfu,0
    };
    t_cpu *cpu;

    if (state == LIB_NULL) return 0;
    lib_memory_set(state, 0, sizeof(*state));
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
        state->machine = LIB_NULL;
        return 0;
    }
    cpu = &state->machine->executor_cpu;
    cpu->data.cr0 = VCPU_CR0_PE;
    cpu->data.gdtr.flagValid = LIB_TRUE;
    cpu->data.gdtr.sregtype = SREG_GDTR;
    cpu->data.gdtr.base = IRET_GDT_BASE;
    cpu->data.gdtr.limit = sizeof(gdt) - 1u;
    cpu->data.cs.flagValid = LIB_TRUE;
    cpu->data.cs.selector = user_cpl ? 0x000bu : 0x0008u;
    cpu->data.cs.sregtype = SREG_CODE;
    cpu->data.cs.base = IRET_CODE_BASE;
    cpu->data.cs.limit = 0xffffu;
    cpu->data.cs.dpl = user_cpl ? 3u : 0u;
    cpu->data.cs.seg.accessed = LIB_FALSE;
    cpu->data.cs.seg.executable = LIB_TRUE;
    cpu->data.cs.seg.exec.defsize = LIB_TRUE;
    cpu->data.cs.seg.exec.conform = LIB_FALSE;
    cpu->data.cs.seg.exec.readable = LIB_TRUE;
    cpu->data.ss.flagValid = LIB_TRUE;
    cpu->data.ss.selector = user_cpl ? 0x0013u : 0x0010u;
    cpu->data.ss.sregtype = SREG_STACK;
    cpu->data.ss.base = 0u;
    cpu->data.ss.limit = negative == IRET_NEGATIVE_STACK_LIMIT ? 1u :
        0xffffffffu;
    cpu->data.ss.dpl = user_cpl ? 3u : 0u;
    cpu->data.ss.seg.accessed = LIB_FALSE;
    cpu->data.ss.seg.executable = LIB_FALSE;
    cpu->data.ss.seg.data.big = small_stack ? LIB_FALSE : LIB_TRUE;
    cpu->data.ss.seg.data.expdown = LIB_FALSE;
    cpu->data.ss.seg.data.writable = LIB_TRUE;
    cpu->data.eip = 0u;
    cpu->data.esp = small_stack ? 0x00018000u : IRET_STACK;
    cpu->data.eflags = user_cpl ? 0x00000002u : 0x00000202u;
    cpu->data.flagHalt = LIB_FALSE;
    return 1;
}

static lib_i32 iret_fault_is(const core_machine_cpu_diagnostic *diagnostic,
    lib_u32 mask, lib_u32 code)
{
    return diagnostic->first_fault.valid && CORE_MACHINE_BIT_IS_SET(
        diagnostic->first_fault.exception_mask, mask) &&
        diagnostic->first_fault.exception_code == code;
}

static lib_i32 iret_run(iret_machine *state, lib_i32 expect_fault, t_cpu *after,
    core_machine_cpu_diagnostic *diagnostic)
{
    const core_machine_run_budget budget = {16u, 0u};
    core_machine_run_result result;
    lib_status status = core_machine_run(state->machine, budget, &result);

    if (core_machine_get_cpu_diagnostic(state->machine, diagnostic) !=
        LIB_STATUS_OK) return 0;
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return status == (expect_fault ? LIB_STATUS_INTERNAL_ERROR : LIB_STATUS_OK) &&
        result.reason == (expect_fault ? CORE_MACHINE_STOP_FAULT :
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT);
}

static lib_i32 iret_test_success(lib_u8 prefix, lib_i32 operand16,
    lib_i32 small_stack, lib_i32 conforming)
{
    iret_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu after;
    lib_u8 code[] = { prefix, 0xcfu, 0xf4u };
    lib_u32 frame32[] = { prefix ? 2u : 1u, 0x0008u, 0x00000203u };
    lib_u16 frame16[] = { 2u, 0x0008u, 0x0203u };
    lib_u32 expected_esp = small_stack ?
        (operand16 ? 0x00018006u : 0x0001800cu) :
        (operand16 ? IRET_STACK + 6u : IRET_STACK + 12u);
    lib_i32 failed = !iret_prepare(&state, IRET_NEGATIVE_NONE, small_stack,
        conforming, 0);

    if (!failed) {
        failed |= !iret_write(&state, IRET_CODE_BASE, prefix ? code : code + 1u,
                prefix ? sizeof(code) : sizeof(code) - 1u) ||
            !iret_write(&state, IRET_STACK, operand16 ? (const void *)frame16 :
                (const void *)frame32, operand16 ? sizeof(frame16) : sizeof(frame32)) ||
            !iret_run(&state, 0, &after, &diagnostic) || diagnostic.first_fault.valid ||
            after.data.eip != (prefix ? 3u : 2u) || after.data.esp != expected_esp ||
            after.data.cs.selector != 0x0008u || after.data.eflags != 0x00000203u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 iret_test_failure(iret_negative negative, lib_u32 mask,
    lib_u32 code)
{
    iret_machine state;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    lib_u8 program[] = {0xcfu,0xf4u};
    lib_u32 frame[] = { negative == IRET_NEGATIVE_LIMIT ? 1u : 1u,
        0x0008u, 0x00000203u };
    lib_u8 access_before = 0u;
    lib_u8 access_after = 0u;
    lib_i32 failed = !iret_prepare(&state, negative, 0, 0, 0);

    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !iret_write(&state, IRET_CODE_BASE, program, sizeof(program)) ||
            !iret_write(&state, IRET_STACK, frame, sizeof(frame)) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                IRET_CODE_ACCESS, CORE_MACHINE_REFERENCE_OF(access_before), 1u) != LIB_STATUS_OK ||
            !iret_run(&state, 1, &after, &diagnostic) ||
            !iret_fault_is(&diagnostic, mask, code) ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                IRET_CODE_ACCESS, CORE_MACHINE_REFERENCE_OF(access_after), 1u) != LIB_STATUS_OK ||
            after.data.eip != before.data.eip || after.data.esp != before.data.esp ||
            after.data.eflags != before.data.eflags ||
            lib_memory_compare(&after.data.cs, &before.data.cs, sizeof(before.data.cs)) != 0 ||
            lib_memory_compare(&after.data.ss, &before.data.ss, sizeof(before.data.ss)) != 0 ||
            access_after != access_before;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 iret_test_user_flags(void)
{
    const core_machine_run_budget budget = {1u, 0u};
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    iret_machine state;
    t_cpu after;
    lib_u8 program[] = {0xcfu,0x90u};
    lib_u32 frame[] = {1u,0x000bu,0x00013203u};
    lib_i32 failed = !iret_prepare(&state, IRET_NEGATIVE_NONE, 0, 0, 1);

    if (!failed) {
        failed |= !iret_write(&state, IRET_CODE_BASE, program, sizeof(program)) ||
            !iret_write(&state, IRET_STACK, frame, sizeof(frame)) ||
            core_machine_run(state.machine, budget, &result) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= diagnostic.first_fault.valid || after.data.eip != 1u ||
            after.data.esp != IRET_STACK + 12u || after.data.cs.selector != 0x000bu ||
            after.data.eflags != 0x00010003u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

lib_i32 main(void)
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
    printf("M5:T306:S2:SAME-CPL-IRET:OK\n");
    return 0;
}
