#include "emulator/product/monitor_interface.h"
#include "lib/types/file.h"
#include "lib/types/test.h"

static lib_u32 fixed_calls;
static lib_u32 extension_calls;

static lib_bool submit_fixed(void *opaque, emulator_product_monitor_command command,
    emulator_session_machine_state state, const char *arguments,
    emulator_session_command_result *out)
{
    (void)opaque;
    (void)state;
    ++fixed_calls;
    if (command != EMULATOR_PRODUCT_MONITOR_COMMAND_START || arguments[0] != '\0')
        return LIB_FALSE;
    out->request = EMULATOR_SESSION_REQUEST_START;
    return LIB_TRUE;
}

static lib_bool submit_extension(void *opaque, emulator_session_machine_state state,
    const char *line, emulator_session_command_result *out)
{
    (void)opaque;
    (void)state;
    ++extension_calls;
    if (lib_text_compare(line, "media attach disk.img") != 0) return LIB_FALSE;
    (void)lib_c_snprintf(out->text, sizeof(out->text), "Media attached.\r\n\r\n");
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
    char text[EMULATOR_PRODUCT_MONITOR_TEXT_CAPACITY];

    lib_test_assert(emulator_product_monitor_format_startup("Test Machine", text,
        sizeof(text)) == LIB_STATUS_OK);
    lib_test_assert(lib_text_find_substring(text, "Test Machine\n\nBuilt on ") != LIB_NULL);
    lib_test_assert(emulator_product_monitor_format_help("  disk attach <file>\r\n",
        "  Ctrl+Alt+P     pause or resume\r\n",
        text, sizeof(text)) == LIB_STATUS_OK);
    lib_test_assert(lib_text_find_substring(text, "  start          cold-reset") != LIB_NULL);
    lib_test_assert(lib_text_find_substring(text, "  save <file>") != LIB_NULL);
    lib_test_assert(lib_text_find_substring(text, "  disk attach <file>") != LIB_NULL);
    lib_test_assert(lib_text_find_substring(text, "While the machine is running:") != LIB_NULL);
    lib_test_assert(emulator_product_monitor_format_window_status("Test Machine",
        "  Ctrl+Alt+P     pause or resume\r\n", text, sizeof(text)) == LIB_STATUS_OK);
    lib_test_assert(lib_text_find_substring(text,
        "Test Machine is running in the Window.") != LIB_NULL);
    lib_test_assert(lib_text_find_substring(text, "While the machine is running:") != LIB_NULL);
}

static void provider_contract(void)
{
    const emulator_product_monitor_provider provider = {
        .prompt = "Fixture> ",
        .extension_help = "  media attach <file>\r\n",
        .hotkey_help = "  Ctrl+Alt+P     pause or resume\r\n",
        .submit_fixed = submit_fixed,
        .submit_extension = submit_extension};
    emulator_session_command_result result;

    emulator_product_monitor_provider_open((void *)&provider, &result);
    lib_test_assert(lib_text_find_substring(result.text, "media attach") != LIB_NULL &&
        result.arm_prompt && lib_text_compare(result.prompt, "Fixture> ") == 0);
    emulator_product_monitor_provider_submit_line((void *)&provider,
        EMULATOR_SESSION_MACHINE_STOPPED, "start", &result);
    lib_test_assert(result.request == EMULATOR_SESSION_REQUEST_START && fixed_calls == 1u &&
        extension_calls == 0u);
    emulator_product_monitor_provider_submit_line((void *)&provider,
        EMULATOR_SESSION_MACHINE_STOPPED, "save state.bin", &result);
    lib_test_assert(lib_text_compare(result.text, "Feature not implemented.\r\n\r\n") == 0 &&
        fixed_calls == 2u && extension_calls == 0u);
    emulator_product_monitor_provider_submit_line((void *)&provider,
        EMULATOR_SESSION_MACHINE_STOPPED, "media attach disk.img", &result);
    lib_test_assert(lib_text_compare(result.text, "Media attached.\r\n\r\n") == 0 &&
        extension_calls == 1u && lib_text_compare(result.prompt, "Fixture> ") == 0);
    emulator_product_monitor_provider_submit_line((void *)&provider,
        EMULATOR_SESSION_MACHINE_STOPPED, "help unexpected", &result);
    lib_test_assert(lib_text_compare(result.text, "Unknown command.\r\n\r\n") == 0 &&
        extension_calls == 1u);
    emulator_product_monitor_provider_submit_line((void *)&provider,
        EMULATOR_SESSION_MACHINE_STOPPED, "exit", &result);
    lib_test_assert(result.exit_requested && !result.arm_prompt);
}

int main(void)
{
    command_parse();
    text_format();
    provider_contract();
    return 0;
}
