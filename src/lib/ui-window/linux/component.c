#include "lib/ui-window/window.h"

/* The public Window contract is platform-neutral. Linux native window
 * integration is deliberately deferred; callers receive an explicit status
 * instead of a second fallback implementation. */
lib_status ui_window_worker_start(ui_window *window)
{
    (void)window;
    return LIB_STATUS_UNSUPPORTED;
}

lib_status ui_window_worker_join(ui_window *window, lib_u32 timeout_ms)
{
    (void)window;
    (void)timeout_ms;
    return LIB_STATUS_OK;
}
