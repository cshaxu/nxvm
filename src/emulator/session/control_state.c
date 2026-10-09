#include "emulator/session/control_state.h"

void emulator_session_state_initialize(emulator_session_state *state,
    emulator_session_display display, lib_bool console_control)
{
    if (state == NULL) return;
    *state = (emulator_session_state) { 0 };
    state->monitor_actual = EMULATOR_SESSION_MACHINE_INIT;
    state->display = display;
    state->console_control = console_control != 0;
    state->runtime_actual = EMULATOR_SESSION_MACHINE_STOPPED;
    state->current_console_actual = EMULATOR_SESSION_CONSOLE_MONITOR;
}

void emulator_session_state_note_window_close(emulator_session_state *state)
{
    if (state != NULL) state->window_suppressed = LIB_TRUE;
}

void emulator_session_state_note_runtime(emulator_session_state *state,
    emulator_session_machine_state completed)
{
    emulator_session_machine_state presentation_state = completed;
    lib_bool was_stopped;
    if (state == NULL) return;
    was_stopped = state->runtime_actual == EMULATOR_SESSION_MACHINE_STOPPED;
    if (completed == EMULATOR_SESSION_MACHINE_RESET_COMPLETED ||
        completed == EMULATOR_SESSION_MACHINE_PAUSED)
        state->monitor_actual = EMULATOR_SESSION_MACHINE_PAUSED;
    else if (completed == EMULATOR_SESSION_MACHINE_RUNNING)
        state->monitor_actual = EMULATOR_SESSION_MACHINE_RUNNING;
    else if (completed == EMULATOR_SESSION_MACHINE_STOPPED || completed == EMULATOR_SESSION_MACHINE_ERROR)
        state->monitor_actual = completed;
    if (presentation_state == EMULATOR_SESSION_MACHINE_RESET_COMPLETED)
        presentation_state = EMULATOR_SESSION_MACHINE_PAUSED;
    /* A paused view retains a pre-existing Window; it never synthesizes one.
       This covers X close and a stopped-to-paused state restore alike. */
    if (completed == EMULATOR_SESSION_MACHINE_PAUSED && was_stopped &&
        !state->window_actual)
        state->window_suppressed = LIB_TRUE;
    if (presentation_state == EMULATOR_SESSION_MACHINE_RUNNING &&
        state->runtime_actual != EMULATOR_SESSION_MACHINE_RUNNING)
        state->window_suppressed = LIB_FALSE;
    state->runtime_actual = presentation_state;
    if (presentation_state == EMULATOR_SESSION_MACHINE_STOPPED || presentation_state == EMULATOR_SESSION_MACHINE_ERROR) {
        /* A subsequent run must not inherit the previous run's display
         * route before it has committed a frame of its own. */
        state->frame_actual = LIB_FALSE;
        state->graphics_actual = LIB_FALSE;
    }
}

lib_bool emulator_session_state_note_frame(emulator_session_state *state, lib_u32 sequence,
    lib_bool graphics)
{
    if (state == NULL || !emulator_session_frame_is_newer(sequence,
            state->observed_frame_sequence)) return LIB_FALSE;
    state->observed_frame_sequence = sequence;
    state->frame_actual = LIB_TRUE;
    state->graphics_actual = graphics != 0;
    return LIB_TRUE;
}

void emulator_session_state_note_window(emulator_session_state *state, lib_bool exists)
{
    if (state == NULL) return;
    state->window_actual = exists != 0;
    if ((exists && state->in_flight == EMULATOR_UI_ACTION_CREATE_WINDOW) ||
        (!exists && state->in_flight == EMULATOR_UI_ACTION_DESTROY_WINDOW))
        state->in_flight = EMULATOR_UI_ACTION_NONE;
}

void emulator_session_state_note_vm_console(emulator_session_state *state,
    lib_bool exists)
{
    if (state == NULL) return;
    state->vm_console_actual = exists != 0;
    if ((exists && state->in_flight == EMULATOR_UI_ACTION_CREATE_VM_CONSOLE) ||
        (!exists && state->in_flight == EMULATOR_UI_ACTION_DESTROY_VM_CONSOLE))
        state->in_flight = EMULATOR_UI_ACTION_NONE;
}

