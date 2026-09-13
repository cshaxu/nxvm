#include "lib/types/types_interface.h"
#include "lib/ui-base/linux/input.h"
#include "lib/ui-base/input.h"

static lib_u32 ui_linui_key_identity(ui_linui_key key)
{
    lib_u32 key_identity;

    switch (key) {
    case UI_LINUI_KEY_ENTER: key_identity = UI_KEY_ENTER; break;
    case UI_LINUI_KEY_BACKSPACE: key_identity = UI_KEY_BACKSPACE; break;
    case UI_LINUI_KEY_F1: key_identity = UI_KEY_F1; break;
    case UI_LINUI_KEY_F2: key_identity = UI_KEY_F2; break;
    case UI_LINUI_KEY_F3: key_identity = UI_KEY_F3; break;
    case UI_LINUI_KEY_F4: key_identity = UI_KEY_F4; break;
    case UI_LINUI_KEY_F5: key_identity = UI_KEY_F5; break;
    case UI_LINUI_KEY_F6: key_identity = UI_KEY_F6; break;
    case UI_LINUI_KEY_F7: key_identity = UI_KEY_F7; break;
    case UI_LINUI_KEY_F8: key_identity = UI_KEY_F8; break;
    case UI_LINUI_KEY_F9: key_identity = UI_KEY_F9; break;
    case UI_LINUI_KEY_F10: key_identity = UI_KEY_F10; break;
    case UI_LINUI_KEY_F11: key_identity = UI_KEY_F11; break;
    case UI_LINUI_KEY_F12: key_identity = UI_KEY_F12; break;
    case UI_LINUI_KEY_UP: key_identity = UI_KEY_UP; break;
    case UI_LINUI_KEY_DOWN: key_identity = UI_KEY_DOWN; break;
    case UI_LINUI_KEY_LEFT: key_identity = UI_KEY_LEFT; break;
    case UI_LINUI_KEY_RIGHT: key_identity = UI_KEY_RIGHT; break;
    case UI_LINUI_KEY_HOME: key_identity = UI_KEY_HOME; break;
    case UI_LINUI_KEY_END: key_identity = UI_KEY_END; break;
    case UI_LINUI_KEY_PAGE_UP: key_identity = UI_KEY_PAGE_UP; break;
    case UI_LINUI_KEY_PAGE_DOWN: key_identity = UI_KEY_PAGE_DOWN; break;
    case UI_LINUI_KEY_INSERT: key_identity = UI_KEY_INSERT; break;
    case UI_LINUI_KEY_DELETE: key_identity = UI_KEY_DELETE; break;
    default: return 0u;
    }
    return key_identity;
}

lib_bool ui_keyboard_platform_transition(lib_u16 scan, lib_u16 raw_key,
    lib_u16 *out_scan, lib_u32 *out_key)
{
    (void)scan;
    *out_scan = 0u;
    *out_key = ui_linui_key_identity((ui_linui_key)raw_key);
    return *out_key != 0u;
}

lib_bool ui_keyboard_platform_map_scalar(lib_u32 scalar,
    lib_u16 *out_raw_key, lib_u8 *out_modifiers)
{
    /* Terminal text has no portable physical-key layout: use TEXT delivery. */
    (void)scalar; (void)out_raw_key; (void)out_modifiers;
    return LIB_FALSE;
}
