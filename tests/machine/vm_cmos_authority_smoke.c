#include "type.h"

#include <stdio.h>


#include "vm/composition_live_machine.h"

#include "vm/composition_machine.h"

#include "vm/machine/cmos.h"

C_INT main(C_VOID)
{
    vm_composition_live_machine session = {0};
    const vm_composition_live_machine *machine;

    vm_composition_initialize(&session);
    machine = (&session);
    if (machine == NULL || machine->cmos != &session.cmos_storage) {
        vm_composition_finalize(&session);
        return 1;
    }
    vm_composition_finalize(&session);
    puts("M5:T70:S1:CMOS-AUTHORITY:OK");
    return 0;
}
