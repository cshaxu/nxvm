#include <stdio.h>
#include "core/machine/dma.h"
#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"
int main(void) { const vm_composition_live_machine *m; if(core_machine_dma_primary_current()) return 1; machineInit(); m=vm_composition_live_machine_current(); if(!m || &vdma1!=m->dma_primary || &vdma2!=m->dma_secondary || &vlatch!=m->dma_latch) { machineFinal(); return 1; } machineFinal(); if(core_machine_dma_primary_current()) return 1; puts("M5:T29:S1:DMA-AUTHORITY:OK"); return 0; }
