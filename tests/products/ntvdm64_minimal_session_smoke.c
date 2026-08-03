#include <stdio.h>
#include <string.h>

#include "vdm/composition_minimal_session.h"

int main(void)
{
    ntvdm64_vdm_minimal_session *first = NULL;
    ntvdm64_vdm_minimal_session *second = NULL;

    if (ntvdm64_vdm_minimal_session_create(&first) != NXVM_CORE_STATUS_OK ||
        ntvdm64_vdm_minimal_session_create(&second) != NXVM_CORE_STATUS_OK ||
        first == second ||
        strcmp(ntvdm64_vdm_minimal_session_profile(first)->name,
               "ntvdm64.dos_minimal") != 0 ||
        strcmp(ntvdm64_vdm_minimal_session_profile(second)->name,
               "ntvdm64.dos_minimal") != 0 ||
        ntvdm64_vdm_minimal_session_reset(first) != NXVM_CORE_STATUS_OK ||
        ntvdm64_vdm_minimal_session_reset(second) != NXVM_CORE_STATUS_OK) {
        ntvdm64_vdm_minimal_session_destroy(second);
        ntvdm64_vdm_minimal_session_destroy(first);
        return 1;
    }
    ntvdm64_vdm_minimal_session_destroy(second);
    ntvdm64_vdm_minimal_session_destroy(first);
    puts("M5:T94:S1:VDM-SESSION-ISOLATION:OK");
    return 0;
}
