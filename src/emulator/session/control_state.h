#ifndef EMULATOR_SESSION_CONTROL_STATE_H
#define EMULATOR_SESSION_CONTROL_STATE_H

#include "emulator/session/presentation_plan.h"
#include "emulator/ui/ui_interface.h"

/* Control-thread-only product facts.  This module has no worker, native, or
 * queue dependency: callers feed copied completion facts and consume derived
 * presentation actions. */
typedef struct emulator_session_state {
    emulator_session_display display;
    lib_bool console_control;
    emulator_session_machine_state runtime_actual;
    lib_bool frame_actual;
    lib_bool graphics_actual;
    lib_bool window_actual;
    lib_bool vm_console_actual;
    emulator_session_console_actual current_console_actual;
    /* A requested effect is not an actual fact.  Until its completion is
     * returned on the control FIFO, no second transition may be emitted. */
    emulator_ui_action in_flight;
    /* Paused presentation retains only an already existing Window.  This is
       set by Window close and by stopped-to-paused state restoration, where
       no Window existed to retain.  A later RUNNING completion clears it. */
    lib_bool window_suppressed;
    emulator_session_machine_state monitor_actual;
    lib_u32 observed_frame_sequence;
} emulator_session_state;

/* Nonzero publication serials wrap. Compared positions must be less than
 * 2^31 apart; zero is the initial observation, not a position on that ring. */
static inline lib_bool emulator_session_frame_is_newer(lib_u32 sequence, lib_u32 previous)
{
    lib_u32 distance = (lib_u32)(sequence - previous);
    return sequence != 0u && (previous == 0u ||
        (distance != 0u && distance < 0x80000000u));
}

void emulator_session_state_initialize(emulator_session_state *state,
    emulator_session_display display, lib_bool console_control);
void emulator_session_state_note_window_close(emulator_session_state *state);
/* Records one public runtime completion.  RESET_COMPLETED is normalized to
 * PAUSED only for presentation; its distinct completion identity remains
 * available to the command session that owns monitor wording. */
void emulator_session_state_note_runtime(emulator_session_state *state,
    emulator_session_machine_state completed);
lib_bool emulator_session_state_note_frame(emulator_session_state *state, lib_u32 sequence,
    lib_bool graphics);
void emulator_session_state_note_window(emulator_session_state *state, lib_bool exists);
void emulator_session_state_note_vm_console(emulator_session_state *state, lib_bool exists);
void emulator_session_state_note_current_console(emulator_session_state *state,
    lib_bool vm_console_current);
emulator_session_presentation_plan emulator_session_state_desired(const emulator_session_state *state);
emulator_ui_action emulator_session_state_next_action(const emulator_session_state *state);
emulator_ui_action emulator_session_state_take_action(emulator_session_state *state);
lib_bool emulator_session_state_monitor_is_current(const emulator_session_state *state);
lib_bool emulator_session_state_monitor_is_running_graphics_surface(
    const emulator_session_state *state);
lib_bool emulator_session_state_frame_targets_ready(const emulator_session_state *state);

#endif
