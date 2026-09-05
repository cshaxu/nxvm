#include "lib/ux/presenter.h"

type_status ux_binding_validate(const ux_binding *binding)
{
    return binding == STD_NULL || binding->mailbox == STD_NULL ||
        binding->router == STD_NULL || binding->actions == STD_NULL ||
        binding->input_sink == STD_NULL || binding->get_state == STD_NULL ||
        binding->handle_action == STD_NULL || binding->handle_close == STD_NULL ?
        TYPE_STATUS_INVALID_ARGUMENT : TYPE_STATUS_OK;
}
