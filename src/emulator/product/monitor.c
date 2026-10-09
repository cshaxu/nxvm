#include "emulator/product/monitor_interface.h"
#include "lib/types/file.h"

#define EMULATOR_PRODUCT_HELP_DESCRIPTION_COLUMN 17u

static const emulator_product_help_row emulator_product_monitor_machine_rows[] = {
    {"start", "cold-reset and run the machine"},
    {"reset", "cold-reset and pause at firmware entry"},
    {"stop", "stop execution"},
    {"pause", "request machine pause"},
    {"resume", "continue a paused machine"}
};

static const emulator_product_help_row emulator_product_monitor_snapshot_rows[] = {
    {"load <file>", "load a snapshot while stopped"},
    {"save <file>", "save a running or paused machine"}
};

static const emulator_product_help_row emulator_product_monitor_general_rows[] = {
    {"debug", "enter debugger (q returns to monitor)"},
    {"help", "show this help"},
    {"exit", "quit"}
};

static void emulator_product_monitor_clear_result(
    emulator_session_command_result *out_result)
{
    if (out_result != LIB_NULL)
        *out_result = (emulator_session_command_result){0};
}

static void emulator_product_monitor_message(emulator_session_command_result *out_result,
    const char *text)
{
    if (out_result == LIB_NULL || text == LIB_NULL) return;
    (void)lib_c_snprintf(out_result->text, sizeof(out_result->text), "%s\r\n", text);
    out_result->arm_prompt = LIB_TRUE;
}

static void emulator_product_monitor_set_prompt(emulator_session_command_result *out_result)
{
    if (out_result == LIB_NULL || !out_result->arm_prompt) return;
    if (out_result->prompt[0] != '\0') return;
    (void)lib_c_snprintf(out_result->prompt, sizeof(out_result->prompt), "%s",
        EMULATOR_SESSION_MONITOR_PROMPT);
}

static void emulator_product_monitor_lifecycle_message(
    emulator_session_command_result *out_result, const char *text)
{
    emulator_product_monitor_message(out_result, text);
    emulator_product_monitor_set_prompt(out_result);
}

lib_bool emulator_product_monitor_request_lifecycle(
    const emulator_product_monitor_provider *provider,
    emulator_product_monitor_command command, emulator_session_machine_state state,
    emulator_session_command_result *out_result)
{
    emulator_session_request request = EMULATOR_SESSION_REQUEST_NONE;
    lib_status status = LIB_STATUS_OK;

    emulator_product_monitor_clear_result(out_result);
    if (provider == LIB_NULL || out_result == LIB_NULL) return LIB_FALSE;
    if (command == EMULATOR_PRODUCT_MONITOR_COMMAND_START) {
        if (state == EMULATOR_SESSION_MACHINE_STOPPED)
            request = EMULATOR_SESSION_REQUEST_START;
        else emulator_product_monitor_lifecycle_message(out_result,
            state == EMULATOR_SESSION_MACHINE_PAUSED ?
                "Machine is paused; use resume, reset, or stop." :
                state == EMULATOR_SESSION_MACHINE_RUNNING ?
                    "Machine is already running; use pause, reset, or stop." :
                    "Machine has failed; exit and restart the program.");
    } else if (command == EMULATOR_PRODUCT_MONITOR_COMMAND_PAUSE) {
        if (state == EMULATOR_SESSION_MACHINE_RUNNING)
            request = EMULATOR_SESSION_REQUEST_PAUSE;
        else emulator_product_monitor_lifecycle_message(out_result,
            state == EMULATOR_SESSION_MACHINE_PAUSED ?
                "Machine is paused; use resume, reset, or stop." :
                state == EMULATOR_SESSION_MACHINE_STOPPED ?
                    "Machine is stopped; use start or reset." :
                    "Machine has failed; exit and restart the program.");
    } else if (command == EMULATOR_PRODUCT_MONITOR_COMMAND_RESUME) {
        if (state == EMULATOR_SESSION_MACHINE_PAUSED)
            request = EMULATOR_SESSION_REQUEST_RESUME;
        else emulator_product_monitor_lifecycle_message(out_result,
            state == EMULATOR_SESSION_MACHINE_RUNNING ?
                "Machine is already running; use pause, reset, or stop." :
                state == EMULATOR_SESSION_MACHINE_STOPPED ?
                    "Machine is stopped; use start or reset." :
                    "Machine has failed; exit and restart the program.");
    } else if (command == EMULATOR_PRODUCT_MONITOR_COMMAND_STOP) {
        if (state == EMULATOR_SESSION_MACHINE_RUNNING || state == EMULATOR_SESSION_MACHINE_PAUSED)
            request = EMULATOR_SESSION_REQUEST_STOP;
        else emulator_product_monitor_lifecycle_message(out_result,
            state == EMULATOR_SESSION_MACHINE_STOPPED ?
                "Machine is stopped; use start or reset." :
                "Machine has failed; exit and restart the program.");
    } else if (command == EMULATOR_PRODUCT_MONITOR_COMMAND_RESET) {
        if (state == EMULATOR_SESSION_MACHINE_STOPPED || state == EMULATOR_SESSION_MACHINE_RUNNING ||
            state == EMULATOR_SESSION_MACHINE_PAUSED)
            request = EMULATOR_SESSION_REQUEST_RESET;
        else emulator_product_monitor_lifecycle_message(out_result,
            "Machine has failed; exit and restart the program.");
    } else return LIB_FALSE;
    if (request == EMULATOR_SESSION_REQUEST_NONE) return LIB_TRUE;
    if (provider->lifecycle_preflight != LIB_NULL)
        status = provider->lifecycle_preflight(provider->context, command, out_result);
    if (status != LIB_STATUS_OK) {
        if (out_result->text[0] == '\0')
            emulator_product_monitor_lifecycle_message(out_result, "Feature not implemented.");
        else emulator_product_monitor_set_prompt(out_result);
        return LIB_TRUE;
    }
    out_result->request = request;
    return LIB_TRUE;
}

