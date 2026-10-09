#include "lib/types/file.h"
#include "product/surface/command_interface.h"
#include "product/surface/command_provider_interface.h"

static lib_u32 extension_submissions;

static lib_bool fixture_extension(void *context, emulator_machine *machine,
    emulator_session_machine_state state, const char *line,
    emulator_session_command_result *out)
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
    product_surface_command_context command = {0};
    emulator_session_command_result result = {0};
    emulator_session_command_provider provider;
    const product_surface_command_extensions extensions = {
        .submit = fixture_extension,
        .help_text = "  info           fixture information\r\n"
            "  floppy eject   fixture media command\r\n"
    };

    if (product_surface_command_provider_initialize(&command, (emulator_machine *)&command,
            EMULATOR_SESSION_DISPLAY_CONSOLE,
            &extensions, &provider) != LIB_STATUS_OK) return 1;
    provider.open(provider.context, &result);
    if (lib_text_find_substring(result.text,
            "Control your virtual machine:") == LIB_NULL ||
        lib_text_find_substring(result.text, "Ctrl+Alt+D") == LIB_NULL ||
        lib_text_find_substring(result.text, "fixture information") == LIB_NULL ||
        lib_text_find_substring(result.text, "fixture media command") == LIB_NULL ||
        lib_text_find_substring(result.text, "save <file>") == LIB_NULL)
        return 2;
    if (lib_text_compare((const char *)result.prompt, "> ") != 0)
        return 12;
    if (lib_text_find_substring(result.text, "  exit           quit") >=
            lib_text_find_substring(result.text, "fixture information") ||
        lib_text_find_substring(result.text, "fixture information") >=
            lib_text_find_substring(result.text, "While the machine is running"))
        return 3;

    provider.submit_line(provider.context, EMULATOR_SESSION_MACHINE_STOPPED,
        "start", &result);
    if (result.request != EMULATOR_SESSION_REQUEST_START) return 4;
    provider.note_runtime(provider.context, EMULATOR_SESSION_MACHINE_STOPPED,
        EMULATOR_SESSION_MACHINE_RUNNING, &result);
    if (lib_text_find_substring(result.text, "Machine started.") == LIB_NULL) return 5;

    provider.submit_line(provider.context, EMULATOR_SESSION_MACHINE_PAUSED,
        "resume", &result);
    if (result.request != EMULATOR_SESSION_REQUEST_RESUME || extension_submissions != 0u)
        return 6;

    provider.submit_line(provider.context, EMULATOR_SESSION_MACHINE_PAUSED,
        "info", &result);
    if (lib_text_find_substring(result.text, "Fixture information.") == LIB_NULL)
        return 7;

    provider.submit_line(provider.context, EMULATOR_SESSION_MACHINE_PAUSED,
        "save state.bin", &result);
    if (lib_text_find_substring(result.text, "Feature not implemented.") == LIB_NULL)
        return 8;

    provider.submit_line(provider.context, EMULATOR_SESSION_MACHINE_PAUSED,
        "floppy insert direct disk.img", &result);
    if (lib_text_find_substring(result.text, "Fixture floppy extension.") == LIB_NULL)
        return 9;
    if (extension_submissions != 2u)
        return 10;

    lib_c_printf("IBMPC Product command policy: OK\n");
    return 0;
}
