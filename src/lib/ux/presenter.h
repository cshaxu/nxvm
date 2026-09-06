#ifndef UX_PRESENTER_H
#define UX_PRESENTER_H

#include "lib/ux/actions.h"
#include "lib/ux/event.h"
#include "lib/ux/mailbox.h"
#include "lib/ux/router.h"

typedef enum ux_run_state { UX_RUN_STOPPED, UX_RUN_RUNNING, UX_RUN_PAUSED, UX_RUN_ERROR } ux_run_state;
typedef enum ux_run_result { UX_RUN_CONTINUE, UX_RUN_STOPPED_RESULT, UX_RUN_PAUSED_RESULT, UX_RUN_SWITCH_WINDOW, UX_RUN_SWITCH_CONSOLE, UX_RUN_ERROR_RESULT } ux_run_result;

typedef struct ux_binding {
    void *context;
    ux_mailbox *mailbox;
    ux_router *router;
    const ux_action_registry *actions;
    ux_event_sink input_sink;
    ux_run_state (*get_state)(void *context);
    ux_run_result (*handle_action)(void *context, ux_action action, ux_event_sink input_sink);
    ux_run_result (*handle_close)(void *context, ux_event_sink input_sink);
    void (*get_title)(void *context, char *buffer, lib_u32 buffer_size);
} ux_binding;

lib_status ux_binding_validate(const ux_binding *binding);
ux_run_result ux_binding_invoke_action(const ux_binding *binding,
    ux_action action, lib_u8 modifiers);

#endif