static lib_bool emulator_product_monitor_space(char value)
{
    return value == ' ' || value == '\t' || value == '\n' || value == '\r' ||
        value == '\f' || value == '\v';
}

static char emulator_product_monitor_lowercase(char value)
{
    return value >= 'A' && value <= 'Z' ? (char)(value + ('a' - 'A')) : value;
}

static lib_bool emulator_product_monitor_command_equal(const char *text,
    const char *word)
{
    while (*text != '\0' && *word != '\0') {
        if (emulator_product_monitor_lowercase(*text) != *word)
            return LIB_FALSE;
        ++text;
        ++word;
    }
    return *text == '\0' && *word == '\0';
}

static emulator_product_monitor_command emulator_product_monitor_lookup(
    const char *word)
{
    static const struct {
        const char *word;
        emulator_product_monitor_command command;
    } commands[] = {
        {"start", EMULATOR_PRODUCT_MONITOR_COMMAND_START},
        {"resume", EMULATOR_PRODUCT_MONITOR_COMMAND_RESUME},
        {"pause", EMULATOR_PRODUCT_MONITOR_COMMAND_PAUSE},
        {"stop", EMULATOR_PRODUCT_MONITOR_COMMAND_STOP},
        {"reset", EMULATOR_PRODUCT_MONITOR_COMMAND_RESET},
        {"save", EMULATOR_PRODUCT_MONITOR_COMMAND_SAVE},
        {"load", EMULATOR_PRODUCT_MONITOR_COMMAND_LOAD},
        {"debug", EMULATOR_PRODUCT_MONITOR_COMMAND_DEBUG},
        {"help", EMULATOR_PRODUCT_MONITOR_COMMAND_HELP},
        {"exit", EMULATOR_PRODUCT_MONITOR_COMMAND_EXIT}
    };
    lib_size index;

    for (index = 0u; index < sizeof(commands) / sizeof(commands[0]); ++index) {
        if (emulator_product_monitor_command_equal(word, commands[index].word))
            return commands[index].command;
    }
    return EMULATOR_PRODUCT_MONITOR_COMMAND_NONE;
}

lib_bool emulator_product_monitor_parse(const char *line,
    emulator_product_monitor_command *out_command, const char **out_arguments)
{
    char word[32];
    lib_size length = 0u;

    if (out_command == LIB_NULL || out_arguments == LIB_NULL)
        return LIB_FALSE;
    *out_command = EMULATOR_PRODUCT_MONITOR_COMMAND_NONE;
    *out_arguments = "";
    if (line == LIB_NULL)
        return LIB_FALSE;
    while (emulator_product_monitor_space(*line))
        ++line;
    while (*line != '\0' && !emulator_product_monitor_space(*line)) {
        if (length + 1u >= sizeof(word))
            return LIB_FALSE;
        word[length++] = *line++;
    }
    word[length] = '\0';
    while (emulator_product_monitor_space(*line))
        ++line;
    *out_command = emulator_product_monitor_lookup(word);
    *out_arguments = line;
    return *out_command != EMULATOR_PRODUCT_MONITOR_COMMAND_NONE;
}

