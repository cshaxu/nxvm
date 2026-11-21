#include "lib/types/file.h"
#include "x86-ibmpc/product/command_interface.h"
#include "x86-ibmpc/product/command_provider_interface.h"

static lib_u32 extension_submissions;
static lib_u32 snapshot_submissions;
static lib_u32 hotkey_submissions;

static lib_bool fixture_hotkey(void *context, emulator_machine *machine,
    emulator_session_machine_state state, const char *identifier,
    emulator_session_command_result *out)
{
    if (context != machine || state != EMULATOR_SESSION_MACHINE_RUNNING ||
        lib_text_compare(identifier, "fixture-action") != 0) return LIB_FALSE;
    ++hotkey_submissions;
    out->release_window_mouse = LIB_TRUE;
    return LIB_TRUE;
}

static lib_bool fixture_dispatch_hotkey(void *context, const kvm_input_event *event)
{
    const emulator_session_command_provider *provider = context;
    emulator_session_command_result result = {0};

    return event->type == KVM_EVENT_HOTKEY &&
        provider->handle_hotkey(provider->context, EMULATOR_SESSION_MACHINE_RUNNING,
            event->data.hotkey.identifier, &result) &&
        result.release_window_mouse;
}

static lib_bool fixture_extension(void *context, emulator_machine *machine,
    emulator_session_machine_state state, const char *name,
    const emulator_product_monitor_arguments *arguments,
    emulator_session_command_result *out)
{
    (void)context;
    (void)machine;
    (void)state;
    ++extension_submissions;
    if (lib_text_compare(name, "info") == 0 && arguments->count == 0u)
        (void)lib_c_snprintf((char *)out->text, sizeof(out->text),
            "Fixture information.\r\n");
    else if (lib_text_compare(name, "floppy") == 0 && arguments->count == 3u &&
        lib_text_compare(arguments->values[0], "insert") == 0 &&
        lib_text_compare(arguments->values[1], "direct") == 0 &&
        lib_text_compare(arguments->values[2], "disk image.img") == 0)
        (void)lib_c_snprintf((char *)out->text, sizeof(out->text),
            "Fixture floppy extension.\r\n");
    else
        return LIB_FALSE;
    return LIB_TRUE;
}

static lib_bool fixture_snapshot(void *context, emulator_machine *machine,
    emulator_product_monitor_command command, emulator_session_machine_state state,
    const emulator_product_monitor_arguments *arguments, emulator_session_command_result *out)
{
    (void)context;
    (void)machine;
    (void)state;
    ++snapshot_submissions;
    if (arguments->count != 1u || lib_text_compare(arguments->values[0], "state file.bin") != 0)
        return LIB_FALSE;
    if (command == EMULATOR_PRODUCT_MONITOR_COMMAND_SAVE)
        (void)lib_c_snprintf((char *)out->text, sizeof(out->text),
            "Fixture saved snapshot.\r\n");
    else if (command == EMULATOR_PRODUCT_MONITOR_COMMAND_LOAD)
        (void)lib_c_snprintf((char *)out->text, sizeof(out->text),
            "Fixture loaded snapshot.\r\n");
    else return LIB_FALSE;
    out->arm_prompt = LIB_TRUE;
    return LIB_TRUE;
}

