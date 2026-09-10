#include "lib/ux-window/internal.h"

/* The public Window contract is platform-neutral. Linux native window
 * integration is deliberately deferred; callers receive an explicit status
 * instead of a second fallback implementation. */
lib_status ux_window_native_start(ux_window *window)
{
    (void)window;
    return LIB_STATUS_UNSUPPORTED;
}

void ux_window_native_stop(ux_window *window)
{
    (void)window;
}
