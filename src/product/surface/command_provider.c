#include "product/surface/command_provider_interface.h"
#include "product/surface/keyboard_interface.h"

static lib_bool product_surface_command_provider_hotkey(void *opaque,
    common_session_machine_state state, const lib_u8 *identifier,
    common_session_command_result *out)
{
    product_surface_command_context *command = opaque;
    lib_bool accepted = product_surface_keyboard_handle_hotkey(command->machine, state,
        identifier, out);

    if (accepted && out->request != COMMON_SESSION_REQUEST_NONE &&
        !product_surface_command_provider_begin_external(command, state, out->request))
        out->request = COMMON_SESSION_REQUEST_NONE;
    return accepted;
}

lib_status product_surface_command_provider_initialize(product_surface_command_context *command,
    common_machine *machine, common_session_display display,
    const product_surface_command_extensions *extensions,
    common_session_command_provider *out_provider)
{
    if (command == LIB_NULL || machine == LIB_NULL || out_provider == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_provider = (common_session_command_provider){
        .context = command,
        .open = product_surface_command_provider_open,
        .reject_line = product_surface_command_provider_reject_line,
        .submit_line = product_surface_command_provider_submit_line,
        .begin_external = product_surface_command_provider_begin_external,
        .note_runtime = product_surface_command_provider_note_runtime,
        .note_broker = product_surface_command_provider_note_broker,
        .note_monitor_current = product_surface_command_provider_note_monitor_current,
        .handle_hotkey = product_surface_command_provider_hotkey
    };
    return product_surface_command_initialize(command, machine, display, extensions);
}