void emulator_product_monitor_provider_open(void *opaque,
    emulator_session_command_result *out_result)
{
    const emulator_product_monitor_provider *provider = opaque;

    emulator_product_monitor_clear_result(out_result);
    if (provider == LIB_NULL || out_result == LIB_NULL ||
        emulator_product_monitor_format_help(provider->extension_commands,
            provider->hotkeys, out_result->text, sizeof(out_result->text)) != LIB_STATUS_OK) {
        emulator_product_monitor_message(out_result, "Help text is unavailable.");
        return;
    }
    out_result->arm_prompt = LIB_TRUE;
    emulator_product_monitor_set_prompt(out_result);
}

void emulator_product_monitor_provider_reject_line(void *opaque,
    emulator_session_command_result *out_result)
{
    (void)opaque;
    emulator_product_monitor_clear_result(out_result);
    emulator_product_monitor_message(out_result, "Command is too long.");
    emulator_product_monitor_set_prompt(out_result);
}

void emulator_product_monitor_provider_submit_line(void *opaque,
    emulator_session_machine_state state, const char *line,
    emulator_session_command_result *out_result)
{
    const emulator_product_monitor_provider *provider = opaque;
    emulator_product_monitor_command command;
    const char *arguments;
    const char *cursor;

    emulator_product_monitor_clear_result(out_result);
    if (provider == LIB_NULL || out_result == LIB_NULL || line == LIB_NULL) {
        emulator_product_monitor_message(out_result, "Invalid command context.");
        return;
    }
    cursor = line;
    while (emulator_product_monitor_space(*cursor)) ++cursor;
    if (*cursor == '\0') {
        out_result->arm_prompt = LIB_TRUE;
        emulator_product_monitor_set_prompt(out_result);
        return;
    }
    if (!emulator_product_monitor_parse(line, &command, &arguments)) {
        if (provider->submit_extension != LIB_NULL &&
            provider->submit_extension(provider->context, state, line, out_result)) {
            emulator_product_monitor_set_prompt(out_result);
            return;
        }
        emulator_product_monitor_message(out_result, "Unknown command.");
        emulator_product_monitor_set_prompt(out_result);
        return;
    }
    if (command != EMULATOR_PRODUCT_MONITOR_COMMAND_SAVE &&
        command != EMULATOR_PRODUCT_MONITOR_COMMAND_LOAD && *arguments != '\0') {
        emulator_product_monitor_message(out_result, "Unknown command.");
        emulator_product_monitor_set_prompt(out_result);
        return;
    }
    if (command == EMULATOR_PRODUCT_MONITOR_COMMAND_HELP) {
        emulator_product_monitor_provider_open(opaque, out_result);
        return;
    }
    if (command == EMULATOR_PRODUCT_MONITOR_COMMAND_EXIT) {
        out_result->exit_requested = LIB_TRUE;
        return;
    }
    if (command == EMULATOR_PRODUCT_MONITOR_COMMAND_START ||
        command == EMULATOR_PRODUCT_MONITOR_COMMAND_RESUME ||
        command == EMULATOR_PRODUCT_MONITOR_COMMAND_PAUSE ||
        command == EMULATOR_PRODUCT_MONITOR_COMMAND_STOP ||
        command == EMULATOR_PRODUCT_MONITOR_COMMAND_RESET) {
        (void)emulator_product_monitor_request_lifecycle(provider, command, state, out_result);
        emulator_product_monitor_set_prompt(out_result);
        return;
    }
    if (provider->submit_fixed == LIB_NULL ||
        !provider->submit_fixed(provider->context, command, state, arguments, out_result)) {
        emulator_product_monitor_message(out_result, "Feature not implemented.");
    }
    emulator_product_monitor_set_prompt(out_result);
}

