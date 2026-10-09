#include "ibmpc/product/command_provider_interface.h"
#include "ibmpc/product/keyboard_interface.h"

static lib_bool app_command_provider_hotkey(void *opaque,
    common_session_machine_state state, const lib_u8 *identifier,
    common_session_command_result *out)
{
    app_command_context *command = opaque;
    lib_bool accepted = app_keyboard_handle_hotkey(command->machine, state,
        identifier, out);

    if (accepted && out->request != COMMON_SESSION_REQUEST_NONE &&
        !app_command_provider_begin_external(command, state, out->request))
        out->request = COMMON_SESSION_REQUEST_NONE;
    return accepted;
}

lib_status app_command_provider_initialize(app_command_context *command,
    common_machine *machine, common_session_display display,
    const app_command_extensions *extensions,
    common_session_command_provider *out_provider)
{
    if (command == LIB_NULL || machine == LIB_NULL || out_provider == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_provider = (common_session_command_provider){
        .context = command,
        .open = app_command_provider_open,
        .reject_line = app_command_provider_reject_line,
        .submit_line = app_command_provider_submit_line,
        .begin_external = app_command_provider_begin_external,
        .note_runtime = app_command_provider_note_runtime,
        .note_broker = app_command_provider_note_broker,
        .note_monitor_current = app_command_provider_note_monitor_current,
        .handle_hotkey = app_command_provider_hotkey
    };
    return app_command_initialize(command, machine, display, extensions);
}
