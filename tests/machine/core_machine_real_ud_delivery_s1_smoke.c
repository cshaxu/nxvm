#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

/* T337_REAL_UD_VECTOR6_DELIVERY: this owner proves the shared real #UD path. */

#define REAL_UD_CODE_OFFSET 0x0200u
#define REAL_UD_HANDLER_OFFSET 0x0100u
#define REAL_UD_STACK_OFFSET 0x8000u
#define REAL_UD_VECTOR 0x06u

typedef struct real_ud_machine {
    core_machine *machine;
} real_ud_machine;

typedef struct real_ud_case {
    const type_unsigned_8 *program;
    STD_SIZE_T bytes;
    core_machine_cpu_profile profile;
} real_ud_case;

static C_VOID real_ud_reset(C_VOID *opaque)
{
    real_ud_machine *state = (real_ud_machine *)opaque;

    if (state != STD_NULL)
        (C_VOID)test_core_machine_fixture_reset_real_mode(state->machine);
}

static const core_machine_execution_provider real_ud_provider = {
    real_ud_reset, STD_NULL
};

static C_INT real_ud_prepare(real_ud_machine *state,
    const real_ud_case *test_case, type_unsigned_16 idtr_limit)
{
    static const type_unsigned_8 handler[] = { 0x40u, 0xf4u };
    core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    const type_unsigned_16 handler_offset = REAL_UD_HANDLER_OFFSET;
    const type_unsigned_16 handler_segment = 0u;
    t_cpu *cpu;

    if (state == STD_NULL || test_case == STD_NULL) return 0;
    config.cpu_profile = test_case->profile;
    STD_MEMSET(state, 0, sizeof(*state));
    if (!test_core_machine_fixture_create_bind_freeze_reset(&config,
            &real_ud_provider, state, &state->machine) ||
        !test_core_machine_fixture_prepare_real_mode_execution(state->machine,
            REAL_UD_CODE_OFFSET) ||
        core_machine_memory_write(state->machine, REAL_UD_CODE_OFFSET,
            test_case->program, test_case->bytes) !=
            TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, REAL_UD_VECTOR * 4u,
            &handler_offset, sizeof(handler_offset)) !=
            TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, REAL_UD_VECTOR * 4u + 2u,
            &handler_segment, sizeof(handler_segment)) !=
            TYPE_STATUS_OK ||
        core_machine_memory_write(state->machine, REAL_UD_HANDLER_OFFSET,
            handler, sizeof(handler)) != TYPE_STATUS_OK) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    cpu = &state->machine->executor_cpu;
    cpu->data.idtr.limit = idtr_limit;
    cpu->data.esp = REAL_UD_STACK_OFFSET;
    cpu->data.eax = 0x12340000u;
    cpu->data.eflags = VCPU_EFLAGS_CF | VCPU_EFLAGS_IF | VCPU_EFLAGS_TF;
    return 1;
}

static C_INT real_ud_run(real_ud_machine *state, type_unsigned_32 budget,
    type_status *status, core_machine_run_result *result, t_cpu *after,
    core_machine_cpu_diagnostic *diagnostic)
{
    *status = core_machine_run(state->machine,
        (core_machine_run_budget){ budget, 0u }, result);
    *after = test_core_machine_fixture_capture_cpu_after_run(state->machine);
    return core_machine_get_cpu_diagnostic(state->machine, diagnostic) ==
        TYPE_STATUS_OK;
}

