#include "lib/types/types_interface.h"
#include <stdio.h>
#include "x86/core/device_support_interface.h"

#include "x86/core/debug_interface.h"
#include "x86/ibmpc-common/machine_board_interface.h"

/* T337_REAL_UD_VECTOR6_DELIVERY: this owner proves the shared real #UD path. */

#define REAL_UD_CODE_OFFSET 0x0200u
#define REAL_UD_HANDLER_OFFSET 0x0100u
#define REAL_UD_STACK_OFFSET 0x8000u
#define REAL_UD_VECTOR 0x06u

typedef struct real_ud_machine {
    core_machine *machine;
} real_ud_machine;

typedef struct real_ud_case {
    const lib_u8 *program;
    lib_size bytes;
    core_machine_cpu_profile profile;
} real_ud_case;

static lib_i32 real_ud_prepare(real_ud_machine *state,
    const real_ud_case *test_case, lib_u16 idtr_limit)
{
    static const lib_u8 handler[] = { 0x40u, 0xf4u };
    core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_8086,
        .fpu_profile = X86_FPU_PROFILE_NONE
    };
    const lib_u16 handler_offset = REAL_UD_HANDLER_OFFSET;
    const lib_u16 handler_segment = 0u;
    static const lib_u8 load_idtr[] = { 0x0fu, 0x01u, 0x1eu, 0x00u, 0x04u };
    const lib_u8 idtr[] = { (lib_u8)idtr_limit, (lib_u8)(idtr_limit >> 8u),
        0u, 0u, 0u, 0u };
    const core_machine_debug_register_patch setup = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP),
        .values = { [CORE_MACHINE_DEBUG_EIP] = 0x0300u }
    };
    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ESP) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EAX) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EFLAGS),
        .values = { [CORE_MACHINE_DEBUG_EIP] = REAL_UD_CODE_OFFSET,
            [CORE_MACHINE_DEBUG_ESP] = REAL_UD_STACK_OFFSET,
            [CORE_MACHINE_DEBUG_EAX] = 0x12340000u,
            [CORE_MACHINE_DEBUG_EFLAGS] = CORE_MACHINE_DEBUG_EFLAGS_CF |
                CORE_MACHINE_DEBUG_EFLAGS_IF | CORE_MACHINE_DEBUG_EFLAGS_TF }
    };
    core_machine_run_result result;

    if (state == LIB_NULL || test_case == LIB_NULL) return 0;
    config.cpu_profile = test_case->profile;
    lib_memory_set(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine, LIB_NULL) != LIB_STATUS_OK ||
        core_machine_freeze_execution_providers(state->machine) != LIB_STATUS_OK ||
        core_machine_reset(state->machine) != LIB_STATUS_OK ||
        core_machine_debug_patch_registers(state->machine, &setup) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, 0x0300u, load_idtr,
            sizeof(load_idtr)) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, 0x0400u, idtr,
            sizeof(idtr)) != LIB_STATUS_OK ||
        core_machine_run(state->machine, (core_machine_run_budget){1u, 0u},
            &result) != LIB_STATUS_OK || result.executed != 1u ||
        result.reason != CORE_MACHINE_STOP_BUDGET ||
        core_machine_debug_patch_registers(state->machine, &entry) != LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, REAL_UD_CODE_OFFSET,
            test_case->program, test_case->bytes) !=
            LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, REAL_UD_VECTOR * 4u,
            &handler_offset, sizeof(handler_offset)) !=
            LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, REAL_UD_VECTOR * 4u + 2u,
            &handler_segment, sizeof(handler_segment)) !=
            LIB_STATUS_OK ||
        core_machine_memory_write(state->machine, REAL_UD_HANDLER_OFFSET,
            handler, sizeof(handler)) != LIB_STATUS_OK) {
        core_machine_destroy(state->machine);
        state->machine = LIB_NULL;
        return 0;
    }
    return 1;
}

