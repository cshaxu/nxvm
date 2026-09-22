#include "common/session/control_state.h"

void common_session_state_initialize(common_session_state *state,
    common_session_display display, int console_control)
{
    if (state == NULL) return;
    *state = (common_session_state) { 0 };
    state->monitor_actual = COMMON_SESSION_MACHINE_INIT;
    state->display = display;
    state->console_control = console_control != 0;
    state->runtime_actual = COMMON_SESSION_MACHINE_STOPPED;
    state->current_console_actual = COMMON_SESSION_CONSOLE_MONITOR;
}

void common_session_state_note_window_close(common_session_state *state)
{
    if (state != NULL) state->window_suppressed = 1;
}

void common_session_state_note_runtime(common_session_state *state,
    common_session_machine_state completed)
{
    common_session_machine_state presentation_state = completed;
    lib_bool was_stopped;
    if (state == NULL) return;
    was_stopped = state->runtime_actual == COMMON_SESSION_MACHINE_STOPPED;
    if (completed == COMMON_SESSION_MACHINE_RESET_COMPLETED ||
        completed == COMMON_SESSION_MACHINE_PAUSED)
        state->monitor_actual = COMMON_SESSION_MACHINE_PAUSED;
    else if (completed == COMMON_SESSION_MACHINE_RUNNING)
        state->monitor_actual = COMMON_SESSION_MACHINE_RUNNING;
    else if (completed == COMMON_SESSION_MACHINE_STOPPED || completed == COMMON_SESSION_MACHINE_ERROR)
        state->monitor_actual = completed;
    if (presentation_state == COMMON_SESSION_MACHINE_RESET_COMPLETED)
        presentation_state = COMMON_SESSION_MACHINE_PAUSED;
    /* A paused view retains a pre-existing Window; it never synthesizes one.
       This covers X close and a stopped-to-paused state restore alike. */
    if (completed == COMMON_SESSION_MACHINE_PAUSED && was_stopped &&
        !state->window_actual)
        state->window_suppressed = 1;
    if (presentation_state == COMMON_SESSION_MACHINE_RUNNING &&
        state->runtime_actual != COMMON_SESSION_MACHINE_RUNNING)
        state->window_suppressed = 0;
    state->runtime_actual = presentation_state;
    if (presentation_state == COMMON_SESSION_MACHINE_STOPPED || presentation_state == COMMON_SESSION_MACHINE_ERROR) {
        /* A subsequent run must not inherit the previous run's display
         * route before it has committed a frame of its own. */
        state->frame_actual = 0;
        state->graphics_actual = 0;
    }
}

int common_session_state_note_frame(common_session_state *state, lib_u32 sequence,
    int graphics)
{
    if (state == NULL || !common_session_frame_is_newer(sequence,
            state->observed_frame_sequence)) return 0;
    state->observed_frame_sequence = sequence;
    state->frame_actual = 1;
    state->graphics_actual = graphics != 0;
    return 1;
}

void common_session_state_note_window(common_session_state *state, int exists)
{
    if (state == NULL) return;
    state->window_actual = exists != 0;
    if ((exists && state->in_flight == COMMON_UI_ACTION_CREATE_WINDOW) ||
        (!exists && state->in_flight == COMMON_UI_ACTION_DESTROY_WINDOW))
        state->in_flight = COMMON_UI_ACTION_NONE;
}

void common_session_state_note_vm_console(common_session_state *state,
    int exists)
{
    if (state == NULL) return;
    state->vm_console_actual = exists != 0;
    if ((exists && state->in_flight == COMMON_UI_ACTION_CREATE_VM_CONSOLE) ||
        (!exists && state->in_flight == COMMON_UI_ACTION_DESTROY_VM_CONSOLE))
        state->in_flight = COMMON_UI_ACTION_NONE;
}

