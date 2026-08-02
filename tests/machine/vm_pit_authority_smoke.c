#include <stdio.h>
#include "core/machine/pit.h"
#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"
int main(void) {
 const vm_composition_live_machine *m; if(core_machine_pit_current()) return 1;
 machineInit(); m=vm_composition_live_machine_current();
 if(!m || m->pit!=&m->pit_storage || &vpit!=m->pit) { machineFinal(); return 1; }
 machineFinal(); if(core_machine_pit_current()) return 1;
 puts("M5:T28:S1:PIT-AUTHORITY:OK"); return 0;
}
