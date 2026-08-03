#include "type.h"



#include "core/machine/debug_interface.h"
#include "../support/core_machine_executor_fixture.h"

C_INT main(C_VOID)
{
    core_machine *machine = STD_NULL;
    core_machine_cpu_state cpu;
    core_machine_run_result result;
    core_machine_run_budget budget = { 2u, 0u };
    C_UCHAR byte = 0x5au;
    C_UCHAR nop = 0x90u;

    if (test_core_machine_create_executor(0u, &machine) != NTVDM64_STATUS_OK ||
        core_machine_debug_read_cpu(machine, &cpu) != NTVDM64_STATUS_INVALID_STATE ||
        core_machine_freeze_execution_providers(machine) != NTVDM64_STATUS_OK ||
        core_machine_reset(machine) != NTVDM64_STATUS_OK ||
        core_machine_memory_write(machine, 0u, &byte, 1u) != NTVDM64_STATUS_OK ||
        core_machine_memory_write(machine, 0xffff0u, &nop, 1u) != NTVDM64_STATUS_OK ||
        core_machine_memory_write(machine, 0xffff1u, &nop, 1u) != NTVDM64_STATUS_OK ||
        core_machine_debug_read_cpu(machine, &cpu) != NTVDM64_STATUS_OK ||
        cpu.cs != 0xf000u || cpu.eip != 0xfff0u ||
        core_machine_debug_read_memory(machine, 0u, &byte, 1u) != NTVDM64_STATUS_OK ||
        byte != 0x5au ||
        core_machine_debug_step(machine, &result) != NTVDM64_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET ||
        core_machine_debug_continue(machine, budget, &result) != NTVDM64_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET) {
        core_machine_destroy(machine);
        return 1;
    }

    core_machine_destroy(machine);
    puts("M3:T4:S2:DEBUG:OK");
    return 0;
}
