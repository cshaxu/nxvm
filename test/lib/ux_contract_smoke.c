#include "lib/ux/actions.h"
#include "lib/ux/mailbox.h"
#include "lib/ux/presenter.h"
#include "lib/ux/router.h"

typedef struct ux_contract_capture {
    type_unsigned_32 input_count;
    type_unsigned_32 action_count;
    ux_event events[3];
} ux_contract_capture;

static ux_run_state ux_contract_state(C_VOID *context)
{
    (C_VOID)context;
    return UX_RUN_RUNNING;
}

static ux_run_result ux_contract_action(C_VOID *context, ux_action action,
    ux_event_sink sink)
{
    ux_contract_capture *capture = context;

    (C_VOID)sink;
    if (capture == STD_NULL || action != UX_ACTION_PAUSE_TOGGLE ||
        capture->input_count != 3u) return UX_RUN_ERROR_RESULT;
    ++capture->action_count;
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
    ux_contract_capture *capture = context;

    if (capture == STD_NULL || event == STD_NULL ||
        capture->input_count >= 3u) return TYPE_FALSE;
    capture->events[capture->input_count++] = *event;
    return TYPE_TRUE;
}

int main(void)
{
    ux_mailbox *mailbox = STD_NULL;
    static ux_frame published;
    static ux_frame captured;
    ux_action_registry actions;
    ux_router router;
    static ux_frame frame;
    ux_binding binding = { 0 };
    ux_contract_capture capture = { 0 };

    if (ux_mailbox_create(&mailbox) != TYPE_STATUS_OK) return 1;
    published.valid = TYPE_TRUE;
    published.graphics = TYPE_TRUE;
    published.graphics_width = 320u;
    published.graphics_height = 200u;
    published.graphics_palette[1u] = 0x00112233u;
    published.graphics_pixels[0u] = 1u;
    if (ux_mailbox_publish(mailbox, &published) != TYPE_STATUS_OK ||
        ux_mailbox_capture(mailbox, &captured) != TYPE_STATUS_OK ||
        captured.sequence != ux_mailbox_generation(mailbox) ||
        captured.sequence == 0u || captured.graphics_width != 320u ||
        captured.graphics_palette[1u] != 0x00112233u ||
        captured.graphics_pixels[0u] != 1u) {
        ux_mailbox_destroy(mailbox);
        return 1;
    }
    ux_actions_initialize(&actions);
    if (ux_actions_register(&actions, 'P', UX_MODIFIER_CONTROL | UX_MODIFIER_ALT,
            UX_ACTION_PAUSE_TOGGLE) != TYPE_STATUS_OK ||
        ux_actions_match(&actions, 'P', UX_MODIFIER_CONTROL | UX_MODIFIER_ALT) !=
            UX_ACTION_PAUSE_TOGGLE) return 1;
    ux_router_initialize(&router, UX_DISPLAY_CONSOLE);
    binding.context = &capture;
    binding.mailbox = mailbox;
    binding.router = &router;
    binding.actions = &actions;
    binding.input_sink = ux_contract_event;
    binding.get_state = ux_contract_state;
    binding.handle_action = ux_contract_action;
    binding.handle_close = ux_contract_close;
    if (ux_binding_validate(&binding) != TYPE_STATUS_OK) return 1;
    if (ux_binding_invoke_action(&binding, UX_ACTION_PAUSE_TOGGLE,
            UX_MODIFIER_CONTROL | UX_MODIFIER_ALT | UX_MODIFIER_SHIFT) !=
            UX_RUN_CONTINUE || capture.action_count != 1u ||
        capture.events[0u].data.key.scan_code != 0x1du ||
        capture.events[1u].data.key.scan_code != 0x38u ||
        capture.events[2u].data.key.scan_code != 0x2au ||
        capture.events[0u].data.key.pressed || capture.events[1u].data.key.pressed ||
        capture.events[2u].data.key.pressed) return 1;
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
