#ifndef UX_PRESENTER_H
#define UX_PRESENTER_H

#include "lib/ux/actions.h"
#include "lib/ux/event.h"
#include "lib/ux/mailbox.h"
#include "lib/ux/router.h"

typedef enum ux_run_state { UX_RUN_STOPPED, UX_RUN_RUNNING, UX_RUN_PAUSED, UX_RUN_ERROR } ux_run_state;
typedef enum ux_run_result { UX_RUN_CONTINUE, UX_RUN_STOPPED_RESULT, UX_RUN_PAUSED_RESULT, UX_RUN_SWITCH_WINDOW, UX_RUN_SWITCH_CONSOLE, UX_RUN_ERROR_RESULT } ux_run_result;

typedef struct ux_binding {
    C_VOID *context;
    ux_mailbox *mailbox;
    ux_router *router;
    const ux_action_registry *actions;
    ux_event_sink input_sink;
    ux_run_state (*get_state)(C_VOID *context);
    ux_run_result (*handle_action)(C_VOID *context, ux_action action, ux_event_sink input_sink);
    ux_run_result (*handle_close)(C_VOID *context, ux_event_sink input_sink);
    C_VOID (*get_title)(C_VOID *context, C_CHAR *buffer, type_unsigned_32 buffer_size);
} ux_binding;

type_status ux_binding_validate(const ux_binding *binding);

#endif
