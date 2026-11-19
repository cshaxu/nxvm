#include "emulator/product/monitor_interface.h"
#include "lib/types/file.h"
#include "lib/types/test.h"

static lib_u32 fixed_calls[EMULATOR_PRODUCT_MONITOR_COMMAND_EXIT + 1u];
static lib_u32 extension_calls;
static emulator_product_monitor_command last_fixed_command;
static char last_fixed_arguments[1024];
static lib_size last_fixed_count;

static lib_bool submit_fixed(void *opaque, emulator_product_monitor_command command,
                             emulator_session_machine_state state, const emulator_product_monitor_arguments *arguments,
                             emulator_session_command_result *out)
{
    (void)opaque;
    (void)state;
    ++fixed_calls[command];
    last_fixed_command = command;
    last_fixed_count = arguments->count;
    lib_c_snprintf(last_fixed_arguments, sizeof(last_fixed_arguments), "%s",
        arguments->count != 0u ? arguments->values[0] : "");
    out->request = command == EMULATOR_PRODUCT_MONITOR_COMMAND_START ? EMULATOR_SESSION_REQUEST_START : EMULATOR_SESSION_REQUEST_NONE;
    return LIB_TRUE;
}

static lib_bool submit_extension(void *opaque, emulator_session_machine_state state,
                                 const char *name, const emulator_product_monitor_arguments *arguments,
                                 emulator_session_command_result *out)
{
    (void)opaque;
    (void)state;
    ++extension_calls;
    if (lib_text_compare(name, "capture") == 0)
        return submit_fixed(opaque, EMULATOR_PRODUCT_MONITOR_COMMAND_SAVE, state, arguments, out);
    if (lib_text_compare(name, "media") != 0 || arguments->count != 2u ||
        lib_text_compare(arguments->values[0], "attach") != 0 ||
        lib_text_compare(arguments->values[1], "disk.img") != 0)
        return LIB_FALSE;
    (void)lib_c_snprintf(out->text, sizeof(out->text), "Media attached.\r\n");
    out->arm_prompt = LIB_TRUE;
    return LIB_TRUE;
}

