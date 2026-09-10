#ifndef UX_EVENT_H
#define UX_EVENT_H

#include "lib/base/base.h"

/* Product-neutral host input emitted by the presentation library. Key
 * identities and scan codes describe a host physical transition; text is the
 * Unicode scalar that could not be represented by such a transition. */
typedef enum ux_event_type {
    UX_EVENT_KEY,
    UX_EVENT_TEXT,
    UX_EVENT_MOUSE,
    UX_EVENT_HOTKEY,
    UX_EVENT_WINDOW_CLOSE,
    /* A raw component is permanently retiring. This is its final asynchronous
     * lifetime fact; the application clears pressed state by source_identity
     * and must not dereference the borrowed source handle. */
    UX_EVENT_SOURCE_RETIRED
} ux_event_type;

#define UX_HOTKEY_IDENTIFIER_CAPACITY 64u

typedef lib_u32 ux_key;

enum {
    /* Neutral key identities. Printable keys use their ASCII scalar. These
     * values describe input only; product hotkey interpretation remains an
     * application concern. */
    UX_KEY_F1 = 0x00010001u,
    UX_KEY_F2 = 0x00010002u,
    UX_KEY_F3 = 0x00010003u,
    UX_KEY_F4 = 0x00010004u,
    UX_KEY_F5 = 0x00010005u,
    UX_KEY_F6 = 0x00010006u,
    UX_KEY_F7 = 0x00010007u,
    UX_KEY_F8 = 0x00010008u,
    UX_KEY_F9 = 0x00010009u,
    UX_KEY_F10 = 0x0001000au,
    UX_KEY_F11 = 0x0001000bu,
    UX_KEY_F12 = 0x0001000cu,
    UX_KEY_ENTER = 0x00020001u,
    UX_KEY_BACKSPACE = 0x00020002u,
    UX_KEY_UP = 0x00020003u,
    UX_KEY_DOWN = 0x00020004u,
    UX_KEY_LEFT = 0x00020005u,
    UX_KEY_RIGHT = 0x00020006u,
    UX_KEY_HOME = 0x00020007u,
    UX_KEY_END = 0x00020008u,
    UX_KEY_PAGE_UP = 0x00020009u,
    UX_KEY_PAGE_DOWN = 0x0002000au,
    UX_KEY_INSERT = 0x0002000bu,
    UX_KEY_DELETE = 0x0002000cu,
    UX_KEY_ESCAPE = 0x00030001u,
    UX_KEY_TAB = 0x00030002u,
    UX_KEY_SHIFT = 0x00030003u,
    UX_KEY_CONTROL = 0x00030004u,
    UX_KEY_ALT = 0x00030005u,
    UX_KEY_CAPS_LOCK = 0x00030006u,
    UX_KEY_NUM_LOCK = 0x00030007u,
    UX_KEY_SCROLL_LOCK = 0x00030008u,
    UX_KEY_PAUSE = 0x00030009u,
    UX_KEY_PRINT_SCREEN = 0x0003000au,
    UX_KEY_LEFT_WINDOWS = 0x0003000bu,
    UX_KEY_RIGHT_WINDOWS = 0x0003000cu,
    UX_KEY_MENU = 0x0003000du,
    UX_KEY_KEYPAD_0 = 0x00040000u,
    UX_KEY_KEYPAD_1,
    UX_KEY_KEYPAD_2,
    UX_KEY_KEYPAD_3,
    UX_KEY_KEYPAD_4,
    UX_KEY_KEYPAD_5,
    UX_KEY_KEYPAD_6,
    UX_KEY_KEYPAD_7,
    UX_KEY_KEYPAD_8,
    UX_KEY_KEYPAD_9,
    UX_KEY_KEYPAD_MULTIPLY,
    UX_KEY_KEYPAD_ADD,
    UX_KEY_KEYPAD_SUBTRACT,
    UX_KEY_KEYPAD_DECIMAL,
    UX_KEY_KEYPAD_DIVIDE,
    UX_KEY_F13 = 0x0005000du,
    UX_KEY_F14,
    UX_KEY_F15,
    UX_KEY_F16,
    UX_KEY_F17,
    UX_KEY_F18,
    UX_KEY_F19,
    UX_KEY_F20,
    UX_KEY_F21,
    UX_KEY_F22,
    UX_KEY_F23,
    UX_KEY_F24,
    UX_MOUSE_BUTTON_LEFT = 0x01u,
    UX_MOUSE_BUTTON_RIGHT = 0x02u,
    UX_MOUSE_BUTTON_MIDDLE = 0x04u
};

enum {
    UX_KEY_FLAG_EXTENDED = 0x01u,
    UX_KEY_MODIFIER_CONTROL = 0x01u,
    UX_KEY_MODIFIER_ALT = 0x02u,
    UX_KEY_MODIFIER_SHIFT = 0x04u
};

typedef struct ux_input_event {
    /* Borrowed opaque component handle. It is for lifetime tracing only;
     * product action policy never branches on input source. */
    const void *source;
    /* Monotonic instance identity.  Unlike the borrowed address above this
     * value remains unambiguous after the component has been retired and its
     * storage can be reused.  Consumers use it only for lifetime hygiene. */
    lib_u64 source_identity;
    ux_event_type type;
    union {
        struct {
            lib_u16 scan_code;
            /* A ux-base-defined key identity. Zero means absent. */
            ux_key key;
            /* Platform-neutral physical-key facts. */
            lib_u32 flags;
            /* Current Ctrl/Alt/Shift state for the generic matcher. */
            lib_u8 modifiers;
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
        struct {
            char identifier[UX_HOTKEY_IDENTIFIER_CAPACITY];
        } hotkey;
    } data;
} ux_input_event;

/* Legacy spellings preserve the existing input producer ABI while split
 * components and the application FIFO migrate to the explicit input-event
 * name. */
typedef ux_input_event ux_event;

typedef int (*ux_input_sink)(void *context, const ux_input_event *event);
typedef ux_input_sink ux_event_sink;

static inline void ux_input_event_set_source(ux_input_event *event,
    const void *source, lib_u64 source_identity)
{
    if (event != LIB_NULL) {
        event->source = source;
        event->source_identity = source_identity;
    }
}

#endif
