#include <stdio.h>

#include "core/machine/debug_interface.h"

int main(void)
{
    core_machine *machine = NULL;
    core_machine_config config = {
        CORE_MACHINE_PROFILE_TEST_MINIMAL, 0u
    };
    core_machine_cpu_state cpu;
    core_machine_run_result result;
    core_machine_run_budget budget = { 2u, 0u };
    unsigned char byte = 0x5au;

    if (core_machine_create(&config, &machine) != NXVM_CORE_STATUS_OK ||
        core_machine_debug_read_cpu(machine, &cpu) != NXVM_CORE_STATUS_INVALID_STATE ||
        core_machine_reset(machine) != NXVM_CORE_STATUS_OK ||
        core_machine_memory_write(machine, 0u, &byte, 1u) != NXVM_CORE_STATUS_OK ||
        core_machine_debug_read_cpu(machine, &cpu) != NXVM_CORE_STATUS_OK ||
        cpu.cs != 0xf000u || cpu.eip != 0xfff0u ||
        core_machine_debug_read_memory(machine, 0u, &byte, 1u) != NXVM_CORE_STATUS_OK ||
        byte != 0x5au ||
        core_machine_debug_step(machine, &result) != NXVM_CORE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET ||
        core_machine_debug_continue(machine, budget, &result) != NXVM_CORE_STATUS_OK ||
        result.reason != CORE_MACHINE_STOP_BUDGET) {
        core_machine_destroy(machine);
        return 1;
    }

    core_machine_destroy(machine);
    puts("M3:T4:S2:DEBUG:OK");
    return 0;
}
