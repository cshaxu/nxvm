#include "lib/ui-base/win32/input.h"
#include "lib/ui-base/hotkey_interface.h"

#ifdef _WIN32
static WORD ui_win32_keyboard_decode_scan(WORD raw_scan)
{
    return (raw_scan & 0xff00u) == 0xe000u ?
        (WORD)(0x0100u | (raw_scan & 0x00ffu)) : (WORD)(raw_scan & 0x00ffu);
}

static WORD ui_win32_keyboard_resolve_scan(WORD virtual_key)
{
    return ui_win32_keyboard_decode_scan((WORD)MapVirtualKeyExW(
        virtual_key, MAPVK_VK_TO_VSC_EX, GetKeyboardLayout(0u)));
}

/* Win32 values end at this private adapter boundary.  The copied event always
 * carries a ui-base identity, so importing products never receive VK_* data. */
static lib_u32 ui_win32_keyboard_key(WORD virtual_key)
{
    if ((virtual_key >= '0' && virtual_key <= '9') ||
        (virtual_key >= 'A' && virtual_key <= 'Z')) return virtual_key;
    if (virtual_key >= VK_NUMPAD0 && virtual_key <= VK_NUMPAD9)
        return UI_KEY_KEYPAD_0 + virtual_key - VK_NUMPAD0;
    if (virtual_key >= VK_F1 && virtual_key <= VK_F12)
        return UI_KEY_F1 + virtual_key - VK_F1;
    if (virtual_key >= VK_F13 && virtual_key <= VK_F24)
        return UI_KEY_F13 + virtual_key - VK_F13;
    switch (virtual_key) {
    case VK_SPACE: return ' ';
    case VK_OEM_1: return ';'; case VK_OEM_PLUS: return '=';
    case VK_OEM_COMMA: return ','; case VK_OEM_MINUS: return '-';
    case VK_OEM_PERIOD: return '.'; case VK_OEM_2: return '/';
    case VK_OEM_3: return '`'; case VK_OEM_4: return '[';
    case VK_OEM_5: return '\\'; case VK_OEM_6: return ']';
    case VK_OEM_7: return '\'';
    case VK_BACK: return UI_KEY_BACKSPACE; case VK_TAB: return UI_KEY_TAB;
    case VK_RETURN: return UI_KEY_ENTER; case VK_ESCAPE: return UI_KEY_ESCAPE;
    case VK_SHIFT: case VK_LSHIFT: case VK_RSHIFT: return UI_KEY_SHIFT;
    case VK_CONTROL: case VK_LCONTROL: case VK_RCONTROL: return UI_KEY_CONTROL;
    case VK_MENU: case VK_LMENU: case VK_RMENU: return UI_KEY_ALT;
    case VK_CAPITAL: return UI_KEY_CAPS_LOCK; case VK_NUMLOCK: return UI_KEY_NUM_LOCK;
    case VK_SCROLL: return UI_KEY_SCROLL_LOCK; case VK_PAUSE: return UI_KEY_PAUSE;
    case VK_SNAPSHOT: return UI_KEY_PRINT_SCREEN; case VK_HOME: return UI_KEY_HOME;
    case VK_END: return UI_KEY_END; case VK_PRIOR: return UI_KEY_PAGE_UP;
    case VK_NEXT: return UI_KEY_PAGE_DOWN; case VK_LEFT: return UI_KEY_LEFT;
    case VK_UP: return UI_KEY_UP; case VK_RIGHT: return UI_KEY_RIGHT;
    case VK_DOWN: return UI_KEY_DOWN; case VK_INSERT: return UI_KEY_INSERT;
    case VK_DELETE: return UI_KEY_DELETE; case VK_LWIN: return UI_KEY_LEFT_WINDOWS;
    case VK_RWIN: return UI_KEY_RIGHT_WINDOWS; case VK_APPS: return UI_KEY_MENU;
    case VK_MULTIPLY: return UI_KEY_KEYPAD_MULTIPLY;
    case VK_ADD: return UI_KEY_KEYPAD_ADD; case VK_SUBTRACT: return UI_KEY_KEYPAD_SUBTRACT;
    case VK_DECIMAL: return UI_KEY_KEYPAD_DECIMAL; case VK_DIVIDE: return UI_KEY_KEYPAD_DIVIDE;
    default: return 0u;
    }
}

static int ui_win32_keyboard_emit(void *context,
    ui_event_sink sink, WORD scan, WORD virtual_key,
    DWORD control_state, lib_u8 hotkey_modifiers, int pressed)
{
    ui_event event;
    lib_u32 key = ui_win32_keyboard_key(virtual_key);
    if (sink == NULL || key == 0u || scan == 0u) return 0;
    ZeroMemory(&event, sizeof(event));
    event.type = UI_EVENT_KEY;
    event.data.key.pressed = pressed != 0;
    event.data.key.key = key;
    event.data.key.scan_code = scan;
    event.data.key.flags = (control_state & ENHANCED_KEY) != 0u ?
        UI_KEY_FLAG_EXTENDED : 0u;
    event.data.key.modifiers = hotkey_modifiers;
    return sink(context, &event);
}

int ui_win32_keyboard_submit_transition(void *context,
    ui_event_sink sink, WORD scan, WORD virtual_key,
    DWORD control_state, lib_u8 hotkey_modifiers, int pressed)
{
    if (scan == 0u) scan = ui_win32_keyboard_resolve_scan(virtual_key);
    return ui_win32_keyboard_emit(context, sink, scan, virtual_key,
        control_state, hotkey_modifiers, pressed);
}

