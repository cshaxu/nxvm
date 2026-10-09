#include <assert.h>

#include "product/command.h"

static lib_i32 app_command_output_compare(const char *actual, const char *expected)
{
    lib_size length = lib_text_length(expected);
    return lib_memory_compare(actual, expected, length) != 0 || actual[length] != '\n' ||
        actual[length + 1u] != '\0';
}

static lib_i32 app_command_monitor_output_compare(const char *actual,
    const char *expected)
{
    lib_size length = lib_text_length(expected);

    return length == 0u || expected[length - 1u] != '\n' ||
        lib_memory_compare(actual, expected, length - 1u) != 0 ||
        lib_text_compare(actual + length - 1u, "\r\n\r\n") != 0;
}

static lib_bool app_command_contains(const char *text, const char *needle)
{
    lib_size needle_length = lib_text_length(needle);

    while (*text != '\0') {
        if (lib_memory_compare(text, needle, needle_length) == 0) return LIB_TRUE;
        ++text;
    }
    return needle_length == 0u;
}

static void verify_stopped_startup(void)
{
    const emulator_session_display displays[] = {
        EMULATOR_SESSION_DISPLAY_WINDOW, EMULATOR_SESSION_DISPLAY_CONSOLE};
    for (lib_size display = 0u; display < 2u; ++display) {
        for (lib_u32 present = 0u; present < 2u; ++present) {
            app_command_context context;
            emulator_session_command_result result;
            app_command_initialize(&context, LIB_NULL, present != 0u, displays[display]);
            app_command_open(&context, &result);
            assert(result.request == EMULATOR_SESSION_REQUEST_NONE && !result.arm_prompt);
            assert(!context.run_after_reset);
            app_command_note_runtime(&context, EMULATOR_SESSION_MACHINE_INIT,
                EMULATOR_SESSION_MACHINE_STOPPED, &result);
            assert(result.request == EMULATOR_SESSION_REQUEST_NONE && result.arm_prompt);
            assert(!context.initial_state_pending);
        }
    }
}