static lib_i32 real_ud_test_delivery_case(const real_ud_case *test_case)
{
    real_ud_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result = {0};
    lib_u16 frame[3] = { 0u, 0u, 0u };
    core_machine_debug_cpu_snapshot before = {0};
    core_machine_debug_cpu_snapshot after = {0};
    lib_i32 failed = !real_ud_prepare(&state, test_case,
        REAL_UD_VECTOR * 4u + 3u);

    if (!failed) {
        failed |= core_machine_debug_capture_cpu_snapshot(state.machine,
            CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK ||
            core_machine_run(state.machine,
                (core_machine_run_budget){ 1u, 0u }, &result) != LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_BUDGET ||
            diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_UD) || after.eip !=
            REAL_UD_HANDLER_OFFSET || after.esp !=
            ((before.esp & 0xffff0000u) |
                (lib_u16)(before.esp - 6u)) ||
            after.eflags != (before.eflags &
                ~(CORE_MACHINE_DEBUG_EFLAGS_IF | CORE_MACHINE_DEBUG_EFLAGS_TF)) ||
            core_machine_debug_read_linear(state.machine,
                after.ss.base + (lib_u16)after.esp,
                frame, sizeof(frame)) != LIB_STATUS_OK || frame[0] !=
            REAL_UD_CODE_OFFSET || frame[1] != before.cs.selector ||
            frame[2] != (lib_u16)((before.eflags &
                0x00037fd5u) | 0x02u);
    }
    if (!failed) {
        failed |= core_machine_run(state.machine,
                (core_machine_run_budget){ 2u, 0u }, &result) != LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_WAITING_FOR_INTERRUPT ||
            after.eip != REAL_UD_HANDLER_OFFSET + 2u ||
            after.eax != before.eax + 1u;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

static lib_i32 real_ud_test_delivery(void)
{
    static const lib_u8 primary[] = { 0xf1u };
    static const lib_u8 escape[] = { 0x0fu, 0xffu };
    static const lib_u8 operand[] = { 0x62u, 0xc0u };
    static const lib_u8 profile[] = { 0x66u, 0x90u };
    static const lib_u8 lock[] = { 0xf0u, 0x90u };
    static const real_ud_case cases[] = {
        { primary, sizeof(primary), CORE_MACHINE_CPU_PROFILE_80386 },
        { escape, sizeof(escape), CORE_MACHINE_CPU_PROFILE_80386 },
        { operand, sizeof(operand), CORE_MACHINE_CPU_PROFILE_80386 },
        { profile, sizeof(profile), CORE_MACHINE_CPU_PROFILE_80286 },
        { lock, sizeof(lock), CORE_MACHINE_CPU_PROFILE_80386 }
    };
    lib_size index;

    for (index = 0u; index < sizeof(cases) / sizeof(cases[0]); ++index)
        if (!real_ud_test_delivery_case(&cases[index])) return 0;
    return 1;
}

static lib_i32 real_ud_test_delivery_failure(void)
{
    static const lib_u8 program[] = { 0xf1u };
    const real_ud_case test_case = {
        program, sizeof(program), CORE_MACHINE_CPU_PROFILE_80386
    };
    real_ud_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_run_result result = {0};
    core_machine_debug_cpu_snapshot before = {0};
    core_machine_debug_cpu_snapshot after = {0};
    lib_i32 failed = !real_ud_prepare(&state, &test_case,
        REAL_UD_VECTOR * 4u - 1u);

    if (!failed) {
        failed |= core_machine_debug_write_register(state.machine,
            CORE_MACHINE_DEBUG_EFLAGS, CORE_MACHINE_DEBUG_EFLAGS_CF |
            CORE_MACHINE_DEBUG_EFLAGS_IF) != LIB_STATUS_OK ||
            core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &before) != LIB_STATUS_OK ||
            core_machine_run(state.machine,
                (core_machine_run_budget){ 1u, 0u }, &result) != LIB_STATUS_INTERNAL_ERROR ||
            core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &after) != LIB_STATUS_OK ||
            core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != LIB_STATUS_OK ||
            result.reason != CORE_MACHINE_STOP_FAULT ||
            !diagnostic.first_fault.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.first_fault.exception_mask, VCPUINS_EXCEPT_UD) ||
            diagnostic.last_delivered_exception.valid || after.eip !=
            before.eip || after.esp != before.esp ||
            after.eax != before.eax || after.eflags !=
            before.eflags || lib_memory_compare(&after.cs, &before.cs,
                sizeof(after.cs)) != 0 || lib_memory_compare(&after.ss,
                &before.ss, sizeof(after.ss)) != 0;
    }
    core_machine_destroy(state.machine);
    return !failed;
}

lib_i32 main(void)
{
    if (!real_ud_test_delivery() || !real_ud_test_delivery_failure()) return 1;
    printf("M5:T337:S1:REAL-UD-DELIVERY:OK\n");
    return 0;
}
