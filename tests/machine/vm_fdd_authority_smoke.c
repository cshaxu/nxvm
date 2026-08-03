#include "type.h"



#include "vm/composition/composition_live_machine.h"

#include "vm/composition/composition_machine.h"

#include "vm/machine/fdd.h"

C_INT main(C_VOID)
{
    vm_composition_live_machine session = {0};
    const vm_composition_live_machine *machine;

    vm_composition_initialize(&session);
    machine = (&session);
    if (machine == STD_NULL || machine->fdd != &machine->fdd_storage) {
        vm_composition_finalize(&session);
        return 1;
    }
    vm_composition_finalize(&session);
    puts("M5:T70:S1:P7:FDD-AUTHORITY:OK");
    return 0;
}
