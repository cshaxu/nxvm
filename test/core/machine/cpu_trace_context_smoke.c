#include "type.h"




#include "core/machine/cpu.h"

#include "core/machine/cpu_instructions.h"

#include "core/machine/memory.h"

#include "core/machine/port.h"

C_INT main(C_VOID)
{
    t_cpu *first_cpu = STD_CALLOC(1u, sizeof(*first_cpu));
    t_cpu *second_cpu = STD_CALLOC(1u, sizeof(*second_cpu));
    t_cpuins *first_instructions = STD_CALLOC(1u, sizeof(*first_instructions));
    t_cpuins *second_instructions = STD_CALLOC(1u, sizeof(*second_instructions));
    t_ram *first_memory = STD_CALLOC(1u, sizeof(*first_memory));
    t_ram *second_memory = STD_CALLOC(1u, sizeof(*second_memory));
    t_port *first_port = STD_CALLOC(1u, sizeof(*first_port));
    t_port *second_port = STD_CALLOC(1u, sizeof(*second_port));
    core_machine_cpu_execution_context first = {0};
    core_machine_cpu_execution_context second = {0};
    C_INT failed = 0;

    if (first_cpu == STD_NULL || second_cpu == STD_NULL || first_instructions == STD_NULL ||
        second_instructions == STD_NULL || first_memory == STD_NULL ||
        second_memory == STD_NULL || first_port == STD_NULL || second_port == STD_NULL) {
        failed = 1;
        goto cleanup;
    }

    core_machine_cpu_execution_context_initialize(
        &first, first_cpu, first_instructions, first_memory, first_port);
    core_machine_cpu_execution_context_initialize(
        &second, second_cpu, second_instructions, second_memory, second_port);

    failed |= first.trace == STD_NULL || second.trace == STD_NULL ||
        first.trace == second.trace;
    first.trace->flagError = 1;
    first.trace->callCount = 1u;
    failed |= second.trace->flagError != 0;
    failed |= second.trace->callCount != 0u;

    core_machine_cpu_execution_context_initialize(
        &first, first_cpu, first_instructions, first_memory, first_port);
    failed |= first.trace->flagError != 0;
    failed |= first.trace->callCount != 0u;

    core_machine_cpu_execution_finalize(&second);
    core_machine_cpu_execution_finalize(&first);

cleanup:
    STD_FREE(second_port);
    STD_FREE(first_port);
    STD_FREE(second_memory);
    STD_FREE(first_memory);
    STD_FREE(second_instructions);
    STD_FREE(first_instructions);
    STD_FREE(second_cpu);
    STD_FREE(first_cpu);
    if (failed) return 1;
    puts("M5:T88:S1:TRACE-CONTEXT:OK");
    return 0;
}
