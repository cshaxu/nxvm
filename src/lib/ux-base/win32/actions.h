#ifndef UX_WIN32_ACTIONS_H
#define UX_WIN32_ACTIONS_H

#include "lib/ux-base/hotkey_interface.h"

#ifdef _WIN32
#include <windows.h>

/* Window messages expose no per-record modifier mask, so the Window leaf
 * samples its own native input state before submitting a transition. Chord
 * ownership and matching remain in ux/. */
lib_u8 ux_win32_modifiers_from_key_state(void);
#endif

#endif