void emulator_product_monitor_provider_note_runtime(void *opaque,
    emulator_session_machine_state prior, emulator_session_machine_state completed,
    emulator_session_command_result *out_result)
{
    (void)opaque;
    emulator_product_monitor_clear_result(out_result);
    if (out_result == LIB_NULL || completed == EMULATOR_SESSION_MACHINE_INIT)
        return;
    if (completed == EMULATOR_SESSION_MACHINE_RESET_COMPLETED)
        emulator_product_monitor_message(out_result, "Machine reset and paused.");
    else if (completed == EMULATOR_SESSION_MACHINE_PAUSED &&
        prior != EMULATOR_SESSION_MACHINE_PAUSED)
        emulator_product_monitor_message(out_result, "Machine paused.");
    else if (completed == EMULATOR_SESSION_MACHINE_RUNNING &&
        prior == EMULATOR_SESSION_MACHINE_STOPPED)
        emulator_product_monitor_message(out_result, "Machine started.");
    else if (completed == EMULATOR_SESSION_MACHINE_RUNNING &&
        prior == EMULATOR_SESSION_MACHINE_PAUSED)
        emulator_product_monitor_message(out_result, "Machine resumed.");
    else if (completed == EMULATOR_SESSION_MACHINE_STOPPED &&
        prior != EMULATOR_SESSION_MACHINE_STOPPED)
        emulator_product_monitor_message(out_result, "Machine stopped.");
    else if (completed == EMULATOR_SESSION_MACHINE_ERROR)
        emulator_product_monitor_message(out_result, "Machine error.");
    out_result->arm_prompt = LIB_TRUE;
    emulator_product_monitor_set_prompt(out_result);
}

