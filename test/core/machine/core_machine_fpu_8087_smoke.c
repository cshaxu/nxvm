#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/cpu_instructions.h"
#include "core/machine/fpu_interface.h"
#include "core/machine/machine_interface.h"
#include "../support/core_machine_cpu_fixture.h"

#define FPU_TEST_ONE 0x00000100u
#define FPU_TEST_ZERO 0x00000104u
#define FPU_TEST_RESULT 0x00000108u
#define FPU_TEST_CONTROL 0x00000110u

typedef struct fpu_test_machine {
    core_machine *machine;
} fpu_test_machine;

static C_VOID fpu_test_reset(C_VOID *opaque)
{
    fpu_test_machine *state = (fpu_test_machine *)opaque;

    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider fpu_test_provider = {
    fpu_test_reset, STD_NULL
};

static C_INT fpu_test_prepare(fpu_test_machine *state,
    core_machine_cpu_profile cpu_profile, core_machine_fpu_profile fpu_profile)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = cpu_profile,
        .fpu_profile = fpu_profile
    };

    STD_MEMSET(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK) return 0;
    if (!test_core_machine_fixture_bind_freeze_reset(state->machine,
            &fpu_test_provider, state)) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 0;
    }
    return 1;
}

static C_INT fpu_test_write(fpu_test_machine *state, type_unsigned_32 physical,
    const C_VOID *data, STD_SIZE_T size)
{
    return core_machine_memory_write(state->machine, physical, data, size) ==
        TYPE_STATUS_OK;
}

static C_INT fpu_test_run(fpu_test_machine *state, type_unsigned_64 instructions,
    type_status expected_status, core_machine_cpu_diagnostic *out_diagnostic)
{
    core_machine_run_budget budget = { instructions, 0u };
    core_machine_run_result result;

    if (core_machine_run(state->machine, budget, &result) != expected_status) return 0;
    return out_diagnostic == STD_NULL || core_machine_get_cpu_diagnostic(
        state->machine, out_diagnostic) == TYPE_STATUS_OK;
}

