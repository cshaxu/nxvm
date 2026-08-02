#include <stdio.h>

#include "core/machine/pic.h"
#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"

int main(void)
{
    const vm_composition_live_machine *machine;

    if (core_machine_pic_master_current() != NULL ||
        core_machine_pic_slave_current() != NULL) return 1;
    machineInit();
    machine = vm_composition_live_machine_current();
    if (machine == NULL || machine->pic_master != &machine->pic_master_storage ||
        machine->pic_slave != &machine->pic_slave_storage ||
        &vpic1 != machine->pic_master || &vpic2 != machine->pic_slave) {
        machineFinal();
        return 1;
    }
    vpicSetIRQ(1u);
    if ((vpic1.data.irr & VPIC_IRR_IRQ(1u)) == 0u) {
        machineFinal();
        return 1;
    }
    machineFinal();
    if (core_machine_pic_master_current() != NULL ||
        core_machine_pic_slave_current() != NULL) return 1;
    puts("M5:T27:S1:PIC-AUTHORITY:OK");
    return 0;
}
