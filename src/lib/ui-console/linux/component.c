#include "lib/ui-console/console.h"

lib_status ui_console_worker_start(ui_console *console)
{
    (void)console;
    return LIB_STATUS_UNSUPPORTED;
}

lib_status ui_console_worker_join(ui_console *console, lib_u32 timeout_ms)
{
    (void)console;
    (void)timeout_ms;
    return LIB_STATUS_OK;
}
