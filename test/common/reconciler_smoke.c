#include "lib/types/test.h"
#include "common/session/control_state.h"


static void check_close_lifetime(common_session_display display, lib_i32 control,
    lib_i32 restart)
{
    common_session_state state;
    common_session_state_initialize(&state, display, control);
    common_session_state_note_runtime(&state, COMMON_SESSION_MACHINE_RUNNING);
    common_session_state_note_frame(&state, 1u, 1);
    common_session_state_note_window(&state, 1);
    common_session_state_note_window_close(&state);
    /* Re-observing RUNNING is not a new run, and cannot cancel pending X. */
    common_session_state_note_runtime(&state, COMMON_SESSION_MACHINE_RUNNING);
    common_session_state_note_runtime(&state, COMMON_SESSION_MACHINE_PAUSED);
    lib_test_assert(!common_session_state_desired(&state).window_enabled);
    common_session_state_note_window(&state, 0);
    common_session_state_note_frame(&state, 2u, 1);
    lib_test_assert(!common_session_state_desired(&state).window_enabled);
    if (restart) common_session_state_note_runtime(&state, COMMON_SESSION_MACHINE_STOPPED);
    common_session_state_note_runtime(&state, COMMON_SESSION_MACHINE_RUNNING);
    common_session_state_note_frame(&state, 3u, 1);
    lib_test_assert(common_session_state_desired(&state).window_enabled);
    common_session_state_note_window(&state, 1);
    /* CAP pause/resume uses completed runtime facts, never the old X. */
    common_session_state_note_runtime(&state, COMMON_SESSION_MACHINE_PAUSED);
    lib_test_assert(common_session_state_desired(&state).window_enabled);
    common_session_state_note_runtime(&state, COMMON_SESSION_MACHINE_RUNNING);
    lib_test_assert(common_session_state_desired(&state).window_enabled);
}

/* Presentation reducer tests deliberately contain no lifecycle command.  The
 * control reducer owns those requests; this unit proves that completed facts
 * alone produce one-way component/Current-Console work. */
int main(void)
{
    common_session_state state;
    lib_i32 control, restart;

    for (control = 0; control != 2; ++control)
        for (restart = 0; restart != 2; ++restart) {
            check_close_lifetime(COMMON_SESSION_DISPLAY_CONSOLE, control, restart);
            check_close_lifetime(COMMON_SESSION_DISPLAY_WINDOW, control, restart);
        }

    common_session_state_initialize(&state, COMMON_SESSION_DISPLAY_CONSOLE, 1);
    common_session_state_note_runtime(&state, COMMON_SESSION_MACHINE_RUNNING);
    common_session_state_note_frame(&state, 4u, 0);
    lib_test_assert(common_session_state_take_action(&state) ==
        COMMON_UI_ACTION_CREATE_VM_CONSOLE);
    common_session_state_note_vm_console(&state, 1);
    lib_test_assert(common_session_state_take_action(&state) ==
        COMMON_UI_ACTION_BIND_VM_CONSOLE);
    common_session_state_note_current_console(&state, COMMON_SESSION_CONSOLE_VM);
    lib_test_assert(common_session_state_next_action(&state) == COMMON_UI_ACTION_NONE);

    /* Console ownership work precedes Window creation/final activation. */
    common_session_state_note_frame(&state, 5u, 1);
    lib_test_assert(common_session_state_take_action(&state) ==
        COMMON_UI_ACTION_BIND_MONITOR);
    common_session_state_note_current_console(&state, COMMON_SESSION_CONSOLE_MONITOR);
    lib_test_assert(common_session_state_take_action(&state) ==
        COMMON_UI_ACTION_DESTROY_VM_CONSOLE);
    common_session_state_note_vm_console(&state, 0);
    lib_test_assert(common_session_state_take_action(&state) ==
        COMMON_UI_ACTION_CREATE_WINDOW);
    common_session_state_note_window(&state, 1);

    /* Pause is a completed runtime fact.  It retains this graphics Window
       but keeps monitor current; Window close only changes presentation. */
    common_session_state_note_runtime(&state, COMMON_SESSION_MACHINE_PAUSED);
    lib_test_assert(common_session_state_next_action(&state) == COMMON_UI_ACTION_NONE);
    common_session_state_note_window_close(&state);
    lib_test_assert(common_session_state_take_action(&state) ==
        COMMON_UI_ACTION_DESTROY_WINDOW);
    common_session_state_note_window(&state, 0);
    lib_test_assert(common_session_state_next_action(&state) == COMMON_UI_ACTION_NONE);

    /* Stopped clears the previous route; a later running fact cannot inherit
       an old Window until runtime has supplied a new completed frame. */
    common_session_state_note_runtime(&state, COMMON_SESSION_MACHINE_STOPPED);
    common_session_state_note_runtime(&state, COMMON_SESSION_MACHINE_RUNNING);
    lib_test_assert(common_session_state_next_action(&state) == COMMON_UI_ACTION_NONE);
    common_session_state_note_frame(&state, 6u, 0);
    lib_test_assert(common_session_state_take_action(&state) ==
        COMMON_UI_ACTION_CREATE_VM_CONSOLE);
    return 0;
}
