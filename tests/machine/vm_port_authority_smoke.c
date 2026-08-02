#include <stdio.h>

#include "core/machine/memory.h"
#include "core/machine/port.h"
#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"

int main(void)
{
    vm_composition_live_machine session = {0};
    const vm_composition_live_machine *machine;
    int failed = 0;

    machineInit(&session);
    machine = (&session);
    if (machine == NULL || machine->port != &machine->port_storage ||
        machine->port != &machine->port_storage) {
        machineFinal(&session);
        return 1;
    }

    machine->port->data.ioByte = VRAM_FLAG_A20;
    core_machine_port_execute_write(machine->port, 0x0092u);
    machine->port->data.ioByte = 0u;
    core_machine_port_execute_read(machine->port, 0x0092u);
    failed |= machine->port->data.ioByte != VRAM_FLAG_A20;

    machineFinal(&session);
    if (failed) return 1;

    puts("M5:T26:S1:PORT-AUTHORITY:OK");
    return 0;
}
