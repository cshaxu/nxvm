#include <stdio.h>

#include "core/machine/pic.h"
#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"

int main(void)
{
    vm_composition_live_machine session = {0};
    const vm_composition_live_machine *machine;

    machineInit(&session);
    machine = (&session);
    if (machine == NULL || machine->pic_master != &machine->pic_master_storage ||
        machine->pic_slave != &machine->pic_slave_storage) {
        machineFinal(&session);
        return 1;
    }
    core_machine_pic_set_irq(machine->pic_master, machine->pic_slave, 1u);
    if ((machine->pic_master->data.irr & VPIC_IRR_IRQ(1u)) == 0u) {
        machineFinal(&session);
        return 1;
    }
    machineFinal(&session);
    puts("M5:T27:S1:PIC-AUTHORITY:OK");
    return 0;
}
