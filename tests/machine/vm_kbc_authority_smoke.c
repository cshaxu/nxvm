#include <stdio.h>
#include "core/machine/kbc.h"
#include "vm/composition_live_machine.h"
#include "vm/composition_machine.h"
int main(void)
{
    vm_composition_live_machine session = {0};const vm_composition_live_machine *m; machineInit(&session);m=(&session);if(!m||m->kbc!=&m->kbc_storage){machineFinal(&session);return 1;}machineFinal(&session);puts("M5:T30:S1:KBC-AUTHORITY:OK");return 0;}
