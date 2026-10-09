#ifndef APP_COMMAND_H
#define APP_COMMAND_H

#include "emulator/product/monitor_interface.h"
#include "emulator/session/session_interface.h"

#define APP_COMMAND_TEXT_CAPACITY EMULATOR_SESSION_TEXT_CAPACITY

typedef enum app_command_snapshot_result {
    APP_COMMAND_SNAPSHOT_NONE,
    APP_COMMAND_SNAPSHOT_SAVED,
    APP_COMMAND_SNAPSHOT_LOADED,
    APP_COMMAND_SNAPSHOT_SAVE_FAILED
} app_command_snapshot_result;

typedef struct app_command_context {
    /* This is MyNES's one Session-provider adapter. It delegates the primary
     * monitor grammar to Emulator Product and owns only NES extensions/debug. */
    emulator_machine *machine;
    emulator_product_monitor_provider monitor;
    emulator_session_display display;
    lib_bool debug_active;
    lib_bool cartridge_present;
    lib_bool run_after_reset;
    lib_bool started_after_reset;
    lib_bool initial_state_pending;
    /* Window mode keeps a stopped cartridge reset in the cooked monitor until
     * the user starts or resumes it.  App maps that one completion to PAUSED
     * for Common's presentation policy, while retaining reset wording here. */
    lib_bool suppress_window_after_reset;
    lib_bool report_suppressed_reset;
    app_command_snapshot_result pending_snapshot;
    void *media_context;
    lib_bool (*set_media)(void *context, const char *path);
} app_command_context;

void app_command_initialize(app_command_context *context, emulator_machine *machine,
    lib_bool cartridge_present,
    emulator_session_display display);
void app_command_open(void *context, emulator_session_command_result *out_result);
void app_command_reject_line(void *context, emulator_session_command_result *out_result);
void app_command_submit_line(void *context, emulator_session_machine_state state,
    const char *line, emulator_session_command_result *out_result);
lib_bool app_command_handle_hotkey(void *context, emulator_session_machine_state state,
    const lib_u8 *identifier, emulator_session_command_result *out_result);
void app_command_note_runtime(void *context, emulator_session_machine_state prior,
    emulator_session_machine_state completed, emulator_session_command_result *out_result);

#endif
