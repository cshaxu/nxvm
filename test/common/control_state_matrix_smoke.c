#include "common/session/control_state.h"

#include <assert.h>

static void complete_window(common_session_state *state)
{
    assert(common_session_state_take_action(state) ==
        COMMON_UI_ACTION_CREATE_WINDOW);
    /* A completion barrier prevents duplicate create work. */
    assert(common_session_state_take_action(state) == COMMON_UI_ACTION_NONE);
    common_session_state_note_window(state, 1);
}

static void complete_vm_console(common_session_state *state)
{
    assert(common_session_state_take_action(state) ==
        COMMON_UI_ACTION_CREATE_VM_CONSOLE);
    common_session_state_note_vm_console(state, 1);
    assert(common_session_state_take_action(state) ==
        COMMON_UI_ACTION_BIND_VM_CONSOLE);
    common_session_state_note_current_console(state, 1);
}

static void test_console_text_to_graphics_monitor(void)
{
    common_session_state state;
    common_session_state_initialize(&state, COMMON_SESSION_DISPLAY_CONSOLE, 1);
    assert(state.monitor_actual == COMMON_SESSION_MACHINE_INIT);
    common_session_state_note_runtime(&state, COMMON_SESSION_MACHINE_RUNNING);
    assert(state.monitor_actual == COMMON_SESSION_MACHINE_RUNNING);
    assert(common_session_state_take_action(&state) == COMMON_UI_ACTION_NONE);

    assert(common_session_state_note_frame(&state, 1u, 0));
    assert(!common_session_state_note_frame(&state, 1u, 1));
    complete_vm_console(&state);
    assert(!common_session_state_monitor_is_current(&state));
    assert(common_session_state_frame_targets_ready(&state));

    assert(common_session_state_note_frame(&state, 2u, 1));
    assert(common_session_state_take_action(&state) ==
        COMMON_UI_ACTION_BIND_MONITOR);
    common_session_state_note_current_console(&state, 0);
    assert(common_session_state_take_action(&state) ==
        COMMON_UI_ACTION_DESTROY_VM_CONSOLE);
    common_session_state_note_vm_console(&state, 0);
    complete_window(&state);
    assert(common_session_state_monitor_is_current(&state));
    assert(common_session_state_monitor_is_running_graphics_surface(&state));
}

static void test_console_graphics_vm_console(void)
{
    common_session_state state;
    common_session_state_initialize(&state, COMMON_SESSION_DISPLAY_CONSOLE, 0);
    common_session_state_note_runtime(&state, COMMON_SESSION_MACHINE_RUNNING);
    assert(common_session_state_note_frame(&state, 1u, 1));
    complete_vm_console(&state);
    complete_window(&state);
    assert(!common_session_state_monitor_is_current(&state));
    assert(common_session_state_frame_targets_ready(&state));
}

static void test_static_window_pause_stop_and_close(void)
{
    common_session_state state;
    common_session_state_initialize(&state, COMMON_SESSION_DISPLAY_WINDOW, 0);
    common_session_state_note_runtime(&state, COMMON_SESSION_MACHINE_RUNNING);
    complete_window(&state);
    assert(common_session_state_monitor_is_current(&state));
    common_session_state_note_runtime(&state, COMMON_SESSION_MACHINE_PAUSED);
    assert(state.monitor_actual == COMMON_SESSION_MACHINE_PAUSED);
    assert(common_session_state_take_action(&state) == COMMON_UI_ACTION_NONE);
    common_session_state_note_window_close(&state);
    assert(common_session_state_take_action(&state) ==
        COMMON_UI_ACTION_DESTROY_WINDOW);
    common_session_state_note_window(&state, 0);
    common_session_state_note_runtime(&state, COMMON_SESSION_MACHINE_STOPPED);
    assert(state.monitor_actual == COMMON_SESSION_MACHINE_STOPPED);
    assert(common_session_state_take_action(&state) == COMMON_UI_ACTION_NONE);
}

static void test_reset_completion_restores_paused_view(void)
{
    common_session_state state;
    common_session_state_initialize(&state, COMMON_SESSION_DISPLAY_CONSOLE, 0);
    common_session_state_note_runtime(&state, COMMON_SESSION_MACHINE_RUNNING);
    assert(common_session_state_note_frame(&state, 1u, 1));
    complete_vm_console(&state);
    complete_window(&state);

    common_session_state_note_runtime(&state, COMMON_SESSION_MACHINE_RESET_COMPLETED);
    assert(state.monitor_actual == COMMON_SESSION_MACHINE_PAUSED);
    assert(common_session_state_take_action(&state) ==
        COMMON_UI_ACTION_BIND_MONITOR);
    common_session_state_note_current_console(&state, 0);
    assert(common_session_state_take_action(&state) ==
        COMMON_UI_ACTION_DESTROY_VM_CONSOLE);
    common_session_state_note_vm_console(&state, 0);
    assert(common_session_state_take_action(&state) == COMMON_UI_ACTION_NONE);

    /* Reset completes paused, retaining the last visible Window.  Resume
       restores the selected running ownership before the VM publishes its
       next frame; it does not synthesize a new route. */
    common_session_state_note_runtime(&state, COMMON_SESSION_MACHINE_RUNNING);
    assert(!common_session_state_frame_targets_ready(&state));
    assert(common_session_state_take_action(&state) == COMMON_UI_ACTION_CREATE_VM_CONSOLE);
    common_session_state_note_vm_console(&state, 1);
    assert(!common_session_state_frame_targets_ready(&state));
    assert(common_session_state_take_action(&state) == COMMON_UI_ACTION_BIND_VM_CONSOLE);
    assert(!common_session_state_frame_targets_ready(&state));
    common_session_state_note_current_console(&state, 1);
    assert(common_session_state_frame_targets_ready(&state));
    assert(common_session_state_note_frame(&state, 2u, 0));
    assert(common_session_state_frame_targets_ready(&state));
}

static void test_stopped_restore_paused_does_not_create_window(void)
{
    common_session_state state;

    common_session_state_initialize(&state, COMMON_SESSION_DISPLAY_WINDOW, 1);
    /* A restored graphics frame is machine state, not evidence that this
       newly stopped product had a Window to retain. */
    assert(common_session_state_note_frame(&state, 1u, 1));
    common_session_state_note_runtime(&state, COMMON_SESSION_MACHINE_PAUSED);
    assert(!common_session_state_desired(&state).window_enabled);
    assert(common_session_state_take_action(&state) == COMMON_UI_ACTION_NONE);

    /* Resume starts a new active presentation lifetime normally. */
    common_session_state_note_runtime(&state, COMMON_SESSION_MACHINE_RUNNING);
    assert(common_session_state_take_action(&state) ==
        COMMON_UI_ACTION_CREATE_WINDOW);
}

int main(void)
{
    test_console_text_to_graphics_monitor();
    test_console_graphics_vm_console();
    test_static_window_pause_stop_and_close();
    test_reset_completion_restores_paused_view();
    test_stopped_restore_paused_does_not_create_window();
    return 0;
}
