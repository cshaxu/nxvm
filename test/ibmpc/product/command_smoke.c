#include "lib/types/file.h"
#include "ibmpc/product/command_interface.h"
#include "ibmpc/product/command_provider_interface.h"

static const lib_u8 *hotkey_identifier;
static lib_u32 extension_submissions;

lib_bool app_keyboard_handle_hotkey(common_machine *machine,
    common_session_machine_state state, const lib_u8 *identifier,
    common_session_command_result *out)
{
    (void)machine;
    (void)state;
    hotkey_identifier = identifier;
    *out = (common_session_command_result){0};
    return LIB_TRUE;
}

static lib_bool fixture_extension(void *context, common_machine *machine,
    common_session_machine_state state, const char *line,
    common_session_command_result *out)
{
    (void)context;
    (void)machine;
    (void)state;
    ++extension_submissions;
    if (lib_text_compare(line, "info") == 0)
        (void)lib_c_snprintf((char *)out->text, sizeof(out->text),
            "Fixture information.\r\n\r\n");
    else if (lib_text_compare(line, "floppy insert direct disk.img") == 0)
        (void)lib_c_snprintf((char *)out->text, sizeof(out->text),
            "Fixture floppy extension.\r\n\r\n");
    else
        return LIB_FALSE;
    return LIB_TRUE;
}

lib_i32 main(void)
{
    app_command_context command = {0};
    app_command_effect effect = {0};
    common_session_command_result result = {0};
    common_session_command_provider provider;
    const app_command_extensions extensions = {
        .submit = fixture_extension,
        .help_text = "  info           fixture information\r\n"
            "  floppy eject   fixture media command\r\n"
    };

    app_command_session_initialize(&command.session,
        COMMON_SESSION_DISPLAY_CONSOLE);
    if (app_command_provider_initialize(&command, (common_machine *)&command,
            COMMON_SESSION_DISPLAY_CONSOLE,
            &extensions, &provider) != LIB_STATUS_OK) return 1;
    app_command_provider_open(&command, &result);
    if (lib_text_find_substring(result.text,
            "Control your virtual machine:") == LIB_NULL ||
        lib_text_find_substring(result.text, "Ctrl+Alt+D") == LIB_NULL ||
        lib_text_find_substring(result.text, "fixture information") == LIB_NULL ||
        lib_text_find_substring(result.text, "fixture media command") == LIB_NULL ||
        lib_text_find_substring(result.text, "save <file>") != LIB_NULL)
        return 2;
    if (lib_text_compare((const char *)result.prompt, "> ") != 0)
        return 12;
    if (lib_text_find_substring(result.text, "  exit           quit") >=
            lib_text_find_substring(result.text, "fixture information") ||
        lib_text_find_substring(result.text, "fixture information") >=
            lib_text_find_substring(result.text, "While the guest is running"))
        return 3;

    app_command_session_submit_line(&command.session, APP_MONITOR_STOPPED,
        "start", &effect);
    if (app_command_session_take_request(&command.session) !=
        APP_LIFECYCLE_REQUEST_START) return 4;
    app_command_session_note_runtime(&command.session, APP_MONITOR_STOPPED,
        COMMON_MACHINE_RUNNING, &effect);
    app_command_session_note_monitor_current(&command.session, LIB_TRUE, &effect);
    if (lib_text_find_substring(effect.text, "Machine started.") == LIB_NULL)
        return 5;

    app_command_provider_submit_line(&command, COMMON_SESSION_MACHINE_PAUSED,
        "resume", &result);
    if (result.request != COMMON_SESSION_REQUEST_RESUME || extension_submissions != 0u)
        return 6;
    app_command_session_take_request(&command.session);
    app_command_session_note_runtime(&command.session, APP_MONITOR_PAUSED,
        COMMON_MACHINE_RUNNING, &effect);

    app_command_provider_submit_line(&command, COMMON_SESSION_MACHINE_PAUSED,
        "info", &result);
    if (lib_text_find_substring(result.text, "Fixture information.") == LIB_NULL)
        return 7;

    app_command_provider_submit_line(&command, COMMON_SESSION_MACHINE_PAUSED,
        "save state.bin", &result);
    if (lib_text_find_substring(result.text, "Unknown command.") == LIB_NULL)
        return 8;

    if (!provider.handle_hotkey(provider.context, COMMON_SESSION_MACHINE_RUNNING,
            (const lib_u8 *)"send-ctrl-alt-del", &result) ||
        lib_text_compare((const char *)hotkey_identifier,
            "send-ctrl-alt-del") != 0) return 9;

    app_command_provider_submit_line(&command, COMMON_SESSION_MACHINE_PAUSED,
        "floppy insert direct disk.img", &result);
    if (lib_text_find_substring(result.text, "Fixture floppy extension.") == LIB_NULL)
        return 10;
    if (extension_submissions != 3u)
        return 11;

    lib_c_printf("IBMPC Product command policy: OK\n");
    return 0;
}
