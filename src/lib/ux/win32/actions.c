#include "lib/ux/win32/actions.h"

#ifdef _WIN32
#include <windows.h>

type_unsigned_8 ux_win32_modifiers_from_key_state(C_VOID)
{
    type_unsigned_8 modifiers = 0u;

    if ((GetKeyState(VK_CONTROL) & 0x8000) != 0)
        modifiers |= UX_MODIFIER_CONTROL;
    if ((GetKeyState(VK_MENU) & 0x8000) != 0)
        modifiers |= UX_MODIFIER_ALT;
    if ((GetKeyState(VK_SHIFT) & 0x8000) != 0)
        modifiers |= UX_MODIFIER_SHIFT;
    return modifiers;
}

type_unsigned_8 ux_win32_modifiers_from_console_state(DWORD control_state)
{
    type_unsigned_8 modifiers = 0u;

    if ((control_state & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0u)
        modifiers |= UX_MODIFIER_CONTROL;
    if ((control_state & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) != 0u)
        modifiers |= UX_MODIFIER_ALT;
    if ((control_state & SHIFT_PRESSED) != 0u)
        modifiers |= UX_MODIFIER_SHIFT;
    return modifiers;
}
#endif
