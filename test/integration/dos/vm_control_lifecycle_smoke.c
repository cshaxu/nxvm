#include "type.h"

#include "test/integration/support/session_ini.h"

int main(C_INT argc, C_CHAR **argv)
{
    integration_ini_session ini_session;

    if (argc != 3 || integration_ini_session_open(argv[1], argv[2],
            &ini_session) != TYPE_STATUS_OK) return 77;
    if (integration_ini_session_start(&ini_session) != TYPE_STATUS_OK ||
        integration_ini_session_wait_for_state(&ini_session,
            COMMON_MACHINE_RUNNING, 2000u) == 0 ||
        integration_ini_session_pause(&ini_session, 2000u) != TYPE_STATUS_OK ||
        integration_ini_session_reset(&ini_session, 2000u) != TYPE_STATUS_OK ||
        integration_ini_session_resume(&ini_session, 2000u) != TYPE_STATUS_OK ||
        integration_ini_session_wait_for_state(&ini_session,
            COMMON_MACHINE_RUNNING, 2000u) == 0 ||
        !common_machine_stop(ini_session.common_machine) ||
        integration_ini_session_wait_for_state(&ini_session,
            COMMON_MACHINE_STOPPED, 2000u) == 0) {
        integration_ini_session_close(&ini_session);
        return 1;
    }
    integration_ini_session_close(&ini_session);
    puts("M5:T534:S26:CONTEXT-LIFECYCLE:OK");
    return 0;
}
