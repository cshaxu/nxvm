#include "lib/types/types_interface.h"
#include <stdio.h>

#include "test/core/setup/session_ini.h"

static lib_status session_ini_reject_transform(integration_ini_session *session,
    void *opaque)
{
    (void)session;
    (void)opaque;
    return LIB_STATUS_UNSUPPORTED;
}

int main(int argc, char **argv)
{
    integration_ini_session session = {0};

    if (integration_ini_session_restart(&session) != LIB_STATUS_INVALID_ARGUMENT ||
        session.session != LIB_NULL || session.emulator_machine != LIB_NULL)
        return 1;
    if (argc != 3 || integration_ini_session_open_with_overlay_transform(argv[1],
            argv[2], session_ini_reject_transform, LIB_NULL, &session) !=
            LIB_STATUS_UNSUPPORTED || session.session != LIB_NULL ||
        session.emulator_machine != LIB_NULL)
        return 2;
    integration_ini_session_close(&session);
    printf("CORE:SESSION-INI-STATUS:OK\n");
    return 0;
}
