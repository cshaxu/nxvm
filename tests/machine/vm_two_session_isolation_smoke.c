#include <stdio.h>
#include <stdlib.h>

#include "core/machine/memory.h"
#include "vm/composition_live_machine.h"

int main(void)
{
    vm_composition_live_machine *first;
    vm_composition_live_machine *second;
    unsigned char first_value = 0x11u;
    unsigned char second_value = 0x22u;
    unsigned char observed = 0u;
    int failed = 0;

    first = (vm_composition_live_machine *)calloc(1u, sizeof(*first));
    second = (vm_composition_live_machine *)calloc(1u, sizeof(*second));
    if (first == NULL || second == NULL) {
        free(second);
        free(first);
        return 1;
    }

    vm_composition_live_machine_initialize(first);
    vm_composition_live_machine_initialize(second);
    core_machine_memory_initialize(first->ram);
    core_machine_memory_initialize(second->ram);

    failed |= first->cpu == second->cpu;
    failed |= first->cpuins == second->cpuins;
    failed |= first->cpu_execution == second->cpu_execution;
    failed |= first->ram == second->ram;
    failed |= first->port == second->port;
    failed |= first->cpu_execution->cpu != first->cpu;
    failed |= second->cpu_execution->cpu != second->cpu;
    failed |= first->cpu_execution->instructions != first->cpuins;
    failed |= second->cpu_execution->instructions != second->cpuins;

    core_machine_memory_write_physical(first->ram, 0u,
        (ntvdm64_type_virtual_address)&first_value, 1u);
    core_machine_memory_write_physical(second->ram, 0u,
        (ntvdm64_type_virtual_address)&second_value, 1u);
    core_machine_memory_read_physical(first->ram, 0u,
        (ntvdm64_type_virtual_address)&observed, 1u);
    failed |= observed != first_value;
    core_machine_memory_read_physical(second->ram, 0u,
        (ntvdm64_type_virtual_address)&observed, 1u);
    failed |= observed != second_value;

    first->cpu->data.eax = 0x11111111u;
    second->cpu->data.eax = 0x22222222u;
    first->cpuins->data.flagWR = NTVDM64_TYPE_TRUE;
    failed |= second->cpu->data.eax != 0x22222222u;
    failed |= second->cpuins->data.flagWR != NTVDM64_TYPE_FALSE;

    core_machine_memory_finalize(second->ram);
    core_machine_memory_finalize(first->ram);
    vm_composition_live_machine_finalize(second);
    vm_composition_live_machine_finalize(first);
    free(second);
    free(first);

    if (failed) return 1;
    puts("M5:T73:S1:TWO-SESSION-ISOLATION:OK");
    return 0;
}
