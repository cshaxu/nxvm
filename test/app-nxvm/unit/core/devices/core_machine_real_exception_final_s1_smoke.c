#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "app-nxvm/devices/cpu.h"
#include "app-nxvm/devices/machine_interface.h"
#include "support/core_machine_cpu_fixture.h"

#define REAL_FINAL_CODE_OFFSET 0x0200u
#define REAL_FINAL_HANDLER_OFFSET 0x0100u
#define REAL_FINAL_STACK_OFFSET 0x8000u
#define REAL_FINAL_GP_VECTOR 0x0du

typedef struct real_final_machine {
    core_machine *machine;
} real_final_machine;

static void real_final_reset(void *opaque)
{
    real_final_machine *state = (real_final_machine *)opaque;

    if (state != LIB_NULL)
        (void)test_core_machine_fixture_reset_real_mode(state->machine);
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
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    const lib_u16 handler_offset = REAL_FINAL_HANDLER_OFFSET;
    const lib_u16 handler_segment = 0u;
    t_cpu *cpu;

    if (state == LIB_NULL)
        return 0;
    lib_memory_set(state, 0, sizeof(*state));
    if (!test_core_machine_fixture_create_bind_freeze_reset(&config,
            &real_final_provider, state, &state->machine) ||
        !test_core_machine_fixture_prepare_real_mode_execution(state->machine,
            REAL_FINAL_CODE_OFFSET) ||
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
    cpu = &state->machine->executor_cpu;
    cpu->data.idtr.limit = idtr_limit;
    cpu->data.esp = REAL_FINAL_STACK_OFFSET;
    cpu->data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF;
    return 1;
}

static lib_i32 real_final_run(real_final_machine *state, lib_status *status,
    core_machine_run_result *result, t_cpu *after,
    core_machine_cpu_diagnostic *diagnostic)
{
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){ 1u, 0u }, result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        LIB_STATUS_OK;
}

static lib_i32 real_final_test_gp_delivery(void)
{
    real_final_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    lib_u16 frame[3] = { 0u, 0u, 0u };
    t_cpu before;
    t_cpu after;
    lib_status status;
    lib_i32 failed = !real_final_prepare(&state,
        REAL_FINAL_GP_VECTOR * 4u + 3u);

    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !real_final_run(&state, &status, &result, &after,
            &diagnostic) || status != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_GP) || after.data.eip !=
            REAL_FINAL_HANDLER_OFFSET || after.data.esp !=
            ((before.data.esp & 0xffff0000u) |
                (lib_u16)(before.data.esp - 6u)) ||
            after.data.eflags != (before.data.eflags &
                ~(VCPU_EFLAGS_IF | VCPU_EFLAGS_TF)) ||
            !test_core_machine_fixture_read_linear(state.machine,
                after.data.ss.base + (lib_u16)after.data.esp,
                CORE_MACHINE_REFERENCE_OF(frame), sizeof(frame)) || frame[0] !=
            REAL_FINAL_CODE_OFFSET || frame[1] != before.data.cs.selector ||
            frame[2] != (lib_u16)((before.data.eflags &
                ~VCPU_EFLAGS_RESERVED) | 0x02u);
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 real_final_test_gp_delivery_failure(void)
{
    real_final_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    lib_status status;
    lib_i32 failed = !real_final_prepare(&state,
        REAL_FINAL_GP_VECTOR * 4u - 1u);

    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !real_final_run(&state, &status, &result, &after,
            &diagnostic) || status != LIB_STATUS_INTERNAL_ERROR ||
            result.reason != CORE_MACHINE_STOP_FAULT ||
            !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_GP) ||
            diagnostic.last_delivered_exception.valid || after.data.eip !=
            before.data.eip || after.data.esp != before.data.esp ||
            after.data.eflags != before.data.eflags;
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
