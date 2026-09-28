/* Copyright 2012-2026 Neko. */
#ifndef X86_XTKEYBOARD_INTERFACE_H
#define X86_XTKEYBOARD_INTERFACE_H
#include "lib/types/types_interface.h"

typedef struct x86_xt_keyboard x86_xt_keyboard;
typedef struct x86_xt_keyboard_timing {
    lib_u64 reset_ticks;
    lib_u64 bat_ticks;
    lib_u64 first_edge_ticks;
    lib_u64 clock_ticks;
} x86_xt_keyboard_timing;

/* Qualified nine-bit XT serial model, not a keyboard MCU. Durations use the
 * caller's single service unit. All zero disables timed delivery; otherwise
 * all four durations must be positive. No host or board clock is owned here.
 * One execution owner. The borrowed receiver/context live until destroy and
 * must not reenter this object. OK accepts one byte; any failure accepts none.
 * Refused completion remains pending until receiver_ready or line release;
 * it has no clock deadline and never repeats completed serial edges. */
typedef lib_status (*x86_xt_keyboard_receive)(void *context, lib_u8 byte);
lib_status x86_xt_keyboard_create(const x86_xt_keyboard_timing *timing,
    x86_xt_keyboard_receive receive, void *context, x86_xt_keyboard **out_keyboard);
void x86_xt_keyboard_destroy(x86_xt_keyboard *keyboard);
void x86_xt_keyboard_reset(x86_xt_keyboard *keyboard);
void x86_xt_keyboard_set_lines(x86_xt_keyboard *keyboard,
    lib_bool clock_held, lib_bool clear_asserted);
void x86_xt_keyboard_receiver_ready(x86_xt_keyboard *keyboard);
lib_status x86_xt_keyboard_receive_native_bytes(x86_xt_keyboard *keyboard,
    const lib_u8 *bytes, lib_size count);
void x86_xt_keyboard_advance(x86_xt_keyboard *keyboard, lib_u64 ticks);
/* Observation only; UNSUPPORTED means no timed event, not a zero delay. */
lib_status x86_xt_keyboard_ticks_until_event(const x86_xt_keyboard *keyboard,
    lib_u64 *out_ticks);
#endif
