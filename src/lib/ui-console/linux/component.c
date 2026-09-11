#include "lib/ui-console/console.h"

#if !defined(_WIN32)
lib_status ui_console_native_start(ui_console *console)
{
    (void)console;
    return LIB_STATUS_UNSUPPORTED;
}

void ui_console_native_stop(ui_console *console)
{
    (void)console;
}
#endif