lib_i32 main(void)
{
    static const emulator_product_help_row help_rows[] = {
        {"info", "fixture information"},
        {"floppy eject", "fixture media command"}};
    static const x86_ibmpc_product_hotkey hotkeys[] = {
        {{"Ctrl+Alt+W", "fixture action"},
            {'W', BASE_KEY_MODIFIER_CONTROL | BASE_KEY_MODIFIER_ALT, "fixture-action"}},
        {.help = {"Ctrl+Alt+O", "fixture App input"}}
    };
    x86_ibmpc_product_command_context command = {0};
    kvm_hotkey_matcher matcher;
    x86_ibmpc_product_hotkey conflict = hotkeys[0];
    x86_ibmpc_product_command_extensions conflicting;
    x86_ibmpc_product_command_context unsupported = {0};
    emulator_session_command_result result = {0};
    emulator_session_command_provider provider;
    const x86_ibmpc_product_command_extensions extensions = {
        .context = &command,
        .submit = fixture_extension,
        .submit_snapshot = fixture_snapshot,
        .help = {help_rows, sizeof(help_rows) / sizeof(help_rows[0])},
        .hotkeys = hotkeys,
        .hotkey_count = sizeof(hotkeys) / sizeof(hotkeys[0]),
        .submit_hotkey = fixture_hotkey
    };

    if (x86_ibmpc_product_command_provider_initialize(&unsupported,
            (emulator_machine *)&unsupported, EMULATOR_SESSION_DISPLAY_CONSOLE,
            LIB_NULL, &provider) != LIB_STATUS_OK) return 14;
    provider.submit_line(provider.context, EMULATOR_SESSION_MACHINE_STOPPED,
        "load \"state file.bin\"", &result);
    if (lib_text_find_substring(result.text, "Feature not implemented.") == LIB_NULL ||
        !result.arm_prompt) return 15;
    provider.open(provider.context, &result);
    if (lib_text_find_substring(result.text, "Ctrl+Alt+W") != LIB_NULL ||
        lib_text_find_substring(result.text, "Ctrl+Alt+O") != LIB_NULL) return 19;
    x86_ibmpc_product_command_dispose(&unsupported);
    if (x86_ibmpc_product_command_provider_initialize(&command, (emulator_machine *)&command,
            EMULATOR_SESSION_DISPLAY_CONSOLE,
            &extensions, &provider) != LIB_STATUS_OK) return 16;
    kvm_hotkey_matcher_initialize(&matcher, &command.hotkey_registry);
    const lib_bool delivered = kvm_hotkey_matcher_submit(&matcher,
        &(kvm_input_event){.type = KVM_EVENT_KEY, .data.key = {.key = 'W',
            .modifiers = BASE_KEY_MODIFIER_CONTROL | BASE_KEY_MODIFIER_ALT, .pressed = LIB_TRUE}},
        fixture_dispatch_hotkey, &provider, LIB_TRUE);
    kvm_hotkey_matcher_discard(&matcher);
    if (!delivered || hotkey_submissions != 1u || command.hotkey_registry.count != 6u) return 20;

    conflicting = extensions;
    conflicting.hotkeys = &conflict;
    conflicting.hotkey_count = 1u;
    conflict.registration.key = 'D';
    if (x86_ibmpc_product_command_initialize(&unsupported, (emulator_machine *)&unsupported,
            EMULATOR_SESSION_DISPLAY_CONSOLE, &conflicting) != LIB_STATUS_INVALID_STATE) return 21;
    conflict.registration = (kvm_hotkey_registration){
        'W', BASE_KEY_MODIFIER_CONTROL | BASE_KEY_MODIFIER_ALT, "pause-toggle"};
    if (x86_ibmpc_product_command_initialize(&unsupported, (emulator_machine *)&unsupported,
            EMULATOR_SESSION_DISPLAY_CONSOLE, &conflicting) != LIB_STATUS_INVALID_STATE) return 22;

    provider.submit_line(provider.context, EMULATOR_SESSION_MACHINE_STOPPED, "help", &result);
    if (lib_text_find_substring(result.text, "fixture action") == LIB_NULL ||
        lib_text_find_substring(result.text, "fixture App input") == LIB_NULL) return 23;
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
    if (provider.handle_hotkey(provider.context, EMULATOR_SESSION_MACHINE_PAUSED,
            LIB_NULL, &result)) return 18;

    provider.submit_line(provider.context, EMULATOR_SESSION_MACHINE_PAUSED,
        "info", &result);
    if (lib_text_find_substring(result.text, "Fixture information.") == LIB_NULL)
        return 7;

    provider.submit_line(provider.context, EMULATOR_SESSION_MACHINE_PAUSED,
        "save \"state file.bin\"", &result);
    if (lib_text_find_substring(result.text, "Fixture saved snapshot.") == LIB_NULL ||
        !result.arm_prompt || snapshot_submissions != 1u)
        return 8;

    provider.submit_line(provider.context, EMULATOR_SESSION_MACHINE_STOPPED,
        "load \"state file.bin\"", &result);
    if (lib_text_find_substring(result.text, "Fixture loaded snapshot.") == LIB_NULL ||
        !result.arm_prompt || snapshot_submissions != 2u)
        return 13;

    provider.submit_line(provider.context, EMULATOR_SESSION_MACHINE_ERROR,
        "save \"state file.bin\"", &result);
    if (lib_text_find_substring(result.text,
            "Machine has failed; exit and restart the program.") == LIB_NULL ||
        !result.arm_prompt || snapshot_submissions != 2u)
        return 17;

    provider.submit_line(provider.context, EMULATOR_SESSION_MACHINE_PAUSED,
        "floppy insert direct \"disk image.img\"", &result);
    if (lib_text_find_substring(result.text, "Fixture floppy extension.") == LIB_NULL)
        return 9;
    if (extension_submissions != 2u)
        return 10;

    lib_c_printf("X86 Product command policy: OK\n");
    return 0;
}
