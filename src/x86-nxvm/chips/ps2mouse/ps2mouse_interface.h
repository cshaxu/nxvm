/* Copyright 2012-2026 Neko. */
#ifndef X86_PS2MOUSE_INTERFACE_H
#define X86_PS2MOUSE_INTERFACE_H
#include "lib/types/types_interface.h"

typedef struct x86_ps2_mouse x86_ps2_mouse;

typedef struct x86_ps2_mouse_reply {
    lib_u8 bytes[4];
    lib_u8 count;
} x86_ps2_mouse_reply;

/* Existing three-byte AUX-device model, not a complete PS/2 implementation.
 * One execution owner; no controller, host input, clock or IRQ ownership.
 * Commands produce copied replies for the caller's serial transport. */
lib_status x86_ps2_mouse_create(x86_ps2_mouse **out_mouse);
void x86_ps2_mouse_destroy(x86_ps2_mouse *mouse);
void x86_ps2_mouse_reset(x86_ps2_mouse *mouse);
lib_status x86_ps2_mouse_write(x86_ps2_mouse *mouse, lib_u8 value,
    x86_ps2_mouse_reply *out_reply);

/* The scoped receiver accepts all three copied bytes or none. It must not
 * reenter or destroy this device. A failed delivery leaves button state
 * unchanged; unchanged zero-motion input needs no delivery. No callback or
 * packet pointer is retained. Transport readiness belongs to the caller. */
typedef lib_status (*x86_ps2_mouse_receive)(void *context,
    const lib_u8 packet[3]);
lib_status x86_ps2_mouse_report(x86_ps2_mouse *mouse,
    lib_i16 delta_x, lib_i16 delta_y, lib_u8 buttons,
    x86_ps2_mouse_receive receive, void *context);
#endif
