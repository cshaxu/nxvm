#include "emulator/product/monitor_interface.h"
#include "lib/types/file.h"
#include "lib/types/test.h"

static lib_u32 fixed_calls[EMULATOR_PRODUCT_MONITOR_COMMAND_EXIT + 1u];
static lib_u32 extension_calls;
static emulator_product_monitor_command last_fixed_command;
static const char *last_fixed_arguments;

static lib_bool submit_fixed(void *opaque, emulator_product_monitor_command command,
    emulator_session_machine_state state, const char *arguments,
    emulator_session_command_result *out)
{
    (void)opaque;
    (void)state;
    ++fixed_calls[command];
    last_fixed_command = command;
    last_fixed_arguments = arguments;
    out->request = command == EMULATOR_PRODUCT_MONITOR_COMMAND_START ?
        EMULATOR_SESSION_REQUEST_START : EMULATOR_SESSION_REQUEST_NONE;
    return LIB_TRUE;
}

static lib_bool submit_extension(void *opaque, emulator_session_machine_state state,
    const char *line, emulator_session_command_result *out)
{
    (void)opaque;
    (void)state;
    ++extension_calls;
    if (lib_text_compare(line, "media attach disk.img") != 0) return LIB_FALSE;
    (void)lib_c_snprintf(out->text, sizeof(out->text), "Media attached.\r\n");
    out->arm_prompt = LIB_TRUE;
    return LIB_TRUE;
}

static void command_parse(void)
{
    emulator_product_monitor_command command;
    const char *arguments;

    lib_test_assert(emulator_product_monitor_parse("  StArT  ", &command, &arguments));
    lib_test_assert(command == EMULATOR_PRODUCT_MONITOR_COMMAND_START && arguments[0] == '\0');
    lib_test_assert(emulator_product_monitor_parse("save state.bin", &command, &arguments));
    lib_test_assert(command == EMULATOR_PRODUCT_MONITOR_COMMAND_SAVE &&
        lib_text_compare(arguments, "state.bin") == 0);
    lib_test_assert(emulator_product_monitor_parse("debug regs", &command, &arguments));
    lib_test_assert(command == EMULATOR_PRODUCT_MONITOR_COMMAND_DEBUG &&
        lib_text_compare(arguments, "regs") == 0);
    lib_test_assert(!emulator_product_monitor_parse("floppy eject", &command, &arguments));
}

static void text_format(void)
{
    static const emulator_product_help_row extensions[] = {
        {"disk attach <file>", "attach a test disk"},
        {"floppy insert <mode> <image>", "insert a test disk"}};
    static const emulator_product_help_row hotkeys[] = {
        {"Ctrl+Alt+P", "pause or resume"}};
    char text[EMULATOR_SESSION_TEXT_CAPACITY];
    const char *start;
    const char *reset;
    const char *load;
    const char *debug;
    char running_title[64];
    char paused_title[64];

    lib_test_assert(emulator_product_monitor_format_help(
        (emulator_product_help_map){extensions, sizeof(extensions) / sizeof(extensions[0])},
        (emulator_product_help_map){hotkeys, sizeof(hotkeys) / sizeof(hotkeys[0])},
        text, sizeof(text)) == LIB_STATUS_OK);
    lib_test_assert(lib_text_find_substring(text, "  start          cold-reset") != LIB_NULL);
    lib_test_assert(lib_text_find_substring(text, "  save <file>") != LIB_NULL);
    lib_test_assert(lib_text_find_substring(text, "  disk attach <file>") != LIB_NULL);
    lib_test_assert(lib_text_find_substring(text,
        "  floppy insert <mode> <image>\r\n                 insert a test disk\r\n") != LIB_NULL);
    lib_test_assert(lib_text_find_substring(text, "While the machine is running:") != LIB_NULL);
    lib_test_assert(emulator_product_monitor_format_help(
        (emulator_product_help_map){(const emulator_product_help_row[]){{"bad\r\nkey", "text"}}, 1u},
        (emulator_product_help_map){LIB_NULL, 0u}, text, sizeof(text)) ==
        LIB_STATUS_INVALID_ARGUMENT);
    start = lib_text_find_substring(text, "  start          cold-reset");
    reset = lib_text_find_substring(text, "  reset          cold-reset");
    load = lib_text_find_substring(text, "  load <file>");
    debug = lib_text_find_substring(text, "  debug          enter debugger");
    lib_test_assert(start != LIB_NULL && reset != LIB_NULL && load != LIB_NULL &&
        debug != LIB_NULL && start < reset && reset < load && load < debug);
    lib_test_assert(emulator_product_monitor_format_window_titles("Test Machine",
        running_title, sizeof(running_title), paused_title, sizeof(paused_title)) ==
        LIB_STATUS_OK && lib_text_compare(running_title, "Test Machine (Running)") == 0 &&
        lib_text_compare(paused_title, "Test Machine (Paused)") == 0);
    lib_test_assert(emulator_product_monitor_format_window_status("Test Machine",
        (emulator_product_help_map){hotkeys, sizeof(hotkeys) / sizeof(hotkeys[0])},
        text, sizeof(text)) == LIB_STATUS_OK);
    lib_test_assert(lib_text_find_substring(text,
        "Test Machine is running in the Window.") != LIB_NULL);
    lib_test_assert(lib_text_find_substring(text, "While the machine is running:") != LIB_NULL);
}

