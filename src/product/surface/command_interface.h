#ifndef PRODUCT_SURFACE_COMMAND_H
#define PRODUCT_SURFACE_COMMAND_H

#include "emulator/session/session_interface.h"
#include "emulator/machine/machine_interface.h"
#include "product/debug/debug_interface.h"

#define PRODUCT_SURFACE_COMMAND_TEXT_CAPACITY 2048u
#define PRODUCT_SURFACE_COMMAND_PATH_CAPACITY 1024u

typedef enum app_monitor_state { APP_MONITOR_STOPPED, APP_MONITOR_PAUSED, APP_MONITOR_RUNNING, APP_MONITOR_ERROR } app_monitor_state;
typedef enum product_surface_command_action {
    PRODUCT_SURFACE_COMMAND_ACTION_NONE,
    PRODUCT_SURFACE_COMMAND_ACTION_HELP,
    PRODUCT_SURFACE_COMMAND_ACTION_DEBUG
} product_surface_command_action;
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
typedef struct product_surface_command_session {
    emulator_session_display display;
    app_lifecycle_request pending_request;
    int dispatch_pending;
    int transition_pending;
    int prompt_due;
    char pending_monitor_text[PRODUCT_SURFACE_COMMAND_TEXT_CAPACITY];
} product_surface_command_session;

typedef struct product_surface_command_effect {
    product_surface_command_action action;
    int exit_requested, arm_prompt, unrecognized;
    char text[PRODUCT_SURFACE_COMMAND_TEXT_CAPACITY];
} product_surface_command_effect;

/* Optional App commands are deliberately outside the shared command grammar.
 * The Product still owns monitor admission, prompts, hotkeys, Debug and the
 * lifecycle path; an App extension can only recognize and complete its own
 * command synchronously through the copied Emulator result. */
typedef lib_bool (*product_surface_command_extension_submit)(void *context,
    emulator_machine *machine, emulator_session_machine_state state,
    const char *line, emulator_session_command_result *out);

typedef struct product_surface_command_extensions {
    void *context;
    product_surface_command_extension_submit submit;
    /* Command rows only: Product supplies the title, canonical rows, blank
     * separators and shared hotkey section around this App-owned middle. */
    const char *help_text;
} product_surface_command_extensions;

/* The registered raw-Console hotkey section is product help text shared by
 * monitor `help` and the graphical raw-Console status surface. */
const char *product_surface_command_hotkey_help(void);
void product_surface_command_session_initialize(product_surface_command_session *, emulator_session_display);
void product_surface_command_session_open(product_surface_command_session *, product_surface_command_effect *);
void product_surface_command_session_reject_line(product_surface_command_session *, product_surface_command_effect *);
void product_surface_command_session_submit_line(product_surface_command_session *, app_monitor_state,
    const char *, product_surface_command_effect *);
/* The only monitor lifecycle-request path.  An accepted command is taken
 * exactly once; rejected and local commands have no request. */
app_lifecycle_request product_surface_command_session_take_request(product_surface_command_session *);
/* Registered KVM hotkeys are product control input too.  They do not create a
 * monitor-line request, but they reserve the same transition boundary before
 * control dispatches their already-derived runtime command. */
int product_surface_command_session_begin_external(product_surface_command_session *,
    app_monitor_state, app_lifecycle_request);
void product_surface_command_session_note_runtime(product_surface_command_session *, app_monitor_state,
    emulator_machine_state, product_surface_command_effect *);
void product_surface_command_session_note_broker(product_surface_command_session *, app_monitor_state,
    int vm, int monitor_running_surface);
void product_surface_command_session_note_monitor_current(product_surface_command_session *, int, product_surface_command_effect *);

/* Product CLI state and callbacks; composition installs these directly. */
/* The app chooses its CLI. Session continues owning dispatch and Console I/O. */
typedef struct product_surface_command_context {
    product_surface_command_session session;
    emulator_machine *machine;
    product_debug *debug;
    lib_bool debug_active;
    product_debug_result debug_completed;
    lib_bool debug_completed_pending;
    char debug_prompt[PRODUCT_DEBUG_PROMPT_CAPACITY];
    product_surface_command_extensions extensions;
} product_surface_command_context;

lib_status product_surface_command_initialize(product_surface_command_context *, emulator_machine *,
    emulator_session_display,
    const product_surface_command_extensions *);
void product_surface_command_dispose(product_surface_command_context *);
void product_surface_command_provider_open(void *, emulator_session_command_result *);
void product_surface_command_provider_reject_line(void *, emulator_session_command_result *);
void product_surface_command_provider_submit_line(void *, emulator_session_machine_state,
    const char *, emulator_session_command_result *);
lib_bool product_surface_command_provider_begin_external(void *, emulator_session_machine_state,
    emulator_session_request);
void product_surface_command_provider_note_runtime(void *, emulator_session_machine_state,
    emulator_session_machine_state, emulator_session_command_result *);
void product_surface_command_provider_note_broker(void *, emulator_session_machine_state, lib_bool, lib_bool);
void product_surface_command_provider_note_monitor_current(void *, lib_bool, emulator_session_command_result *);

#endif
