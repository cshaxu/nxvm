#include <stdio.h>
#include "core/machine/dma.h"
#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"
int main(void)
{
    vm_composition_live_machine session = {0}; const vm_composition_live_machine *m; machineInit(&session); m=(&session); if(!m || m->dma_primary != &m->dma_primary_storage || m->dma_secondary != &m->dma_secondary_storage || m->dma_latch != &m->dma_latch_storage) { machineFinal(&session); return 1; } machineFinal(&session); puts("M5:T29:S1:DMA-AUTHORITY:OK"); return 0; }
