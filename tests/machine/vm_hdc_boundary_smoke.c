#include "type.h"



#include "vm/composition/composition_machine.h"

#include "vm/machine/hdc.h"

C_INT main(C_VOID)
{
    vm_composition_live_machine session = {0};
    vm_composition_initialize(&session);
    vm_machine_hdc_reset();
    vm_machine_hdc_refresh();
    vm_composition_finalize(&session);
    puts("M5:T37:S1:HDC-BOUNDARY:OK");
    return 0;
}
