#ifndef UX_EVENT_H
#define UX_EVENT_H

#include "lib/base/base.h"

/* Product-neutral host input emitted by the presentation library.  Virtual
 * keys and scan codes describe a host physical transition; text is the
 * Unicode scalar that could not be represented by such a transition. */
typedef enum ux_event_type {
    UX_EVENT_KEY,
    UX_EVENT_TEXT,
    UX_EVENT_MOUSE
} ux_event_type;

enum {
    UX_MOUSE_BUTTON_LEFT = 0x01u,
    UX_MOUSE_BUTTON_RIGHT = 0x02u,
    UX_MOUSE_BUTTON_MIDDLE = 0x04u
};

typedef struct ux_event {
    ux_event_type type;
    union {
        struct {
            lib_u16 scan_code;
            lib_u16 virtual_key;
            lib_u32 modifiers;
            lib_u8 pressed;
        } key;
        struct {
            lib_u32 scalar;
        } text;
        struct {
            lib_i32 delta_x;
            lib_i32 delta_y;
            lib_i32 absolute_x;
            lib_i32 absolute_y;
            lib_i32 wheel_x;
            lib_i32 wheel_y;
            lib_u32 buttons;
            lib_u8 relative;
        } mouse;
    } data;
} ux_event;

typedef int (*ux_event_sink)(void *context, const ux_event *event);

#endif
