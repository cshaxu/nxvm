#ifndef UX_PRESENTER_H
#define UX_PRESENTER_H

#include "lib/ux/actions.h"
#include "lib/ux/event.h"
#include "lib/ux/frame.h"

#define UX_WINDOW_TITLE_CAPACITY 128u

typedef enum ux_target { UX_TARGET_NONE, UX_TARGET_WINDOW, UX_TARGET_CONSOLE } ux_target;
typedef struct ux_presenter ux_presenter;

/* This is a copied presenter state, never a native handle.  CAPTURED means
 * the active Window UI surface has completed the native capture operation. */
typedef enum ux_mouse_capture_state {
    UX_MOUSE_CAPTURE_RELEASED,
    UX_MOUSE_CAPTURE_CAPTURED
} ux_mouse_capture_state;

typedef enum ux_run_state { UX_RUN_STOPPED, UX_RUN_RUNNING, UX_RUN_PAUSED, UX_RUN_ERROR } ux_run_state;
typedef enum ux_run_result { UX_RUN_CONTINUE, UX_RUN_STOPPED_RESULT, UX_RUN_PAUSED_RESULT, UX_RUN_SWITCH_WINDOW, UX_RUN_SWITCH_CONSOLE, UX_RUN_ERROR_RESULT } ux_run_result;

typedef struct ux_binding {
    void *context;
    ux_presenter *presenter;
    const ux_action_registry *actions;
    ux_event_sink input_sink;
    int (*release_pressed_keys)(void *context, ux_event_sink input_sink);
    ux_run_state (*get_state)(void *context);
    ux_run_result (*handle_action)(void *context, ux_action action, ux_event_sink input_sink);
    ux_run_result (*handle_close)(void *context, ux_event_sink input_sink);
    char window_initial_title[UX_WINDOW_TITLE_CAPACITY];
} ux_binding;

/* The presenter owns private latest-state target, title, mouse-control and
 * frame mailboxes and their one native wake mechanism. Mouse capture policy
 * and idempotent release request are separate latest-state slots. Native
 * Window runners process target, title, capturable, release, then frame. */
lib_status ux_presenter_create(ux_presenter **out_presenter);
void ux_presenter_destroy(ux_presenter *presenter);
lib_status ux_presenter_publish_frame(ux_presenter *presenter,
    const ux_frame *frame);
lib_status ux_presenter_set_target(ux_presenter *presenter, ux_target target);
lib_status ux_presenter_set_window_title(ux_presenter *presenter,
    const char *title);
/* App policy controls whether the active Window may acquire relative capture.
 * Disabling and explicit release are delivered asynchronously to the native
 * presenter; Console and absent-Window requests are harmless no-ops. */
lib_status ux_presenter_set_mouse_capturable(ux_presenter *presenter,
    lib_bool capturable);
lib_status ux_presenter_release_mouse(ux_presenter *presenter);
ux_mouse_capture_state ux_presenter_mouse_capture_state(
    const ux_presenter *presenter);
ux_run_result ux_run(const ux_binding *binding);
lib_status ux_binding_validate(const ux_binding *binding);
ux_run_result ux_binding_invoke_action(const ux_binding *binding,
    ux_action action);

#endif
