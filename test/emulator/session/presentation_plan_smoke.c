#include "lib/types/test.h"
#include "emulator/session/presentation_plan.h"


int main(void)
{
    emulator_session_presentation_plan plan;

    /* Console display keeps the cooked monitor through VM startup.  A raw
       Console may take ownership only after a completed guest frame exists. */
    plan = emulator_session_derive_presentation(EMULATOR_SESSION_DISPLAY_CONSOLE, 1,
        EMULATOR_SESSION_MACHINE_RUNNING, 0, 0);
    lib_test_assert(!plan.window_enabled && !plan.vm_console_enabled &&
        plan.monitor_console_enabled);
    plan = emulator_session_derive_presentation(EMULATOR_SESSION_DISPLAY_CONSOLE, 1,
        EMULATOR_SESSION_MACHINE_RUNNING, 1, 0);
    lib_test_assert(!plan.window_enabled && plan.vm_console_enabled &&
        !plan.monitor_console_enabled);
    plan = emulator_session_derive_presentation(EMULATOR_SESSION_DISPLAY_CONSOLE, 0,
        EMULATOR_SESSION_MACHINE_RUNNING, 1, 1);
    lib_test_assert(plan.window_enabled && plan.vm_console_enabled &&
        !plan.monitor_console_enabled);
    plan = emulator_session_derive_presentation(EMULATOR_SESSION_DISPLAY_CONSOLE, 1,
        EMULATOR_SESSION_MACHINE_RUNNING, 1, 1);
    lib_test_assert(plan.window_enabled && !plan.vm_console_enabled &&
        plan.monitor_console_enabled);
    plan = emulator_session_derive_presentation(EMULATOR_SESSION_DISPLAY_WINDOW, 0,
        EMULATOR_SESSION_MACHINE_RUNNING, 0, 0);
    lib_test_assert(plan.window_enabled && !plan.vm_console_enabled &&
        plan.monitor_console_enabled);
    plan = emulator_session_derive_presentation(EMULATOR_SESSION_DISPLAY_WINDOW, 1,
        EMULATOR_SESSION_MACHINE_PAUSED, 1, 1);
    lib_test_assert(plan.window_enabled && !plan.vm_console_enabled &&
        plan.monitor_console_enabled);
    /* Static Window ignores console_control for both guest frame routes. */
    plan = emulator_session_derive_presentation(EMULATOR_SESSION_DISPLAY_WINDOW, 0,
        EMULATOR_SESSION_MACHINE_RUNNING, 1, 1);
    lib_test_assert(plan.window_enabled && !plan.vm_console_enabled &&
        plan.monitor_console_enabled);
    plan = emulator_session_derive_presentation(EMULATOR_SESSION_DISPLAY_WINDOW, 1,
        EMULATOR_SESSION_MACHINE_RUNNING, 1, 0);
    lib_test_assert(plan.window_enabled && !plan.vm_console_enabled &&
        plan.monitor_console_enabled);
    /* Console display uses its raw VM Console only for text or explicit
       graphical console-control=0. */
    plan = emulator_session_derive_presentation(EMULATOR_SESSION_DISPLAY_CONSOLE, 0,
        EMULATOR_SESSION_MACHINE_RUNNING, 1, 0);
    lib_test_assert(!plan.window_enabled && plan.vm_console_enabled &&
        !plan.monitor_console_enabled);
    plan = emulator_session_derive_presentation(EMULATOR_SESSION_DISPLAY_CONSOLE, 1,
        EMULATOR_SESSION_MACHINE_RUNNING, 1, 1);
    lib_test_assert(plan.window_enabled && !plan.vm_console_enabled &&
        plan.monitor_console_enabled);
    /* Paused/stopped never retain raw VM input. */
    plan = emulator_session_derive_presentation(EMULATOR_SESSION_DISPLAY_CONSOLE, 0,
        EMULATOR_SESSION_MACHINE_PAUSED, 1, 0);
    lib_test_assert(!plan.window_enabled && !plan.vm_console_enabled &&
        plan.monitor_console_enabled);
    plan = emulator_session_derive_presentation(EMULATOR_SESSION_DISPLAY_CONSOLE, 0,
        EMULATOR_SESSION_MACHINE_PAUSED, 1, 1);
    lib_test_assert(plan.window_enabled && !plan.vm_console_enabled &&
        plan.monitor_console_enabled);
    plan = emulator_session_derive_presentation(EMULATOR_SESSION_DISPLAY_WINDOW, 1,
        EMULATOR_SESSION_MACHINE_STOPPED, 1, 1);
    lib_test_assert(!plan.window_enabled && !plan.vm_console_enabled &&
        plan.monitor_console_enabled);
    return 0;
}
