#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "app-nxvm/devices/debug_interface.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include "support/core_machine_board_fixture.h"

#define REAL_FINAL_CODE_OFFSET 0x0200u
#define REAL_FINAL_HANDLER_OFFSET 0x0100u
#define REAL_FINAL_STACK_OFFSET 0x8000u
#define REAL_FINAL_GP_VECTOR 0x0du

typedef struct real_final_machine {
    core_machine *machine;
    lib_status reset_status;
} real_final_machine;

static void real_final_reset(void *opaque)
{
    real_final_machine *state = (real_final_machine *)opaque;

    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP),
        .values = {[CORE_MACHINE_DEBUG_EIP] = 0x0500u}
    };
    if (state != LIB_NULL) state->reset_status =
        core_machine_cpu_debug_patch_registers(
            state->machine->executor_cpu_execution, &entry);
}

static const core_machine_execution_provider real_final_provider = {
    real_final_reset, LIB_NULL
};

static lib_i32 real_final_prepare(real_final_machine *state,
    lib_u16 idtr_limit)
{
    static const lib_u8 program[] = { 0xcdu, 0x0fu };
    static const lib_u8 handler[] = { 0xf4u };
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const lib_u16 handler_offset = REAL_FINAL_HANDLER_OFFSET;
    const lib_u16 handler_segment = 0u;
    const lib_u8 lidt[] = {0x0fu, 0x01u, 0x1eu, 0x00u, 0x06u};
    const lib_u8 idt_pointer[] = {
        (lib_u8)idtr_limit, (lib_u8)(idtr_limit >> 8u), 0u, 0u, 0u, 0u
    };
    core_machine_run_result setup;
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
        .values = {
            [CORE_MACHINE_DEBUG_EIP] = REAL_FINAL_CODE_OFFSET,
            [CORE_MACHINE_DEBUG_ESP] = REAL_FINAL_STACK_OFFSET,
            [CORE_MACHINE_DEBUG_EFLAGS] = CORE_MACHINE_DEBUG_EFLAGS_CF |
                CORE_MACHINE_DEBUG_EFLAGS_IF
        }
    };

    if (state == LIB_NULL)
        return 0;
    lib_memory_set(state, 0, sizeof(*state));
    if (!test_core_machine_fixture_create_bind_freeze_reset(&config,
            &real_final_provider, state, &state->machine, LIB_NULL) ||
        state->reset_status != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, 0x0500u, lidt,
            sizeof(lidt)) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, 0x0600u, idt_pointer,
            sizeof(idt_pointer)) != LIB_STATUS_OK ||
        core_machine_run(state->machine, (core_machine_run_budget){1u, 0u},
            &setup) != LIB_STATUS_OK || setup.reason != CORE_MACHINE_STOP_BUDGET ||
        core_machine_debug_patch_registers(state->machine, &entry) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, REAL_FINAL_CODE_OFFSET,
            program, sizeof(program)) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine,
            REAL_FINAL_GP_VECTOR * 4u, &handler_offset,
            sizeof(handler_offset)) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine,
            REAL_FINAL_GP_VECTOR * 4u + 2u, &handler_segment,
            sizeof(handler_segment)) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, REAL_FINAL_HANDLER_OFFSET,
            handler, sizeof(handler)) != LIB_STATUS_OK) {
        core_machine_destroy(state->machine);
        state->machine = LIB_NULL;
        return 0;
    }
    return 1;
}

static lib_i32 real_final_run(real_final_machine *state, lib_status *status,
    core_machine_run_result *result, core_machine_debug_cpu_snapshot *after,
    core_machine_cpu_diagnostic *diagnostic)
{
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){ 1u, 0u }, result);
    if (core_machine_debug_capture_cpu_snapshot(state->machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, after) != LIB_STATUS_OK) return 0;
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        LIB_STATUS_OK;
}

static lib_i32 real_final_test_gp_delivery(void)
{
    real_final_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    lib_u16 frame[3] = { 0u, 0u, 0u };
    core_machine_debug_cpu_snapshot before = {0};
    core_machine_debug_cpu_snapshot after = {0};
    lib_status status;
    lib_i32 failed = !real_final_prepare(&state,
        REAL_FINAL_GP_VECTOR * 4u + 3u);

    if (!failed) {
        failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
        failed |= before.eflags !=
            (CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_IF);
        failed |= !real_final_run(&state, &status, &result, &after,
            &diagnostic) || status != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_GP) || after.eip !=
            REAL_FINAL_HANDLER_OFFSET || after.esp !=
            ((before.esp & 0xffff0000u) |
                (lib_u16)(before.esp - 6u)) ||
            after.eflags != (before.eflags &
                ~(CORE_MACHINE_DEBUG_EFLAGS_IF | CORE_MACHINE_DEBUG_EFLAGS_TF)) ||
            core_machine_debug_read_linear(state.machine,
                after.ss.base + (lib_u16)after.esp, frame, sizeof(frame)) !=
            LIB_STATUS_OK || frame[0] != REAL_FINAL_CODE_OFFSET ||
            frame[1] != before.cs.selector || frame[2] !=
            (CORE_MACHINE_DEBUG_EFLAGS_CF | CORE_MACHINE_DEBUG_EFLAGS_IF | 0x02u);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 real_final_test_gp_delivery_failure(void)
{
    real_final_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    core_machine_debug_cpu_snapshot before = {0};
    core_machine_debug_cpu_snapshot after = {0};
    lib_status status;
    lib_i32 failed = !real_final_prepare(&state,
        REAL_FINAL_GP_VECTOR * 4u - 1u);

    if (!failed) {
        failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK;
        failed |= !real_final_run(&state, &status, &result, &after,
            &diagnostic) || status != LIB_STATUS_INTERNAL_ERROR ||
            result.reason != CORE_MACHINE_STOP_FAULT ||
            !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_GP) ||
            diagnostic.last_delivered_exception.valid || after.eip !=
            before.eip || after.esp != before.esp ||
            after.eflags != before.eflags;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!real_final_test_gp_delivery() ||
        !real_final_test_gp_delivery_failure())
        return 1;
    printf("M5:T331:S1:REAL-EXCEPTION-FINAL:OK\n");
    return 0;
}
