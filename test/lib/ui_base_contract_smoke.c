#include "lib/ui-base/frame_interface.h"
#include "lib/ui-base/hotkey_interface.h"

#include <string.h>

typedef struct capture { ui_input_event event; int count; } capture;
static int sink(void *opaque, const ui_input_event *event)
{ capture *out = opaque; if (out == LIB_NULL || event == LIB_NULL) return 0; out->event = *event; ++out->count; return 1; }

int main(void)
{
    ui_hotkey_registry registry;
    ui_hotkey_matcher matcher;
    ui_input_event key = { 0 };
    ui_frame frame = { 0 };
    capture captured = { 0 };
    ui_hotkey_registry_initialize(&registry);
    if (ui_hotkey_registry_register(&registry, 'P', UI_HOTKEY_MODIFIER_CONTROL |
            UI_HOTKEY_MODIFIER_ALT, "pause") != LIB_STATUS_OK) return 1;
    ui_hotkey_matcher_initialize(&matcher, &registry);
    key.type = UI_EVENT_KEY; key.data.key.key = 'P'; key.data.key.pressed = LIB_TRUE;
    key.data.key.modifiers = UI_KEY_MODIFIER_CONTROL | UI_KEY_MODIFIER_ALT;
    if (!ui_hotkey_matcher_submit(&matcher, &key, sink, &captured) || captured.count != 1 ||
        captured.event.type != UI_EVENT_HOTKEY || strcmp(captured.event.data.hotkey.identifier, "pause") != 0) return 1;
    frame.valid = 1u; frame.text_columns = UI_TEXT_COLUMNS; frame.text_rows = UI_TEXT_ROWS;
    frame.text_palette[1] = 0x00ff0000u;
    return ui_frame_is_valid(&frame) ? 0 : 1;
}
