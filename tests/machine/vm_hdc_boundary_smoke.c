#include "type.h"



#include "vm/composition/session_lifecycle.h"

#include "vm/machine/hdc.h"

C_INT main(C_VOID)
{
    vm_session session = {0};
    vm_session_initialize(&session);
    vm_machine_hdc_reset();
    vm_machine_hdc_refresh();
    vm_session_finalize(&session);
    puts("M5:T37:S1:HDC-BOUNDARY:OK");
    return 0;
}