static void command_parse(void)
{
    const emulator_product_monitor_provider provider = {
        .submit_fixed = submit_fixed, .submit_extension = submit_extension};
    const char *tails[] = { " word", "  \t\"O:\\\xe6\xb5\x8b space\\file\" \t", " \"\"", " one two" };
    const char *expected[] = { "word", "O:\\\xe6\xb5\x8b space\\file", "", "one" };
    const char *invalid[] = { "save \"open", "capture \"abc\"def", "start bad\"quote", "exit \"open" };
    emulator_session_command_result result;
    char line[1025];
    for (lib_size i = 0; i < 4u; ++i) {
        for (lib_size extension = 0; extension < 2u; ++extension) {
            lib_c_snprintf(line, sizeof(line), "%s%s", extension ? "capture" : "SaVe", tails[i]);
            emulator_product_monitor_provider_submit_line((void *)&provider,
                EMULATOR_SESSION_MACHINE_PAUSED, line, &result);
            lib_test_assert(last_fixed_count == (i == 3u ? 2u : 1u));
            lib_test_assert(lib_text_compare(last_fixed_arguments, expected[i]) == 0);
        }
    }
    lib_u32 fixed_before = fixed_calls[EMULATOR_PRODUCT_MONITOR_COMMAND_SAVE];
    lib_u32 extension_before = extension_calls;
    for (lib_size i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
        emulator_product_monitor_provider_submit_line((void *)&provider,
            EMULATOR_SESSION_MACHINE_PAUSED, invalid[i], &result);
        lib_test_assert(lib_text_compare(result.text, "Invalid command syntax.\r\n") == 0);
        lib_test_assert(result.request == EMULATOR_SESSION_REQUEST_NONE && !result.exit_requested);
    }
    lib_test_assert(fixed_calls[EMULATOR_PRODUCT_MONITOR_COMMAND_SAVE] == fixed_before &&
        extension_calls == extension_before);
    lib_memory_copy(line, "save ", 5u);
    for (lib_size i = 5u; i < 1023u; ++i) line[i] = i % 2u ? 'a' : ' ';
    line[1023] = '\0';
    emulator_product_monitor_provider_submit_line((void *)&provider,
        EMULATOR_SESSION_MACHINE_PAUSED, line, &result);
    lib_test_assert(last_fixed_count == 509u && lib_text_compare(last_fixed_arguments, "a") == 0);
    lib_memory_set(line, 'a', sizeof(line) - 1u);
    line[sizeof(line) - 1u] = '\0';
    emulator_product_monitor_provider_submit_line((void *)&provider,
        EMULATOR_SESSION_MACHINE_PAUSED, line, &result);
    lib_test_assert(lib_text_compare(result.text, "Command is too long.\r\n") == 0);
    emulator_product_monitor_provider_submit_line((void *)&provider,
        EMULATOR_SESSION_MACHINE_PAUSED, " \t  ", &result);
    lib_test_assert(result.text[0] == '\0' && result.arm_prompt);
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
                        LIB_STATUS_OK &&
                    lib_text_compare(running_title, "Test Machine (Running)") == 0 &&
                    lib_text_compare(paused_title, "Test Machine (Paused)") == 0);
    lib_test_assert(emulator_product_monitor_format_window_status("Test Machine",
                                                                  (emulator_product_help_map){hotkeys, sizeof(hotkeys) / sizeof(hotkeys[0])},
                                                                  text, sizeof(text)) == LIB_STATUS_OK);
    lib_test_assert(lib_text_find_substring(text,
                                            "Test Machine is running in the window.") != LIB_NULL);
    lib_test_assert(lib_text_find_substring(text, "While the machine is running:") != LIB_NULL);
}

