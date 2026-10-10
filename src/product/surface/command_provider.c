#include "product/surface/command_provider_interface.h"
#include "product/surface/keyboard_interface.h"

static lib_bool product_surface_command_provider_hotkey(void *opaque,
    emulator_session_machine_state state, const lib_u8 *identifier,
    emulator_session_command_result *out)
{
    emulator_product_monitor_provider *monitor = opaque;
    product_surface_command_context *command = monitor != LIB_NULL ? monitor->context : LIB_NULL;
    if (command == LIB_NULL || identifier == LIB_NULL) return LIB_FALSE;
    if (lib_text_compare((const char *)identifier, "pause-toggle") == 0) {
        emulator_session_request request;

        if (!emulator_session_request_pause_toggle(state, &request))
            request = EMULATOR_SESSION_REQUEST_PAUSE;
        return emulator_product_monitor_request_lifecycle(monitor,
            request == EMULATOR_SESSION_REQUEST_RESUME ?
                EMULATOR_PRODUCT_MONITOR_COMMAND_RESUME :
                EMULATOR_PRODUCT_MONITOR_COMMAND_PAUSE, state, out);
    }
    lib_bool accepted = product_surface_keyboard_handle_hotkey(command->machine, state,
        identifier, out);

    return accepted;
}

lib_status product_surface_command_provider_initialize(product_surface_command_context *command,
    emulator_machine *machine, emulator_session_display display,
    const product_surface_command_extensions *extensions,
    emulator_session_command_provider *out_provider)
{
    if (command == LIB_NULL || machine == LIB_NULL || out_provider == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    *out_provider = (emulator_session_command_provider){
        .context = &command->monitor,
        .open = emulator_product_monitor_provider_open,
        .reject_line = emulator_product_monitor_provider_reject_line,
        .submit_line = product_surface_command_provider_submit_line,
        .note_runtime = product_surface_command_provider_note_runtime,
        .handle_hotkey = product_surface_command_provider_hotkey
    };
    return product_surface_command_initialize(command, machine, display, extensions);
}
