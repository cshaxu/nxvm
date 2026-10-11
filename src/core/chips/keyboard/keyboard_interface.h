/* Copyright 2012-2026 Neko. */
#ifndef X86_KEYBOARD_INTERFACE_H
#define X86_KEYBOARD_INTERFACE_H
#include "lib/types/types_interface.h"

typedef struct x86_keyboard x86_keyboard;

typedef struct x86_keyboard_signals {
    lib_u8 scan_set;
    lib_u8 leds;
    lib_bool scanning;
    lib_bool bat_ready;
    lib_u8 repeat_bytes;
} x86_keyboard_signals;

/* Scoped connection, not retained. A reply is copied before return. A stream
 * reset discards unsent native bytes and partial encoding, never controller
 * output already accepted or command replies.
 * Delivery may record accepted output and take pending BAT; it may not issue
 * another command, reset, advance or destroy this keyboard recursively. */
typedef struct x86_keyboard_link {
    void (*reply)(void *context, const lib_u8 *bytes, lib_u8 count);
    void (*stream_reset)(void *context);
    void *context;
} x86_keyboard_link;

/* Qualified existing AT keyboard model. One execution owner; no host, IRQ,
 * controller, output FIFO or board-clock ownership. */
lib_status x86_keyboard_create(x86_keyboard **out_keyboard);
void x86_keyboard_destroy(x86_keyboard *keyboard);
void x86_keyboard_reset(x86_keyboard *keyboard);
void x86_keyboard_write(x86_keyboard *keyboard, lib_u8 value,
    const x86_keyboard_link *link);
/* Abort only the incomplete command/parameter transaction. */
void x86_keyboard_cancel_parameter(x86_keyboard *keyboard);
x86_keyboard_signals x86_keyboard_get_signals(const x86_keyboard *keyboard);

/* First interface release produces one startup BAT unless explicit reset
 * already owns it. Transport takes BAT only when it can admit that byte. */
lib_bool x86_keyboard_start(x86_keyboard *keyboard);
lib_bool x86_keyboard_take_bat(x86_keyboard *keyboard);
void x86_keyboard_clear_bat(x86_keyboard *keyboard);
/* Record the transport-accepted replay byte; controller-origin bytes never
 * enter this history. Preserves the existing qualified resend observation. */
void x86_keyboard_note_output(x86_keyboard *keyboard, lib_u8 byte);

/* Admit a bounded native stream and prepare bytes for the caller's transport.
 * The caller queues output only on OK. Failure leaves endpoint state intact
 * and output_count zero; output storage itself may have been written. Partial
 * wire prefixes may span calls, but one submitted array is failure-atomic. */
lib_status x86_keyboard_admit(x86_keyboard *keyboard, const lib_u8 *bytes,
    lib_size count, lib_u8 *output, lib_size capacity, lib_size *output_count);
void x86_keyboard_set_typematic_timing(x86_keyboard *keyboard,
    lib_u32 initial_ticks, lib_u32 repeat_ticks);
/* Units are caller-configured service ticks. The scoped sink receives repeat
 * sequences through the same bounded transport as ordinary input; no byte
 * backlog is kept here. True means at least one sequence was accepted. Failed
 * delivery retains one due repeat, not a backlog of elapsed repeats. */
lib_bool x86_keyboard_advance(x86_keyboard *keyboard, lib_u64 elapsed_ticks,
    lib_status (*repeat)(void *context, const lib_u8 *bytes, lib_size count),
    void *context);
lib_status x86_keyboard_ticks_until_repeat(const x86_keyboard *keyboard,
    lib_u64 *out_ticks);

/* Stateless scan-code vocabulary shared by keyboard repeat classification
 * and an independently composed controller's optional translation. */
lib_u8 x86_keyboard_set2_to_set1(lib_u8 byte, lib_bool *out_known);
#endif
