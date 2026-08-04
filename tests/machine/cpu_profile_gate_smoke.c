#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "core/machine/memory.h"

typedef struct cpu_profile_machine {
    core_machine *machine;
    t_cpu *cpu;
    core_machine_cpu_execution_context *execution;
} cpu_profile_machine;

static C_VOID cpu_profile_machine_reset(C_VOID *opaque)
{
    cpu_profile_machine *state = (cpu_profile_machine *)opaque;

    if (state == STD_NULL || state->cpu == STD_NULL ||
        state->execution == STD_NULL) return;
    (C_VOID)core_machine_cpu_execution_load_segment(state->execution,
        &state->cpu->data.cs, 0u);
    (C_VOID)core_machine_cpu_execution_load_segment(state->execution,
        &state->cpu->data.ds, 0u);
    (C_VOID)core_machine_cpu_execution_load_segment(state->execution,
        &state->cpu->data.es, 0u);
    (C_VOID)core_machine_cpu_execution_load_segment(state->execution,
        &state->cpu->data.ss, 0u);
    state->cpu->data.eip = 0u;
}

static const core_machine_execution_provider cpu_profile_execution_provider = {
    cpu_profile_machine_reset,
    STD_NULL
};

static C_INT prepare_machine(core_machine_cpu_profile profile,
    cpu_profile_machine *state)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
    };
    if (state == STD_NULL) return 1;
    STD_MEMSET(state, 0, sizeof(*state));
    if (core_machine_create(&config, &state->machine) != TYPE_STATUS_OK) return 1;
    state->cpu = core_machine_configuration_cpu_borrow(state->machine);
    state->execution = core_machine_configuration_cpu_execution_borrow(state->machine);
    if (state->cpu == STD_NULL || state->execution == STD_NULL ||
        core_machine_bind_execution_provider(state->machine,
            &cpu_profile_execution_provider, state) != TYPE_STATUS_OK ||
        core_machine_freeze_execution_providers(state->machine) != TYPE_STATUS_OK ||
        core_machine_reset(state->machine) != TYPE_STATUS_OK) {
        core_machine_destroy(state->machine);
        state->machine = STD_NULL;
        return 1;
    }
    return 0;
}

static C_INT run_case(core_machine_cpu_profile profile, const C_UCHAR *program,
    STD_SIZE_T program_size, C_INT expect_ud)
{
    cpu_profile_machine state;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    C_INT failed = prepare_machine(profile, &state);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0u, program,
            program_size) != TYPE_STATUS_OK;
        failed |= core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK;
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
        if (expect_ud) {
            failed |= !diagnostic.first_fault.valid ||
                !TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_UD) ||
                diagnostic.first_fault.point.bytes[0] != program[0];
        } else {
            failed |= diagnostic.first_fault.valid;
        }
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT run_pop_cs_8086(C_VOID)
{
    static const C_UCHAR program[] = { 0xbcu, 0x00u, 0x01u, 0x0fu };
    C_UCHAR selector[2] = { 0u, 0u };
    cpu_profile_machine state;
    core_machine_cpu_state cpu;
    core_machine_run_budget budget = { 2u, 0u };
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    C_INT failed = prepare_machine(CORE_MACHINE_CPU_PROFILE_8086, &state);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0u, program,
            sizeof(program)) != TYPE_STATUS_OK;
        failed |= core_machine_memory_write(state.machine, 0x0100u, selector,
            sizeof(selector)) != TYPE_STATUS_OK;
        failed |= core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK;
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) != TYPE_STATUS_OK;
        failed |= core_machine_get_cpu_state(state.machine, &cpu) != TYPE_STATUS_OK;
        failed |= diagnostic.first_fault.valid || cpu.cs != 0u ||
            cpu.eip != 4u;
    }
    core_machine_destroy(state.machine);
    return failed;
}

static C_INT run_mov_rm8_imm8_8086(C_VOID)
{
    static const C_UCHAR program[] = { 0xc6u, 0x06u, 0xbeu, 0x1fu, 0x01u };
    C_UCHAR value = 0u;
    cpu_profile_machine state;
    core_machine_cpu_state cpu;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    C_INT failed = prepare_machine(CORE_MACHINE_CPU_PROFILE_8086, &state);

    if (!failed) {
        failed |= core_machine_memory_write(state.machine, 0u, program,
            sizeof(program)) != TYPE_STATUS_OK;
        failed |= core_machine_run(state.machine, budget, &result) != TYPE_STATUS_OK;
        failed |= core_machine_memory_read(state.machine, 0x1fbeu,
            &value, sizeof(value)) != TYPE_STATUS_OK;
        failed |= core_machine_get_cpu_diagnostic(state.machine, &diagnostic) !=
            TYPE_STATUS_OK;
        failed |= core_machine_get_cpu_state(state.machine, &cpu) != TYPE_STATUS_OK;
        failed |= diagnostic.first_fault.valid || value != 1u ||
            cpu.eip != sizeof(program);
    }
    core_machine_destroy(state.machine);
    return failed;
}

C_INT main(C_VOID)
{
    static const C_UCHAR nop[] = { 0x90u };
    static const C_UCHAR pusha[] = { 0x60u };
    static const C_UCHAR arpl[] = { 0x63u, 0xc0u };
    static const C_UCHAR fs_prefix[] = { 0x64u, 0x90u };
    static const C_UCHAR jcc_near[] = { 0x0fu, 0x80u, 0u, 0u };
    static const C_UCHAR shift_rm16_imm8[] = { 0xc1u, 0xeau, 0x04u };
    C_INT failed = 0;

    failed |= run_case(CORE_MACHINE_CPU_PROFILE_8086, nop, sizeof(nop), 0);
    failed |= run_pop_cs_8086();
    failed |= run_mov_rm8_imm8_8086();
    failed |= run_case(CORE_MACHINE_CPU_PROFILE_8086, pusha, sizeof(pusha), 1);
    failed |= run_case(CORE_MACHINE_CPU_PROFILE_8086, shift_rm16_imm8,
        sizeof(shift_rm16_imm8), 1);
    failed |= run_case(CORE_MACHINE_CPU_PROFILE_80186, shift_rm16_imm8,
        sizeof(shift_rm16_imm8), 0);
    failed |= run_case(CORE_MACHINE_CPU_PROFILE_80186, arpl, sizeof(arpl), 1);
    failed |= run_case(CORE_MACHINE_CPU_PROFILE_80286, fs_prefix, sizeof(fs_prefix), 1);
    failed |= run_case(CORE_MACHINE_CPU_PROFILE_80286, jcc_near, sizeof(jcc_near), 1);
    if (failed) return 1;
    STD_PRINTF("M5:T155:S1:CPU-PROFILE-GATE:OK\n");
    return 0;
}