static C_INT test_arithmetic_and_fninit(C_VOID)
{
    static const type_unsigned_8 program[] = {
        0xd9u, 0x06u, 0x00u, 0x01u, /* FLD dword [0100] */
        0xd9u, 0x06u, 0x04u, 0x01u, /* FLD dword [0104] */
        0xd8u, 0xc1u,             /* FADD ST(0), ST(1) */
        0xd9u, 0x1eu, 0x08u, 0x01u, /* FSTP dword [0108] */
        0x9bu, 0xf4u
    };
    const type_unsigned_32 first = 0x3fc00000u;  /* 1.5 */
    const type_unsigned_32 second = 0x40100000u; /* 2.25 */
    const type_unsigned_32 expected = 0x40700000u; /* 3.75 */
    fpu_test_machine state;
    core_machine_fpu_state fpu_state;
    type_unsigned_32 observed = 0u;
    C_INT failed = !fpu_test_prepare(&state, CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_FPU_PROFILE_8087);

    if (!failed) {
        failed |= !fpu_test_write(&state, 0u, program, sizeof(program));
        failed |= !fpu_test_write(&state, FPU_TEST_ONE, &first, sizeof(first));
        failed |= !fpu_test_write(&state, FPU_TEST_ZERO, &second, sizeof(second));
        failed |= !fpu_test_run(&state, 6u, TYPE_STATUS_OK, STD_NULL);
        failed |= core_machine_memory_read(state.machine, FPU_TEST_RESULT,
            &observed, sizeof(observed)) != TYPE_STATUS_OK || observed != expected;
        failed |= core_machine_get_fpu_state(state.machine, &fpu_state) !=
            TYPE_STATUS_OK || fpu_state.control_word != 0x037fu ||
            (fpu_state.status_word & 0x00ffu) != 0u ||
            fpu_state.pending_unmasked_exception ||
            fpu_state.tags[fpu_state.top] != CORE_MACHINE_FPU_TAG_VALID;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT test_stack_fault_and_reset(C_VOID)
{
    static const type_unsigned_8 fld[] = { 0xd9u, 0x06u, 0x00u, 0x01u };
    static const type_unsigned_8 fninit[] = { 0xdbu, 0xe3u };
    const type_unsigned_32 one = 0x3f800000u;
    fpu_test_machine state;
    core_machine_fpu_state fpu_state;
    C_INT failed = !fpu_test_prepare(&state, CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_FPU_PROFILE_8087);

    if (!failed) {
        for (type_unsigned_8 index = 0u; index < 9u; ++index) {
            failed |= !fpu_test_write(&state, (type_unsigned_32)index * 4u, fld, sizeof(fld));
        }
        failed |= !fpu_test_write(&state, FPU_TEST_ONE, &one, sizeof(one));
        failed |= !fpu_test_run(&state, 9u, TYPE_STATUS_OK, STD_NULL);
        failed |= core_machine_get_fpu_state(state.machine, &fpu_state) !=
            TYPE_STATUS_OK || (fpu_state.status_word & 0x0041u) != 0x0041u;
        failed |= !fpu_test_write(&state, 36u, fninit, sizeof(fninit));
        failed |= !fpu_test_run(&state, 1u, TYPE_STATUS_OK, STD_NULL);
        failed |= core_machine_get_fpu_state(state.machine, &fpu_state) !=
            TYPE_STATUS_OK || fpu_state.status_word != 0u ||
            fpu_state.control_word != 0x037fu ||
            fpu_state.tags[0] != CORE_MACHINE_FPU_TAG_EMPTY;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT test_unmasked_fwait(C_VOID)
{
    static const type_unsigned_8 program[] = {
        0xd9u, 0x2eu, 0x10u, 0x01u, /* FLDCW word [0110] */
        0xd9u, 0x06u, 0x04u, 0x01u, /* FLD dword [0104] */
        0xd9u, 0x06u, 0x00u, 0x01u, /* FLD dword [0100] */
        0xd8u, 0xf1u,             /* FDIV ST(0), ST(1) */
        0x9bu
    };
    const type_unsigned_32 one = 0x3f800000u;
    const type_unsigned_32 zero = 0u;
    const type_unsigned_16 unmask_zero_divide = 0x037bu;
    static const type_unsigned_8 handler[] = { 0xf4u };
    const type_unsigned_16 handler_offset = 0x0200u;
    const type_unsigned_16 handler_segment = 0u;
    type_unsigned_16 frame[3] = { 0u, 0u, 0u };
    fpu_test_machine state;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_fpu_state fpu_state;
    C_INT failed = !fpu_test_prepare(&state, CORE_MACHINE_CPU_PROFILE_8086,
        CORE_MACHINE_FPU_PROFILE_8087);

    if (!failed) {
        failed |= !fpu_test_write(&state, 0u, program, sizeof(program));
        failed |= !fpu_test_write(&state, FPU_TEST_ONE, &one, sizeof(one));
        failed |= !fpu_test_write(&state, FPU_TEST_ZERO, &zero, sizeof(zero));
        failed |= !fpu_test_write(&state, FPU_TEST_CONTROL, &unmask_zero_divide,
            sizeof(unmask_zero_divide));
        state.machine->executor_cpu.data.esp = 0x00008000u;
        failed |= !fpu_test_write(&state, 0x0040u, &handler_offset,
            sizeof(handler_offset)) || !fpu_test_write(&state, 0x0042u,
            &handler_segment, sizeof(handler_segment)) || !fpu_test_write(
            &state, handler_offset, handler, sizeof(handler));
        failed |= !fpu_test_run(&state, 5u, TYPE_STATUS_OK, &diagnostic);
        failed |= diagnostic.first_fault.valid ||
            !diagnostic.last_delivered_exception.valid || !TYPE_GET_BIT(
                diagnostic.last_delivered_exception.exception_mask,
                VCPUINS_EXCEPT_MF) || state.machine->executor_cpu.data.eip !=
                handler_offset || state.machine->executor_cpu.data.esp !=
                0x00007ffau || !test_core_machine_fixture_read_linear(
                state.machine, 0x00007ffau, TYPE_REFERENCE_OF(frame),
                sizeof(frame)) || frame[0] != 14u || frame[1] != 0u;
        failed |= core_machine_get_fpu_state(state.machine, &fpu_state) !=
            TYPE_STATUS_OK || (fpu_state.status_word & 0x0084u) != 0x0084u ||
            !fpu_state.pending_unmasked_exception;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT test_profile_gates(C_VOID)
{
    static const type_unsigned_8 fninit[] = { 0xdbu, 0xe3u };
    static const type_unsigned_8 unsupported_m32[] = { 0xd9u, 0x06u, 0x00u, 0x01u };
    const type_unsigned_32 nan = 0x7fc00000u;
    const core_machine_cpu_profile profiles[] = {
        CORE_MACHINE_CPU_PROFILE_8086, CORE_MACHINE_CPU_PROFILE_80186,
        CORE_MACHINE_CPU_PROFILE_80286, CORE_MACHINE_CPU_PROFILE_80386
    };
    core_machine_fpu_operation_metadata metadata;
    C_INT failed = 0;

    metadata = core_machine_fpu_operation_metadata_get(0xd9u, 0x06u);
    failed |= !metadata.valid || metadata.minimum_cpu != CORE_MACHINE_CPU_PROFILE_8086 ||
        metadata.minimum_fpu != CORE_MACHINE_FPU_PROFILE_8087 ||
        metadata.operation != CORE_MACHINE_FPU_OPERATION_FLD_M32;
    for (type_unsigned_8 index = 0u; index < 2u; ++index) {
        fpu_test_machine state;
        failed |= !fpu_test_prepare(&state, profiles[index],
            CORE_MACHINE_FPU_PROFILE_8087);
        if (state.machine != STD_NULL) {
            failed |= !fpu_test_write(&state, 0u, fninit, sizeof(fninit));
            failed |= !fpu_test_run(&state, 1u, TYPE_STATUS_OK, STD_NULL);
        }
        core_machine_destroy(state.machine);
    }
    {
        fpu_test_machine state;
        failed |= !fpu_test_prepare(&state, CORE_MACHINE_CPU_PROFILE_80386,
            CORE_MACHINE_FPU_PROFILE_80287);
        if (state.machine != STD_NULL) {
            failed |= !fpu_test_write(&state, 0u, fninit, sizeof(fninit));
            failed |= !fpu_test_run(&state, 1u, TYPE_STATUS_OK, STD_NULL) ||
                !state.machine->fpu.busy;
        }
        core_machine_destroy(state.machine);
    }
    {
        fpu_test_machine state;
        failed |= !fpu_test_prepare(&state, CORE_MACHINE_CPU_PROFILE_8086,
            CORE_MACHINE_FPU_PROFILE_8087);
        if (state.machine != STD_NULL) {
            failed |= !fpu_test_write(&state, 0u, unsupported_m32,
                sizeof(unsupported_m32));
            failed |= !fpu_test_write(&state, FPU_TEST_ONE, &nan, sizeof(nan));
            failed |= !fpu_test_run(&state, 1u, TYPE_STATUS_OK, STD_NULL) ||
                !state.machine->fpu.busy;
        }
        core_machine_destroy(state.machine);
    }
    return failed;
}

C_INT main(C_VOID)
{
    C_INT failed = test_arithmetic_and_fninit() || test_stack_fault_and_reset() ||
        test_unmasked_fwait() || test_profile_gates();

    if (failed) return 1;
    STD_PRINTF("M5:T262:S3:FPU-8087:OK\n");
    return 0;
}