void common_session_state_note_current_console(common_session_state *state,
    int vm_console_current)
{
    common_session_console_actual current = vm_console_current ?
        COMMON_SESSION_CONSOLE_VM : COMMON_SESSION_CONSOLE_MONITOR;
    if (state == NULL) return;
    state->current_console_actual = current;
    if ((current == COMMON_SESSION_CONSOLE_VM &&
         state->in_flight == COMMON_UI_ACTION_BIND_VM_CONSOLE) ||
        (current == COMMON_SESSION_CONSOLE_MONITOR &&
         state->in_flight == COMMON_UI_ACTION_BIND_MONITOR))
        state->in_flight = COMMON_UI_ACTION_NONE;
}

common_ui_action common_session_state_take_action(common_session_state *state)
{
    common_ui_action action;
    if (state == NULL) return COMMON_UI_ACTION_NONE;
    action = common_session_state_next_action(state);
    if (action != COMMON_UI_ACTION_NONE)
        state->in_flight = action;
    return action;
}

int common_session_state_monitor_is_current(const common_session_state *state)
{
    common_session_presentation_plan desired;
    if (state == NULL) return 0;
    desired = common_session_state_desired(state);
    return desired.monitor_console_enabled &&
        state->current_console_actual ==
            COMMON_SESSION_CONSOLE_MONITOR &&
        state->in_flight != COMMON_UI_ACTION_BIND_VM_CONSOLE;
}

int common_session_state_monitor_is_running_graphics_surface(
    const common_session_state *state)
{
    return state != NULL &&
        state->display == COMMON_SESSION_DISPLAY_CONSOLE &&
        state->runtime_actual == COMMON_SESSION_MACHINE_RUNNING &&
        state->frame_actual && state->graphics_actual &&
        common_session_state_monitor_is_current(state);
}

int common_session_state_frame_targets_ready(const common_session_state *state)
{
    common_session_presentation_plan desired;
    if (state == NULL) return 0;
    desired = common_session_state_desired(state);
    if (!desired.window_enabled && !desired.vm_console_enabled) return 0;
    if (desired.window_enabled && !state->window_actual) return 0;
    return !desired.vm_console_enabled ||
        (state->vm_console_actual &&
         state->current_console_actual == COMMON_SESSION_CONSOLE_VM);
}

common_session_presentation_plan common_session_state_desired(const common_session_state *state)
{
    common_session_presentation_plan plan = { 0, 0, 1 };
    if (state == NULL) return plan;
    plan = common_session_derive_presentation(state->display,
        state->console_control, state->runtime_actual,
        state->frame_actual,
        state->graphics_actual);
    if (state->window_suppressed &&
        state->runtime_actual == COMMON_SESSION_MACHINE_PAUSED)
        plan.window_enabled = 0;
    return plan;
}

common_ui_action common_session_state_next_action(const common_session_state *state)
{
    common_session_presentation_plan desired;
    if (state == NULL) return COMMON_UI_ACTION_NONE;
    if (state->in_flight != COMMON_UI_ACTION_NONE)
        return COMMON_UI_ACTION_NONE;

    desired = common_session_state_desired(state);
    if (desired.vm_console_enabled && !state->vm_console_actual)
        return COMMON_UI_ACTION_CREATE_VM_CONSOLE;
    if (desired.vm_console_enabled &&
        state->current_console_actual != COMMON_SESSION_CONSOLE_VM)
        return COMMON_UI_ACTION_BIND_VM_CONSOLE;
    if (!desired.vm_console_enabled &&
        state->current_console_actual != COMMON_SESSION_CONSOLE_MONITOR)
        return COMMON_UI_ACTION_BIND_MONITOR;
    if (!desired.vm_console_enabled && state->vm_console_actual)
        return COMMON_UI_ACTION_DESTROY_VM_CONSOLE;
    /* Console activation may request foreground. Finish its ownership work
       before creating the Window that should receive the final activation. */
    /* Paused state can retain a Window, but only a running machine may
       synthesize one after it is absent. */
    if (desired.window_enabled && !state->window_actual &&
        state->runtime_actual == COMMON_SESSION_MACHINE_RUNNING)
        return COMMON_UI_ACTION_CREATE_WINDOW;
    if (!desired.window_enabled && state->window_actual)
        return COMMON_UI_ACTION_DESTROY_WINDOW;
    return COMMON_UI_ACTION_NONE;
}
