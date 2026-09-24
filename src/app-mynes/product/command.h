#ifndef APP_COMMAND_H
#define APP_COMMAND_H

#include "common/session/session_interface.h"

#define APP_COMMAND_TEXT_CAPACITY COMMON_SESSION_TEXT_CAPACITY

typedef enum app_command_snapshot_result {
    APP_COMMAND_SNAPSHOT_NONE,
    APP_COMMAND_SNAPSHOT_SAVED,
    APP_COMMAND_SNAPSHOT_LOADED,
    APP_COMMAND_SNAPSHOT_SAVE_FAILED
} app_command_snapshot_result;

typedef struct app_command_session {
    common_session_display display;
    lib_bool transition_pending;
    lib_bool prompt_due;
    app_command_snapshot_result pending_snapshot;
    lib_u8 pending_monitor_text[APP_COMMAND_TEXT_CAPACITY];
} app_command_session;

typedef struct app_command_context {
    app_command_session session;
    common_machine *machine;
    lib_bool cartridge_present;
    lib_bool run_after_reset;
    lib_bool started_after_reset;
    lib_bool initial_reset;
    lib_bool initial_state_pending;
    /* Window mode keeps a stopped cartridge reset in the cooked monitor until
     * the user starts or resumes it.  App maps that one completion to PAUSED
     * for Common's presentation policy, while retaining reset wording here. */
    lib_bool suppress_window_after_reset;
    lib_bool report_suppressed_reset;
    void *media_context;
    lib_bool (*set_media)(void *context, const char *path);
} app_command_context;

void app_command_initialize(app_command_context *context, common_machine *machine,
    lib_bool cartridge_present, lib_bool initial_reset,
    common_session_display display);
void app_command_open(void *context, common_session_command_result *out_result);
void app_command_reject_line(void *context, common_session_command_result *out_result);
void app_command_submit_line(void *context, common_session_machine_state state,
    const char *line, common_session_command_result *out_result);
lib_bool app_command_begin_external(void *context, common_session_machine_state state,
    common_session_request request);
lib_bool app_command_handle_hotkey(void *context, common_session_machine_state state,
    const char *identifier, common_session_command_result *out_result);
void app_command_note_runtime(void *context, common_session_machine_state prior,
    common_session_machine_state completed, common_session_command_result *out_result);
void app_command_note_broker(void *context, common_session_machine_state state,
    lib_bool vm_console_current, lib_bool monitor_running_surface);
void app_command_note_monitor_current(void *context, lib_bool current,
    common_session_command_result *out_result);

#endif
