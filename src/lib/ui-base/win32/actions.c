#include "lib/types/types_interface.h"
#include "lib/ui-base/win32/actions.h"

#ifdef _WIN32
#include <windows.h>

lib_u8 ui_win32_modifiers_from_key_state(void)
{
    lib_u8 modifiers = 0u;

    if ((GetKeyState(VK_CONTROL) & 0x8000) != 0)
        modifiers |= UI_HOTKEY_MODIFIER_CONTROL;
    if ((GetKeyState(VK_MENU) & 0x8000) != 0)
        modifiers |= UI_HOTKEY_MODIFIER_ALT;
    if ((GetKeyState(VK_SHIFT) & 0x8000) != 0)
        modifiers |= UI_HOTKEY_MODIFIER_SHIFT;
    return modifiers;
}

#endif
