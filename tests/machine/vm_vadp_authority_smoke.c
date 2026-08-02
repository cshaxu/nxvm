#include <stdio.h>
#include "core/machine/vadp.h"
#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"
int main(void)
{
    vm_composition_live_machine session = {0};const vm_composition_live_machine*m;machineInit(&session);m=(&session);if(!m||&vvadp!=m->vadp){machineFinal(&session);return 1;}machineFinal(&session);puts("M5:T31:S1:VADP-AUTHORITY:OK");return 0;}
