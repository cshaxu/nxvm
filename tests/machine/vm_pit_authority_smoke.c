#include <stdio.h>
#include "core/machine/pit.h"
#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"
int main(void)
{
    vm_composition_live_machine session = {0};
 const vm_composition_live_machine *m; if(core_machine_pit_current()) return 1;
 machineInit(&session); m=(&session);
 if(!m || m->pit!=&m->pit_storage || &vpit!=m->pit) { machineFinal(&session); return 1; }
 machineFinal(&session); if(core_machine_pit_current()) return 1;
 puts("M5:T28:S1:PIT-AUTHORITY:OK"); return 0;
}
