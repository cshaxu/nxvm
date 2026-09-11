#ifndef UI_WIN32_INPUT_H
#define UI_WIN32_INPUT_H

#include "lib/ui-base/event_interface.h"

#ifdef _WIN32
#include <windows.h>

/* This component normalizes Win32 input only. It emits product-neutral input
 * events and neither knows nor maps any product input protocol. */

typedef struct ui_win32_keyboard_normalizer {
    WORD pending_high_surrogate;
    WORD recovered_virtual_key;
    WORD suppressed_virtual_key;
} ui_win32_keyboard_normalizer;

/* These functions only normalize host packets.  A project binding maps each
   physical record to its own input protocol and owns its input queue. */
int ui_win32_keyboard_submit_transition(void *context,
    ui_event_sink sink, WORD scan, WORD virtual_key,
    DWORD control_state, lib_u8 hotkey_modifiers, int pressed);
int ui_win32_keyboard_submit_utf16(
    ui_win32_keyboard_normalizer *state, void *context,
    ui_event_sink sink, WORD code_unit);
void ui_win32_keyboard_note_recovered_key(
    ui_win32_keyboard_normalizer *state, WORD virtual_key);
void ui_win32_keyboard_release_recovered_key(
    ui_win32_keyboard_normalizer *state, WORD virtual_key);
int ui_win32_keyboard_consume_duplicate_character(
    ui_win32_keyboard_normalizer *state, WORD code_unit);
#endif

#endif
