#include <stdio.h>
#include "core/machine/kbc.h"
#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"
int main(void){const vm_composition_live_machine *m; machineInit();m=vm_composition_live_machine_current();if(!m||&vkbc!=m->kbc){machineFinal();return 1;}machineFinal();puts("M5:T30:S1:KBC-AUTHORITY:OK");return 0;}
