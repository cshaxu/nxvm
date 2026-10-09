#ifndef EMULATOR_SESSION_INTERFACE_H
#define EMULATOR_SESSION_INTERFACE_H

#include "lib/types/types_interface.h"
#include "lib/kvm-base/event_interface.h"
#include "lib/kvm-base/frame_interface.h"
#include "emulator/machine/machine_interface.h"

#define EMULATOR_SESSION_TEXT_CAPACITY 16384u
#define EMULATOR_SESSION_PROMPT_CAPACITY 64u

typedef struct emulator_session emulator_session;

/* Copied facts consumed by the one emulator session reducer.  Product adapters
 * map their executor and configuration vocabulary at the boundary. */
typedef enum emulator_session_machine_state {
    EMULATOR_SESSION_MACHINE_INIT,
    EMULATOR_SESSION_MACHINE_STOPPED,
    EMULATOR_SESSION_MACHINE_RUNNING,
    EMULATOR_SESSION_MACHINE_PAUSED,
    EMULATOR_SESSION_MACHINE_ERROR,
    EMULATOR_SESSION_MACHINE_RESET_COMPLETED
} emulator_session_machine_state;

typedef enum emulator_session_display {
    EMULATOR_SESSION_DISPLAY_CONSOLE,
    EMULATOR_SESSION_DISPLAY_WINDOW
} emulator_session_display;

typedef enum emulator_session_console_actual {
    EMULATOR_SESSION_CONSOLE_MONITOR,
    EMULATOR_SESSION_CONSOLE_VM
} emulator_session_console_actual;

typedef struct emulator_session_presentation_plan {
    lib_bool window_enabled;
    lib_bool vm_console_enabled;
    lib_bool monitor_console_enabled;
} emulator_session_presentation_plan;

/* These are neutral control requests, not a product command vocabulary.
 * A caller supplies the one machine adapter that accepts or rejects them. */
typedef enum emulator_session_request {
    EMULATOR_SESSION_REQUEST_NONE,
    EMULATOR_SESSION_REQUEST_START,
    EMULATOR_SESSION_REQUEST_RESUME,
    EMULATOR_SESSION_REQUEST_PAUSE,
    EMULATOR_SESSION_REQUEST_STOP,
    EMULATOR_SESSION_REQUEST_RESET
} emulator_session_request;

/* A product command provider parses one line and returns copied presentation
 * text plus, at most, one neutral lifecycle request. Session is the unique
 * lifecycle dispatcher, reader owner and delayed-result owner. Synchronous
 * debug/media access uses the machine's serialized executor boundary from
 * this same control thread. When text/detail re-arms a prompt, normal Monitor
 * output ends in one line ending and Session adds one CRLF blank separator;
 * Debug detail has no terminal line ending, so its prompt begins on the next
 * line without a blank separator. */
typedef struct emulator_session_command_result {
    char text[EMULATOR_SESSION_TEXT_CAPACITY];
    /* Optional additional text, borrowed until the next provider call.
     * Consumed synchronously by Session after text, before requests/prompt.
     * LF and CRLF are accepted; no fixed-size copy or ownership transfer. */
    const char *detail;
    char prompt[EMULATOR_SESSION_PROMPT_CAPACITY];
    emulator_session_request request;
    lib_bool exit_requested;
    /* Level-triggered readiness, not proof that a reader was requested.
     * Supply prompt whenever true; session admits it only without a pending
     * line and while the monitor is Current. Text/requests are still handled. */
    lib_bool arm_prompt;
    lib_bool release_window_mouse;
} emulator_session_command_result;

typedef struct emulator_session_command_provider {
    void *context;
    void (*open)(void *context, emulator_session_command_result *out_result);
    void (*reject_line)(void *context, emulator_session_command_result *out_result);
    void (*submit_line)(void *context, emulator_session_machine_state state,
        const char *line, emulator_session_command_result *out_result);
    void (*note_runtime)(void *context, emulator_session_machine_state prior,
        emulator_session_machine_state completed,
        emulator_session_command_result *out_result);
    /* Product-owned hotkeys may request a neutral lifecycle action, release
     * Window capture, or inject product input through their own adapter. */
    lib_bool (*handle_hotkey)(void *context, emulator_session_machine_state state,
        const lib_u8 *identifier, emulator_session_command_result *out_result);
} emulator_session_command_provider;

typedef struct emulator_session_options {
    emulator_session_display display;
    lib_bool console_control;
    emulator_machine *machine;
    emulator_session_command_provider command;
} emulator_session_options;

struct emulator_ui;
typedef struct emulator_ui emulator_ui;
typedef struct emulator_ui_event emulator_ui_event;

lib_status emulator_session_create(emulator_session **out_session,
    const emulator_session_options *options);
lib_status emulator_session_bind_ui(emulator_session *session, emulator_ui *ui);
lib_status emulator_session_destroy(emulator_session *session);
lib_bool emulator_session_run(emulator_session *session);

/* These are the only async entry points. UI and machine adapters enqueue
 * copied facts; neither calls the reducer or product command provider. */
lib_bool emulator_session_enqueue_ui_event(void *context,
    const emulator_ui_event *event);
lib_bool emulator_session_enqueue_runtime_completed(emulator_session *session,
    emulator_session_machine_state state, lib_u32 run_generation);
lib_bool emulator_session_enqueue_frame_completed(emulator_session *session,
    lib_u32 sequence, lib_bool graphics, lib_u32 run_generation);

#endif
