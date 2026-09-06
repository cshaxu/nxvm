#include "lib/base/base.h"
#include "lib/ux/presenter.h"

lib_status ux_binding_validate(const ux_binding *binding)
{
    return binding == LIB_NULL || binding->mailbox == LIB_NULL ||
        binding->router == LIB_NULL || binding->actions == LIB_NULL ||
        binding->input_sink == LIB_NULL || binding->get_state == LIB_NULL ||
        binding->handle_action == LIB_NULL || binding->handle_close == LIB_NULL ?
        LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_OK;
}

static int ux_binding_release_modifier(const ux_binding *binding,
    lib_u16 scan_code, lib_u16 virtual_key)
{
    ux_event event = { 0 };

    event.type = UX_EVENT_KEY;
    event.data.key.scan_code = scan_code;
    event.data.key.virtual_key = virtual_key;
    event.data.key.pressed = LIB_FALSE;
    return binding->input_sink(binding->context, &event);
}

ux_run_result ux_binding_invoke_action(const ux_binding *binding,
    ux_action action, lib_u8 modifiers)
{
    if (ux_binding_validate(binding) != LIB_STATUS_OK || action == UX_ACTION_NONE)
        return UX_RUN_ERROR_RESULT;
    if (((modifiers & UX_MODIFIER_CONTROL) != 0u &&
            !ux_binding_release_modifier(binding, 0x1du, 0x11u)) ||
        ((modifiers & UX_MODIFIER_ALT) != 0u &&
            !ux_binding_release_modifier(binding, 0x38u, 0x12u)) ||
        ((modifiers & UX_MODIFIER_SHIFT) != 0u &&
            !ux_binding_release_modifier(binding, 0x2au, 0x10u))) {
        return UX_RUN_ERROR_RESULT;
    }
    return binding->handle_action(binding->context, action, binding->input_sink);
}
