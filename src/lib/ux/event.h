#ifndef UX_EVENT_H
#define UX_EVENT_H

#include "type.h"

/* Product-neutral host input emitted by the presentation library.  Virtual
 * keys and scan codes describe a host physical transition; text is the
 * Unicode scalar that could not be represented by such a transition. */
typedef enum ux_event_type {
    UX_EVENT_KEY,
    UX_EVENT_TEXT,
    UX_EVENT_MOUSE
} ux_event_type;

typedef struct ux_event {
    ux_event_type type;
    union {
        struct {
            type_unsigned_16 scan_code;
            type_unsigned_16 virtual_key;
            type_unsigned_32 modifiers;
            type_unsigned_8 pressed;
        } key;
        struct {
            type_unsigned_32 scalar;
        } text;
        struct {
            type_signed_32 delta_x;
            type_signed_32 delta_y;
            type_unsigned_8 left_down;
            type_unsigned_8 right_down;
        } mouse;
    } data;
} ux_event;

typedef int (*ux_event_sink)(C_VOID *context, const ux_event *event);

#endif
