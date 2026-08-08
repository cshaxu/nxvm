#ifndef TEST_CORE_MACHINE_CPU_FIXTURE_H
#define TEST_CORE_MACHINE_CPU_FIXTURE_H

/* Private prepared-state operations for CPU execution corpus fixtures. */
#include "core/machine/machine.h"

static C_INT test_core_machine_fixture_reset_real_mode(core_machine *machine)
{
    t_cpu *cpu;
    core_machine_cpu_execution_context *execution;

    if (machine == STD_NULL) return 0;
    cpu = &machine->executor_cpu;
    execution = &machine->executor_cpu_execution;
    return core_machine_cpu_execution_load_segment(execution, &cpu->data.cs, 0u) == 0 &&
        core_machine_cpu_execution_load_segment(execution, &cpu->data.ds, 0u) == 0 &&
        core_machine_cpu_execution_load_segment(execution, &cpu->data.es, 0u) == 0 &&
        core_machine_cpu_execution_load_segment(execution, &cpu->data.ss, 0u) == 0 &&
        ((cpu->data.eip = 0u), 1);
}

static C_INT test_core_machine_fixture_set_control_zero(
    core_machine *machine, uint32_t value)
{
    if (machine == STD_NULL) return 0;
    machine->executor_cpu.data.cr0 = value;
    return 1;
}

static t_cpu test_core_machine_fixture_capture_cpu_after_run(
    core_machine *machine)
{
    t_cpu observation = {0};

    if (machine != STD_NULL) observation = machine->executor_cpu;
    return observation;
}

static type_status test_core_machine_fixture_register_reset_mapping(
    core_machine *machine, uint32_t linear, uint32_t physical,
    STD_SIZE_T bytes)
{
    return machine == STD_NULL ? TYPE_STATUS_INVALID_ARGUMENT :
        core_machine_memory_register_mapping(&machine->executor_memory, linear,
            physical, bytes);
}

#endif
