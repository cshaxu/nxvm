#ifndef EMULATOR_PRODUCT_MONITOR_INTERFACE_H
#define EMULATOR_PRODUCT_MONITOR_INTERFACE_H

#include "emulator/session/session_interface.h"
#include "lib/types/types_interface.h"

/* Apps provide content only. Keys and descriptions must not include a line
 * ending or heading; Emulator Product owns every visible separator and row
 * layout. A null or empty description renders a key-only row. */
typedef struct emulator_product_help_row {
    const char *key;
    const char *description;
} emulator_product_help_row;

typedef struct emulator_product_help_map {
    const emulator_product_help_row *rows;
    lib_size count;
} emulator_product_help_map;

typedef enum emulator_product_monitor_command {
    EMULATOR_PRODUCT_MONITOR_COMMAND_NONE,
    EMULATOR_PRODUCT_MONITOR_COMMAND_START,
    EMULATOR_PRODUCT_MONITOR_COMMAND_RESUME,
    EMULATOR_PRODUCT_MONITOR_COMMAND_PAUSE,
    EMULATOR_PRODUCT_MONITOR_COMMAND_STOP,
    EMULATOR_PRODUCT_MONITOR_COMMAND_RESET,
    EMULATOR_PRODUCT_MONITOR_COMMAND_SAVE,
    EMULATOR_PRODUCT_MONITOR_COMMAND_LOAD,
    EMULATOR_PRODUCT_MONITOR_COMMAND_DEBUG,
    EMULATOR_PRODUCT_MONITOR_COMMAND_HELP,
    EMULATOR_PRODUCT_MONITOR_COMMAND_EXIT
} emulator_product_monitor_command;

/* Emulator owns monitor syntax, fixed-command order and result framing.
 * Product code owns the actual machine, snapshot and Debug operations. */
typedef lib_bool (*emulator_product_monitor_fixed_submit)(void *context,
    emulator_product_monitor_command command,
    emulator_session_machine_state state, const char *arguments,
    emulator_session_command_result *out_result);
/* Runs only after Emulator has accepted the fixed lifecycle state transition.
 * An App may reject an otherwise legal command for an App-private prerequisite
 * (for example, an absent cartridge), but it cannot choose another lifecycle
 * transition or completion message. */
typedef lib_status (*emulator_product_monitor_lifecycle_preflight)(void *context,
    emulator_product_monitor_command command,
    emulator_session_command_result *out_result);
typedef lib_bool (*emulator_product_monitor_extension_submit)(void *context,
    emulator_session_machine_state state, const char *line,
    emulator_session_command_result *out_result);

typedef struct emulator_product_monitor_provider {
    void *context;
    emulator_product_help_map extension_commands;
    emulator_product_help_map hotkeys;
    emulator_product_monitor_lifecycle_preflight lifecycle_preflight;
    emulator_product_monitor_fixed_submit submit_fixed;
    emulator_product_monitor_extension_submit submit_extension;
} emulator_product_monitor_provider;

/* The command shell recognizes one fixed vocabulary.  Only save/load retain
 * their original argument tail for the selected product's snapshot grammar;
 * the other fixed commands accept no arguments. */
lib_bool emulator_product_monitor_parse(const char *line,
    emulator_product_monitor_command *out_command, const char **out_arguments);

/* These callbacks are directly suitable for Emulator Session's command
 * provider when its context is an emulator_product_monitor_provider. */
void emulator_product_monitor_provider_open(void *opaque,
    emulator_session_command_result *out_result);
void emulator_product_monitor_provider_reject_line(void *opaque,
    emulator_session_command_result *out_result);
void emulator_product_monitor_provider_submit_line(void *opaque,
    emulator_session_machine_state state, const char *line,
    emulator_session_command_result *out_result);
void emulator_product_monitor_provider_note_runtime(void *opaque,
    emulator_session_machine_state prior, emulator_session_machine_state completed,
    emulator_session_command_result *out_result);

/* This is the sole fixed lifecycle admission table.  Command and hotkey
 * adapters call this same entry rather than open-coding state/request pairs. */
lib_bool emulator_product_monitor_request_lifecycle(
    const emulator_product_monitor_provider *provider,
    emulator_product_monitor_command command, emulator_session_machine_state state,
    emulator_session_command_result *out_result);

/* Extensions are inserted after the fixed commands and before hotkey help. */
lib_status emulator_product_monitor_format_help(emulator_product_help_map extensions,
    emulator_product_help_map hotkeys, char *out_text, lib_size capacity);

/* Emulator owns the one Window title convention and raw-Console framing. */
lib_status emulator_product_monitor_format_window_titles(const char *name,
    char *out_running, lib_size running_capacity, char *out_paused,
    lib_size paused_capacity);
lib_status emulator_product_monitor_format_window_status(const char *name,
    emulator_product_help_map hotkeys, char *out_text, lib_size capacity);

#endif