static void provider_contract(void)
{
    static const emulator_product_help_row extensions[] = {
        {"media attach <file>", "attach test media"}};
    static const emulator_product_help_row hotkeys[] = {
        {"Ctrl+Alt+P", "pause or resume"}};
    static const struct {
        const char *line;
        emulator_product_monitor_command command;
    } provider_commands[] = {
        {"debug", EMULATOR_PRODUCT_MONITOR_COMMAND_DEBUG}};
    static const struct {
        const char *line;
        emulator_product_monitor_command command;
        emulator_session_machine_state state;
        emulator_session_request request;
    } lifecycle_commands[] = {
        {"start", EMULATOR_PRODUCT_MONITOR_COMMAND_START,
            EMULATOR_SESSION_MACHINE_STOPPED, EMULATOR_SESSION_REQUEST_START},
        {"resume", EMULATOR_PRODUCT_MONITOR_COMMAND_RESUME,
            EMULATOR_SESSION_MACHINE_PAUSED, EMULATOR_SESSION_REQUEST_RESUME},
        {"pause", EMULATOR_PRODUCT_MONITOR_COMMAND_PAUSE,
            EMULATOR_SESSION_MACHINE_RUNNING, EMULATOR_SESSION_REQUEST_PAUSE},
        {"stop", EMULATOR_PRODUCT_MONITOR_COMMAND_STOP,
            EMULATOR_SESSION_MACHINE_RUNNING, EMULATOR_SESSION_REQUEST_STOP},
        {"stop", EMULATOR_PRODUCT_MONITOR_COMMAND_STOP,
            EMULATOR_SESSION_MACHINE_PAUSED, EMULATOR_SESSION_REQUEST_STOP},
        {"reset", EMULATOR_PRODUCT_MONITOR_COMMAND_RESET,
            EMULATOR_SESSION_MACHINE_STOPPED, EMULATOR_SESSION_REQUEST_RESET},
        {"reset", EMULATOR_PRODUCT_MONITOR_COMMAND_RESET,
            EMULATOR_SESSION_MACHINE_RUNNING, EMULATOR_SESSION_REQUEST_RESET},
        {"reset", EMULATOR_PRODUCT_MONITOR_COMMAND_RESET,
            EMULATOR_SESSION_MACHINE_PAUSED, EMULATOR_SESSION_REQUEST_RESET}};
    const emulator_product_monitor_provider provider = {
        .extension_commands = {extensions, sizeof(extensions) / sizeof(extensions[0])},
        .hotkeys = {hotkeys, sizeof(hotkeys) / sizeof(hotkeys[0])},
        .submit_fixed = submit_fixed,
        .submit_extension = submit_extension};
    emulator_session_command_result result;
    lib_size index;
    char invalid_line[64];

    emulator_product_monitor_provider_open((void *)&provider, &result);
    lib_test_assert(lib_text_find_substring(result.text, "media attach") != LIB_NULL &&
        result.arm_prompt && lib_text_compare(result.prompt,
            EMULATOR_SESSION_MONITOR_PROMPT) == 0);
    for (index = 0u; index < sizeof(provider_commands) /
            sizeof(provider_commands[0]); ++index) {
        emulator_product_monitor_provider_submit_line((void *)&provider,
            EMULATOR_SESSION_MACHINE_STOPPED, provider_commands[index].line, &result);
        lib_test_assert(last_fixed_command == provider_commands[index].command &&
            last_fixed_arguments[0] == '\0' && fixed_calls[last_fixed_command] == 1u &&
            extension_calls == 0u);
        lib_test_assert(lib_c_snprintf(invalid_line, sizeof(invalid_line), "%s unexpected",
            provider_commands[index].line) >= 0);
        emulator_product_monitor_provider_submit_line((void *)&provider,
            EMULATOR_SESSION_MACHINE_STOPPED, invalid_line, &result);
        lib_test_assert(lib_text_compare(result.text, "Unknown command.\r\n") == 0 &&
            extension_calls == 0u);
    }
    for (index = 0u; index < sizeof(lifecycle_commands) /
            sizeof(lifecycle_commands[0]); ++index) {
        emulator_product_monitor_provider_submit_line((void *)&provider,
            lifecycle_commands[index].state, lifecycle_commands[index].line, &result);
        lib_test_assert(result.request == lifecycle_commands[index].request &&
            !result.arm_prompt && extension_calls == 0u);
    }
    emulator_product_monitor_provider_submit_line((void *)&provider,
        EMULATOR_SESSION_MACHINE_STOPPED, "save state.bin", &result);
    lib_test_assert(last_fixed_command == EMULATOR_PRODUCT_MONITOR_COMMAND_SAVE &&
        lib_text_compare(last_fixed_arguments, "state.bin") == 0 && extension_calls == 0u);
    emulator_product_monitor_provider_submit_line((void *)&provider,
        EMULATOR_SESSION_MACHINE_STOPPED, "load state.bin", &result);
    lib_test_assert(last_fixed_command == EMULATOR_PRODUCT_MONITOR_COMMAND_LOAD &&
        lib_text_compare(last_fixed_arguments, "state.bin") == 0 && extension_calls == 0u);
    emulator_product_monitor_provider_submit_line((void *)&provider,
        EMULATOR_SESSION_MACHINE_ERROR, "debug", &result);
    lib_test_assert(last_fixed_command == EMULATOR_PRODUCT_MONITOR_COMMAND_DEBUG &&
        fixed_calls[EMULATOR_PRODUCT_MONITOR_COMMAND_DEBUG] == 2u &&
        lib_text_compare(last_fixed_arguments, "") == 0 && extension_calls == 0u);
    for (index = 0u; index < sizeof(lifecycle_commands) /
            sizeof(lifecycle_commands[0]); ++index) {
        emulator_product_monitor_provider_submit_line((void *)&provider,
            EMULATOR_SESSION_MACHINE_ERROR, lifecycle_commands[index].line, &result);
        lib_test_assert(lib_text_compare(result.text,
            "Machine has failed; exit and restart the program.\r\n") == 0 &&
            fixed_calls[lifecycle_commands[index].command] == 0u);
    }
    emulator_product_monitor_provider_submit_line((void *)&provider,
        EMULATOR_SESSION_MACHINE_ERROR, "save state.bin", &result);
    lib_test_assert(lib_text_compare(result.text,
        "Machine has failed; exit and restart the program.\r\n") == 0);
    emulator_product_monitor_provider_submit_line((void *)&provider,
        EMULATOR_SESSION_MACHINE_ERROR, "load state.bin", &result);
    lib_test_assert(lib_text_compare(result.text,
        "Machine has failed; exit and restart the program.\r\n") == 0);
    emulator_product_monitor_provider_submit_line((void *)&provider,
        EMULATOR_SESSION_MACHINE_STOPPED, "media attach disk.img", &result);
    lib_test_assert(lib_text_compare(result.text, "Media attached.\r\n") == 0 &&
        extension_calls == 1u && lib_text_compare(result.prompt,
            EMULATOR_SESSION_MONITOR_PROMPT) == 0);
    emulator_product_monitor_provider_submit_line((void *)&provider,
        EMULATOR_SESSION_MACHINE_ERROR, "media attach disk.img", &result);
    lib_test_assert(lib_text_compare(result.text, "Media attached.\r\n") == 0 &&
        extension_calls == 2u && lib_text_compare(result.prompt,
            EMULATOR_SESSION_MONITOR_PROMPT) == 0);
    emulator_product_monitor_provider_submit_line((void *)&provider,
        EMULATOR_SESSION_MACHINE_STOPPED, "help unexpected", &result);
    lib_test_assert(lib_text_compare(result.text, "Unknown command.\r\n") == 0 &&
        extension_calls == 2u);
    emulator_product_monitor_provider_submit_line((void *)&provider,
        EMULATOR_SESSION_MACHINE_STOPPED, "exit", &result);
    lib_test_assert(result.exit_requested && !result.arm_prompt);
    emulator_product_monitor_provider_submit_line((void *)&provider,
        EMULATOR_SESSION_MACHINE_STOPPED, "exit unexpected", &result);
    lib_test_assert(lib_text_compare(result.text, "Unknown command.\r\n") == 0 &&
        extension_calls == 2u);
}

