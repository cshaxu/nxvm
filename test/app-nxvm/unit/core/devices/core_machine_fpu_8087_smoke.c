#include "lib/types/types_interface.h"
#include <stdio.h>
#include "app-nxvm/devices/device_support.h"

#include "app-nxvm/devices/debug_interface.h"
#include "x86/chips/fpu/fpu_interface.h"
#include "app-nxvm/devices/machine_board_interface.h"
#include "support/core_machine_board_fixture.h"

#define FPU_TEST_ONE 0x00000100u
#define FPU_TEST_ZERO 0x00000104u
#define FPU_TEST_RESULT 0x00000108u
#define FPU_TEST_CONTROL 0x00000110u

typedef struct fpu_test_machine {
    core_machine *machine;
    lib_status reset_status;
} fpu_test_machine;

static void fpu_test_reset(void *opaque)
{
    fpu_test_machine *state = (fpu_test_machine *)opaque;

    const core_machine_debug_register_patch entry = {
        .mask = CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_CS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_DS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_ES) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_SS) |
            CORE_MACHINE_DEBUG_REGISTER_MASK(CORE_MACHINE_DEBUG_EIP)
    };
    if (state != LIB_NULL) state->reset_status =
        core_machine_cpu_debug_patch_registers(
            state->machine->executor_cpu_execution, &entry);
}

static const core_machine_execution_provider fpu_test_provider = {
    fpu_test_reset, LIB_NULL
};

static lib_i32 fpu_test_prepare(fpu_test_machine *state,
    core_machine_cpu_profile cpu_profile, x86_fpu_profile fpu_profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = cpu_profile,
        .fpu_profile = fpu_profile
    };

    lib_memory_set(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != LIB_STATUS_OK) return 0;
    if (!test_core_machine_fixture_bind_freeze_reset(state->machine,
            &fpu_test_provider, state) || state->reset_status != LIB_STATUS_OK) {
        printf("FPU prepare profile=%u reset=%u\n", cpu_profile, state->reset_status);
        core_machine_destroy(state->machine);
        state->machine = LIB_NULL;
        return 0;
    }
    return 1;
}

static lib_i32 fpu_test_write(fpu_test_machine *state, lib_u32 physical,
    const void *data, lib_size size)
{
    return core_machine_memory_write(state->machine, physical, data, size) ==
        LIB_STATUS_OK;
}

static lib_i32 fpu_test_run(fpu_test_machine *state, lib_u64 instructions,
    lib_status expected_status, core_machine_cpu_diagnostic *out_diagnostic)
{
    core_machine_run_budget budget = { instructions, 0u };
    core_machine_run_result result;

    if (core_machine_run(state->machine, budget, &result) != expected_status) {
        printf("FPU run reason=%u detail=%u pc=%x\n", result.reason,
            result.detail, result.linear_pc);
        return 0;
    }
    return out_diagnostic == LIB_NULL || core_machine_get_cpu_diagnostic(
        state->machine, out_diagnostic) == LIB_STATUS_OK;
}

