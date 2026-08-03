#include "type.h"

#include "core/machine/cpu.h"
#include "core/machine/machine_interface.h"
#include "core/machine/memory.h"

static C_INT prepare_machine(core_machine_cpu_profile profile,
    core_machine **out_machine, t_cpu **out_cpu)
{
    const core_machine_config config = {
        .memory_bytes = CORE_MACHINE_MINIMUM_MEMORY_BYTES,
        .cpu_profile = profile,
        .fpu_profile = CORE_MACHINE_FPU_PROFILE_NONE
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
    *out_cpu = cpu;
    return 0;
}

static C_INT run_case(core_machine_cpu_profile profile, const C_UCHAR *program,
    STD_SIZE_T program_size, C_INT expect_ud)
{
    core_machine *machine = STD_NULL;
    t_cpu *cpu = STD_NULL;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    C_INT failed = prepare_machine(profile, &machine, &cpu);

    if (!failed) {
        core_machine_memory_write_real_to(core_machine_executor_memory_borrow(machine),
            0u, 0u, program, program_size);
        failed |= core_machine_run(machine, budget, &result) != NTVDM64_STATUS_OK;
        failed |= core_machine_get_cpu_diagnostic(machine, &diagnostic) != NTVDM64_STATUS_OK;
        if (expect_ud) {
            failed |= !diagnostic.first_fault.valid ||
                !NTVDM64_TYPE_GET_BIT(diagnostic.first_fault.exception_mask,
                    VCPUINS_EXCEPT_UD) ||
                diagnostic.first_fault.point.bytes[0] != program[0];
        } else {
            failed |= diagnostic.first_fault.valid;
        }
    }
    core_machine_destroy(machine);
    return failed;
}

static C_INT run_pop_cs_8086(C_VOID)
{
    static const C_UCHAR program[] = { 0x0fu };
    C_UCHAR selector[2] = { 0u, 0u };
    core_machine *machine = STD_NULL;
    t_cpu *cpu = STD_NULL;
    core_machine_run_budget budget = { 1u, 0u };
    core_machine_run_result result;
    core_machine_cpu_diagnostic diagnostic;
    C_INT failed = prepare_machine(CORE_MACHINE_CPU_PROFILE_8086, &machine, &cpu);

    if (!failed) {
        cpu->data.sp = 0x0100u;
        core_machine_memory_write_real_to(core_machine_executor_memory_borrow(machine),
            0u, 0u, program, sizeof(program));
        core_machine_memory_write_real_to(core_machine_executor_memory_borrow(machine),
            0u, 0x0100u, selector, sizeof(selector));
        failed |= core_machine_run(machine, budget, &result) != NTVDM64_STATUS_OK;
        failed |= core_machine_get_cpu_diagnostic(machine, &diagnostic) != NTVDM64_STATUS_OK;
        failed |= diagnostic.first_fault.valid || cpu->data.cs.selector != 0u ||
            cpu->data.eip != 1u || cpu->data.sp != 0x0102u;
    }
    core_machine_destroy(machine);
    return failed;
}

C_INT main(C_VOID)
{
    static const C_UCHAR nop[] = { 0x90u };
    static const C_UCHAR pusha[] = { 0x60u };
    static const C_UCHAR arpl[] = { 0x63u, 0xc0u };
    static const C_UCHAR fs_prefix[] = { 0x64u, 0x90u };
    static const C_UCHAR jcc_near[] = { 0x0fu, 0x80u, 0u, 0u };
    C_INT failed = 0;

    failed |= run_case(CORE_MACHINE_CPU_PROFILE_8086, nop, sizeof(nop), 0);
    failed |= run_pop_cs_8086();
    failed |= run_case(CORE_MACHINE_CPU_PROFILE_8086, pusha, sizeof(pusha), 1);
    failed |= run_case(CORE_MACHINE_CPU_PROFILE_80186, arpl, sizeof(arpl), 1);
    failed |= run_case(CORE_MACHINE_CPU_PROFILE_80286, fs_prefix, sizeof(fs_prefix), 1);
    failed |= run_case(CORE_MACHINE_CPU_PROFILE_80286, jcc_near, sizeof(jcc_near), 1);
    if (failed) return 1;
    STD_PRINTF("M5:T155:S1:CPU-PROFILE-GATE:OK\n");
    return 0;
}
