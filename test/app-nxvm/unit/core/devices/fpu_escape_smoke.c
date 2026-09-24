#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/machine_interface.h"
#include "app-nxvm/devices/memory.h"
#include "support/core_machine_cpu_fixture.h"

typedef struct fpu_escape_machine {
    core_machine *machine;
} fpu_escape_machine;

static void fpu_escape_reset(void *opaque)
{
    fpu_escape_machine *state = (fpu_escape_machine *)opaque;

    if (state != LIB_NULL) (void)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider fpu_escape_provider = {
    fpu_escape_reset, LIB_NULL
};

static lib_i32 prepare_machine(core_machine_fpu_profile fpu_profile,
    lib_u32 cr0, fpu_escape_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = fpu_profile
    };
    if (state == LIB_NULL) return 1;
    lib_memory_set(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != LIB_STATUS_OK) return 1;
    if (!test_core_machine_fixture_bind_freeze_reset(state->machine,
            &fpu_escape_provider, state)) {
        core_machine_destroy(state->machine);
        state->machine = LIB_NULL;
        return 1;
    }
    (void)test_core_machine_fixture_set_control_zero(state->machine, cr0);
    return 0;
}

static lib_i32 run_case(const lib_u8 *program, lib_size program_size,
    core_machine_fpu_profile fpu_profile, lib_u32 cr0, lib_u32 expected_exception,
    lib_u32 expected_eip)
{
    fpu_escape_machine state;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_cpu_state cpu;
    lib_i32 failed = prepare_machine(fpu_profile, cr0, &state);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0u, program,
            program_size) != LIB_STATUS_OK;
        if (expected_exception != 0u) {
            failed |= core_machine_run(state.machine, budget, &result) != LIB_STATUS_INTERNAL_ERROR;
        } else {
            failed |= core_machine_run(state.machine, budget, &result) != LIB_STATUS_OK;
        }
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK;
        failed |= core_machine_get_cpu_state(state.machine, &cpu) != LIB_STATUS_OK;
        if (expected_exception != 0u) {
            failed |= !diagnostic.first_fault.valid ||
                !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
                    expected_exception) ||
                CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_UD);
        } else {
            failed |= diagnostic.first_fault.valid || cpu.eip != expected_eip;
        }
    }
    core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 run_nm_delivery_case(const lib_u8 *program,
    lib_size program_size, lib_u32 cr0)
{
    static const lib_u8 handler[] = { 0x40u, 0xf4u };
    const lib_u16 handler_offset = 0x0100u;
    const lib_u16 handler_segment = 0u;
    fpu_escape_machine state;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    t_cpu before;
    t_cpu after;
    lib_u16 frame[3] = { 0u, 0u, 0u };
    lib_u32 original_eax = 0u;
    lib_i32 failed = prepare_machine(CORE_MACHINE_FPU_PROFILE_NONE, cr0, &state);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0u, program,
            program_size) != LIB_STATUS_OK || core_machine_memory_write(
            state.machine, 0x1cu, &handler_offset, sizeof(handler_offset)) !=
            LIB_STATUS_OK || core_machine_memory_write(state.machine, 0x1eu,
            &handler_segment, sizeof(handler_segment)) != LIB_STATUS_OK ||
            core_machine_memory_write(state.machine, handler_offset, handler,
            sizeof(handler)) != LIB_STATUS_OK;
    }
    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        original_eax = before.data.eax;
        failed |= core_machine_run(state.machine, budget, &result) !=
            LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_NM) || after.data.eip != handler_offset ||
            after.data.esp != ((before.data.esp & 0xffff0000u) |
                (lib_u16)(before.data.esp - 6u)) ||
            after.data.ss.selector != before.data.ss.selector ||
            after.data.ss.base != before.data.ss.base ||
            core_machine_memory_read_physical(&state.machine->executor_memory,
                after.data.ss.base + (lib_u16)after.data.esp,
                CORE_MACHINE_REFERENCE_OF(frame), sizeof(frame)) != LIB_STATUS_OK ||
            frame[0] != 0u ||
            frame[1] != before.data.cs.selector || frame[2] !=
                (lib_u16)before.data.eflags;
    }
    if (!failed) {
        budget.instructions = 2u;
        failed |= core_machine_run(state.machine, budget, &result) !=
            LIB_STATUS_OK || result.reason !=
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        after = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= after.data.eip != handler_offset + sizeof(handler) ||
            after.data.eax != original_eax + 1u;
    }
    core_machine_destroy(state.machine);
    return failed;
}

lib_i32 main(void)
{
    static const lib_u8 fninit[] = { 0xdbu, 0xe3u };
    static const lib_u8 memory_escape[] = { 0xd8u, 0x06u, 0x34u, 0x12u };
    static const lib_u8 fwait[] = { 0x9bu };
    lib_i32 failed = 0;

    failed |= run_case(fninit, sizeof(fninit), CORE_MACHINE_FPU_PROFILE_NONE,
        0u, 0u, 2u);
    failed |= run_case(memory_escape, sizeof(memory_escape),
        CORE_MACHINE_FPU_PROFILE_NONE, 0u, 0u, 4u);
    failed |= run_nm_delivery_case(fninit, sizeof(fninit), VCPU_CR0_EM);
    failed |= run_case(fwait, sizeof(fwait), CORE_MACHINE_FPU_PROFILE_NONE,
        VCPU_CR0_TS, 0u, 1u);
    failed |= run_nm_delivery_case(fwait, sizeof(fwait),
        VCPU_CR0_TS | VCPU_CR0_MP);
    failed |= run_case(fninit, sizeof(fninit), CORE_MACHINE_FPU_PROFILE_80387,
        0u, 0u, 2u);
    if (failed) return 1;
    printf("M5:T156:S1:FPU-ESC:OK\n");
    return 0;
}
