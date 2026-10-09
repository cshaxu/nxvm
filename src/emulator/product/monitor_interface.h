#ifndef EMULATOR_PRODUCT_MONITOR_INTERFACE_H
#define EMULATOR_PRODUCT_MONITOR_INTERFACE_H

#include "emulator/session/session_interface.h"
#include "lib/types/types_interface.h"

#define EMULATOR_PRODUCT_MONITOR_PROMPT "> "

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
typedef lib_bool (*emulator_product_monitor_extension_submit)(void *context,
    emulator_session_machine_state state, const char *line,
    emulator_session_command_result *out_result);

typedef struct emulator_product_monitor_provider {
    void *context;
    const char *extension_help;
    const char *hotkey_help;
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

/* Extensions are inserted after the fixed commands and before hotkey help. */
lib_status emulator_product_monitor_format_help(const char *extensions,
    const char *hotkeys, char *out_text, lib_size capacity);

/* Keyboard policy supplies only its rows. Emulator owns the stable heading
 * and the Window raw-Console status framing. */
lib_status emulator_product_monitor_format_window_status(const char *name,
    const char *hotkeys, char *out_text, lib_size capacity);

#endif
