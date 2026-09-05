#include "lib/ux/win32/runner.h"

#ifdef _WIN32
#include "lib/ux/win32/console.h"
#include "lib/ux/win32/window.h"

ux_run_result ux_win32_run(const ux_binding *binding)
{
    ux_run_result result;

    if (ux_binding_validate(binding) != TYPE_STATUS_OK)
        return UX_RUN_ERROR_RESULT;
    for (;;) {
        result = ux_router_target(binding->router) == UX_TARGET_CONSOLE ?
            ux_win32_run_console(binding) : ux_win32_run_window(binding);
        if (result == UX_RUN_SWITCH_WINDOW || result == UX_RUN_SWITCH_CONSOLE)
            continue;
        return result;
    }
}
#endif