void ui_win32_keyboard_note_recovered_key(
    ui_win32_keyboard_normalizer *state, WORD virtual_key)
{
    if (state != NULL) state->recovered_virtual_key =
        ui_win32_keyboard_resolve_scan(virtual_key) == 0u ? 0u : virtual_key;
}

void ui_win32_keyboard_release_recovered_key(
    ui_win32_keyboard_normalizer *state, WORD virtual_key)
{
    if (state != NULL && state->recovered_virtual_key == virtual_key)
        state->recovered_virtual_key = 0u;
}

int ui_win32_keyboard_consume_duplicate_character(
    ui_win32_keyboard_normalizer *state, WORD code_unit)
{
    SHORT mapped;
    int duplicate;

    if (state == NULL || state->recovered_virtual_key == 0u || code_unit == 0u ||
        (code_unit >= 0xd800u && code_unit <= 0xdfffu)) return 0;
    mapped = VkKeyScanExW((WCHAR)code_unit, GetKeyboardLayout(0u));
    duplicate = mapped != -1 && (WORD)(mapped & 0xffu) ==
        state->recovered_virtual_key;
    state->recovered_virtual_key = 0u;
    return duplicate;
}

static int ui_win32_keyboard_submit_character(void *context,
    ui_event_sink sink, lib_u32 scalar)
{
    SHORT mapped;
    WORD virtual_key;
    WORD scan;
    lib_u8 modifiers;
    lib_u8 hotkey_modifiers = 0u;

    if (scalar == 0u || scalar > 0xffffu ||
        (scalar >= 0xd800u && scalar <= 0xdfffu)) return 0;
    mapped = VkKeyScanExW((WCHAR)scalar, GetKeyboardLayout(0u));
    if (mapped == -1) {
        ui_event event;
        ZeroMemory(&event, sizeof(event));
        event.type = UI_EVENT_TEXT;
        event.data.text.scalar = scalar;
        return sink != NULL && sink(context, &event);
    }
    virtual_key = (WORD)(mapped & 0xffu);
    scan = ui_win32_keyboard_resolve_scan(virtual_key);
    if (scan == 0u) return 0;
    modifiers = (lib_u8)((mapped >> 8u) & 0xffu);
    if ((modifiers & 2u) != 0u) {
        hotkey_modifiers |= UI_HOTKEY_MODIFIER_CONTROL;
        if (!ui_win32_keyboard_emit(context, sink, 0x1du, VK_CONTROL, 0u,
                hotkey_modifiers, 1)) return 0;
    }
    if ((modifiers & 4u) != 0u) {
        hotkey_modifiers |= UI_HOTKEY_MODIFIER_ALT;
        if (!ui_win32_keyboard_emit(context, sink, 0x38u, VK_MENU, 0u,
                hotkey_modifiers, 1)) return 0;
    }
    if ((modifiers & 1u) != 0u) {
        hotkey_modifiers |= UI_HOTKEY_MODIFIER_SHIFT;
        if (!ui_win32_keyboard_emit(context, sink, 0x2au, VK_SHIFT, 0u,
                hotkey_modifiers, 1)) return 0;
    }
    if (!ui_win32_keyboard_emit(context, sink, scan, virtual_key, 0u,
            hotkey_modifiers, 1) ||
        !ui_win32_keyboard_emit(context, sink, scan, virtual_key, 0u,
            hotkey_modifiers, 0))
        return 0;
    if ((modifiers & 1u) != 0u) {
        hotkey_modifiers &= (lib_u8)~UI_HOTKEY_MODIFIER_SHIFT;
        if (!ui_win32_keyboard_emit(context, sink, 0x2au, VK_SHIFT, 0u,
                hotkey_modifiers, 0)) return 0;
    }
    if ((modifiers & 4u) != 0u) {
        hotkey_modifiers &= (lib_u8)~UI_HOTKEY_MODIFIER_ALT;
        if (!ui_win32_keyboard_emit(context, sink, 0x38u, VK_MENU, 0u,
                hotkey_modifiers, 0)) return 0;
    }
    if ((modifiers & 2u) != 0u) {
        hotkey_modifiers &= (lib_u8)~UI_HOTKEY_MODIFIER_CONTROL;
        if (!ui_win32_keyboard_emit(context, sink, 0x1du, VK_CONTROL, 0u,
                hotkey_modifiers, 0)) return 0;
    }
    return 1;
}

int ui_win32_keyboard_submit_utf16(ui_win32_keyboard_normalizer *state,
    void *context, ui_event_sink sink, WORD code_unit)
{
    lib_u32 scalar;

    if (state == NULL) return 0;
    if (code_unit >= 0xd800u && code_unit <= 0xdbffu) {
        if (state->pending_high_surrogate != 0u) return 0;
        state->pending_high_surrogate = code_unit;
        return 1;
    }
    if (code_unit >= 0xdc00u && code_unit <= 0xdfffu) {
        if (state->pending_high_surrogate == 0u) return 0;
        scalar = 0x10000u + (((lib_u32)state->pending_high_surrogate -
            0xd800u) << 10u) + ((lib_u32)code_unit - 0xdc00u);
        state->pending_high_surrogate = 0u;
        return ui_win32_keyboard_submit_character(context, sink, scalar);
    }
    if (state->pending_high_surrogate != 0u) {
        state->pending_high_surrogate = 0u;
        return 0;
    }
    return ui_win32_keyboard_submit_character(context, sink, code_unit);
}
#endif
