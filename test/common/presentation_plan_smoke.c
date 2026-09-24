#include "lib/types/test.h"
#include "lib/types/file.h"
#include "common/session/presentation_plan.h"


int main(void)
{
    common_session_presentation_plan plan;

    /* Console display keeps the cooked monitor through VM startup.  A raw
       Console may take ownership only after a completed guest frame exists. */
    plan = common_session_derive_presentation(COMMON_SESSION_DISPLAY_CONSOLE, 1,
        COMMON_SESSION_MACHINE_RUNNING, 0, 0);
    lib_test_assert(!plan.window_enabled && !plan.vm_console_enabled &&
        plan.monitor_console_enabled);
    plan = common_session_derive_presentation(COMMON_SESSION_DISPLAY_CONSOLE, 1,
        COMMON_SESSION_MACHINE_RUNNING, 1, 0);
    lib_test_assert(!plan.window_enabled && plan.vm_console_enabled &&
        !plan.monitor_console_enabled);
    plan = common_session_derive_presentation(COMMON_SESSION_DISPLAY_CONSOLE, 0,
        COMMON_SESSION_MACHINE_RUNNING, 1, 1);
    lib_test_assert(plan.window_enabled && plan.vm_console_enabled &&
        !plan.monitor_console_enabled);
    plan = common_session_derive_presentation(COMMON_SESSION_DISPLAY_CONSOLE, 1,
        COMMON_SESSION_MACHINE_RUNNING, 1, 1);
    lib_test_assert(plan.window_enabled && !plan.vm_console_enabled &&
        plan.monitor_console_enabled);
    plan = common_session_derive_presentation(COMMON_SESSION_DISPLAY_WINDOW, 0,
        COMMON_SESSION_MACHINE_RUNNING, 0, 0);
    lib_test_assert(plan.window_enabled && !plan.vm_console_enabled &&
        plan.monitor_console_enabled);
    plan = common_session_derive_presentation(COMMON_SESSION_DISPLAY_WINDOW, 1,
        COMMON_SESSION_MACHINE_PAUSED, 1, 1);
    lib_test_assert(plan.window_enabled && !plan.vm_console_enabled &&
        plan.monitor_console_enabled);
    /* Static Window ignores console_control for both guest frame routes. */
    plan = common_session_derive_presentation(COMMON_SESSION_DISPLAY_WINDOW, 0,
        COMMON_SESSION_MACHINE_RUNNING, 1, 1);
    lib_test_assert(plan.window_enabled && !plan.vm_console_enabled &&
        plan.monitor_console_enabled);
    plan = common_session_derive_presentation(COMMON_SESSION_DISPLAY_WINDOW, 1,
        COMMON_SESSION_MACHINE_RUNNING, 1, 0);
    lib_test_assert(plan.window_enabled && !plan.vm_console_enabled &&
        plan.monitor_console_enabled);
    /* Console display uses its raw VM Console only for text or explicit
       graphical console-control=0. */
    plan = common_session_derive_presentation(COMMON_SESSION_DISPLAY_CONSOLE, 0,
        COMMON_SESSION_MACHINE_RUNNING, 1, 0);
    lib_test_assert(!plan.window_enabled && plan.vm_console_enabled &&
        !plan.monitor_console_enabled);
    plan = common_session_derive_presentation(COMMON_SESSION_DISPLAY_CONSOLE, 1,
        COMMON_SESSION_MACHINE_RUNNING, 1, 1);
    lib_test_assert(plan.window_enabled && !plan.vm_console_enabled &&
        plan.monitor_console_enabled);
    /* Paused/stopped never retain raw VM input. */
    plan = common_session_derive_presentation(COMMON_SESSION_DISPLAY_CONSOLE, 0,
        COMMON_SESSION_MACHINE_PAUSED, 1, 0);
    lib_test_assert(!plan.window_enabled && !plan.vm_console_enabled &&
        plan.monitor_console_enabled);
    plan = common_session_derive_presentation(COMMON_SESSION_DISPLAY_CONSOLE, 0,
        COMMON_SESSION_MACHINE_PAUSED, 1, 1);
    lib_test_assert(plan.window_enabled && !plan.vm_console_enabled &&
        plan.monitor_console_enabled);
    plan = common_session_derive_presentation(COMMON_SESSION_DISPLAY_WINDOW, 1,
        COMMON_SESSION_MACHINE_STOPPED, 1, 1);
    lib_test_assert(!plan.window_enabled && !plan.vm_console_enabled &&
        plan.monitor_console_enabled);
    return 0;
}
