#include "lib/ux/presenter.h"

type_status ux_binding_validate(const ux_binding *binding)
{
    return binding == STD_NULL || binding->mailbox == STD_NULL ||
        binding->router == STD_NULL || binding->actions == STD_NULL ||
        binding->input_sink == STD_NULL || binding->get_state == STD_NULL ||
        binding->handle_action == STD_NULL || binding->handle_close == STD_NULL ?
        TYPE_STATUS_INVALID_ARGUMENT : TYPE_STATUS_OK;
}

static C_INT ux_binding_release_modifier(const ux_binding *binding,
    type_unsigned_16 scan_code, type_unsigned_16 virtual_key)
{
    ux_event event = { 0 };

    event.type = UX_EVENT_KEY;
    event.data.key.scan_code = scan_code;
    event.data.key.virtual_key = virtual_key;
    event.data.key.pressed = TYPE_FALSE;
    return binding->input_sink(binding->context, &event);
}

ux_run_result ux_binding_invoke_action(const ux_binding *binding,
    ux_action action, type_unsigned_8 modifiers)
{
    if (ux_binding_validate(binding) != TYPE_STATUS_OK || action == UX_ACTION_NONE)
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
