#include "lib/ui-window/window.h"

/* The public Window contract is platform-neutral. Linux native window
 * integration is deliberately deferred; callers receive an explicit status
 * instead of a second fallback implementation. */
lib_status ui_window_native_start(ui_window *window)
{
    (void)window;
    return LIB_STATUS_UNSUPPORTED;
}

void ui_window_native_stop(ui_window *window)
{
    (void)window;
}
