#include "type.h"




#include "vdm/composition/composition_minimal_session.h"

C_INT main(C_VOID)
{
    vdm_composition_minimal_session *first = STD_NULL;
    vdm_composition_minimal_session *second = STD_NULL;

    if (vdm_composition_minimal_session_create(&first) != NTVDM64_STATUS_OK ||
        vdm_composition_minimal_session_create(&second) != NTVDM64_STATUS_OK ||
        first == second ||
        STD_STRCMP(vdm_composition_minimal_session_profile(first)->name,
               "ntvdm64.dos_minimal") != 0 ||
        STD_STRCMP(vdm_composition_minimal_session_profile(second)->name,
               "ntvdm64.dos_minimal") != 0 ||
        vdm_composition_minimal_session_reset(first) != NTVDM64_STATUS_OK ||
        vdm_composition_minimal_session_reset(second) != NTVDM64_STATUS_OK) {
        vdm_composition_minimal_session_destroy(second);
        vdm_composition_minimal_session_destroy(first);
        return 1;
    }
    vdm_composition_minimal_session_destroy(second);
    vdm_composition_minimal_session_destroy(first);
    puts("M5:T94:S1:VDM-SESSION-ISOLATION:OK");
    return 0;
}
