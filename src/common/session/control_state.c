#include "common/session/control_state.h"

void common_session_state_initialize(common_session_state *state,
    common_session_display display, int console_control)
{
    if (state == NULL) return;
    *state = (common_session_state) { 0 };
    state->monitor_actual = COMMON_SESSION_MACHINE_INIT;
    common_session_reconciler_initialize(&state->presentation, display, console_control);
}

void common_session_state_note_window_close(common_session_state *state)
{
    if (state != NULL) common_session_reconciler_note_window_close(&state->presentation);
}

void common_session_state_note_runtime(common_session_state *state,
    common_session_machine_state completed)
{
    common_session_machine_state presentation_state = completed;
    if (state == NULL) return;
    if (completed == COMMON_SESSION_MACHINE_RESET_COMPLETED ||
        completed == COMMON_SESSION_MACHINE_PAUSED)
        state->monitor_actual = COMMON_SESSION_MACHINE_PAUSED;
    else if (completed == COMMON_SESSION_MACHINE_RUNNING)
        state->monitor_actual = COMMON_SESSION_MACHINE_RUNNING;
    else if (completed == COMMON_SESSION_MACHINE_STOPPED || completed == COMMON_SESSION_MACHINE_ERROR)
        state->monitor_actual = COMMON_SESSION_MACHINE_STOPPED;
    if (presentation_state == COMMON_SESSION_MACHINE_RESET_COMPLETED)
        presentation_state = COMMON_SESSION_MACHINE_PAUSED;
    common_session_reconciler_note_runtime(&state->presentation, presentation_state);
}

int common_session_state_note_frame(common_session_state *state, lib_u32 sequence,
    int graphics)
{
    if (state == NULL || sequence == 0u ||
        sequence <= state->observed_frame_sequence) return 0;
    state->observed_frame_sequence = sequence;
    common_session_reconciler_note_frame(&state->presentation, graphics);
    return 1;
}

void common_session_state_note_window(common_session_state *state, int exists)
{
    if (state != NULL) common_session_reconciler_note_window(&state->presentation, exists);
}

void common_session_state_note_vm_console(common_session_state *state, int exists)
{
    if (state != NULL)
        common_session_reconciler_note_vm_console(&state->presentation, exists);
}

void common_session_state_note_current_console(common_session_state *state,
    int vm_console_current)
{
    if (state != NULL) common_session_reconciler_note_current_console(&state->presentation,
        vm_console_current ? COMMON_SESSION_CONSOLE_VM :
        COMMON_SESSION_CONSOLE_MONITOR);
}

common_session_ui_action common_session_state_take_action(common_session_state *state)
{
    return state == NULL ? COMMON_SESSION_UI_ACTION_NONE :
        common_session_reconciler_take_action(&state->presentation);
}

int common_session_state_monitor_is_current(const common_session_state *state)
{
    common_session_presentation_plan desired;
    if (state == NULL) return 0;
    desired = common_session_reconciler_desired(&state->presentation);
    return desired.monitor_console_enabled &&
        state->presentation.current_console_actual ==
            COMMON_SESSION_CONSOLE_MONITOR &&
        state->presentation.in_flight != COMMON_SESSION_UI_ACTION_BIND_VM_CONSOLE;
}

int common_session_state_monitor_is_running_graphics_surface(
    const common_session_state *state)
{
    return state != NULL &&
        state->presentation.display == COMMON_SESSION_DISPLAY_CONSOLE &&
        state->presentation.runtime_actual == COMMON_SESSION_MACHINE_RUNNING &&
        state->presentation.frame_actual && state->presentation.graphics_actual &&
        common_session_state_monitor_is_current(state);
}

int common_session_state_frame_targets_ready(const common_session_state *state)
{
    common_session_presentation_plan desired;
    if (state == NULL) return 0;
    desired = common_session_reconciler_desired(&state->presentation);
    if (!desired.window_enabled && !desired.vm_console_enabled) return 0;
    if (desired.window_enabled && !state->presentation.window_actual) return 0;
    return !desired.vm_console_enabled ||
        (state->presentation.vm_console_actual &&
         state->presentation.current_console_actual == COMMON_SESSION_CONSOLE_VM);
}
