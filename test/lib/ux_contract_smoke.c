#include "lib/ux/actions.h"
#include "lib/ux/mailbox.h"
#include "lib/ux/presenter.h"
#include "lib/ux/router.h"

static ux_run_state ux_contract_state(C_VOID *context)
{
    (C_VOID)context;
    return UX_RUN_RUNNING;
}

static ux_run_result ux_contract_action(C_VOID *context, ux_action action,
    ux_event_sink sink)
{
    (C_VOID)context;
    (C_VOID)action;
    (C_VOID)sink;
    return UX_RUN_CONTINUE;
}

static ux_run_result ux_contract_close(C_VOID *context, ux_event_sink sink)
{
    (C_VOID)context;
    (C_VOID)sink;
    return UX_RUN_PAUSED_RESULT;
}

static C_INT ux_contract_event(C_VOID *context, const ux_event *event)
{
    (C_VOID)context;
    return event != STD_NULL;
}

int main(void)
{
    ux_mailbox *mailbox = STD_NULL;
    static ux_frame published;
    static ux_frame captured;
    ux_action_registry actions;
    ux_router router;
    ux_frame frame = {0};
    ux_binding binding = { 0 };

    if (ux_mailbox_create(&mailbox) != TYPE_STATUS_OK) return 1;
    published.valid = TYPE_TRUE;
    if (ux_mailbox_publish(mailbox, &published) != TYPE_STATUS_OK ||
        ux_mailbox_capture(mailbox, &captured) != TYPE_STATUS_OK ||
        captured.sequence != ux_mailbox_generation(mailbox) ||
        captured.sequence == 0u) {
        ux_mailbox_destroy(mailbox);
        return 1;
    }
    ux_actions_initialize(&actions);
    if (ux_actions_register(&actions, 'P', UX_MODIFIER_CONTROL | UX_MODIFIER_ALT,
            UX_ACTION_PAUSE_TOGGLE) != TYPE_STATUS_OK ||
        ux_actions_match(&actions, 'P', UX_MODIFIER_CONTROL | UX_MODIFIER_ALT) !=
            UX_ACTION_PAUSE_TOGGLE) return 1;
    ux_router_initialize(&router, UX_DISPLAY_CONSOLE);
    binding.mailbox = mailbox;
    binding.router = &router;
    binding.actions = &actions;
    binding.input_sink = ux_contract_event;
    binding.get_state = ux_contract_state;
    binding.handle_action = ux_contract_action;
    binding.handle_close = ux_contract_close;
    if (ux_binding_validate(&binding) != TYPE_STATUS_OK) return 1;
    binding.input_sink = STD_NULL;
    if (ux_binding_validate(&binding) != TYPE_STATUS_INVALID_ARGUMENT) return 1;
    if (ux_router_observe(&router, &frame) != UX_TARGET_CONSOLE) return 1;
    frame.valid = 1u;
    frame.graphics = 1u;
    if (ux_router_observe(&router, &frame) != UX_TARGET_WINDOW) {
        ux_mailbox_destroy(mailbox);
        return 1;
    }
    ux_mailbox_destroy(mailbox);
    return 0;
}
