#include <stdio.h>

#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"

static int expect_physical(t_nubit32 physical, t_nubit8 expected)
{
    t_nubit8 actual = 0u;

    vramReadPhysical(physical, (t_vaddrcc)&actual, 1u);
    return actual == expected;
}

int main(void)
{
    vm_composition_live_machine session = {0};
    const vm_composition_live_machine *machine;
    t_nubit8 value;
    int failed = 0;

    if (core_machine_memory_current() != NULL) return 1;

    machineInit(&session);
    machine = (&session);
    if (machine == NULL || machine->ram != &machine->ram_storage ||
        machine->ram != core_machine_memory_current() || &vram != machine->ram) {
        machineFinal(&session);
        return 1;
    }

    value = 0x11u;
    vramWritePhysical(0x00000000u, (t_vaddrcc)&value, 1u);
    value = 0x22u;
    vramWritePhysical(0x00100000u, (t_vaddrcc)&value, 1u);
    failed |= !expect_physical(0x00000000u, 0x22u);

    vport.data.ioByte = VRAM_FLAG_A20;
    vportExecWrite(0x0092u);
    value = 0x33u;
    vramWritePhysical(0x00000000u, (t_vaddrcc)&value, 1u);
    value = 0x44u;
    vramWritePhysical(0x00100000u, (t_vaddrcc)&value, 1u);
    failed |= !expect_physical(0x00000000u, 0x33u);
    failed |= !expect_physical(0x00100000u, 0x44u);

    value = 0x5au;
    vramWritePhysical(0x001e0000u, (t_vaddrcc)&value, 1u);
    failed |= !expect_physical(0xfffe0000u, 0x5au);

    machineFinal(&session);
    if (core_machine_memory_current() != NULL) return 1;
    if (failed) return 1;

    puts("M5:T25:S2:RAM-AUTHORITY:OK");
    return 0;
}
