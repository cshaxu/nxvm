#ifndef COMMON_SESSION_INTERFACE_H
#define COMMON_SESSION_INTERFACE_H

#include "lib/types/types_interface.h"
#include "lib/ui-base/event_interface.h"
#include "lib/ui-base/frame_interface.h"

#define COMMON_SESSION_LINE_CAPACITY 1024u
#define COMMON_SESSION_CLI_TEXT_CAPACITY 8192u
#define COMMON_SESSION_CLI_PROMPT_CAPACITY 64u

typedef struct common_session common_session;

typedef enum common_session_lifecycle_request_kind {
    COMMON_SESSION_LIFECYCLE_NONE,
    COMMON_SESSION_LIFECYCLE_PAUSE,
    COMMON_SESSION_LIFECYCLE_RESET,
    COMMON_SESSION_LIFECYCLE_RESUME,
    COMMON_SESSION_LIFECYCLE_STEP,
    COMMON_SESSION_LIFECYCLE_STOP
} common_session_lifecycle_request_kind;

typedef struct common_session_cli_result {
    char text[COMMON_SESSION_CLI_TEXT_CAPACITY];
    char prompt[COMMON_SESSION_CLI_PROMPT_CAPACITY];
    lib_bool prompt_ready;
    lib_bool keep_active;
    common_session_lifecycle_request_kind lifecycle_request;
} common_session_cli_result;

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

typedef lib_status (*common_session_cli_provider)(void *context,
    const char *line, common_session_cli_result *out_result);
typedef lib_status (*common_session_cli_machine_observer)(void *context,
    common_session_machine_state state, lib_status status,
    common_session_cli_result *out_result);
typedef lib_status (*common_session_lifecycle_sink)(void *context,
    common_session_lifecycle_request_kind request);

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
    char console_text[COMMON_SESSION_CLI_TEXT_CAPACITY];
    char console_prompt[COMMON_SESSION_CLI_PROMPT_CAPACITY];
    lib_bool console_prompt_ready;
} common_session_plan;

typedef lib_status (*common_session_input_sink)(void *context,
    const ui_input_event *event);

lib_status common_session_create(common_session **out_session);
void common_session_destroy(common_session *session);
void common_session_close(common_session *session);
lib_status common_session_set_target(common_session *session,
    common_session_target target);
lib_status common_session_set_cli_provider(common_session *session,
    common_session_cli_provider provider, void *context);
lib_status common_session_set_cli_machine_observer(common_session *session,
    common_session_cli_machine_observer observer, void *context);
lib_bool common_session_has_cli_provider(const common_session *session);
lib_status common_session_set_lifecycle_sink(common_session *session,
    common_session_lifecycle_sink sink, void *context);
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
