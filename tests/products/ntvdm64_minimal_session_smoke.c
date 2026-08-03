#include "type.h"




#include "vdm/composition/session.h"

C_INT main(C_VOID)
{
    vdm_session *first = STD_NULL;
    vdm_session *second = STD_NULL;

    if (vdm_session_create(&first) != NTVDM64_STATUS_OK ||
        vdm_session_create(&second) != NTVDM64_STATUS_OK ||
        first == second ||
        vdm_session_reset(first) != NTVDM64_STATUS_OK ||
        vdm_session_reset(second) != NTVDM64_STATUS_OK) {
        vdm_session_destroy(second);
        vdm_session_destroy(first);
        return 1;
    }
    vdm_session_destroy(second);
    vdm_session_destroy(first);
    puts("M5:T94:S1:VDM-SESSION-ISOLATION:OK");
    return 0;
}
