#include <stdio.h>

#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"
#include "vm/machine/fdc.h"

int main(void)
{
    vm_composition_live_machine session = {0};
    const vm_composition_live_machine *machine;

    machineInit(&session);
    machine = (&session);
    if (machine == NULL || machine->fdc == NULL ||
        machine->fdc->connect.fdd != machine->fdd ||
        machine->fdc->connect.dma_latch != machine->dma_latch ||
        machine->fdc->connect.dma_primary != machine->dma_primary ||
        machine->fdc->connect.dma_secondary != machine->dma_secondary ||
        machine->fdc->connect.pic_master != machine->pic_master ||
        machine->fdc->connect.pic_slave != machine->pic_slave ||
        machine->fdc->connect.port != machine->port) {
        machineFinal(&session);
        return 1;
    }
    machineFinal(&session);
    puts("M5:T70:S1:P5:FDC-CONNECT:OK");
    return 0;
}