void emulator_session_state_note_current_console(emulator_session_state *state,
    lib_bool vm_console_current)
{
    emulator_session_console_actual current = vm_console_current ?
        EMULATOR_SESSION_CONSOLE_VM : EMULATOR_SESSION_CONSOLE_MONITOR;
    if (state == NULL) return;
    state->current_console_actual = current;
    if ((current == EMULATOR_SESSION_CONSOLE_VM &&
         state->in_flight == EMULATOR_UI_ACTION_BIND_VM_CONSOLE) ||
        (current == EMULATOR_SESSION_CONSOLE_MONITOR &&
         state->in_flight == EMULATOR_UI_ACTION_BIND_MONITOR))
        state->in_flight = EMULATOR_UI_ACTION_NONE;
}

emulator_ui_action emulator_session_state_take_action(emulator_session_state *state)
{
    emulator_ui_action action;
    if (state == NULL) return EMULATOR_UI_ACTION_NONE;
    action = emulator_session_state_next_action(state);
    if (action != EMULATOR_UI_ACTION_NONE)
        state->in_flight = action;
    return action;
}

lib_bool emulator_session_state_monitor_is_current(const emulator_session_state *state)
{
    emulator_session_presentation_plan desired;
    if (state == NULL) return LIB_FALSE;
    desired = emulator_session_state_desired(state);
    return desired.monitor_console_enabled &&
        state->current_console_actual ==
            EMULATOR_SESSION_CONSOLE_MONITOR &&
        state->in_flight != EMULATOR_UI_ACTION_BIND_VM_CONSOLE;
}

lib_bool emulator_session_state_monitor_is_running_graphics_surface(
    const emulator_session_state *state)
{
    return state != NULL &&
        state->display == EMULATOR_SESSION_DISPLAY_CONSOLE &&
        state->runtime_actual == EMULATOR_SESSION_MACHINE_RUNNING &&
        state->frame_actual && state->graphics_actual &&
        emulator_session_state_monitor_is_current(state);
}

lib_bool emulator_session_state_frame_targets_ready(const emulator_session_state *state)
{
    emulator_session_presentation_plan desired;
    if (state == NULL) return LIB_FALSE;
    desired = emulator_session_state_desired(state);
    if (!desired.window_enabled && !desired.vm_console_enabled) return LIB_FALSE;
    if (desired.window_enabled && !state->window_actual) return LIB_FALSE;
    return !desired.vm_console_enabled ||
        (state->vm_console_actual &&
         state->current_console_actual == EMULATOR_SESSION_CONSOLE_VM);
}

emulator_session_presentation_plan emulator_session_state_desired(const emulator_session_state *state)
{
    emulator_session_presentation_plan plan = { LIB_FALSE, LIB_FALSE, LIB_TRUE };
    if (state == NULL) return plan;
    plan = emulator_session_derive_presentation(state->display,
        state->console_control, state->runtime_actual,
        state->frame_actual,
        state->graphics_actual);
    if (state->window_suppressed &&
        state->runtime_actual == EMULATOR_SESSION_MACHINE_PAUSED)
        plan.window_enabled = 0;
    return plan;
}

emulator_ui_action emulator_session_state_next_action(const emulator_session_state *state)
{
    emulator_session_presentation_plan desired;
    if (state == NULL) return EMULATOR_UI_ACTION_NONE;
    if (state->in_flight != EMULATOR_UI_ACTION_NONE)
        return EMULATOR_UI_ACTION_NONE;

    desired = emulator_session_state_desired(state);
    if (desired.vm_console_enabled && !state->vm_console_actual)
        return EMULATOR_UI_ACTION_CREATE_VM_CONSOLE;
    if (desired.vm_console_enabled &&
        state->current_console_actual != EMULATOR_SESSION_CONSOLE_VM)
        return EMULATOR_UI_ACTION_BIND_VM_CONSOLE;
    if (!desired.vm_console_enabled &&
        state->current_console_actual != EMULATOR_SESSION_CONSOLE_MONITOR)
        return EMULATOR_UI_ACTION_BIND_MONITOR;
    if (!desired.vm_console_enabled && state->vm_console_actual)
        return EMULATOR_UI_ACTION_DESTROY_VM_CONSOLE;
    /* Console activation may request foreground. Finish its ownership work
       before creating the Window that should receive the final activation. */
    /* Paused state can retain a Window, but only a running machine may
       synthesize one after it is absent. */
    if (desired.window_enabled && !state->window_actual &&
        state->runtime_actual == EMULATOR_SESSION_MACHINE_RUNNING)
        return EMULATOR_UI_ACTION_CREATE_WINDOW;
    if (!desired.window_enabled && state->window_actual)
        return EMULATOR_UI_ACTION_DESTROY_WINDOW;
    return EMULATOR_UI_ACTION_NONE;
}
