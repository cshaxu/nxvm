#include "lib/base/base.h"
#include "lib/ux/win32/runner.h"

#ifdef _WIN32
#include "lib/ux/win32/console.h"
#include "lib/ux/win32/window.h"

ux_run_result ux_win32_run(const ux_binding *binding)
{
    ux_run_result result;

    if (ux_binding_validate(binding) != LIB_STATUS_OK)
        return UX_RUN_ERROR_RESULT;
    for (;;) {
        ux_target target = ux_router_target(binding->router);

        if (target == UX_TARGET_NONE) return UX_RUN_STOPPED_RESULT;
        result = target == UX_TARGET_CONSOLE ? ux_win32_run_console(binding) :
            ux_win32_run_window(binding);
        if (result == UX_RUN_SWITCH_WINDOW || result == UX_RUN_SWITCH_CONSOLE)
            continue;
        return result;
    }
}
#endif
