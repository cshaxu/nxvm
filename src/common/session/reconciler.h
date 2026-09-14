#ifndef COMMON_SESSION_RECONCILER_H
#define COMMON_SESSION_RECONCILER_H

#include "common/session/presentation_plan.h"
#include "common/ui/ui_interface.h"

/* This reducer is presentation-only.  The control loop owns lifecycle
 * requests and runtime dispatch; this type sees completed runtime facts and
 * derives only component/Current-Console work. */

typedef struct common_session_reconciler {
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
} common_session_reconciler;

void common_session_reconciler_initialize(common_session_reconciler *reconciler,
    common_session_display display, lib_bool console_control);
void common_session_reconciler_note_window_close(common_session_reconciler *reconciler);
void common_session_reconciler_note_runtime(common_session_reconciler *reconciler,
    common_session_machine_state state);
void common_session_reconciler_note_frame(common_session_reconciler *reconciler, lib_bool graphics);
void common_session_reconciler_note_window(common_session_reconciler *reconciler, lib_bool exists);
void common_session_reconciler_note_vm_console(common_session_reconciler *reconciler, lib_bool exists);
void common_session_reconciler_note_current_console(common_session_reconciler *reconciler,
    common_session_console_actual current);
common_session_presentation_plan common_session_reconciler_desired(const common_session_reconciler *reconciler);
common_ui_action common_session_reconciler_next_action(const common_session_reconciler *reconciler);
common_ui_action common_session_reconciler_take_action(common_session_reconciler *reconciler);

#endif
