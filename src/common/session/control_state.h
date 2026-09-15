#ifndef COMMON_SESSION_CONTROL_STATE_H
#define COMMON_SESSION_CONTROL_STATE_H

#include "common/session/presentation_plan.h"
#include "common/ui/ui_interface.h"

/* Control-thread-only product facts.  This module has no worker, native, or
 * queue dependency: callers feed copied completion facts and consume derived
 * presentation actions. */
typedef struct common_session_state {
    common_session_display display;
    lib_bool console_control;
    common_session_machine_state runtime_actual;
    lib_bool frame_actual;
    lib_bool graphics_actual;
    lib_bool window_actual;
    lib_bool vm_console_actual;
    common_session_console_actual current_console_actual;
    /* A requested effect is not an actual fact.  Until its completion is
     * returned on the control FIFO, no second transition may be emitted. */
    common_ui_action in_flight;
    lib_bool close_requested;
    common_session_machine_state monitor_actual;
    lib_u32 observed_frame_sequence;
} common_session_state;

void common_session_state_initialize(common_session_state *state,
    common_session_display display, int console_control);
void common_session_state_note_window_close(common_session_state *state);
/* Records one public runtime completion.  RESET_COMPLETED is normalized to
 * PAUSED only for presentation; its distinct completion identity remains
 * available to the command session that owns monitor wording. */
void common_session_state_note_runtime(common_session_state *state,
    common_session_machine_state completed);
int common_session_state_note_frame(common_session_state *state, lib_u32 sequence,
    int graphics);
void common_session_state_note_window(common_session_state *state, int exists);
void common_session_state_note_vm_console(common_session_state *state, int exists);
void common_session_state_note_current_console(common_session_state *state,
    int vm_console_current);
common_session_presentation_plan common_session_state_desired(const common_session_state *state);
common_ui_action common_session_state_next_action(const common_session_state *state);
common_ui_action common_session_state_take_action(common_session_state *state);
int common_session_state_monitor_is_current(const common_session_state *state);
int common_session_state_monitor_is_running_graphics_surface(
    const common_session_state *state);
int common_session_state_frame_targets_ready(const common_session_state *state);

#endif
