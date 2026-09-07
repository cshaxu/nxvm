#include "lib/base/base.h"
#include "lib/ux/presenter.h"

#ifdef _WIN32
#include "lib/ux/internal/win32_console.h"
#include "lib/ux/internal/win32_window.h"

ux_run_result ux_run(const ux_binding *binding)
{
    ux_run_result result;

    if (ux_binding_validate(binding) != LIB_STATUS_OK)
        return UX_RUN_ERROR_RESULT;
    result = ux_win32_run_console(binding);
    for (;;) {
        if (result == UX_RUN_SWITCH_WINDOW || result == UX_RUN_SWITCH_CONSOLE)
            result = result == UX_RUN_SWITCH_WINDOW ? ux_win32_run_window(binding) :
                ux_win32_run_console(binding);
        else
            return result;
    }
}
#endif
