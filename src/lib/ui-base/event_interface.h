#ifndef UI_EVENT_INTERFACE_H
#define UI_EVENT_INTERFACE_H

#include "lib/types/types_interface.h"

/* Product-neutral host input emitted by the presentation library. Key
 * identities and scan codes describe a host physical transition; text is the
 * Unicode scalar that could not be represented by such a transition. */
typedef enum ui_event_type {
    UI_EVENT_KEY,
    UI_EVENT_TEXT,
    UI_EVENT_MOUSE,
    UI_EVENT_HOTKEY,
    UI_EVENT_WINDOW_CLOSE,
    /* A raw component is permanently retiring. This is its final asynchronous
     * lifetime fact; the application clears pressed state by source_identity
     * and must not dereference the borrowed source handle. */
    UI_EVENT_SOURCE_RETIRED
} ui_event_type;

#define UI_HOTKEY_IDENTIFIER_CAPACITY 64u

typedef lib_u32 ui_key;

enum {
    /* Neutral key identities. Printable keys use their ASCII scalar. These
     * values describe input only; product hotkey interpretation remains an
     * application concern. */
    UI_KEY_F1 = 0x00010001u,
    UI_KEY_F2 = 0x00010002u,
    UI_KEY_F3 = 0x00010003u,
    UI_KEY_F4 = 0x00010004u,
    UI_KEY_F5 = 0x00010005u,
    UI_KEY_F6 = 0x00010006u,
    UI_KEY_F7 = 0x00010007u,
    UI_KEY_F8 = 0x00010008u,
    UI_KEY_F9 = 0x00010009u,
    UI_KEY_F10 = 0x0001000au,
    UI_KEY_F11 = 0x0001000bu,
    UI_KEY_F12 = 0x0001000cu,
    UI_KEY_ENTER = 0x00020001u,
    UI_KEY_BACKSPACE = 0x00020002u,
    UI_KEY_UP = 0x00020003u,
    UI_KEY_DOWN = 0x00020004u,
    UI_KEY_LEFT = 0x00020005u,
    UI_KEY_RIGHT = 0x00020006u,
    UI_KEY_HOME = 0x00020007u,
    UI_KEY_END = 0x00020008u,
    UI_KEY_PAGE_UP = 0x00020009u,
    UI_KEY_PAGE_DOWN = 0x0002000au,
    UI_KEY_INSERT = 0x0002000bu,
    UI_KEY_DELETE = 0x0002000cu,
    UI_KEY_ESCAPE = 0x00030001u,
    UI_KEY_TAB = 0x00030002u,
    UI_KEY_SHIFT = 0x00030003u,
    UI_KEY_CONTROL = 0x00030004u,
    UI_KEY_ALT = 0x00030005u,
    UI_KEY_CAPS_LOCK = 0x00030006u,
    UI_KEY_NUM_LOCK = 0x00030007u,
    UI_KEY_SCROLL_LOCK = 0x00030008u,
    UI_KEY_PAUSE = 0x00030009u,
    UI_KEY_PRINT_SCREEN = 0x0003000au,
    UI_KEY_LEFT_WINDOWS = 0x0003000bu,
    UI_KEY_RIGHT_WINDOWS = 0x0003000cu,
    UI_KEY_MENU = 0x0003000du,
    UI_KEY_KEYPAD_0 = 0x00040000u,
    UI_KEY_KEYPAD_1,
    UI_KEY_KEYPAD_2,
    UI_KEY_KEYPAD_3,
    UI_KEY_KEYPAD_4,
    UI_KEY_KEYPAD_5,
    UI_KEY_KEYPAD_6,
    UI_KEY_KEYPAD_7,
    UI_KEY_KEYPAD_8,
    UI_KEY_KEYPAD_9,
    UI_KEY_KEYPAD_MULTIPLY,
    UI_KEY_KEYPAD_ADD,
    UI_KEY_KEYPAD_SUBTRACT,
    UI_KEY_KEYPAD_DECIMAL,
    UI_KEY_KEYPAD_DIVIDE,
    UI_KEY_F13 = 0x0005000du,
    UI_KEY_F14,
    UI_KEY_F15,
    UI_KEY_F16,
    UI_KEY_F17,
    UI_KEY_F18,
    UI_KEY_F19,
    UI_KEY_F20,
    UI_KEY_F21,
    UI_KEY_F22,
    UI_KEY_F23,
    UI_KEY_F24,
    UI_MOUSE_BUTTON_LEFT = 0x01u,
    UI_MOUSE_BUTTON_RIGHT = 0x02u,
    UI_MOUSE_BUTTON_MIDDLE = 0x04u
};

enum {
    UI_KEY_FLAG_EXTENDED = 0x01u,
    UI_KEY_MODIFIER_CONTROL = 0x01u,
    UI_KEY_MODIFIER_ALT = 0x02u,
    UI_KEY_MODIFIER_SHIFT = 0x04u
};

typedef struct ui_input_event {
    /* Borrowed opaque component handle. It is for lifetime tracing only;
     * product action policy never branches on input source. */
    const void *source;
    /* Monotonic instance identity.  Unlike the borrowed address above this
     * value remains unambiguous after the component has been retired and its
     * storage can be reused.  Consumers use it only for lifetime hygiene. */
    lib_u64 source_identity;
    ui_event_type type;
    union {
        struct {
            lib_u16 scan_code;
            /* A ui-base-defined key identity. Zero means absent. */
            ui_key key;
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
            char identifier[UI_HOTKEY_IDENTIFIER_CAPACITY];
        } hotkey;
    } data;
} ui_input_event;

/* Legacy spellings preserve the existing input producer ABI while split
 * components and the application FIFO migrate to the explicit input-event
 * name. */
typedef ui_input_event ui_event;

typedef int (*ui_input_sink)(void *context, const ui_input_event *event);
typedef ui_input_sink ui_event_sink;

static inline void ui_input_event_set_source(ui_input_event *event,
    const void *source, lib_u64 source_identity)
{
    if (event != LIB_NULL) {
        event->source = source;
        event->source_identity = source_identity;
    }
}

#endif