static lib_status emulator_product_monitor_append(char *out_text, lib_size capacity,
    lib_size *used, const char *text)
{
    lib_size length;

    if (out_text == LIB_NULL || used == LIB_NULL || text == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    length = lib_text_length(text);
    if (*used >= capacity || length >= capacity - *used)
        return LIB_STATUS_LIMIT_EXCEEDED;
    lib_memory_copy(out_text + *used, text, length + 1u);
    *used += length;
    return LIB_STATUS_OK;
}

static lib_status emulator_product_monitor_append_spaces(char *out_text, lib_size capacity,
    lib_size *used, lib_size count)
{
    if (out_text == LIB_NULL || used == LIB_NULL || *used >= capacity ||
        count >= capacity - *used)
        return LIB_STATUS_LIMIT_EXCEEDED;
    lib_memory_set(out_text + *used, ' ', count);
    *used += count;
    out_text[*used] = '\0';
    return LIB_STATUS_OK;
}

static lib_status emulator_product_monitor_validate_rows(emulator_product_help_map rows)
{
    lib_size index;

    if (rows.count != 0u && rows.rows == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    for (index = 0u; index < rows.count; ++index) {
        const emulator_product_help_row *row = &rows.rows[index];
        const char *description;

        if (row->key == LIB_NULL || lib_text_find_substring(row->key, "\r") != LIB_NULL ||
            lib_text_find_substring(row->key, "\n") != LIB_NULL)
            return LIB_STATUS_INVALID_ARGUMENT;
        description = row->description;
        if (description != LIB_NULL && (lib_text_find_substring(description, "\r") != LIB_NULL ||
            lib_text_find_substring(description, "\n") != LIB_NULL))
            return LIB_STATUS_INVALID_ARGUMENT;
    }
    return LIB_STATUS_OK;
}

static lib_status emulator_product_monitor_append_rows(emulator_product_help_map rows,
    char *out_text, lib_size capacity, lib_size *used)
{
    lib_size index;

    if (emulator_product_monitor_validate_rows(rows) != LIB_STATUS_OK)
        return LIB_STATUS_INVALID_ARGUMENT;
    for (index = 0u; index < rows.count; ++index) {
        const emulator_product_help_row *row = &rows.rows[index];
        const char *description = row->description;
        lib_size key_length;

        key_length = lib_text_length(row->key);
        if (emulator_product_monitor_append(out_text, capacity, used, "  ") != LIB_STATUS_OK ||
            emulator_product_monitor_append(out_text, capacity, used, row->key) != LIB_STATUS_OK)
            return LIB_STATUS_LIMIT_EXCEEDED;
        if (description == LIB_NULL || description[0] == '\0') {
            if (emulator_product_monitor_append(out_text, capacity, used, "\r\n") != LIB_STATUS_OK)
                return LIB_STATUS_LIMIT_EXCEEDED;
        } else if (key_length + 2u < EMULATOR_PRODUCT_HELP_DESCRIPTION_COLUMN) {
            if (emulator_product_monitor_append_spaces(out_text, capacity, used,
                    EMULATOR_PRODUCT_HELP_DESCRIPTION_COLUMN - key_length - 2u) != LIB_STATUS_OK ||
                emulator_product_monitor_append(out_text, capacity, used, description) != LIB_STATUS_OK ||
                emulator_product_monitor_append(out_text, capacity, used, "\r\n") != LIB_STATUS_OK)
                return LIB_STATUS_LIMIT_EXCEEDED;
        } else {
            if (emulator_product_monitor_append(out_text, capacity, used, "\r\n") != LIB_STATUS_OK ||
                emulator_product_monitor_append_spaces(out_text, capacity, used,
                    EMULATOR_PRODUCT_HELP_DESCRIPTION_COLUMN) != LIB_STATUS_OK ||
                emulator_product_monitor_append(out_text, capacity, used, description) != LIB_STATUS_OK ||
                emulator_product_monitor_append(out_text, capacity, used, "\r\n") != LIB_STATUS_OK)
                return LIB_STATUS_LIMIT_EXCEEDED;
        }
    }
    return LIB_STATUS_OK;
}

lib_status emulator_product_monitor_format_help(emulator_product_help_map extensions,
    emulator_product_help_map hotkeys, char *out_text, lib_size capacity)
{
    lib_size used = 0u;
    emulator_product_help_map machine = {
        emulator_product_monitor_machine_rows,
        sizeof(emulator_product_monitor_machine_rows) / sizeof(emulator_product_monitor_machine_rows[0])};
    emulator_product_help_map snapshots = {
        emulator_product_monitor_snapshot_rows,
        sizeof(emulator_product_monitor_snapshot_rows) / sizeof(emulator_product_monitor_snapshot_rows[0])};
    emulator_product_help_map general = {
        emulator_product_monitor_general_rows,
        sizeof(emulator_product_monitor_general_rows) / sizeof(emulator_product_monitor_general_rows[0])};

    if (out_text == LIB_NULL || capacity == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (emulator_product_monitor_validate_rows(extensions) != LIB_STATUS_OK ||
        emulator_product_monitor_validate_rows(hotkeys) != LIB_STATUS_OK)
        return LIB_STATUS_INVALID_ARGUMENT;
    out_text[0] = '\0';
    if (emulator_product_monitor_append(out_text, capacity, &used,
            "Control your virtual machine:\r\n") != LIB_STATUS_OK ||
        emulator_product_monitor_append_rows(machine, out_text, capacity, &used) != LIB_STATUS_OK ||
        emulator_product_monitor_append(out_text, capacity, &used, "\r\n") != LIB_STATUS_OK ||
        emulator_product_monitor_append_rows(snapshots, out_text, capacity, &used) != LIB_STATUS_OK ||
        emulator_product_monitor_append(out_text, capacity, &used, "\r\n") != LIB_STATUS_OK ||
        emulator_product_monitor_append_rows(general, out_text, capacity, &used) != LIB_STATUS_OK)
        return LIB_STATUS_LIMIT_EXCEEDED;
    if (extensions.count != 0u) {
        if (emulator_product_monitor_append(out_text, capacity, &used, "\r\n") != LIB_STATUS_OK ||
            emulator_product_monitor_append_rows(extensions, out_text, capacity, &used) != LIB_STATUS_OK)
            return LIB_STATUS_LIMIT_EXCEEDED;
    }
    if (hotkeys.count != 0u) {
        if (emulator_product_monitor_append(out_text, capacity, &used,
                "\r\nWhile the machine is running:\r\n") != LIB_STATUS_OK ||
            emulator_product_monitor_append_rows(hotkeys, out_text, capacity, &used) != LIB_STATUS_OK)
            return LIB_STATUS_LIMIT_EXCEEDED;
    }
    return LIB_STATUS_OK;
}

lib_status emulator_product_monitor_format_window_titles(const char *name,
    char *out_running, lib_size running_capacity, char *out_paused,
    lib_size paused_capacity)
{
    if (name == LIB_NULL || out_running == LIB_NULL || out_paused == LIB_NULL ||
        running_capacity == 0u || paused_capacity == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (lib_c_snprintf(out_running, running_capacity, "%s (Running)", name) < 0 ||
        lib_c_snprintf(out_paused, paused_capacity, "%s (Paused)", name) < 0)
        return LIB_STATUS_LIMIT_EXCEEDED;
    return LIB_STATUS_OK;
}

lib_status emulator_product_monitor_format_window_status(const char *name,
    emulator_product_help_map hotkeys, char *out_text, lib_size capacity)
{
    lib_size used = 0u;

    if (name == LIB_NULL || out_text == LIB_NULL || capacity == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (emulator_product_monitor_validate_rows(hotkeys) != LIB_STATUS_OK)
        return LIB_STATUS_INVALID_ARGUMENT;
    out_text[0] = '\0';
    if (lib_c_snprintf(out_text, capacity,
            "%s is running in the Window.\r\n\r\nWhile the machine is running:\r\n",
            name) < 0)
        return LIB_STATUS_LIMIT_EXCEEDED;
    used = lib_text_length(out_text);
    return emulator_product_monitor_append_rows(hotkeys, out_text, capacity, &used);
}
