#include "lib/ux-base/frame.h"
#include "lib/ux-base/hotkey.h"

#include <string.h>

typedef struct capture { ux_input_event event; int count; } capture;
static int sink(void *opaque, const ux_input_event *event)
{ capture *out = opaque; if (out == LIB_NULL || event == LIB_NULL) return 0; out->event = *event; ++out->count; return 1; }

int main(void)
{
    ux_hotkey_registry registry;
    ux_hotkey_matcher matcher;
    ux_input_event key = { 0 };
    ux_frame frame = { 0 };
    capture captured = { 0 };
    ux_hotkey_registry_initialize(&registry);
    if (ux_hotkey_registry_register(&registry, 'P', UX_HOTKEY_MODIFIER_CONTROL |
            UX_HOTKEY_MODIFIER_ALT, "pause") != LIB_STATUS_OK) return 1;
    ux_hotkey_matcher_initialize(&matcher, &registry);
    key.type = UX_EVENT_KEY; key.data.key.key = 'P'; key.data.key.pressed = LIB_TRUE;
    key.data.key.modifiers = UX_KEY_MODIFIER_CONTROL | UX_KEY_MODIFIER_ALT;
    if (!ux_hotkey_matcher_submit(&matcher, &key, sink, &captured) || captured.count != 1 ||
        captured.event.type != UX_EVENT_HOTKEY || strcmp(captured.event.data.hotkey.identifier, "pause") != 0) return 1;
    frame.valid = 1u; frame.text_columns = UX_TEXT_COLUMNS; frame.text_rows = UX_TEXT_ROWS;
    frame.text_palette[1] = 0x00ff0000u;
    return ux_frame_is_valid(&frame) ? 0 : 1;
}
