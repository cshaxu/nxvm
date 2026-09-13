#include "lib/ui-base/input.h"
#include "lib/types/win32/input.h"
#include "lib/types/win32/scalar.h"

static lib_u16 ui_keyboard_resolve_scan(lib_u16 virtual_key)
{
    return (lib_u16)lib_win32_map_virtual_key((lib_win32_uint)virtual_key, LIB_WIN32_MAPVK_VK_TO_VSC);
}

/* Layout interpretation belongs to the UI keyboard adapter.  types exposes
 * only raw key-state and scan-code queries. */
lib_bool ui_keyboard_platform_map_scalar(lib_u32 scalar,
    lib_u16 *out_virtual_key, lib_u8 *out_modifiers)
{
    lib_win32_key_state mapped;

    if (out_virtual_key == LIB_NULL || out_modifiers == LIB_NULL || scalar > 0xffffu)
        return LIB_FALSE;
    mapped = lib_win32_key_scan((lib_win32_wchar)scalar);
    if (mapped == -1) return LIB_FALSE;
    *out_virtual_key = (lib_u16)(mapped & 0xff);
    *out_modifiers = 0u;
    if (((mapped >> 8) & LIB_WIN32_KEY_SCAN_SHIFT) != 0u)
        *out_modifiers |= UI_INPUT_MODIFIER_SHIFT;
    if (((mapped >> 8) & LIB_WIN32_KEY_SCAN_CONTROL) != 0u)
        *out_modifiers |= UI_INPUT_MODIFIER_CONTROL;
    if (((mapped >> 8) & LIB_WIN32_KEY_SCAN_ALT) != 0u)
        *out_modifiers |= UI_INPUT_MODIFIER_ALT;
    return LIB_TRUE;
}

/* Native values stop at this adapter boundary.  This component decides how
 * they become neutral UI key identities. */
static lib_u32 ui_keyboard_key(lib_u16 virtual_key)
{
    if ((virtual_key >= LIB_WIN32_KEY_0 && virtual_key <= LIB_WIN32_KEY_9) ||
        (virtual_key >= LIB_WIN32_KEY_A && virtual_key <= LIB_WIN32_KEY_Z))
        return virtual_key;
    if (virtual_key >= LIB_WIN32_KEY_NUMPAD_0 && virtual_key <= LIB_WIN32_KEY_NUMPAD_9)
        return UI_KEY_KEYPAD_0 + virtual_key - LIB_WIN32_KEY_NUMPAD_0;
    if (virtual_key >= LIB_WIN32_KEY_F1 && virtual_key <= LIB_WIN32_KEY_F12)
        return UI_KEY_F1 + virtual_key - LIB_WIN32_KEY_F1;
    if (virtual_key >= LIB_WIN32_KEY_F13 && virtual_key <= LIB_WIN32_KEY_F24)
        return UI_KEY_F13 + virtual_key - LIB_WIN32_KEY_F13;
    switch (virtual_key) {
    case LIB_WIN32_KEY_SPACE: return ' ';
    case LIB_WIN32_KEY_OEM_1: return ';'; case LIB_WIN32_KEY_OEM_PLUS: return '=';
    case LIB_WIN32_KEY_OEM_COMMA: return ','; case LIB_WIN32_KEY_OEM_MINUS: return '-';
    case LIB_WIN32_KEY_OEM_PERIOD: return '.'; case LIB_WIN32_KEY_OEM_2: return '/';
    case LIB_WIN32_KEY_OEM_3: return '`'; case LIB_WIN32_KEY_OEM_4: return '[';
    case LIB_WIN32_KEY_OEM_5: return '\\'; case LIB_WIN32_KEY_OEM_6: return ']';
    case LIB_WIN32_KEY_OEM_7: return '\'';
    case LIB_WIN32_KEY_BACK: return UI_KEY_BACKSPACE; case LIB_WIN32_KEY_TAB: return UI_KEY_TAB;
    case LIB_WIN32_KEY_RETURN: return UI_KEY_ENTER; case LIB_WIN32_KEY_ESCAPE: return UI_KEY_ESCAPE;
    case LIB_WIN32_KEY_SHIFT: case LIB_WIN32_KEY_LEFT_SHIFT: case LIB_WIN32_KEY_RIGHT_SHIFT: return UI_KEY_SHIFT;
    case LIB_WIN32_KEY_CONTROL: case LIB_WIN32_KEY_LEFT_CONTROL: case LIB_WIN32_KEY_RIGHT_CONTROL: return UI_KEY_CONTROL;
    case LIB_WIN32_KEY_ALT: case LIB_WIN32_KEY_LEFT_ALT: case LIB_WIN32_KEY_RIGHT_ALT: return UI_KEY_ALT;
    case LIB_WIN32_KEY_CAPS_LOCK: return UI_KEY_CAPS_LOCK; case LIB_WIN32_KEY_NUM_LOCK: return UI_KEY_NUM_LOCK;
    case LIB_WIN32_KEY_SCROLL_LOCK: return UI_KEY_SCROLL_LOCK; case LIB_WIN32_KEY_PAUSE: return UI_KEY_PAUSE;
    case LIB_WIN32_KEY_SNAPSHOT: return UI_KEY_PRINT_SCREEN; case LIB_WIN32_KEY_HOME: return UI_KEY_HOME;
    case LIB_WIN32_KEY_END: return UI_KEY_END; case LIB_WIN32_KEY_PAGE_UP: return UI_KEY_PAGE_UP;
    case LIB_WIN32_KEY_PAGE_DOWN: return UI_KEY_PAGE_DOWN; case LIB_WIN32_KEY_LEFT: return UI_KEY_LEFT;
    case LIB_WIN32_KEY_UP: return UI_KEY_UP; case LIB_WIN32_KEY_RIGHT: return UI_KEY_RIGHT;
    case LIB_WIN32_KEY_DOWN: return UI_KEY_DOWN; case LIB_WIN32_KEY_INSERT: return UI_KEY_INSERT;
    case LIB_WIN32_KEY_DELETE: return UI_KEY_DELETE; case LIB_WIN32_KEY_LEFT_WINDOWS: return UI_KEY_LEFT_WINDOWS;
    case LIB_WIN32_KEY_RIGHT_WINDOWS: return UI_KEY_RIGHT_WINDOWS; case LIB_WIN32_KEY_APPS: return UI_KEY_MENU;
    case LIB_WIN32_KEY_MULTIPLY: return UI_KEY_KEYPAD_MULTIPLY;
    case LIB_WIN32_KEY_ADD: return UI_KEY_KEYPAD_ADD; case LIB_WIN32_KEY_SUBTRACT: return UI_KEY_KEYPAD_SUBTRACT;
    case LIB_WIN32_KEY_DECIMAL: return UI_KEY_KEYPAD_DECIMAL; case LIB_WIN32_KEY_DIVIDE: return UI_KEY_KEYPAD_DIVIDE;
    default: return 0u;
    }
}

lib_bool ui_keyboard_platform_transition(lib_u16 scan, lib_u16 raw_key,
    lib_u16 *out_scan, lib_u32 *out_key)
{
    if (scan == 0u) scan = ui_keyboard_resolve_scan(raw_key);
    *out_scan = scan & 0xffu;
    *out_key = ui_keyboard_key(raw_key);
    return scan != 0u && *out_key != 0u;
}
