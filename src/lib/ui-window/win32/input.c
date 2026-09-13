#include "lib/ui-window/win32/input.h"
#include "lib/types/win32/input.h"
#include "lib/ui-base/hotkey_interface.h"

lib_u8 ui_window_modifiers_from_key_state(void)
{
    lib_u8 modifiers = 0u;

    if ((lib_win32_get_key_state(LIB_WIN32_KEY_CONTROL) & 0x8000) != 0)
        modifiers |= UI_HOTKEY_MODIFIER_CONTROL;
    if ((lib_win32_get_key_state(LIB_WIN32_KEY_ALT) & 0x8000) != 0)
        modifiers |= UI_HOTKEY_MODIFIER_ALT;
    if ((lib_win32_get_key_state(LIB_WIN32_KEY_SHIFT) & 0x8000) != 0)
        modifiers |= UI_HOTKEY_MODIFIER_SHIFT;
    return modifiers;
}

lib_u8 ui_window_keyboard_flags_from_lparam(lib_u64 message_lparam)
{
    return (message_lparam & 0x01000000u) != 0u ? UI_KEY_FLAG_EXTENDED : 0u;
}