static void unavailable_fixed_commands(void)
{
    static const char *lines[] = {"save state.bin", "load state.bin", "debug"};
    const emulator_product_monitor_provider provider = {0};
    emulator_session_command_result result;
    lib_size index;

    for (index = 0u; index < sizeof(lines) / sizeof(lines[0]); ++index) {
        emulator_product_monitor_provider_submit_line((void *)&provider,
            EMULATOR_SESSION_MACHINE_STOPPED, lines[index], &result);
        lib_test_assert(lib_text_compare(result.text, "Feature not implemented.\r\n") == 0 &&
            result.arm_prompt && lib_text_compare(result.prompt,
                EMULATOR_SESSION_MONITOR_PROMPT) == 0);
    }
}

static void lifecycle_contract(void)
{
    const emulator_product_monitor_provider provider = {0};
    emulator_session_command_result result;

    emulator_product_monitor_provider_submit_line((void *)&provider,
        EMULATOR_SESSION_MACHINE_PAUSED, "start", &result);
    lib_test_assert(lib_text_compare(result.text,
        "Machine is paused; use resume, reset, or stop.\r\n") == 0);
    emulator_product_monitor_provider_submit_line((void *)&provider,
        EMULATOR_SESSION_MACHINE_STOPPED, "pause", &result);
    lib_test_assert(lib_text_compare(result.text,
        "Machine is stopped; use start or reset.\r\n") == 0);
    emulator_product_monitor_provider_submit_line((void *)&provider,
        EMULATOR_SESSION_MACHINE_ERROR, "reset", &result);
    lib_test_assert(lib_text_compare(result.text,
        "Machine has failed; exit and restart the program.\r\n") == 0);
    emulator_product_monitor_provider_note_runtime((void *)&provider,
        EMULATOR_SESSION_MACHINE_STOPPED, EMULATOR_SESSION_MACHINE_RUNNING, &result);
    lib_test_assert(lib_text_compare(result.text, "Machine started.\r\n") == 0 &&
        result.arm_prompt);
    emulator_product_monitor_provider_note_runtime((void *)&provider,
        EMULATOR_SESSION_MACHINE_RUNNING, EMULATOR_SESSION_MACHINE_RESET_COMPLETED, &result);
    lib_test_assert(lib_text_compare(result.text, "Machine reset and paused.\r\n") == 0 &&
        result.arm_prompt);
    emulator_product_monitor_provider_note_runtime((void *)&provider,
        EMULATOR_SESSION_MACHINE_STOPPED, EMULATOR_SESSION_MACHINE_PAUSED, &result);
    lib_test_assert(result.text[0] == '\0' && !result.arm_prompt);
    emulator_product_monitor_provider_note_runtime((void *)&provider,
        EMULATOR_SESSION_MACHINE_INIT, EMULATOR_SESSION_MACHINE_PAUSED, &result);
    lib_test_assert(result.text[0] == '\0' && !result.arm_prompt);
}

int main(void)
{
    command_parse();
    text_format();
    provider_contract();
    unavailable_fixed_commands();
    lifecycle_contract();
    return 0;
}
