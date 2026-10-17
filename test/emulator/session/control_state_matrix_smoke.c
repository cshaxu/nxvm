#include "lib/types/test.h"
#include "emulator/session/control_state.h"


static void complete_window(emulator_session_state *state)
{
    lib_test_assert(emulator_session_state_take_action(state) ==
        EMULATOR_UI_ACTION_CREATE_WINDOW);
    /* A completion barrier prevents duplicate create work. */
    lib_test_assert(emulator_session_state_take_action(state) == EMULATOR_UI_ACTION_NONE);
    emulator_session_state_note_window(state, 1);
}

static void complete_vm_console(emulator_session_state *state)
{
    lib_test_assert(emulator_session_state_take_action(state) ==
        EMULATOR_UI_ACTION_CREATE_VM_CONSOLE);
    emulator_session_state_note_vm_console(state, 1);
    lib_test_assert(emulator_session_state_take_action(state) ==
        EMULATOR_UI_ACTION_BIND_VM_CONSOLE);
    emulator_session_state_note_current_console(state, 1);
}

static void test_console_text_to_graphics_monitor(void)
{
    emulator_session_state state;
    emulator_session_state_initialize(&state, EMULATOR_SESSION_DISPLAY_CONSOLE, 1);
    lib_test_assert(state.monitor_actual == EMULATOR_SESSION_MACHINE_INIT);
    emulator_session_state_note_runtime(&state, EMULATOR_SESSION_MACHINE_RUNNING);
    lib_test_assert(state.monitor_actual == EMULATOR_SESSION_MACHINE_RUNNING);
    lib_test_assert(emulator_session_state_take_action(&state) == EMULATOR_UI_ACTION_NONE);

    lib_test_assert(emulator_session_state_note_frame(&state, 1u, 0));
    lib_test_assert(!emulator_session_state_note_frame(&state, 1u, 1));
    complete_vm_console(&state);
    lib_test_assert(!emulator_session_state_monitor_is_current(&state));
    lib_test_assert(emulator_session_state_frame_targets_ready(&state));

    lib_test_assert(emulator_session_state_note_frame(&state, 2u, 1));
    lib_test_assert(emulator_session_state_take_action(&state) ==
        EMULATOR_UI_ACTION_BIND_MONITOR);
    emulator_session_state_note_current_console(&state, 0);
    lib_test_assert(emulator_session_state_take_action(&state) ==
        EMULATOR_UI_ACTION_DESTROY_VM_CONSOLE);
    emulator_session_state_note_vm_console(&state, 0);
    complete_window(&state);
    lib_test_assert(emulator_session_state_monitor_is_current(&state));
    lib_test_assert(emulator_session_state_monitor_is_running_graphics_surface(&state));
}

static void test_console_graphics_vm_console(void)
{
    emulator_session_state state;
    emulator_session_state_initialize(&state, EMULATOR_SESSION_DISPLAY_CONSOLE, 0);
    emulator_session_state_note_runtime(&state, EMULATOR_SESSION_MACHINE_RUNNING);
    lib_test_assert(emulator_session_state_note_frame(&state, 1u, 1));
    complete_vm_console(&state);
    complete_window(&state);
    lib_test_assert(!emulator_session_state_monitor_is_current(&state));
    lib_test_assert(emulator_session_state_frame_targets_ready(&state));
}

static void test_static_window_pause_stop_and_close(void)
{
    emulator_session_state state;
    emulator_session_state_initialize(&state, EMULATOR_SESSION_DISPLAY_WINDOW, 0);
    emulator_session_state_note_runtime(&state, EMULATOR_SESSION_MACHINE_RUNNING);
    complete_window(&state);
    lib_test_assert(emulator_session_state_monitor_is_current(&state));
    emulator_session_state_note_runtime(&state, EMULATOR_SESSION_MACHINE_PAUSED);
    lib_test_assert(state.monitor_actual == EMULATOR_SESSION_MACHINE_PAUSED);
    lib_test_assert(emulator_session_state_take_action(&state) == EMULATOR_UI_ACTION_NONE);
    emulator_session_state_note_window_close(&state);
    lib_test_assert(emulator_session_state_take_action(&state) ==
        EMULATOR_UI_ACTION_DESTROY_WINDOW);
    emulator_session_state_note_window(&state, 0);
    emulator_session_state_note_runtime(&state, EMULATOR_SESSION_MACHINE_STOPPED);
    lib_test_assert(state.monitor_actual == EMULATOR_SESSION_MACHINE_STOPPED);
    lib_test_assert(emulator_session_state_take_action(&state) == EMULATOR_UI_ACTION_NONE);
    emulator_session_state_note_runtime(&state, EMULATOR_SESSION_MACHINE_ERROR);
    lib_test_assert(state.monitor_actual == EMULATOR_SESSION_MACHINE_ERROR);
    lib_test_assert(emulator_session_state_take_action(&state) == EMULATOR_UI_ACTION_NONE);
}

