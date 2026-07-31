#include <stdio.h>

#include "core/debug.h"

int main(void)
{
    nxvm_core_machine *machine = NULL;
    nxvm_core_machine_config config = {
        NXVM_CORE_ABI_VERSION, NXVM_CORE_PROFILE_TEST_MINIMAL, 0u
    };
    nxvm_core_cpu_state cpu;
    nxvm_core_run_result result;
    nxvm_core_run_budget budget = { 2u, 0u };
    unsigned char byte = 0x5au;

    if (nxvm_core_machine_create(&config, &machine) != NXVM_CORE_STATUS_OK ||
        nxvm_core_debug_read_cpu(machine, &cpu) != NXVM_CORE_STATUS_INVALID_STATE ||
        nxvm_core_machine_reset(machine) != NXVM_CORE_STATUS_OK ||
        nxvm_core_machine_memory_write(machine, 0u, &byte, 1u) != NXVM_CORE_STATUS_OK ||
        nxvm_core_debug_read_cpu(machine, &cpu) != NXVM_CORE_STATUS_OK ||
        cpu.cs != 0xf000u || cpu.eip != 0xfff0u ||
        nxvm_core_debug_read_memory(machine, 0u, &byte, 1u) != NXVM_CORE_STATUS_OK ||
        byte != 0x5au ||
        nxvm_core_debug_step(machine, &result) != NXVM_CORE_STATUS_OK ||
        result.reason != NXVM_CORE_STOP_BUDGET ||
        nxvm_core_debug_continue(machine, budget, &result) != NXVM_CORE_STATUS_OK ||
        result.reason != NXVM_CORE_STOP_BUDGET) {
        nxvm_core_machine_destroy(machine);
        return 1;
    }

    nxvm_core_machine_destroy(machine);
    puts("M3:T4:S2:DEBUG:OK");
    return 0;
}
