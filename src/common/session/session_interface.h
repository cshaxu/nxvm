#ifndef COMMON_SESSION_INTERFACE_H
#define COMMON_SESSION_INTERFACE_H

#include "lib/types/types_interface.h"
#include "lib/ui-base/event_interface.h"
#include "lib/ui-base/frame_interface.h"

#define COMMON_SESSION_LINE_CAPACITY 1024u

typedef struct common_session common_session;

typedef enum common_session_target {
    COMMON_SESSION_TARGET_NONE,
    COMMON_SESSION_TARGET_CONSOLE,
    COMMON_SESSION_TARGET_WINDOW
} common_session_target;

typedef enum common_session_machine_state {
    COMMON_SESSION_MACHINE_RUNNING,
    COMMON_SESSION_MACHINE_PAUSED,
    COMMON_SESSION_MACHINE_RESET,
    COMMON_SESSION_MACHINE_STOPPED,
    COMMON_SESSION_MACHINE_FAULT
} common_session_machine_state;

typedef enum common_session_fact_kind {
    COMMON_SESSION_FACT_CONSOLE_LINE,
    COMMON_SESSION_FACT_MACHINE,
    COMMON_SESSION_FACT_UI_INPUT,
    COMMON_SESSION_FACT_FRAME
} common_session_fact_kind;

typedef struct common_session_fact {
    common_session_fact_kind kind;
    lib_u32 run_id;
    union {
        char line[COMMON_SESSION_LINE_CAPACITY];
        struct {
            common_session_machine_state state;
            lib_status status;
        } machine;
        ui_input_event input;
    } value;
} common_session_fact;

typedef enum common_session_notice {
    COMMON_SESSION_NOTICE_NONE,
    COMMON_SESSION_NOTICE_STARTED,
    COMMON_SESSION_NOTICE_RESUMED,
    COMMON_SESSION_NOTICE_PAUSED,
    COMMON_SESSION_NOTICE_RESET,
    COMMON_SESSION_NOTICE_STOPPED
} common_session_notice;

typedef struct common_session_plan {
    lib_bool target_changed;
    common_session_target target;
    lib_bool mouse_capturable_changed;
    lib_bool mouse_capturable;
    lib_bool release_mouse;
    lib_bool frame_ready;
    ui_frame frame;
    common_session_notice notice;
} common_session_plan;

typedef lib_status (*common_session_input_sink)(void *context,
    const ui_input_event *event);

lib_status common_session_create(common_session **out_session);
void common_session_destroy(common_session *session);
void common_session_close(common_session *session);
lib_status common_session_set_target(common_session *session,
    common_session_target target);
lib_u32 common_session_begin_run(common_session *session,
    common_session_plan *out_plan);
lib_bool common_session_is_running(const common_session *session);
lib_status common_session_publish_console_line(void *context, const char *line);
lib_status common_session_publish_ui_input(void *context,
    const ui_input_event *event);
lib_status common_session_publish_machine(common_session *session,
    common_session_machine_state state, lib_status status);
lib_status common_session_publish_frame(common_session *session,
    const ui_frame *frame);
lib_status common_session_take(common_session *session,
    common_session_fact *out_fact, ui_frame *out_frame,
    lib_u32 timeout_milliseconds);
lib_status common_session_reduce_fact(common_session *session,
    const common_session_fact *fact, const ui_frame *frame,
    common_session_plan *out_plan);
lib_status common_session_dispatch_host_input(common_session *session,
    const ui_input_event *event, common_session_input_sink sink,
    void *sink_context);
#endif