static C_INT real_ud_test_delivery_case(const real_ud_case *test_case)
{
    real_ud_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    type_unsigned_16 frame[3] = { 0u, 0u, 0u };
    t_cpu before;
    t_cpu after;
    type_status status;
    C_INT failed = !real_ud_prepare(&state, test_case,
        REAL_UD_VECTOR * 4u + 3u);

    if (!failed) {
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !real_ud_run(&state, 1u, &status, &result, &after,
            &diagnostic) || status != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid || !TYPE_GET_BIT(
                diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_UD) || after.data.eip !=
            REAL_UD_HANDLER_OFFSET || after.data.esp !=
            ((before.data.esp & 0xffff0000u) |
                (type_unsigned_16)(before.data.esp - 6u)) ||
            after.data.eflags != (before.data.eflags &
                ~(VCPU_EFLAGS_IF | VCPU_EFLAGS_TF)) ||
            !test_core_machine_fixture_read_linear(state.machine,
                after.data.ss.base + (type_unsigned_16)after.data.esp,
                TYPE_REFERENCE_OF(frame), sizeof(frame)) || frame[0] !=
            REAL_UD_CODE_OFFSET || frame[1] != before.data.cs.selector ||
            frame[2] != (type_unsigned_16)((before.data.eflags &
                ~VCPU_EFLAGS_RESERVED) | 0x02u);
        failed |= !real_ud_run(&state, 2u, &status, &result, &after,
            &diagnostic) || status != TYPE_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            after.data.eip != REAL_UD_HANDLER_OFFSET + 2u ||
            after.data.eax != before.data.eax + 1u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static C_INT real_ud_test_delivery(C_VOID)
{
    static const type_unsigned_8 primary[] = { 0xf1u };
    static const type_unsigned_8 escape[] = { 0x0fu, 0xffu };
    static const type_unsigned_8 operand[] = { 0x62u, 0xc0u };
    static const type_unsigned_8 profile[] = { 0x66u, 0x90u };
    static const type_unsigned_8 lock[] = { 0xf0u, 0x90u };
    static const real_ud_case cases[] = {
        { primary, sizeof(primary), CORE_MACHINE_CPU_PROFILE_80386 },
        { escape, sizeof(escape), CORE_MACHINE_CPU_PROFILE_80386 },
        { operand, sizeof(operand), CORE_MACHINE_CPU_PROFILE_80386 },
        { profile, sizeof(profile), CORE_MACHINE_CPU_PROFILE_80286 },
        { lock, sizeof(lock), CORE_MACHINE_CPU_PROFILE_80386 }
    };
    STD_SIZE_T index;

    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index)
        if (!real_ud_test_delivery_case(&cases[index])) return 0;
    return 1;
}

static C_INT real_ud_test_delivery_failure(C_VOID)
{
    static const type_unsigned_8 program[] = { 0xf1u };
    const real_ud_case test_case = {
        program, sizeof(program), CORE_MACHINE_CPU_PROFILE_80386
    };
    real_ud_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result;
    t_cpu before;
    t_cpu after;
    type_status status;
    C_INT failed = !real_ud_prepare(&state, &test_case,
        REAL_UD_VECTOR * 4u - 1u);

    if (!failed) {
        state.machine->executor_cpu.data.eflags = VCPU_EFLAGS_CF |
            VCPU_EFLAGS_IF;
        before = test_core_machine_fixture_capture_cpu_after_run(state.machine);
        failed |= !real_ud_run(&state, 1u, &status, &result, &after,
            &diagnostic) || status != TYPE_STATUS_FAULT ||
            result.reason != CORE_MACHINE_STOP_FAULT ||
            !diagnostic.first_fault.valid || !TYPE_GET_BIT(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            diagnostic.last_delivered_exception.valid || after.data.eip !=
            before.data.eip || after.data.esp != before.data.esp ||
            after.data.eax != before.data.eax || after.data.eflags !=
            before.data.eflags || STD_MEMCMP(&after.data.cs, &before.data.cs,
                sizeof(after.data.cs)) != 0 || STD_MEMCMP(&after.data.ss,
                &before.data.ss, sizeof(after.data.ss)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

C_INT main(C_VOID)
{
    if (!real_ud_test_delivery() || !real_ud_test_delivery_failure()) return 1;
    STD_PRINTF("M5:T337:S1:REAL-UD-DELIVERY:OK\n");
    return 0;
}
