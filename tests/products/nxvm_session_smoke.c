#include "type.h"

#include "vm/composition/session.h"

static C_INT verify(const C_CHAR *fdd, const C_CHAR *hdd, C_INT boot_hdd)
{
    vm_session_config config = { fdd, hdd, 0, 0u, boot_hdd };
    vm_session_reset_vector vector;
    vm_session *session = STD_NULL;

    if (vm_session_create(&config, &session) != NTVDM64_STATUS_OK ||
        session->core_machine == STD_NULL ||
        vm_session_get_reset_vector(session, &vector) != NTVDM64_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_destroy(session);
    return 0;
}

static C_INT verify_created(C_VOID)
{
    vm_session_config config = { STD_NULL, STD_NULL, 1, 1u, 1 };
    vm_session *session = STD_NULL;

    if (vm_session_create(&config, &session) != NTVDM64_STATUS_OK ||
        !session->fdd->connect.flagDiskExist || !session->hdd->connect.flagDiskExist ||
        session->hdd->data.ncyl != 1u) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_destroy(session);
    return 0;
}

C_INT main(C_INT argc, C_CHAR **argv)
{
    if (argc != 3 || verify(argv[1], argv[2], 0) != 0 ||
        verify(argv[1], argv[2], 1) != 0 || verify_created() != 0) return 1;
    puts("M5:T7:S1:NXVM-SESSION:OK");
    return 0;
}