int main(void)
{
    static const lib_u8 pause_toggle[] = "pause-toggle";
    static const lib_u8 unknown_hotkey[] = "unknown";
    app_command_context context;
    app_command_context startup_context;
    app_command_context loaded_context;
    app_command_context console_context;
    app_command_context window_start_context;
    emulator_session_command_result result;
    char path_line[11u + 1025u];

    verify_stopped_startup();
    app_command_initialize(&startup_context, 0, LIB_TRUE,
        EMULATOR_SESSION_DISPLAY_WINDOW);
    app_command_open(&startup_context, &result);
    app_command_note_runtime(&startup_context, EMULATOR_SESSION_MACHINE_INIT,
        EMULATOR_SESSION_MACHINE_STOPPED, &result);
    app_command_submit_line(&startup_context, EMULATOR_SESSION_MACHINE_STOPPED,
        "reset", &result);
    assert(result.request == EMULATOR_SESSION_REQUEST_RESET && !result.arm_prompt);
    app_command_note_runtime(&startup_context, EMULATOR_SESSION_MACHINE_STOPPED,
        EMULATOR_SESSION_MACHINE_INIT, &result);
    assert(result.text[0] == '\0' && !result.arm_prompt);
    app_command_note_runtime(&startup_context, EMULATOR_SESSION_MACHINE_STOPPED,
        EMULATOR_SESSION_MACHINE_RESET_COMPLETED, &result);
    assert(result.text[0] != '\0' && result.arm_prompt);
    assert(app_command_output_compare(result.text, "Reset complete; machine paused.\n") == 0);
    app_command_note_runtime(&startup_context, EMULATOR_SESSION_MACHINE_PAUSED,
        EMULATOR_SESSION_MACHINE_RUNNING, &result);
    assert(app_command_output_compare(result.text, "Machine resumed.\n") == 0);
    app_command_note_runtime(&startup_context, EMULATOR_SESSION_MACHINE_RUNNING,
        EMULATOR_SESSION_MACHINE_PAUSED, &result);
    assert(app_command_output_compare(result.text, "Machine paused.\n") == 0);
    app_command_note_runtime(&startup_context, EMULATOR_SESSION_MACHINE_PAUSED,
        EMULATOR_SESSION_MACHINE_STOPPED, &result);
    assert(app_command_output_compare(result.text, "Machine stopped.\n") == 0);
    app_command_note_runtime(&startup_context, EMULATOR_SESSION_MACHINE_RUNNING,
        EMULATOR_SESSION_MACHINE_ERROR, &result);
    assert(app_command_output_compare(result.text, "Machine host error; exit and restart.\n") == 0);
    app_command_initialize(&window_start_context, 0, LIB_TRUE,
        EMULATOR_SESSION_DISPLAY_WINDOW);
    window_start_context.started_after_reset = LIB_TRUE;
    app_command_note_runtime(&window_start_context, EMULATOR_SESSION_MACHINE_PAUSED,
        EMULATOR_SESSION_MACHINE_RUNNING, &result);
    assert(result.text[0] != '\0' && result.arm_prompt);
    assert(app_command_output_compare(result.text, "Machine started.\n") == 0);
    app_command_initialize(&console_context, 0, LIB_TRUE,
        EMULATOR_SESSION_DISPLAY_CONSOLE);
    app_command_note_runtime(&console_context, EMULATOR_SESSION_MACHINE_PAUSED,
        EMULATOR_SESSION_MACHINE_RUNNING, &result);
    assert(result.text[0] == '\0' && !result.arm_prompt);
    app_command_initialize(&context, 0, 0,
        EMULATOR_SESSION_DISPLAY_WINDOW);
    app_command_open(&context, &result);
    assert(app_command_contains(result.text, "Control your virtual machine:") &&
        !result.arm_prompt);
    assert(lib_text_length(result.text) >= 4u &&
        lib_memory_compare(result.text + lib_text_length(result.text) - 4u,
            "\r\n\r\n", 4u) == 0);
    assert(app_command_contains(result.text, "start          cold-reset"));
    assert(app_command_contains(result.text, "debug          enter debugger"));
    assert(app_command_contains(result.text, "save <file>    save a running or paused machine"));
    assert(app_command_contains(result.text, "load <file>    load a snapshot while stopped"));
    assert(!app_command_contains(result.text, "mem <addr> [count]"));
    assert(app_command_contains(result.text, "exit           quit"));
    assert(app_command_contains(result.text, "Enter               Start"));
    assert(app_command_contains(result.text, "Shift               Select"));
    assert(app_command_contains(result.text, "Esc                 Pause or Resume"));
    app_command_note_runtime(&context, EMULATOR_SESSION_MACHINE_INIT,
        EMULATOR_SESSION_MACHINE_STOPPED, &result);
    assert(result.text[0] == '\0' && result.arm_prompt);
    assert(lib_text_compare(result.prompt, "MyNes> ") == 0);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, "", &result);
    assert(result.text[0] == '\0' && result.arm_prompt &&
        lib_text_compare(result.prompt, "MyNes> ") == 0);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, "start", &result);
    assert(app_command_output_compare(result.text, "Insert a cartridge before starting.\n") == 0);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, "save", &result);
    assert(app_command_output_compare(result.text, "Usage: save <file>\n") == 0);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, "save state.mns", &result);
    assert(app_command_output_compare(result.text, "Machine is stopped; use start before save.\n") == 0);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_PAUSED, "load", &result);
    assert(app_command_output_compare(result.text, "Usage: load <file>\n") == 0);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_RUNNING, "load state.mns", &result);
    assert(app_command_output_compare(result.text, "Machine is running; stop it before load.\n") == 0);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_PAUSED, "load state.mns", &result);
    assert(app_command_output_compare(result.text, "Machine is paused; stop it before load.\n") == 0);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, "debug", &result);
    assert(app_command_contains(result.text, "MyNES debug commands"));
    assert(app_command_contains(result.text, "debug regs"));
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, "debug regs", &result);
    assert(app_command_output_compare(result.text, "Pause with a cartridge before debugging.\n") == 0);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, "regs", &result);
    assert(app_command_monitor_output_compare(result.text, "Unknown command.\n") == 0);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, "reset", &result);
    assert(app_command_output_compare(result.text, "Insert a cartridge before reset.\n") == 0);
    app_command_initialize(&loaded_context, 0, LIB_TRUE,
        EMULATOR_SESSION_DISPLAY_WINDOW);
    assert(app_command_handle_hotkey(&loaded_context, EMULATOR_SESSION_MACHINE_PAUSED,
        pause_toggle, &result));
    assert(result.request == EMULATOR_SESSION_REQUEST_RESUME);
    assert(app_command_handle_hotkey(&loaded_context, EMULATOR_SESSION_MACHINE_RUNNING,
        pause_toggle, &result));
    assert(result.request == EMULATOR_SESSION_REQUEST_PAUSE);
    assert(!app_command_handle_hotkey(&loaded_context, EMULATOR_SESSION_MACHINE_STOPPED,
        pause_toggle, &result));
    assert(!app_command_handle_hotkey(&loaded_context, EMULATOR_SESSION_MACHINE_ERROR,
        pause_toggle, &result));
    assert(!app_command_handle_hotkey(&loaded_context, EMULATOR_SESSION_MACHINE_RUNNING,
        unknown_hotkey, &result));
    app_command_submit_line(&loaded_context, EMULATOR_SESSION_MACHINE_STOPPED, "start", &result);
    assert(result.request == EMULATOR_SESSION_REQUEST_RESET && !result.arm_prompt);
    app_command_submit_line(&loaded_context, EMULATOR_SESSION_MACHINE_STOPPED, "help", &result);
    assert(app_command_contains(result.text, "Control your virtual machine:"));
    app_command_note_runtime(&loaded_context, EMULATOR_SESSION_MACHINE_STOPPED,
        EMULATOR_SESSION_MACHINE_RESET_COMPLETED, &result);
    assert(result.request == EMULATOR_SESSION_REQUEST_RESUME && !result.arm_prompt);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_RUNNING, "pause", &result);
    assert(result.request == EMULATOR_SESSION_REQUEST_PAUSE && !result.arm_prompt);
    app_command_note_runtime(&context, EMULATOR_SESSION_MACHINE_RUNNING,
        EMULATOR_SESSION_MACHINE_PAUSED, &result);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_PAUSED, "stop", &result);
    assert(result.request == EMULATOR_SESSION_REQUEST_STOP && !result.arm_prompt);
    app_command_note_runtime(&context, EMULATOR_SESSION_MACHINE_PAUSED,
        EMULATOR_SESSION_MACHINE_STOPPED, &result);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, "pause", &result);
    assert(app_command_output_compare(result.text, "Machine is not running.\n") == 0);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, "stop", &result);
    assert(app_command_output_compare(result.text, "Machine is already stopped.\n") == 0);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_ERROR, "start", &result);
    assert(app_command_output_compare(result.text, "Machine host error; exit and restart.\n") == 0);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_ERROR, "help", &result);
    assert(app_command_contains(result.text, "While the machine is running:"));
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, "rom insert", &result);
    assert(app_command_output_compare(result.text, "rom insert requires one ASCII file path.\n") == 0);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, "rom insert \"\"", &result);
    assert(app_command_output_compare(result.text, "rom insert requires one ASCII file path.\n") == 0);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, "rom insert demo.nes extra", &result);
    assert(app_command_output_compare(result.text, "rom insert requires one ASCII file path.\n") == 0);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, "rom insert \"demo game.nes\"", &result);
    assert(app_command_output_compare(result.text, "Cartridge insertion failed.\n") == 0);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, "rom insert \x80", &result);
    assert(app_command_output_compare(result.text, "rom insert requires one ASCII file path.\n") == 0);
    lib_memory_copy(path_line, "rom insert ", 11u);
    lib_memory_set(path_line + 11u, 'a', 1023u);
    path_line[11u + 1023u] = '\0';
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, path_line, &result);
    assert(app_command_output_compare(result.text, "Cartridge insertion failed.\n") == 0);
    path_line[11u + 1023u] = 'a';
    path_line[11u + 1024u] = '\0';
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, path_line, &result);
    assert(app_command_output_compare(result.text, "rom insert requires one ASCII file path.\n") == 0);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, "rom insert a\n", &result);
    assert(app_command_output_compare(result.text, "rom insert requires one ASCII file path.\n") == 0);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, "rom eject extra", &result);
    assert(app_command_output_compare(result.text, "rom eject accepts no arguments.\n") == 0);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, "rom", &result);
    assert(app_command_monitor_output_compare(result.text, "Unknown command.\n") == 0);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, "set mem.count 65536", &result);
    assert(app_command_monitor_output_compare(result.text, "Unknown command.\n") == 0);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, "set disasm.count 0x10000", &result);
    assert(app_command_monitor_output_compare(result.text, "Unknown command.\n") == 0);
    app_command_reject_line(&context, &result);
    assert(app_command_monitor_output_compare(result.text, "Command is too long.\n") == 0);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, "exit", &result);
    assert(result.exit_requested && !result.arm_prompt);
    app_command_submit_line(&context, EMULATOR_SESSION_MACHINE_STOPPED, "unknown", &result);
    assert(app_command_monitor_output_compare(result.text, "Unknown command.\n") == 0);
    return 0;
}