static lib_i32 test_arithmetic_and_fninit(void)
{
    static const lib_u8 program[] = {
        0xd9u, 0x06u, 0x00u, 0x01u, /* FLD dword [0100] */
        0xd9u, 0x06u, 0x04u, 0x01u, /* FLD dword [0104] */
        0xd8u, 0xc1u,             /* FADD ST(0), ST(1) */
        0xd9u, 0x1eu, 0x08u, 0x01u, /* FSTP dword [0108] */
        0x9bu, 0xf4u
    };
    const lib_u32 first = 0x3fc00000u;  /* 1.5 */
    const lib_u32 second = 0x40100000u; /* 2.25 */
    const lib_u32 expected = 0x40700000u; /* 3.75 */
    fpu_test_machine state;
    x86_fpu_state fpu_state;
    lib_u32 observed = 0u;
    lib_i32 failed = !fpu_test_prepare(&state, CORE_MACHINE_CPU_PROFILE_8086,
        X86_FPU_PROFILE_8087);

    if (!failed) {
        failed |= !fpu_test_write(&state, 0u, program, sizeof(program));
        failed |= !fpu_test_write(&state, FPU_TEST_ONE, &first, sizeof(first));
        failed |= !fpu_test_write(&state, FPU_TEST_ZERO, &second, sizeof(second));
        failed |= !fpu_test_run(&state, 6u, LIB_STATUS_OK, LIB_NULL);
        failed |= core_machine_memory_read(state.machine, FPU_TEST_RESULT,
            &observed, sizeof(observed)) != LIB_STATUS_OK || observed != expected;
        failed |= core_machine_get_fpu_state(state.machine, &fpu_state) !=
            LIB_STATUS_OK || fpu_state.control_word != 0x037fu ||
            (fpu_state.status_word & 0x00ffu) != 0u ||
            fpu_state.pending_unmasked_exception ||
            fpu_state.tags[fpu_state.top] != X86_FPU_TAG_VALID;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 test_stack_fault_and_reset(void)
{
    static const lib_u8 fld[] = { 0xd9u, 0x06u, 0x00u, 0x01u };
    static const lib_u8 fninit[] = { 0xdbu, 0xe3u };
    const lib_u32 one = 0x3f800000u;
    fpu_test_machine state;
    x86_fpu_state fpu_state;
    lib_i32 failed = !fpu_test_prepare(&state, CORE_MACHINE_CPU_PROFILE_8086,
        X86_FPU_PROFILE_8087);

    if (!failed) {
        for (lib_u8 index = 0u; index < 9u; ++index) {
            failed |= !fpu_test_write(&state, (lib_u32)index * 4u, fld, sizeof(fld));
        }
        failed |= !fpu_test_write(&state, FPU_TEST_ONE, &one, sizeof(one));
        failed |= !fpu_test_run(&state, 9u, LIB_STATUS_OK, LIB_NULL);
        failed |= core_machine_get_fpu_state(state.machine, &fpu_state) !=
            LIB_STATUS_OK || (fpu_state.status_word & 0x0041u) != 0x0041u;
        failed |= !fpu_test_write(&state, 36u, fninit, sizeof(fninit));
        failed |= !fpu_test_run(&state, 1u, LIB_STATUS_OK, LIB_NULL);
        failed |= core_machine_get_fpu_state(state.machine, &fpu_state) !=
            LIB_STATUS_OK || fpu_state.status_word != 0u ||
            fpu_state.control_word != 0x037fu ||
            fpu_state.tags[0] != X86_FPU_TAG_EMPTY;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 test_unmasked_fwait(void)
{
    static const lib_u8 program[] = {
        0xd9u, 0x2eu, 0x10u, 0x01u, /* FLDCW word [0110] */
        0xd9u, 0x06u, 0x04u, 0x01u, /* FLD dword [0104] */
        0xd9u, 0x06u, 0x00u, 0x01u, /* FLD dword [0100] */
        0xd8u, 0xf1u,             /* FDIV ST(0), ST(1) */
        0x9bu
    };
    const lib_u32 one = 0x3f800000u;
    const lib_u32 zero = 0u;
    const lib_u16 unmask_zero_divide = 0x037bu;
    static const lib_u8 handler[] = { 0xf4u };
    const lib_u16 handler_offset = 0x0200u;
    const lib_u16 handler_segment = 0u;
    lib_u16 frame[3] = { 0u, 0u, 0u };
    fpu_test_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_debug_cpu_snapshot cpu;
    x86_fpu_state fpu_state;
    lib_i32 failed = !fpu_test_prepare(&state, CORE_MACHINE_CPU_PROFILE_8086,
        X86_FPU_PROFILE_8087);

    if (!failed) {
        failed |= !fpu_test_write(&state, 0u, program, sizeof(program));
        failed |= !fpu_test_write(&state, FPU_TEST_ONE, &one, sizeof(one));
        failed |= !fpu_test_write(&state, FPU_TEST_ZERO, &zero, sizeof(zero));
        failed |= !fpu_test_write(&state, FPU_TEST_CONTROL, &unmask_zero_divide,
            sizeof(unmask_zero_divide));
        failed |= core_machine_debug_write_register(state.machine,
            CORE_MACHINE_DEBUG_ESP, 0x00008000u) != LIB_STATUS_OK;
        failed |= !fpu_test_write(&state, 0x0040u, &handler_offset,
            sizeof(handler_offset)) || !fpu_test_write(&state, 0x0042u,
            &handler_segment, sizeof(handler_segment)) || !fpu_test_write(
            &state, handler_offset, handler, sizeof(handler));
        failed |= !fpu_test_run(&state, 5u, LIB_STATUS_OK, &diagnostic);
        failed |= diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid || !CORE_MACHINE_BIT_IS_SET(
                diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_MF) ||
            core_machine_debug_capture_cpu_snapshot(state.machine,
                CORE_MACHINE_CPU_SNAPSHOT_CURRENT, &cpu) != LIB_STATUS_OK ||
            cpu.eip != handler_offset || cpu.esp != 0x00007ffau ||
            core_machine_debug_read_linear(state.machine, 0x00007ffau,
                frame, sizeof(frame)) != LIB_STATUS_OK ||
            frame[0] != 14u || frame[1] != 0u;
        failed |= core_machine_get_fpu_state(state.machine, &fpu_state) !=
            LIB_STATUS_OK || (fpu_state.status_word & 0x0084u) != 0x0084u ||
            !fpu_state.pending_unmasked_exception;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static lib_i32 test_profile_gates(void)
{
    static const lib_u8 fninit[] = { 0xdbu, 0xe3u };
    static const lib_u8 unsupported_m32[] = { 0xd9u, 0x06u, 0x00u, 0x01u };
    const lib_u32 nan = 0x7fc00000u;
    const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    x86_fpu_operation_metadata metadata;
    lib_i32 failed = 0;

    metadata = x86_fpu_operation_metadata_get(0xd9u, 0x06u);
    failed |= !metadata.valid ||
        metadata.minimum_fpu != X86_FPU_PROFILE_8087 ||
        metadata.operation != X86_FPU_OPERATION_FLD_M32;
    for (lib_u8 index = 0u; index < 2u; ++index) {
        fpu_test_machine state;
        failed |= !fpu_test_prepare(&state, profiles[index],
            X86_FPU_PROFILE_8087);
        if (state.machine != LIB_NULL) {
            failed |= !fpu_test_write(&state, 0u, fninit, sizeof(fninit));
            failed |= !fpu_test_run(&state, 1u, LIB_STATUS_OK, LIB_NULL);
        }
        core_machine_destroy(state.machine);
    }
    {
        fpu_test_machine state;
        failed |= !fpu_test_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386,
            X86_FPU_PROFILE_80287);
        if (state.machine != LIB_NULL) {
            failed |= !fpu_test_write(&state, 0u, fninit, sizeof(fninit));
            failed |= !fpu_test_run(&state, 1u, LIB_STATUS_OK, LIB_NULL) ||
                x86_fpu_ticks_until_completion(state.machine->fpu,
                    &(lib_u64){0}) != LIB_STATUS_OK;
        }
        core_machine_destroy(state.machine);
    }
    {
        fpu_test_machine state;
        failed |= !fpu_test_prepare(&state, CORE_MACHINE_CPU_PROFILE_8086,
            X86_FPU_PROFILE_8087);
        if (state.machine != LIB_NULL) {
            failed |= !fpu_test_write(&state, 0u, unsupported_m32,
                sizeof(unsupported_m32));
            failed |= !fpu_test_write(&state, FPU_TEST_ONE, &nan, sizeof(nan));
            failed |= !fpu_test_run(&state, 1u, LIB_STATUS_OK, LIB_NULL) ||
                x86_fpu_ticks_until_completion(state.machine->fpu,
                    &(lib_u64){0}) != LIB_STATUS_OK;
        }
        core_machine_destroy(state.machine);
    }
    return failed;
}

lib_i32 main(void)
{
    const lib_i32 arithmetic = test_arithmetic_and_fninit();
    const lib_i32 stack = test_stack_fault_and_reset();
    const lib_i32 wait = test_unmasked_fwait();
    const lib_i32 profiles = test_profile_gates();

    if (arithmetic || stack || wait || profiles) {
        printf("FPU-8087 arithmetic=%d stack=%d wait=%d profiles=%d\n",
            arithmetic, stack, wait, profiles);
        return 1;
    }
    printf("M5:T262:S3:FPU-8087:OK\n");
    return 0;
}
