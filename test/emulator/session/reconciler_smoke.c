#include "lib/types/test.h"
#include "emulator/session/control_state.h"


static void check_close_lifetime(emulator_session_display display, lib_i32 control,
    lib_i32 restart)
{
    emulator_session_state state;
    emulator_session_state_initialize(&state, display, control);
    emulator_session_state_note_runtime(&state, EMULATOR_SESSION_MACHINE_RUNNING);
    emulator_session_state_note_frame(&state, 1u, 1);
    emulator_session_state_note_window(&state, 1);
    emulator_session_state_note_window_close(&state);
    /* Re-observing RUNNING is not a new run, and cannot cancel pending X. */
    emulator_session_state_note_runtime(&state, EMULATOR_SESSION_MACHINE_RUNNING);
    emulator_session_state_note_runtime(&state, EMULATOR_SESSION_MACHINE_PAUSED);
    lib_test_assert(!emulator_session_state_desired(&state).window_enabled);
    emulator_session_state_note_window(&state, 0);
    emulator_session_state_note_frame(&state, 2u, 1);
    lib_test_assert(!emulator_session_state_desired(&state).window_enabled);
    if (restart) emulator_session_state_note_runtime(&state, EMULATOR_SESSION_MACHINE_STOPPED);
    emulator_session_state_note_runtime(&state, EMULATOR_SESSION_MACHINE_RUNNING);
    emulator_session_state_note_frame(&state, 3u, 1);
    lib_test_assert(emulator_session_state_desired(&state).window_enabled);
    emulator_session_state_note_window(&state, 1);
    /* CAP pause/resume uses completed runtime facts, never the old X. */
    emulator_session_state_note_runtime(&state, EMULATOR_SESSION_MACHINE_PAUSED);
    lib_test_assert(emulator_session_state_desired(&state).window_enabled);
    emulator_session_state_note_runtime(&state, EMULATOR_SESSION_MACHINE_RUNNING);
    lib_test_assert(emulator_session_state_desired(&state).window_enabled);
}

/* Presentation reducer tests deliberately contain no lifecycle command.  The
 * control reducer owns those requests; this unit proves that completed facts
 * alone produce one-way component/Current-Console work. */
int main(void)
{
    emulator_session_state state;
    lib_i32 control, restart;

    for (control = 0; control != 2; ++control)
        for (restart = 0; restart != 2; ++restart) {
            check_close_lifetime(EMULATOR_SESSION_DISPLAY_CONSOLE, control, restart);
            check_close_lifetime(EMULATOR_SESSION_DISPLAY_WINDOW, control, restart);
        }

    emulator_session_state_initialize(&state, EMULATOR_SESSION_DISPLAY_CONSOLE, 1);
    emulator_session_state_note_runtime(&state, EMULATOR_SESSION_MACHINE_RUNNING);
    emulator_session_state_note_frame(&state, 4u, 0);
    lib_test_assert(emulator_session_state_take_action(&state) ==
        EMULATOR_UI_ACTION_CREATE_VM_CONSOLE);
    emulator_session_state_note_vm_console(&state, 1);
    lib_test_assert(emulator_session_state_take_action(&state) ==
        EMULATOR_UI_ACTION_BIND_VM_CONSOLE);
    emulator_session_state_note_current_console(&state, EMULATOR_SESSION_CONSOLE_VM);
    lib_test_assert(emulator_session_state_next_action(&state) == EMULATOR_UI_ACTION_NONE);

    /* Console ownership work precedes Window creation/final activation. */
    emulator_session_state_note_frame(&state, 5u, 1);
    lib_test_assert(emulator_session_state_take_action(&state) ==
        EMULATOR_UI_ACTION_BIND_MONITOR);
    emulator_session_state_note_current_console(&state, EMULATOR_SESSION_CONSOLE_MONITOR);
    lib_test_assert(emulator_session_state_take_action(&state) ==
        EMULATOR_UI_ACTION_DESTROY_VM_CONSOLE);
    emulator_session_state_note_vm_console(&state, 0);
    lib_test_assert(emulator_session_state_take_action(&state) ==
        EMULATOR_UI_ACTION_CREATE_WINDOW);
    emulator_session_state_note_window(&state, 1);

    /* Pause is a completed runtime fact.  It retains this graphics Window
       but keeps monitor current; Window close only changes presentation. */
    emulator_session_state_note_runtime(&state, EMULATOR_SESSION_MACHINE_PAUSED);
    lib_test_assert(emulator_session_state_next_action(&state) == EMULATOR_UI_ACTION_NONE);
    emulator_session_state_note_window_close(&state);
    lib_test_assert(emulator_session_state_take_action(&state) ==
        EMULATOR_UI_ACTION_DESTROY_WINDOW);
    emulator_session_state_note_window(&state, 0);
    lib_test_assert(emulator_session_state_next_action(&state) == EMULATOR_UI_ACTION_NONE);

    /* Stopped clears the previous route; a later running fact cannot inherit
       an old Window until runtime has supplied a new completed frame. */
    emulator_session_state_note_runtime(&state, EMULATOR_SESSION_MACHINE_STOPPED);
    emulator_session_state_note_runtime(&state, EMULATOR_SESSION_MACHINE_RUNNING);
    lib_test_assert(emulator_session_state_next_action(&state) == EMULATOR_UI_ACTION_NONE);
    emulator_session_state_note_frame(&state, 6u, 0);
    lib_test_assert(emulator_session_state_take_action(&state) ==
        EMULATOR_UI_ACTION_CREATE_VM_CONSOLE);
    return 0;
}
