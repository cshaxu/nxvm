#ifndef UI_LINUI_INPUT_H
#define UI_LINUI_INPUT_H

#include "lib/ui-base/event_interface.h"

typedef enum ui_linui_key {
    UI_LINUI_KEY_ENTER,
    UI_LINUI_KEY_BACKSPACE,
    UI_LINUI_KEY_F1,
    UI_LINUI_KEY_F2,
    UI_LINUI_KEY_F3,
    UI_LINUI_KEY_F4,
    UI_LINUI_KEY_F5,
    UI_LINUI_KEY_F6,
    UI_LINUI_KEY_F7,
    UI_LINUI_KEY_F8,
    UI_LINUI_KEY_F9,
    UI_LINUI_KEY_F10,
    UI_LINUI_KEY_F11,
    UI_LINUI_KEY_F12,
    UI_LINUI_KEY_UP,
    UI_LINUI_KEY_DOWN,
    UI_LINUI_KEY_LEFT,
    UI_LINUI_KEY_RIGHT,
    UI_LINUI_KEY_HOME,
    UI_LINUI_KEY_END,
    UI_LINUI_KEY_PAGE_UP,
    UI_LINUI_KEY_PAGE_DOWN,
    UI_LINUI_KEY_INSERT,
    UI_LINUI_KEY_DELETE
} ui_linui_key;

/* Translate a terminal key to a neutral key identity. Native scan_code is
 * zero because a terminal reports no portable physical transition. Printable
 * text remains Unicode at the native terminal boundary. */
lib_bool ui_linui_key_to_event(ui_linui_key key, ui_event *out_event);

#endif
