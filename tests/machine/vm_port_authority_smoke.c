#include <stdio.h>

#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"

int main(void)
{
    const vm_composition_live_machine *machine;
    int failed = 0;

    if (core_machine_port_current() != NULL) return 1;

    machineInit();
    machine = vm_composition_live_machine_current();
    if (machine == NULL || machine->port != &machine->port_storage ||
        machine->port != core_machine_port_current() || &vport != machine->port) {
        machineFinal();
        return 1;
    }

    vport.data.ioByte = VRAM_FLAG_A20;
    vportExecWrite(0x0092u);
    vport.data.ioByte = 0u;
    vportExecRead(0x0092u);
    failed |= vport.data.ioByte != VRAM_FLAG_A20;

    machineFinal();
    if (core_machine_port_current() != NULL || failed) return 1;

    puts("M5:T26:S1:PORT-AUTHORITY:OK");
    return 0;
}
