#ifndef COMMON_SESSION_CONTROL_STATE_H
#define COMMON_SESSION_CONTROL_STATE_H

#include "common/session/reconciler.h"

/* Control-thread-only product facts.  This module has no worker, native, or
 * queue dependency: callers feed copied completion facts and consume derived
 * presentation actions. */
typedef struct common_session_state {
    common_session_reconciler presentation;
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
common_session_ui_action common_session_state_take_action(common_session_state *state);
int common_session_state_monitor_is_current(const common_session_state *state);
int common_session_state_monitor_is_running_graphics_surface(
    const common_session_state *state);
int common_session_state_frame_targets_ready(const common_session_state *state);

#endif
