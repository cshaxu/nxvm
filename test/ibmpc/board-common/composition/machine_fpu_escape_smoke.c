#include "lib/types/file.h"
#include "lib/types/types_interface.h"
#include "x86/core/device_support_interface.h"

#include "ibmpc/board-common/machine_board_interface.h"
#include "../core_machine_board_fixture.h"

typedef struct fpu_escape_machine {
    core_machine *machine;
} fpu_escape_machine;

static lib_i32 prepare_machine(x86_fpu_profile fpu_profile,
    lib_u32 cr0, fpu_escape_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = fpu_profile
    };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CR0),
        .values = {[CORE_MACHINE_DEBUG_CR0] = cr0}
    };
    if (state == LIB_NULL) return 1;
    lib_memory_set(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine, LIB_NULL) != LIB_STATUS_OK) return 1;
    if (!test_core_machine_fixture_bind_freeze_reset(state->machine,
            LIB_NULL, LIB_NULL) ||
        core_machine_debug_patch_registers(state->machine, &entry) != LIB_STATUS_OK) {
        core_machine_destroy(state->machine);
        state->machine = LIB_NULL;
        return 1;
    }
    return 0;
}

static lib_i32 run_case(const lib_u8 *program, lib_size program_size,
    x86_fpu_profile fpu_profile, lib_u32 cr0, lib_u32 expected_exception,
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
        if (!failed && expected_exception != 0u) {
            failed |= !diagnostic.first_fault.valid ||
                !CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
                    expected_exception) ||
                CORE_MACHINE_BIT_IS_SET(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_UD);
        } else if (!failed) {
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
    core_machine_debug_cpu_snapshot before = {0};
    core_machine_debug_cpu_snapshot after = {0};
    lib_u16 frame[3] = { 0u, 0u, 0u };
    lib_u32 original_eax = 0u;
    lib_i32 failed = prepare_machine(X86_FPU_PROFILE_NONE, cr0, &state);

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
        failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
        original_eax = before.eax;
        failed |= core_machine_run(state.machine, budget, &result) !=
            LIB_STATUS_OK || result.reason != CORE_MACHINE_STOP_BUDGET ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            LIB_STATUS_OK;
        failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
        if (!failed) failed |= diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_NM) || after.eip != handler_offset ||
            after.esp != ((before.esp & 0xffff0000u) |
                (lib_u16)(before.esp - 6u)) ||
            after.ss.selector != before.ss.selector ||
            after.ss.base != before.ss.base ||
            core_machine_memory_read(state.machine,
                after.ss.base + (lib_u16)after.esp,
                (void *)CORE_MACHINE_REFERENCE_OF(frame), sizeof(frame)) != LIB_STATUS_OK ||
            frame[0] != 0u ||
            frame[1] != before.cs.selector || frame[2] !=
                (lib_u16)before.eflags;
    }
    if (!failed) {
        budget.instructions = 2u;
        failed |= core_machine_run(state.machine, budget, &result) !=
            LIB_STATUS_OK || result.reason !=
            CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT;
        failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK;
        failed |= after.eip != handler_offset + sizeof(handler) ||
            after.eax != original_eax + 1u;
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

    failed |= run_case(fninit, sizeof(fninit), X86_FPU_PROFILE_NONE,
        0u, 0u, 2u);
    failed |= run_case(memory_escape, sizeof(memory_escape),
        X86_FPU_PROFILE_NONE, 0u, 0u, 4u);
    failed |= run_nm_delivery_case(fninit, sizeof(fninit), VCPU_CR0_EM);
    failed |= run_case(fwait, sizeof(fwait), X86_FPU_PROFILE_NONE,
        VCPU_CR0_TS, 0u, 1u);
    failed |= run_nm_delivery_case(fwait, sizeof(fwait),
        VCPU_CR0_TS | VCPU_CR0_MP);
    failed |= run_case(fninit, sizeof(fninit), X86_FPU_PROFILE_80387,
        0u, 0u, 2u);
    if (failed) return 1;
    lib_c_printf("FPU-ESC:OK\n");
    return 0;
}
