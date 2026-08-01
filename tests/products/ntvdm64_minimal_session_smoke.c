#include <stdio.h>
#include <string.h>

#include "vdm/product/minimal_session.h"

int main(void)
{
    ntvdm64_vdm_minimal_session *session = NULL;

    if (ntvdm64_vdm_minimal_session_create(&session) != NXVM_CORE_STATUS_OK ||
        strcmp(ntvdm64_vdm_minimal_session_profile(session)->name,
               "ntvdm64.dos_minimal") != 0 ||
        ntvdm64_vdm_minimal_session_reset(session) != NXVM_CORE_STATUS_OK) {
        ntvdm64_vdm_minimal_session_destroy(session);
        return 1;
    }
    ntvdm64_vdm_minimal_session_destroy(session);
    puts("M5:T13:S8:VDM-SESSION:OK");
    return 0;
}
