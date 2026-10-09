#ifndef APP_COMMAND_H
#define APP_COMMAND_H

#include "common/session/session_interface.h"
#include "common/machine/machine_interface.h"
#include "x86/debug/debug_interface.h"

#define APP_COMMAND_TEXT_CAPACITY 2048u
#define APP_COMMAND_PATH_CAPACITY 1024u

typedef enum app_monitor_state { APP_MONITOR_STOPPED, APP_MONITOR_PAUSED, APP_MONITOR_RUNNING, APP_MONITOR_ERROR } app_monitor_state;
typedef enum app_command_action {
    APP_COMMAND_ACTION_NONE,
    APP_COMMAND_ACTION_HELP,
    APP_COMMAND_ACTION_DEBUG
} app_command_action;
/* A parsed lifecycle request is control input, not a presentation intent.
 * Only the control loop may consume it and submit it to runtime. */
typedef enum app_lifecycle_request {
    APP_LIFECYCLE_REQUEST_NONE,
    APP_LIFECYCLE_REQUEST_START,
    APP_LIFECYCLE_REQUEST_RESUME,
    APP_LIFECYCLE_REQUEST_PAUSE,
    APP_LIFECYCLE_REQUEST_STOP,
    APP_LIFECYCLE_REQUEST_RESET
} app_lifecycle_request;

/* Product command policy only: no runtime, presenter, broker, or native I/O. */
typedef struct app_command_session {
    common_session_display display;
    app_lifecycle_request pending_request;
    int dispatch_pending;
    int transition_pending;
    int prompt_due;
    char pending_monitor_text[APP_COMMAND_TEXT_CAPACITY];
} app_command_session;

typedef struct app_command_effect {
    app_command_action action;
    int exit_requested, arm_prompt, unrecognized;
    char text[APP_COMMAND_TEXT_CAPACITY];
} app_command_effect;

/* Optional App commands are deliberately outside the shared command grammar.
 * The Product still owns monitor admission, prompts, hotkeys, Debug and the
 * lifecycle path; an App extension can only recognize and complete its own
 * command synchronously through the copied Common result. */
typedef lib_bool (*app_command_extension_submit)(void *context,
    common_machine *machine, common_session_machine_state state,
    const char *line, common_session_command_result *out);

typedef struct app_command_extensions {
    void *context;
    app_command_extension_submit submit;
    /* Command rows only: Product supplies the title, canonical rows, blank
     * separators and shared hotkey section around this App-owned middle. */
    const char *help_text;
} app_command_extensions;

/* The registered raw-Console hotkey section is product help text shared by
 * monitor `help` and the graphical raw-Console status surface. */
const char *app_command_hotkey_help(void);
void app_command_session_initialize(app_command_session *, common_session_display);
void app_command_session_open(app_command_session *, app_command_effect *);
void app_command_session_reject_line(app_command_session *, app_command_effect *);
void app_command_session_submit_line(app_command_session *, app_monitor_state,
    const char *, app_command_effect *);
/* The only monitor lifecycle-request path.  An accepted command is taken
 * exactly once; rejected and local commands have no request. */
app_lifecycle_request app_command_session_take_request(app_command_session *);
/* Registered KVM hotkeys are product control input too.  They do not create a
 * monitor-line request, but they reserve the same transition boundary before
 * control dispatches their already-derived runtime command. */
int app_command_session_begin_external(app_command_session *,
    app_monitor_state, app_lifecycle_request);
void app_command_session_note_runtime(app_command_session *, app_monitor_state,
    common_machine_state, app_command_effect *);
void app_command_session_note_broker(app_command_session *, app_monitor_state,
    int vm, int monitor_running_surface);
void app_command_session_note_monitor_current(app_command_session *, int, app_command_effect *);

/* Product CLI state and callbacks; composition installs these directly. */
/* The app chooses its CLI. Session continues owning dispatch and Console I/O. */
typedef struct app_command_context {
    app_command_session session;
    common_machine *machine;
    x86_debug *debug;
    lib_bool debug_active;
    x86_debug_result debug_completed;
    lib_bool debug_completed_pending;
    char debug_prompt[X86_DEBUG_PROMPT_CAPACITY];
    app_command_extensions extensions;
} app_command_context;

lib_status app_command_initialize(app_command_context *, common_machine *,
    common_session_display,
    const app_command_extensions *);
void app_command_dispose(app_command_context *);
void app_command_provider_open(void *, common_session_command_result *);
void app_command_provider_reject_line(void *, common_session_command_result *);
void app_command_provider_submit_line(void *, common_session_machine_state,
    const char *, common_session_command_result *);
lib_bool app_command_provider_begin_external(void *, common_session_machine_state,
    common_session_request);
void app_command_provider_note_runtime(void *, common_session_machine_state,
    common_session_machine_state, common_session_command_result *);
void app_command_provider_note_broker(void *, common_session_machine_state, lib_bool, lib_bool);
void app_command_provider_note_monitor_current(void *, lib_bool, common_session_command_result *);

#endif
