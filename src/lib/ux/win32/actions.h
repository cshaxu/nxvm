#ifndef UX_WIN32_ACTIONS_H
#define UX_WIN32_ACTIONS_H

#include "lib/ux/actions.h"

#ifdef _WIN32
#include <windows.h>

/* Native translation only. Chord ownership and matching remain in ux/. */
type_unsigned_8 ux_win32_modifiers_from_key_state(C_VOID);
type_unsigned_8 ux_win32_modifiers_from_console_state(DWORD control_state);
#endif

#endif