static void window_format_boundaries(void)
{
    static const char *names[] = {"", "A much longer machine name for boundary testing"};
    static const emulator_product_help_row row = {"Ctrl+Alt+P", "pause or resume"};
    char running[128], paused[128], text[256], expected[256];
    lib_size n, capacity, length, rows;

    for (n = 0u; n < sizeof(names) / sizeof(names[0]); ++n)
    {
        length = lib_text_length(names[n]) + sizeof(" (Running)");
        for (capacity = 1u; capacity <= length; ++capacity)
        {
            lib_test_assert(emulator_product_monitor_format_window_titles(names[n],
                                                                          running, capacity, paused, sizeof(paused)) ==
                            (capacity == length ? LIB_STATUS_OK : LIB_STATUS_LIMIT_EXCEEDED));
            lib_test_assert(running[capacity - 1u] == '\0');
        }
        length = lib_text_length(names[n]) + sizeof(" (Paused)");
        for (capacity = 1u; capacity <= length; ++capacity)
        {
            lib_test_assert(emulator_product_monitor_format_window_titles(names[n],
                                                                          running, sizeof(running), paused, capacity) ==
                            (capacity == length ? LIB_STATUS_OK : LIB_STATUS_LIMIT_EXCEEDED));
            lib_test_assert(paused[capacity - 1u] == '\0');
        }
        for (rows = 0u; rows != 2u; ++rows)
        {
            emulator_product_help_map map = {rows ? &row : LIB_NULL, rows};
            lib_test_assert(emulator_product_monitor_format_window_status(names[n],
                                                                          map, expected, sizeof(expected)) == LIB_STATUS_OK);
            length = lib_text_length(expected) + 1u;
            for (capacity = 1u; capacity <= length; ++capacity)
            {
                lib_test_assert(emulator_product_monitor_format_window_status(names[n],
                                                                              map, text, capacity) ==
                                (capacity == length ? LIB_STATUS_OK : LIB_STATUS_LIMIT_EXCEEDED));
            }
            lib_test_assert(lib_text_compare(text, expected) == 0);
        }
    }
    lib_test_assert(emulator_product_monitor_format_window_titles(LIB_NULL,
                                                                  running, sizeof(running), paused, sizeof(paused)) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(emulator_product_monitor_format_window_titles("x",
                                                                  running, 0u, paused, sizeof(paused)) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(emulator_product_monitor_format_window_titles("x",
                                                                  running, sizeof(running), LIB_NULL, sizeof(paused)) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(emulator_product_monitor_format_window_status("x",
                                                                  (emulator_product_help_map){LIB_NULL, 0u}, text, 0u) == LIB_STATUS_INVALID_ARGUMENT);
    lib_test_assert(emulator_product_monitor_format_window_status("x",
                                                                  (emulator_product_help_map){LIB_NULL, 1u}, text, sizeof(text)) == LIB_STATUS_INVALID_ARGUMENT);
}

static void provider_contract(void)
{
    static const emulator_product_help_row extensions[] = {
        {"media attach <file>", "attach test media"}};
    static const emulator_product_help_row hotkeys[] = {
        {"Ctrl+Alt+P", "pause or resume"}};
    static const struct
    {
        const char *line;
        emulator_product_monitor_command command;
    } provider_commands[] = {
        {"debug", EMULATOR_PRODUCT_MONITOR_COMMAND_DEBUG}};
    static const struct
    {
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
                    result.arm_prompt && lib_text_compare(result.prompt, EMULATOR_SESSION_MONITOR_PROMPT) == 0);
    for (index = 0u; index < sizeof(provider_commands) /
                                 sizeof(provider_commands[0]);
         ++index)
    {
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
                                 sizeof(lifecycle_commands[0]);
         ++index)
    {
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
                                 sizeof(lifecycle_commands[0]);
         ++index)
    {
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
                    extension_calls == 1u && lib_text_compare(result.prompt, EMULATOR_SESSION_MONITOR_PROMPT) == 0);
    emulator_product_monitor_provider_submit_line((void *)&provider,
                                                  EMULATOR_SESSION_MACHINE_ERROR, "media attach disk.img", &result);
    lib_test_assert(lib_text_compare(result.text, "Media attached.\r\n") == 0 &&
                    extension_calls == 2u && lib_text_compare(result.prompt, EMULATOR_SESSION_MONITOR_PROMPT) == 0);
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

    for (index = 0u; index < sizeof(lines) / sizeof(lines[0]); ++index)
    {
        emulator_product_monitor_provider_submit_line((void *)&provider,
                                                      EMULATOR_SESSION_MACHINE_STOPPED, lines[index], &result);
        lib_test_assert(lib_text_compare(result.text, "Feature not implemented.\r\n") == 0 &&
                        result.arm_prompt && lib_text_compare(result.prompt, EMULATOR_SESSION_MONITOR_PROMPT) == 0);
    }
}

static void fixed_argument_contract(void)
{
    const char *words[] = {"start", "reset", "stop", "pause", "resume",
                           "load", "save", "debug", "help", "exit"};
    const emulator_product_monitor_provider provider = {0};
    emulator_session_command_result result;
    char line[64];
    for (lib_size index = 0u; index < sizeof(words) / sizeof(words[0]); ++index)
    {
        lib_test_assert(lib_c_snprintf(line, sizeof(line), "%s unexpected", words[index]) > 0);
        emulator_product_monitor_provider_submit_line((void *)&provider,
                                                      EMULATOR_SESSION_MACHINE_STOPPED, line, &result);
        lib_test_assert(lib_text_compare(result.text, index == 5u || index == 6u ? "Feature not implemented.\r\n" : "Unknown command.\r\n") == 0);
        lib_test_assert(result.request == EMULATOR_SESSION_REQUEST_NONE &&
                        result.arm_prompt && !result.exit_requested);
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
    text_format();
    window_format_boundaries();
    provider_contract();
    unavailable_fixed_commands();
    fixed_argument_contract();
    lifecycle_contract();
    command_parse();
    return 0;
}
