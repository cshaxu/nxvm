#include "type.h"

#include "vm/composition/session.h"

C_INT main(C_INT argc, C_CHAR **argv)
{
    vm_session_config config = { argv[1], argv[2], 0, 0u, 0 };
    vm_session_reset_vector vector;
    vm_session *session = STD_NULL;

    if (argc != 3 || vm_session_create(&config, &session) != NTVDM64_STATUS_OK ||
        vm_session_get_reset_vector(session, &vector) != NTVDM64_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u) {
        vm_session_destroy(session);
        return 1;
    }
    vm_session_destroy(session);
    puts("M5:T13:S8:VM-SESSION:OK");
    return 0;
}
