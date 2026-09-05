#ifndef UX_WIN32_INPUT_H
#define UX_WIN32_INPUT_H

#include "lib/ux/event.h"

#ifdef _WIN32
#include <windows.h>

/* This component normalizes Win32 input only. It emits product-neutral input
 * events and neither knows nor maps any guest keyboard/controller protocol. */

typedef struct ux_win32_keyboard_normalizer {
    WORD pending_high_surrogate;
    WORD recovered_virtual_key;
    WORD suppressed_virtual_key;
} ux_win32_keyboard_normalizer;

/* These functions only normalize host packets.  A project binding maps each
   physical record to its guest input protocol and owns its input queue. */
C_INT ux_win32_keyboard_submit_transition(C_VOID *context,
    ux_event_sink sink, WORD scan, WORD virtual_key,
    DWORD control_state, int pressed);
C_INT ux_win32_keyboard_submit_utf16(
    ux_win32_keyboard_normalizer *state, C_VOID *context,
    ux_event_sink sink, WORD code_unit);
/* Emit the standard physical Ctrl+Alt+Del make/break sequence. */
C_INT ux_win32_keyboard_submit_ctrl_alt_del(C_VOID *context,
    ux_event_sink sink);
/* A project may consume a Ctrl+Alt host chord after its modifier makes have
   arrived.  Emit releases before the binding pauses or substitutes input. */
C_INT ux_win32_keyboard_release_ctrl_alt(C_VOID *context,
    ux_event_sink sink);
/* Emit Alt+Enter after neutralizing the consumed Ctrl+Alt host chord. */
C_INT ux_win32_keyboard_submit_alt_enter(C_VOID *context,
    ux_event_sink sink);
void ux_win32_keyboard_note_recovered_key(
    ux_win32_keyboard_normalizer *state, WORD virtual_key);
void ux_win32_keyboard_release_recovered_key(
    ux_win32_keyboard_normalizer *state, WORD virtual_key);
C_INT ux_win32_keyboard_consume_duplicate_character(
    ux_win32_keyboard_normalizer *state, WORD code_unit);
#endif

#endif
