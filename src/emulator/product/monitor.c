#include "emulator/product/monitor_interface.h"
#include "lib/types/file.h"

static const char emulator_product_monitor_commands[] =
    "Control your virtual machine:\r\n"
    "  start          cold-reset and run the machine\r\n"
    "  reset          cold-reset and pause at firmware entry\r\n"
    "  stop           stop execution\r\n"
    "  pause          request machine pause\r\n"
    "  resume         continue a paused machine\r\n"
    "\r\n"
    "  load <file>    load a snapshot while stopped\r\n"
    "  save <file>    save a running or paused machine\r\n"
    "\r\n"
    "  debug          enter debugger (q returns to monitor)\r\n"
    "  help           show this help\r\n"
    "  exit           quit\r\n";

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
    (void)lib_c_snprintf(out_result->text, sizeof(out_result->text), "%s", text);
    out_result->arm_prompt = LIB_TRUE;
}

static void emulator_product_monitor_set_prompt(emulator_session_command_result *out_result)
{
    if (out_result == LIB_NULL || !out_result->arm_prompt) return;
    if (out_result->prompt[0] != '\0') return;
    (void)lib_c_snprintf(out_result->prompt, sizeof(out_result->prompt), "%s",
        EMULATOR_PRODUCT_MONITOR_PROMPT);
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
        emulator_product_monitor_format_help(provider->extension_help,
            provider->hotkey_help, out_result->text, sizeof(out_result->text)) != LIB_STATUS_OK) {
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
    if (provider->submit_fixed == LIB_NULL ||
        !provider->submit_fixed(provider->context, command, state, arguments, out_result)) {
        emulator_product_monitor_message(out_result, "Feature not implemented.");
    }
    emulator_product_monitor_set_prompt(out_result);
}

lib_status emulator_product_monitor_format_help(const char *extensions,
    const char *hotkeys, char *out_text, lib_size capacity)
{
    lib_size used;

    if (out_text == LIB_NULL || capacity == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    if (lib_c_snprintf(out_text, capacity, "%s", emulator_product_monitor_commands) < 0)
        return LIB_STATUS_LIMIT_EXCEEDED;
    used = lib_text_length(out_text);
    if (extensions != LIB_NULL && extensions[0] != '\0') {
        if (lib_c_snprintf(out_text + used, capacity - used, "\r\n%s", extensions) < 0)
            return LIB_STATUS_LIMIT_EXCEEDED;
        used = lib_text_length(out_text);
    }
    if (hotkeys != LIB_NULL && hotkeys[0] != '\0' &&
        lib_c_snprintf(out_text + used, capacity - used,
            "\r\nWhile the machine is running:\r\n%s\r\n", hotkeys) < 0)
        return LIB_STATUS_LIMIT_EXCEEDED;
    return LIB_STATUS_OK;
}

lib_status emulator_product_monitor_format_window_status(const char *name,
    const char *hotkeys, char *out_text, lib_size capacity)
{
    if (name == LIB_NULL || hotkeys == LIB_NULL || out_text == LIB_NULL || capacity == 0u)
        return LIB_STATUS_INVALID_ARGUMENT;
    return lib_c_snprintf(out_text, capacity,
        "%s is running in the Window.\r\n\r\nWhile the machine is running:\r\n%s\r\n",
        name, hotkeys) < 0 ? LIB_STATUS_LIMIT_EXCEEDED : LIB_STATUS_OK;
}
