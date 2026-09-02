#include "type.h"



#include "core/machine/cpu.h"

#include "core/machine/cpu_instructions.h"

#include "core/machine/memory.h"

#include "core/machine/port.h"

typedef struct cpu_reset_case {
    core_machine_cpu_profile profile;
    type_unsigned_32 code_base;
    type_unsigned_32 first_fetch;
} cpu_reset_case;

static C_INT cpu_execution_context_reset_case(
    core_machine_cpu_execution_context *context, t_cpu *cpu,
    const cpu_reset_case *test_case)
{
    if (context == STD_NULL || cpu == STD_NULL || test_case == STD_NULL) return 1;
    core_machine_cpu_execution_context_bind_profiles(context, test_case->profile,
        CORE_MACHINE_FPU_PROFILE_NONE, TYPE_FALSE);
    core_machine_cpu_state_reset(context);
    return cpu->data.cs.selector != 0xf000u || cpu->data.eip != 0x0000fff0u ||
        cpu->data.cs.base != test_case->code_base ||
        cpu->data.cs.base + cpu->data.eip != test_case->first_fetch ||
        (test_case->profile == CORE_MACHINE_CPU_PROFILE_80386 &&
         cpu->data.edx != 0x00000300u);
}

C_INT main(C_VOID)
{
    static const cpu_reset_case reset_cases[] = {
        {CORE_MACHINE_CPU_PROFILE_8086, 0x000f0000u, 0x000ffff0u},
        {CORE_MACHINE_CPU_PROFILE_8088, 0x000f0000u, 0x000ffff0u},
        {CORE_MACHINE_CPU_PROFILE_80186, 0x000f0000u, 0x000ffff0u},
        {CORE_MACHINE_CPU_PROFILE_80286, 0x00ff0000u, 0x00fffff0u},
        {CORE_MACHINE_CPU_PROFILE_80386, 0xffff0000u, 0xfffffff0u}
    };
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

    for (STD_SIZE_T index = 0u;
         index < sizeof(reset_cases) / sizeof(reset_cases[0]); ++index) {
        result |= cpu_execution_context_reset_case(&first, &first_cpu,
            &reset_cases[index]);
    }

    core_machine_cpu_execution_finalize(&second);
    core_machine_cpu_execution_finalize(&first);
    if (result != 0) return 1;

    puts("M5:T66:S1:CPU-CONTEXT:OK");
    return 0;
}
