#include "type.h"

#include "app-nxvm/devices/machine_interface.h"
#include "test/app-nxvm/integration/support/session_ini.h"

C_INT main(C_INT argc, C_CHAR **argv)
{
    integration_ini_session ini_session;
    vm_machine_reset_vector vector;
    vm_machine *session;

    if (argc != 3 || integration_ini_session_open(argv[1], argv[2],
            &ini_session) != TYPE_STATUS_OK) return 77;
    session = ini_session.session;
    if (
        vm_machine_get_reset_vector(session, &vector) != TYPE_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u) {
        integration_ini_session_close(&ini_session);
        return 1;
    }
    integration_ini_session_close(&ini_session);
    puts("M5:T13:S8:VM-SESSION:OK");
    return 0;
}
