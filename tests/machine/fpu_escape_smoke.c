#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "core/machine/memory.h"
#include "../support/core_machine_cpu_fixture.h"

typedef struct fpu_escape_machine {
    core_machine *machine;
} fpu_escape_machine;

static C_VOID fpu_escape_reset(C_VOID *opaque)
{
    fpu_escape_machine *state = (fpu_escape_machine *)opaque;

    if (state != STD_NULL) (C_VOID)test_core_machine_fixture_reset_real_mode(
        state->machine);
}

static const core_machine_execution_provider fpu_escape_provider = {
    fpu_escape_reset, STD_NULL, STD_NULL
};

static C_INT prepare_machine(core_machine_fpu_profile fpu_profile,
    uint32_t cr0, fpu_escape_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = fpu_profile
    };
    if (state == STD_NULL) return 1;
    STD_MEMSET(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK) return 1;
    if (core_machine_bind_execution_provider(state->machine, &fpu_escape_provider,
            state) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(state->machine) != TYPE_STATUS_OK ||
        core_machine_reset(state->machine) != TYPE_STATUS_OK) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 1;
    }
    (C_VOID)test_core_machine_fixture_set_control_zero(state->machine, cr0);
    return 0;
}

static C_INT run_case(const C_UCHAR *program, STD_SIZE_T program_size,
    core_machine_fpu_profile fpu_profile, uint32_t cr0, uint32_t expected_exception,
    uint32_t expected_eip)
{
    fpu_escape_machine state;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    core_machine_cpu_state cpu;
    C_INT failed = prepare_machine(fpu_profile, cr0, &state);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0u, program,
            program_size) != TYPE_STATUS_OK;
        if (expected_exception != 0u) {
            failed |= core_machine_run(state.machine, budget, &result) != TYPE_STATUS_FAULT;
        } else {
            failed |= core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK;
        }
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK;
        failed |= core_machine_get_cpu_state(state.machine, &cpu) != TYPE_STATUS_OK;
        if (expected_exception != 0u) {
            failed |= !diagnostic.first_fault.valid ||
                !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                    expected_exception) ||
                TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_UD);
        } else {
            failed |= diagnostic.first_fault.valid || cpu.eip != expected_eip;
        }
    }
    core_machine_destroy(state.machine);
    return failed;
}

C_INT main(C_VOID)
{
    static const C_UCHAR fninit[] = { 0xdbu, 0xe3u };
    static const C_UCHAR memory_escape[] = { 0xd8u, 0x06u, 0x34u, 0x12u };
    static const C_UCHAR fwait[] = { 0x9bu };
    C_INT failed = 0;

    failed |= run_case(fninit, sizeof(fninit), CORE_MACHINE_FPU_PROFILE_NONE,
        0u, 0u, 2u);
    failed |= run_case(memory_escape, sizeof(memory_escape),
        CORE_MACHINE_FPU_PROFILE_NONE, 0u, 0u, 4u);
    failed |= run_case(fninit, sizeof(fninit), CORE_MACHINE_FPU_PROFILE_NONE,
        VCPU_CR0_EM, VCPUINS_EXCEPT_NM, 0u);
    failed |= run_case(fwait, sizeof(fwait), CORE_MACHINE_FPU_PROFILE_NONE,
        VCPU_CR0_TS, 0u, 1u);
    failed |= run_case(fwait, sizeof(fwait), CORE_MACHINE_FPU_PROFILE_NONE,
        VCPU_CR0_TS | VCPU_CR0_MP, VCPUINS_EXCEPT_NM, 0u);
    failed |= run_case(fninit, sizeof(fninit), CORE_MACHINE_FPU_PROFILE_8087,
        0u, 0u, 2u);
    if (failed) return 1;
    STD_PRINTF("M5:T156:S1:FPU-ESC:OK\n");
    return 0;
}