static void test_reset_completion_restores_paused_view(void)
{
    emulator_session_state state;
    emulator_session_state_initialize(&state, EMULATOR_SESSION_DISPLAY_CONSOLE, 0);
    emulator_session_state_note_runtime(&state, EMULATOR_SESSION_MACHINE_RUNNING);
    lib_test_assert(emulator_session_state_note_frame(&state, 1u, 1));
    complete_vm_console(&state);
    complete_window(&state);

    emulator_session_state_note_runtime(&state, EMULATOR_SESSION_MACHINE_RESET_COMPLETED);
    lib_test_assert(state.monitor_actual == EMULATOR_SESSION_MACHINE_PAUSED);
    lib_test_assert(emulator_session_state_take_action(&state) ==
        EMULATOR_UI_ACTION_BIND_MONITOR);
    emulator_session_state_note_current_console(&state, 0);
    lib_test_assert(emulator_session_state_take_action(&state) ==
        EMULATOR_UI_ACTION_DESTROY_VM_CONSOLE);
    emulator_session_state_note_vm_console(&state, 0);
    lib_test_assert(emulator_session_state_take_action(&state) == EMULATOR_UI_ACTION_NONE);

    /* Reset completes paused, retaining the last visible Window.  Resume
       restores the selected running ownership before the VM publishes its
       next frame; it does not synthesize a new route. */
    emulator_session_state_note_runtime(&state, EMULATOR_SESSION_MACHINE_RUNNING);
    lib_test_assert(!emulator_session_state_frame_targets_ready(&state));
    lib_test_assert(emulator_session_state_take_action(&state) == EMULATOR_UI_ACTION_CREATE_VM_CONSOLE);
    emulator_session_state_note_vm_console(&state, 1);
    lib_test_assert(!emulator_session_state_frame_targets_ready(&state));
    lib_test_assert(emulator_session_state_take_action(&state) == EMULATOR_UI_ACTION_BIND_VM_CONSOLE);
    lib_test_assert(!emulator_session_state_frame_targets_ready(&state));
    emulator_session_state_note_current_console(&state, 1);
    lib_test_assert(emulator_session_state_frame_targets_ready(&state));
    lib_test_assert(emulator_session_state_note_frame(&state, 2u, 0));
    lib_test_assert(emulator_session_state_frame_targets_ready(&state));
}

static void test_stopped_restore_paused_does_not_create_window(void)
{
    emulator_session_state state;

    emulator_session_state_initialize(&state, EMULATOR_SESSION_DISPLAY_WINDOW, 1);
    /* A restored graphics frame is machine state, not evidence that this
       newly stopped product had a Window to retain. */
    lib_test_assert(emulator_session_state_note_frame(&state, 1u, 1));
    emulator_session_state_note_runtime(&state, EMULATOR_SESSION_MACHINE_PAUSED);
    lib_test_assert(!emulator_session_state_desired(&state).window_enabled);
    lib_test_assert(emulator_session_state_take_action(&state) == EMULATOR_UI_ACTION_NONE);

    /* Resume starts a new active presentation lifetime normally. */
    emulator_session_state_note_runtime(&state, EMULATOR_SESSION_MACHINE_RUNNING);
    lib_test_assert(emulator_session_state_take_action(&state) ==
        EMULATOR_UI_ACTION_CREATE_WINDOW);
}

static void test_nonrunning_missing_window_never_creates(void)
{
    emulator_session_state state;
    emulator_session_machine_state states[] = {
        EMULATOR_SESSION_MACHINE_INIT,
        EMULATOR_SESSION_MACHINE_STOPPED,
        EMULATOR_SESSION_MACHINE_RESET_COMPLETED,
        EMULATOR_SESSION_MACHINE_PAUSED,
        EMULATOR_SESSION_MACHINE_ERROR
    };
    emulator_session_display displays[] = {
        EMULATOR_SESSION_DISPLAY_CONSOLE,
        EMULATOR_SESSION_DISPLAY_WINDOW
    };
    lib_size display_index;
    lib_size state_index;

    for (display_index = 0u; display_index < sizeof(displays) / sizeof(displays[0]);
         ++display_index) {
        for (state_index = 0u; state_index < sizeof(states) / sizeof(states[0]);
             ++state_index) {
            emulator_session_state_initialize(&state, displays[display_index], 0);
            lib_test_assert(emulator_session_state_note_frame(&state, 1u, 1));
            emulator_session_state_note_runtime(&state, states[state_index]);
            lib_test_assert(!state.window_actual);
            lib_test_assert(emulator_session_state_take_action(&state) !=
                EMULATOR_UI_ACTION_CREATE_WINDOW);
        }
    }
}

int main(void)
{
    test_console_text_to_graphics_monitor();
    test_console_graphics_vm_console();
    test_static_window_pause_stop_and_close();
    test_reset_completion_restores_paused_view();
    test_stopped_restore_paused_does_not_create_window();
    test_nonrunning_missing_window_never_creates();
    return 0;
}
