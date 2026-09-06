#include "lib/base/base.h"
#include "lib/ux/presenter.h"

lib_status ux_binding_validate(const ux_binding *binding)
{
    return binding == LIB_NULL || binding->mailbox == LIB_NULL ||
        binding->router == LIB_NULL || binding->actions == LIB_NULL ||
        binding->input_sink == LIB_NULL || binding->get_state == LIB_NULL ||
        binding->release_inputs == LIB_NULL ||
        binding->handle_action == LIB_NULL || binding->handle_close == LIB_NULL ?
        LIB_STATUS_INVALID_ARGUMENT : LIB_STATUS_OK;
}

ux_run_result ux_binding_invoke_action(const ux_binding *binding,
    ux_action action, lib_u8 modifiers)
{
    if (ux_binding_validate(binding) != LIB_STATUS_OK || action == UX_ACTION_NONE)
        return UX_RUN_ERROR_RESULT;
    (void)modifiers;
    if (!binding->release_inputs(binding->context, binding->input_sink))
        return UX_RUN_ERROR_RESULT;
    return binding->handle_action(binding->context, action, binding->input_sink);
}
