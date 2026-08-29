#include "type.h"




#include "vdm/machine/dos_minimal.h"

C_INT main(C_VOID)
{
    vdm_machine_dos_minimal *first = STD_NULL;
    vdm_machine_dos_minimal *second = STD_NULL;

    if (vdm_machine_dos_minimal_create(&first) != TYPE_STATUS_OK ||
        vdm_machine_dos_minimal_create(&second) != TYPE_STATUS_OK ||
        first == second ||
        vdm_machine_dos_minimal_reset(first) != TYPE_STATUS_OK ||
        vdm_machine_dos_minimal_reset(second) != TYPE_STATUS_OK) {
        vdm_machine_dos_minimal_destroy(second);
        vdm_machine_dos_minimal_destroy(first);
        return 1;
    }
    vdm_machine_dos_minimal_destroy(second);
    vdm_machine_dos_minimal_destroy(first);
    puts("M5:T94:S1:VDM-SESSION-ISOLATION:OK");
    return 0;
}
