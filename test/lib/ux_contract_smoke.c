#include "lib/ux/actions.h"
#include "lib/ux/capture.h"
#include "lib/ux/mailbox.h"
#include "lib/ux/presenter.h"
#include "lib/ux/router.h"

typedef struct ux_contract_capture {
    lib_u32 input_count;
    lib_u32 action_count;
    ux_event events[3];
} ux_contract_capture;

typedef struct ux_contract_frame_case {
    lib_bool graphics;
    lib_u16 columns;
    lib_u16 rows;
    lib_u32 width;
    lib_u32 height;
    lib_u32 stride;
    lib_i32 dirty_left;
    lib_i32 dirty_top;
    lib_i32 dirty_right;
    lib_i32 dirty_bottom;
} ux_contract_frame_case;

static const ux_contract_frame_case ux_contract_frame_cases[] = {
    { LIB_FALSE, 80u, 25u, 0u, 0u, 0u, 0, 0, 0, 0 },
    { LIB_TRUE, 0u, 0u, 320u, 200u, 320u, 3, 4, 5, 6 }
};

enum { UX_CONTRACT_ACTION = 1u };

static ux_run_state ux_contract_state(void *context)
{
    (void)context;
    return UX_RUN_RUNNING;
}

static int ux_contract_release_inputs(void *context, ux_event_sink sink)
{
    static const lib_u16 scan_codes[] = { 0x1du, 0x38u, 0x2au };
    ux_contract_capture *capture = context;
    lib_u32 index;

    if (capture == LIB_NULL || sink == LIB_NULL) return LIB_FALSE;
    for (index = 0u; index < sizeof(scan_codes) / sizeof(scan_codes[0]); ++index) {
        ux_event event = { 0 };
        event.type = UX_EVENT_KEY;
        event.data.key.scan_code = scan_codes[index];
        if (!sink(context, &event)) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static ux_run_result ux_contract_action(void *context, ux_action action,
    ux_event_sink sink)
{
    ux_contract_capture *capture = context;

    (void)sink;
    if (capture == LIB_NULL || action != UX_CONTRACT_ACTION ||
        capture->input_count != 3u) return UX_RUN_ERROR_RESULT;
    ++capture->action_count;
    return UX_RUN_CONTINUE;
}

static ux_run_result ux_contract_close(void *context, ux_event_sink sink)
{
    (void)context;
    (void)sink;
    return UX_RUN_PAUSED_RESULT;
}

static int ux_contract_event(void *context, const ux_event *event)
{
    ux_contract_capture *capture = context;

    if (capture == LIB_NULL || event == LIB_NULL ||
        capture->input_count >= 3u) return LIB_FALSE;
    capture->events[capture->input_count++] = *event;
    return LIB_TRUE;
}

int main(void)
{
    ux_mailbox *mailbox = LIB_NULL;
    static ux_frame published;
    static ux_frame captured;
    ux_action_registry actions;
    ux_action_registry capacity_actions;
    ux_router router;
    ux_capture native_capture;
    ux_binding binding = { 0 };
    ux_contract_capture capture = { 0 };
    ux_event mouse = { 0 };

    if (ux_frame_is_valid(&published)) return 1;

    if (ux_mailbox_create(&mailbox) != LIB_STATUS_OK) return 1;
    ux_capture_initialize(&native_capture);
    if (ux_capture_is_active(&native_capture)) return 1;
    ux_capture_activate(&native_capture);
    if (!ux_capture_is_active(&native_capture)) return 1;
    ux_capture_release(&native_capture);
    if (ux_capture_is_active(&native_capture)) return 1;
    mouse.type = UX_EVENT_MOUSE;
    mouse.data.mouse.delta_x = -12;
    mouse.data.mouse.delta_y = 7;
    mouse.data.mouse.absolute_x = 31;
    mouse.data.mouse.absolute_y = 47;
    mouse.data.mouse.wheel_y = 120;
    mouse.data.mouse.buttons = UX_MOUSE_BUTTON_LEFT | UX_MOUSE_BUTTON_MIDDLE;
    mouse.data.mouse.relative = LIB_TRUE;
    if (mouse.type != UX_EVENT_MOUSE || mouse.data.mouse.delta_x != -12 ||
        mouse.data.mouse.delta_y != 7 || mouse.data.mouse.absolute_x != 31 ||
        mouse.data.mouse.absolute_y != 47 || mouse.data.mouse.wheel_y != 120 ||
        mouse.data.mouse.buttons !=
            (UX_MOUSE_BUTTON_LEFT | UX_MOUSE_BUTTON_MIDDLE) ||
        mouse.data.mouse.relative == 0u) return 1;
    for (lib_u32 index = 0u; index < sizeof(ux_contract_frame_cases) /
            sizeof(ux_contract_frame_cases[0]); ++index) {
        const ux_contract_frame_case *frame_case = &ux_contract_frame_cases[index];

        memset(&published, 0, sizeof(published));
        published.valid = LIB_TRUE;
        published.graphics = frame_case->graphics;
        published.text_columns = frame_case->columns;
        published.text_rows = frame_case->rows;
        published.graphics_width = frame_case->width;
        published.graphics_height = frame_case->height;
        published.graphics_stride = frame_case->stride;
        published.dirty_left = frame_case->dirty_left;
        published.dirty_top = frame_case->dirty_top;
        published.dirty_right = frame_case->dirty_right;
        published.dirty_bottom = frame_case->dirty_bottom;
        published.text[0u] = 'S';
        published.attributes[0u] = 0x1eu;
        published.text_palette[14u] = 0x00ffff00u;
        published.font[16u] = 0x80u;
        published.secondary_font[16u] = 0x01u;
        published.font_height = 16u;
        published.attribute_font_select = 1u;
        published.cursor_column = 1;
        published.cursor_row = 2;
        published.cursor_top = 14u;
        published.cursor_bottom = 15u;
        published.cursor_visible = LIB_TRUE;
        published.cursor_phase = LIB_TRUE;
        published.graphics_palette[1u] = 0x00112233u;
        published.graphics_pixels[0u] = 1u;
        if (!ux_frame_is_valid(&published)) return 1;
        if (ux_mailbox_publish(mailbox, &published) != LIB_STATUS_OK ||
            ux_mailbox_capture(mailbox, &captured) != LIB_STATUS_OK ||
            captured.sequence != ux_mailbox_generation(mailbox) ||
            captured.sequence == 0u || captured.graphics !=
                (lib_u32)frame_case->graphics ||
            captured.text_columns != frame_case->columns ||
            captured.text_rows != frame_case->rows ||
            captured.graphics_width != frame_case->width ||
            captured.graphics_height != frame_case->height ||
            captured.graphics_stride != frame_case->stride ||
            captured.dirty_left != frame_case->dirty_left ||
            captured.dirty_top != frame_case->dirty_top ||
            captured.dirty_right != frame_case->dirty_right ||
            captured.dirty_bottom != frame_case->dirty_bottom ||
            captured.text[0u] != 'S' || captured.attributes[0u] != 0x1eu ||
            captured.text_palette[14u] != 0x00ffff00u ||
            captured.font[16u] != 0x80u || captured.secondary_font[16u] != 0x01u ||
            captured.cursor_top != 14u || captured.cursor_bottom != 15u ||
            captured.cursor_visible == 0u || captured.cursor_phase == 0u ||
            captured.graphics_palette[1u] != 0x00112233u ||
            captured.graphics_pixels[0u] != 1u) {
            ux_mailbox_destroy(mailbox);
            return 1;
        }
    }
    memset(&published, 0, sizeof(published));
    published.valid = LIB_TRUE;
    published.graphics = LIB_TRUE;
    published.graphics_width = UX_GRAPHICS_MAX_WIDTH + 1u;
    published.graphics_height = 1u;
    published.graphics_stride = published.graphics_width;
    if (ux_frame_is_valid(&published) ||
        ux_mailbox_publish(mailbox, &published) != LIB_STATUS_INVALID_ARGUMENT)
        return 1;
    ux_actions_initialize(&actions);
    if (ux_actions_register(&actions, 'P', UX_MODIFIER_CONTROL | UX_MODIFIER_ALT,
            UX_CONTRACT_ACTION) != LIB_STATUS_OK ||
        ux_actions_register(&actions, 'P', UX_MODIFIER_CONTROL | UX_MODIFIER_ALT,
            UX_CONTRACT_ACTION) != LIB_STATUS_INVALID_STATE ||
        ux_actions_match(&actions, 'P', UX_MODIFIER_CONTROL | UX_MODIFIER_ALT) !=
            UX_CONTRACT_ACTION) return 1;
    ux_actions_initialize(&capacity_actions);
    for (lib_u32 index = 0u; index < UX_ACTION_CAPACITY; ++index) {
        if (ux_actions_register(&capacity_actions, 0x1000u + index,
                UX_MODIFIER_SHIFT, index + 1u) != LIB_STATUS_OK) return 1;
    }
    if (ux_actions_register(&capacity_actions, 0x2000u, UX_MODIFIER_SHIFT,
            UX_ACTION_CAPACITY + 1u) != LIB_STATUS_INVALID_STATE) return 1;
    ux_router_initialize(&router, UX_TARGET_CONSOLE);
    /* Publishing a graphics frame cannot route a presenter.  Only the
       product request below is allowed to change this target. */
    if (ux_router_target(&router) != UX_TARGET_CONSOLE) return 1;
    binding.context = &capture;
    binding.mailbox = mailbox;
    binding.router = &router;
    binding.actions = &actions;
    binding.input_sink = ux_contract_event;
    binding.release_inputs = ux_contract_release_inputs;
    binding.get_state = ux_contract_state;
    binding.handle_action = ux_contract_action;
    binding.handle_close = ux_contract_close;
    if (ux_binding_validate(&binding) != LIB_STATUS_OK) return 1;
    if (ux_binding_invoke_action(&binding, UX_CONTRACT_ACTION) !=
            UX_RUN_CONTINUE || capture.action_count != 1u ||
        capture.events[0u].data.key.scan_code != 0x1du ||
        capture.events[1u].data.key.scan_code != 0x38u ||
        capture.events[2u].data.key.scan_code != 0x2au ||
        capture.events[0u].data.key.pressed || capture.events[1u].data.key.pressed ||
        capture.events[2u].data.key.pressed) return 1;
    if (binding.handle_close(binding.context, binding.input_sink) !=
        UX_RUN_PAUSED_RESULT) return 1;
    binding.input_sink = LIB_NULL;
    if (ux_binding_validate(&binding) != LIB_STATUS_INVALID_ARGUMENT) return 1;
    if (ux_router_target(&router) != UX_TARGET_CONSOLE) return 1;
    ux_router_request(&router, UX_TARGET_WINDOW);
    if (ux_router_target(&router) != UX_TARGET_WINDOW) {
        ux_mailbox_destroy(mailbox);
        return 1;
    }
    ux_router_request(&router, UX_TARGET_NONE);
    if (ux_router_target(&router) != UX_TARGET_NONE) {
        ux_mailbox_destroy(mailbox);
        return 1;
    }
    ux_mailbox_destroy(mailbox);
    return 0;
}
