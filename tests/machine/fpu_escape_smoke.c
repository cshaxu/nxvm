#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "core/machine/memory.h"

static C_INT prepare_machine(core_machine_fpu_profile fpu_profile,
    uint32_t cr0, core_machine **out_machine, t_cpu **out_cpu)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = CORE_MACHINE_CPU_PROFILE_80386,
        .fpu_profile = fpu_profile
    };
    core_machine_cpu_execution_context *execution;
    t_cpu *cpu;

    if (core_machine_create(&config, out_machine) != NTVDM64_STATUS_OK ||
        core_machine_freeze_execution_providers(*out_machine) != NTVDM64_STATUS_OK ||
        core_machine_reset(*out_machine) != NTVDM64_STATUS_OK) return 1;
    execution = core_machine_executor_cpu_execution_borrow(*out_machine);
    cpu = core_machine_executor_cpu_borrow(*out_machine);
    if (core_machine_cpu_execution_load_segment(execution, &cpu->data.cs, 0u) ||
        core_machine_cpu_execution_load_segment(execution, &cpu->data.ds, 0u) ||
        core_machine_cpu_execution_load_segment(execution, &cpu->data.es, 0u) ||
        core_machine_cpu_execution_load_segment(execution, &cpu->data.ss, 0u)) return 1;
    cpu->data.eip = 0u;
    cpu->data.cr0 = cr0;
    *out_cpu = cpu;
    return 0;
}

static C_INT run_case(const C_UCHAR *program, STD_SIZE_T program_size,
    core_machine_fpu_profile fpu_profile, uint32_t cr0, uint32_t expected_exception,
    uint32_t expected_eip)
{
    core_machine *machine = STD_NULL;
    t_cpu *cpu = STD_NULL;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    C_INT failed = prepare_machine(fpu_profile, cr0, &machine, &cpu);

    if (!failed) {
        core_machine_memory_write_real_to(
            core_machine_executor_memory_borrow(machine), 0u, 0u, program,
            program_size);
        failed |= core_machine_run(machine, budget, &result) != NTVDM64_STATUS_OK;
        failed |= core_machine_get_cpu_diagnostic(machine, &diagnostic) !=
            NTVDM64_STATUS_OK;
        if (expected_exception != 0u) {
            failed |= !diagnostic.first_fault.valid ||
                !NTVDM64_TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                    expected_exception) ||
                NTVDM64_TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_UD);
        } else {
            failed |= diagnostic.first_fault.valid || cpu->data.eip != expected_eip;
        }
    }
    core_machine_destroy(machine);
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
        0u, VCPUINS_EXCEPT_FPU_UNSUPPORTED, 0u);
    if (failed) return 1;
    STD_PRINTF("M5:T156:S1:FPU-ESC:OK\n");
    return 0;
}
