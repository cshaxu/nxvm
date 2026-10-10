#include "lib/types/types_interface.h"
#include <stdio.h>

#include "core/x86/machine_interface.h"
#include "test/core/setup/session_ini.h"

lib_i32 main(lib_i32 argc, char **argv)
{
    integration_ini_session ini_session;
    vm_machine_reset_vector vector;
    vm_machine *session;

    if (argc != 3 || integration_ini_session_open(argv[1], argv[2],
            &ini_session) != LIB_STATUS_OK) return 77;
    session = ini_session.session;
    if (
        vm_machine_get_reset_vector(session, &vector) != LIB_STATUS_OK ||
        vector.cs != 0xf000u || vector.ip != 0xfff0u) {
        integration_ini_session_close(&ini_session);
        return 1;
    }
    integration_ini_session_close(&ini_session);
    puts("NXVM:VM-SESSION:OK");
    return 0;
}
