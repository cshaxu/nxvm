#include <stdio.h>
#include <stdlib.h>

#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"
#include "vm/machine/fdc.h"

int main(void)
{
    vm_composition_live_machine *session;
    const vm_composition_live_machine *machine;

    session = (vm_composition_live_machine *)calloc(1u, sizeof(*session));
    if (session == NULL) return 1;
    vm_composition_initialize(session);
    machine = session;
    if (machine == NULL || machine->fdc == NULL ||
        machine->fdc->connect.fdd != machine->fdd ||
        machine->fdc->connect.dma_latch != machine->dma_latch ||
        machine->fdc->connect.dma_primary != machine->dma_primary ||
        machine->fdc->connect.dma_secondary != machine->dma_secondary ||
        machine->fdc->connect.pic_master != machine->pic_master ||
        machine->fdc->connect.pic_slave != machine->pic_slave ||
        machine->fdc->connect.port != machine->port) {
        vm_composition_finalize(session);
        free(session);
        return 1;
    }
    vm_composition_finalize(session);
    free(session);
    puts("M5:T70:S1:P5:FDC-CONNECT:OK");
    return 0;
}
