#ifndef COMMON_SESSION_INTERFACE_H
#define COMMON_SESSION_INTERFACE_H

#include "lib/types/types_interface.h"
#include "lib/kvm-base/event_interface.h"
#include "lib/kvm-base/frame_interface.h"
#include "common/machine/machine_interface.h"

#define COMMON_SESSION_TEXT_CAPACITY 16384u
#define COMMON_SESSION_PROMPT_CAPACITY 64u

typedef struct common_session common_session;

/* Copied facts consumed by the one common session reducer.  Product adapters
 * map their executor and configuration vocabulary at the boundary. */
typedef enum common_session_machine_state {
    COMMON_SESSION_MACHINE_INIT,
    COMMON_SESSION_MACHINE_STOPPED,
    COMMON_SESSION_MACHINE_RUNNING,
    COMMON_SESSION_MACHINE_PAUSED,
    COMMON_SESSION_MACHINE_ERROR,
    COMMON_SESSION_MACHINE_RESET_COMPLETED
} common_session_machine_state;

typedef enum common_session_display {
    COMMON_SESSION_DISPLAY_CONSOLE,
    COMMON_SESSION_DISPLAY_WINDOW
} common_session_display;

typedef enum common_session_console_actual {
    COMMON_SESSION_CONSOLE_MONITOR,
    COMMON_SESSION_CONSOLE_VM
} common_session_console_actual;

typedef struct common_session_presentation_plan {
    lib_bool window_enabled;
    lib_bool vm_console_enabled;
    lib_bool monitor_console_enabled;
} common_session_presentation_plan;

/* These are neutral control requests, not a product command vocabulary.
 * A caller supplies the one machine adapter that accepts or rejects them. */
typedef enum common_session_request {
    COMMON_SESSION_REQUEST_NONE,
    COMMON_SESSION_REQUEST_START,
    COMMON_SESSION_REQUEST_RESUME,
    COMMON_SESSION_REQUEST_PAUSE,
    COMMON_SESSION_REQUEST_STOP,
    COMMON_SESSION_REQUEST_RESET
} common_session_request;

/* A product command provider returns copied presentation text plus, at most,
 * one neutral lifecycle request. Session is the unique lifecycle dispatcher
 * and UI owner. Synchronous debug/media access uses the machine's serialized
 * executor boundary from this same control thread. */
typedef struct common_session_command_result {
    char text[COMMON_SESSION_TEXT_CAPACITY];
    char prompt[COMMON_SESSION_PROMPT_CAPACITY];
    common_session_request request;
    lib_bool exit_requested;
    lib_bool arm_prompt;
    lib_bool release_window_mouse;
} common_session_command_result;

typedef struct common_session_command_provider {
    void *context;
    void (*open)(void *context, common_session_command_result *out_result);
    void (*reject_line)(void *context, common_session_command_result *out_result);
    void (*submit_line)(void *context, common_session_machine_state state,
        const char *line, common_session_command_result *out_result);
    lib_bool (*begin_external)(void *context, common_session_machine_state state,
        common_session_request request);
    void (*note_runtime)(void *context, common_session_machine_state prior,
        common_session_machine_state completed,
        common_session_command_result *out_result);
    void (*note_broker)(void *context, common_session_machine_state state,
        lib_bool vm_console_current, lib_bool monitor_running_surface);
    void (*note_monitor_current)(void *context, lib_bool current,
        common_session_command_result *out_result);
    /* Product-owned hotkeys may request a neutral lifecycle action, release
     * Window capture, or inject product input through their own adapter. */
    lib_bool (*handle_hotkey)(void *context, common_session_machine_state state,
        const char *identifier, common_session_command_result *out_result);
} common_session_command_provider;

typedef struct common_session_options {
    common_session_display display;
    lib_bool console_control;
    common_machine *machine;
    common_session_command_provider command;
} common_session_options;

struct common_ui;
typedef struct common_ui common_ui;
typedef struct common_ui_event common_ui_event;

lib_status common_session_create(common_session **out_session,
    const common_session_options *options);
lib_status common_session_bind_ui(common_session *session, common_ui *ui);
lib_status common_session_destroy(common_session *session);
int common_session_run(common_session *session);

/* These are the only async entry points. UI and machine adapters enqueue
 * copied facts; neither calls the reducer or product command provider. */
int common_session_enqueue_ui_event(void *context,
    const common_ui_event *event);
int common_session_enqueue_runtime_completed(common_session *session,
    common_session_machine_state state, lib_u32 run_generation);
int common_session_enqueue_frame_completed(common_session *session,
    lib_u32 sequence, lib_bool graphics, lib_u32 run_generation);

#endif
