#include "common/session/reconciler.h"

void common_session_reconciler_initialize(common_session_reconciler *reconciler,
    common_session_display display, lib_bool console_control)
{
    if (reconciler == NULL) return;
    *reconciler = (common_session_reconciler) { 0 };
    reconciler->display = display;
    reconciler->console_control = console_control != 0;
    reconciler->runtime_actual = COMMON_SESSION_MACHINE_STOPPED;
    reconciler->current_console_actual = COMMON_SESSION_CONSOLE_MONITOR;
}

void common_session_reconciler_note_window_close(common_session_reconciler *reconciler)
{
    if (reconciler != NULL) reconciler->close_requested = 1;
}

void common_session_reconciler_note_runtime(common_session_reconciler *reconciler,
    common_session_machine_state state)
{
    if (reconciler == NULL) return;
    /* X belongs to the closed pause, not future pauses. Keep it through
       destruction; clearing there would recreate the still-paused Window. */
    if (state == COMMON_SESSION_MACHINE_RUNNING &&
        reconciler->runtime_actual != COMMON_SESSION_MACHINE_RUNNING)
        reconciler->close_requested = 0;
    reconciler->runtime_actual = state;
    if (state == COMMON_SESSION_MACHINE_STOPPED || state == COMMON_SESSION_MACHINE_ERROR) {
        /* A subsequent run must not inherit the previous run's display
         * route before it has committed a frame of its own. */
        reconciler->frame_actual = 0;
        reconciler->graphics_actual = 0;
    }
}

void common_session_reconciler_note_frame(common_session_reconciler *reconciler,
    lib_bool graphics)
{
    if (reconciler != NULL) {
        reconciler->frame_actual = 1;
        reconciler->graphics_actual = graphics != 0;
    }
}

void common_session_reconciler_note_window(common_session_reconciler *reconciler,
    lib_bool exists)
{
    if (reconciler == NULL) return;
    reconciler->window_actual = exists != 0;
    if ((exists && reconciler->in_flight == COMMON_SESSION_UI_ACTION_CREATE_WINDOW) ||
        (!exists && reconciler->in_flight == COMMON_SESSION_UI_ACTION_DESTROY_WINDOW))
        reconciler->in_flight = COMMON_SESSION_UI_ACTION_NONE;
}

void common_session_reconciler_note_vm_console(common_session_reconciler *reconciler,
    lib_bool exists)
{
    if (reconciler == NULL) return;
    reconciler->vm_console_actual = exists != 0;
    if ((exists && reconciler->in_flight == COMMON_SESSION_UI_ACTION_CREATE_VM_CONSOLE) ||
        (!exists && reconciler->in_flight == COMMON_SESSION_UI_ACTION_DESTROY_VM_CONSOLE))
        reconciler->in_flight = COMMON_SESSION_UI_ACTION_NONE;
}

void common_session_reconciler_note_current_console(common_session_reconciler *reconciler,
    common_session_console_actual current)
{
    if (reconciler == NULL) return;
    reconciler->current_console_actual = current;
    if ((current == COMMON_SESSION_CONSOLE_VM &&
         reconciler->in_flight == COMMON_SESSION_UI_ACTION_BIND_VM_CONSOLE) ||
        (current == COMMON_SESSION_CONSOLE_MONITOR &&
         reconciler->in_flight == COMMON_SESSION_UI_ACTION_BIND_MONITOR))
        reconciler->in_flight = COMMON_SESSION_UI_ACTION_NONE;
}

common_session_presentation_plan common_session_reconciler_desired(const common_session_reconciler *reconciler)
{
    common_session_presentation_plan plan = { 0, 0, 1 };
    if (reconciler == NULL) return plan;
    plan = common_session_derive_presentation(reconciler->display,
        reconciler->console_control, reconciler->runtime_actual,
        reconciler->frame_actual,
        reconciler->graphics_actual);
    if (reconciler->close_requested &&
        reconciler->runtime_actual == COMMON_SESSION_MACHINE_PAUSED)
        plan.window_enabled = 0;
    return plan;
}

common_session_ui_action common_session_reconciler_next_action(const common_session_reconciler *reconciler)
{
    common_session_presentation_plan desired;
    if (reconciler == NULL) return COMMON_SESSION_UI_ACTION_NONE;
    if (reconciler->in_flight != COMMON_SESSION_UI_ACTION_NONE)
        return COMMON_SESSION_UI_ACTION_NONE;

    desired = common_session_reconciler_desired(reconciler);
    if (desired.vm_console_enabled && !reconciler->vm_console_actual)
        return COMMON_SESSION_UI_ACTION_CREATE_VM_CONSOLE;
    if (desired.vm_console_enabled &&
        reconciler->current_console_actual != COMMON_SESSION_CONSOLE_VM)
        return COMMON_SESSION_UI_ACTION_BIND_VM_CONSOLE;
    if (!desired.vm_console_enabled &&
        reconciler->current_console_actual != COMMON_SESSION_CONSOLE_MONITOR)
        return COMMON_SESSION_UI_ACTION_BIND_MONITOR;
    if (!desired.vm_console_enabled && reconciler->vm_console_actual)
        return COMMON_SESSION_UI_ACTION_DESTROY_VM_CONSOLE;
    /* Console activation may request foreground. Finish its ownership work
       before creating the Window that should receive the final activation. */
    if (desired.window_enabled && !reconciler->window_actual)
        return COMMON_SESSION_UI_ACTION_CREATE_WINDOW;
    if (!desired.window_enabled && reconciler->window_actual)
        return COMMON_SESSION_UI_ACTION_DESTROY_WINDOW;
    return COMMON_SESSION_UI_ACTION_NONE;
}

common_session_ui_action common_session_reconciler_take_action(common_session_reconciler *reconciler)
{
    common_session_ui_action action;
    if (reconciler == NULL) return COMMON_SESSION_UI_ACTION_NONE;
    action = common_session_reconciler_next_action(reconciler);
    if (action != COMMON_SESSION_UI_ACTION_NONE)
        reconciler->in_flight = action;
    return action;
}
