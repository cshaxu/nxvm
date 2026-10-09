#include "emulator/session/presentation_plan.h"

emulator_session_presentation_plan emulator_session_derive_presentation(
    emulator_session_display display, lib_bool console_control,
    emulator_session_machine_state state, lib_bool frame_available,
    lib_bool graphics)
{
    emulator_session_presentation_plan plan = { LIB_FALSE, LIB_FALSE, LIB_TRUE };

    if (state == EMULATOR_SESSION_MACHINE_STOPPED ||
        state == EMULATOR_SESSION_MACHINE_ERROR)
        return plan;
    /* Pause releases the raw VM Console before its component is retired, but
     * an already meaningful Window remains a paused view.  Static Window
     * display always retains it; Console display retains it only when the
     * last completed guest frame was graphical. */
    if (state == EMULATOR_SESSION_MACHINE_PAUSED) {
        plan.window_enabled = display == EMULATOR_SESSION_DISPLAY_WINDOW || graphics;
        return plan;
    }
    if (display == EMULATOR_SESSION_DISPLAY_WINDOW) {
        plan.window_enabled = 1;
        return plan;
    }
    /* A running Console-display machine remains on the cooked monitor until
     * it has actually committed its first guest frame.  In particular, do
     * not cancel a ReadConsole line merely because the VM announced RUNNING. */
    if (!frame_available)
        return plan;
    if (!graphics) {
        plan.vm_console_enabled = 1;
        plan.monitor_console_enabled = 0;
    } else if (!console_control) {
        plan.window_enabled = 1;
        plan.vm_console_enabled = 1;
        plan.monitor_console_enabled = 0;
    } else {
        plan.window_enabled = 1;
    }
    return plan;
}
