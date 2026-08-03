#include "type.h"

#include <stdio.h>


#include "core/machine/cpu.h"

#include "core/machine/cpu_instructions.h"

#include "core/machine/memory.h"

#include "core/machine/port.h"

C_INT main(C_VOID)
{
    t_cpu first_cpu = {0};
    t_cpu second_cpu = {0};
    t_cpuins first_instructions = {0};
    t_cpuins second_instructions = {0};
    t_ram first_memory = {0};
    t_ram second_memory = {0};
    t_port first_port = {0};
    t_port second_port = {0};
    core_machine_cpu_execution_context first = {0};
    core_machine_cpu_execution_context second = {0};
    C_INT result = 0;

    core_machine_cpu_execution_context_initialize(
        &first, &first_cpu, &first_instructions, &first_memory, &first_port);
    core_machine_cpu_execution_context_initialize(
        &second, &second_cpu, &second_instructions, &second_memory,
        &second_port);
    core_machine_cpu_state_initialize(&first);
    core_machine_cpu_state_initialize(&second);
    core_machine_cpu_state_reset(&first);
    core_machine_cpu_state_reset(&second);

    result |= first_cpu.data.cs.selector != 0xf000u;
    result |= first_cpu.data.eip != 0x0000fff0u;
    result |= second_cpu.data.cs.selector != 0xf000u;
    result |= second_cpu.data.eip != 0x0000fff0u;

    first_cpu.data.eip = 0x12345678u;
    first_instructions.data.except = VCPUINS_EXCEPT_UD;
    core_machine_cpu_execution_request_stop(&first);
    core_machine_cpu_execution_request_reset(&first);

    result |= second_cpu.data.eip != 0x0000fff0u;
    result |= second_instructions.data.except != 0u;
    result |= core_machine_cpu_execution_consume_stop_request(&second);
    result |= core_machine_cpu_execution_consume_reset_request(&second);
    result |= !core_machine_cpu_execution_consume_stop_request(&first);
    result |= !core_machine_cpu_execution_consume_reset_request(&first);

    core_machine_cpu_execution_finalize(&second);
    core_machine_cpu_execution_finalize(&first);
    if (result != 0) return 1;

    puts("M5:T66:S1:CPU-CONTEXT:OK");
    return 0;
}
