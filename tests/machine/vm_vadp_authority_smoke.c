#include <stdio.h>
#include "core/machine/vadp.h"
#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"
int main(void){const vm_composition_live_machine*m;machineInit();m=vm_composition_live_machine_current();if(!m||&vvadp!=m->vadp){machineFinal();return 1;}machineFinal();puts("M5:T31:S1:VADP-AUTHORITY:OK");return 0;}
