#ifndef KVM_EVENT_INTERFACE_H
#define KVM_EVENT_INTERFACE_H

#include "lib/types/types_interface.h"

/* Product-neutral host input emitted by the presentation library. Key
 * identities and scan codes describe a host physical transition; text is the
 * Unicode scalar that could not be represented by such a transition. */
typedef enum kvm_event_type {
    KVM_EVENT_KEY,
    KVM_EVENT_TEXT,
    KVM_EVENT_MOUSE,
    KVM_EVENT_HOTKEY,
    KVM_EVENT_WINDOW_CLOSE,
    /* A raw component is permanently retiring. This is its final asynchronous
     * lifetime fact; the application clears pressed state by source_identity
     * and must not dereference the borrowed source handle. */
    KVM_EVENT_SOURCE_RETIRED
} kvm_event_type;

#define KVM_HOTKEY_IDENTIFIER_CAPACITY 64u

typedef lib_u32 kvm_key;

enum {
    /* Neutral key identities. Printable keys use their ASCII scalar. These
     * values describe input only; product hotkey interpretation remains an
     * application concern. */
    KVM_KEY_F1 = 0x00010001u,
    KVM_KEY_F2 = 0x00010002u,
    KVM_KEY_F3 = 0x00010003u,
    KVM_KEY_F4 = 0x00010004u,
    KVM_KEY_F5 = 0x00010005u,
    KVM_KEY_F6 = 0x00010006u,
    KVM_KEY_F7 = 0x00010007u,
    KVM_KEY_F8 = 0x00010008u,
    KVM_KEY_F9 = 0x00010009u,
    KVM_KEY_F10 = 0x0001000au,
    KVM_KEY_F11 = 0x0001000bu,
    KVM_KEY_F12 = 0x0001000cu,
    KVM_KEY_ENTER = 0x00020001u,
    KVM_KEY_BACKSPACE = 0x00020002u,
    KVM_KEY_UP = 0x00020003u,
    KVM_KEY_DOWN = 0x00020004u,
    KVM_KEY_LEFT = 0x00020005u,
    KVM_KEY_RIGHT = 0x00020006u,
    KVM_KEY_HOME = 0x00020007u,
    KVM_KEY_END = 0x00020008u,
    KVM_KEY_PAGE_UP = 0x00020009u,
    KVM_KEY_PAGE_DOWN = 0x0002000au,
    KVM_KEY_INSERT = 0x0002000bu,
    KVM_KEY_DELETE = 0x0002000cu,
    KVM_KEY_ESCAPE = 0x00030001u,
    KVM_KEY_TAB = 0x00030002u,
    KVM_KEY_SHIFT = 0x00030003u,
    KVM_KEY_CONTROL = 0x00030004u,
    KVM_KEY_ALT = 0x00030005u,
    KVM_KEY_CAPS_LOCK = 0x00030006u,
    KVM_KEY_NUM_LOCK = 0x00030007u,
    KVM_KEY_SCROLL_LOCK = 0x00030008u,
    KVM_KEY_PAUSE = 0x00030009u,
    KVM_KEY_PRINT_SCREEN = 0x0003000au,
    KVM_KEY_LEFT_WINDOWS = 0x0003000bu,
    KVM_KEY_RIGHT_WINDOWS = 0x0003000cu,
    KVM_KEY_MENU = 0x0003000du,
    KVM_KEY_KEYPAD_0 = 0x00040000u,
    KVM_KEY_KEYPAD_1,
    KVM_KEY_KEYPAD_2,
    KVM_KEY_KEYPAD_3,
    KVM_KEY_KEYPAD_4,
    KVM_KEY_KEYPAD_5,
    KVM_KEY_KEYPAD_6,
    KVM_KEY_KEYPAD_7,
    KVM_KEY_KEYPAD_8,
    KVM_KEY_KEYPAD_9,
    KVM_KEY_KEYPAD_MULTIPLY,
    KVM_KEY_KEYPAD_ADD,
    KVM_KEY_KEYPAD_SUBTRACT,
    KVM_KEY_KEYPAD_DECIMAL,
    KVM_KEY_KEYPAD_DIVIDE,
    KVM_KEY_F13 = 0x0005000du,
    KVM_KEY_F14,
    KVM_KEY_F15,
    KVM_KEY_F16,
    KVM_KEY_F17,
    KVM_KEY_F18,
    KVM_KEY_F19,
    KVM_KEY_F20,
    KVM_KEY_F21,
    KVM_KEY_F22,
    KVM_KEY_F23,
    KVM_KEY_F24,
    KVM_MOUSE_BUTTON_LEFT = 0x01u,
    KVM_MOUSE_BUTTON_RIGHT = 0x02u,
    KVM_MOUSE_BUTTON_MIDDLE = 0x04u
};

enum {
    KVM_KEY_FLAG_EXTENDED = 0x01u,
    KVM_KEY_MODIFIER_CONTROL = 0x01u,
    KVM_KEY_MODIFIER_ALT = 0x02u,
    KVM_KEY_MODIFIER_SHIFT = 0x04u
};

typedef struct kvm_input_event {
    /* Borrowed opaque component handle. It is for lifetime tracing only;
     * product action policy never branches on input source. */
    const void *source;
    /* Monotonic instance identity.  Unlike the borrowed address above this
     * value remains unambiguous after the component has been retired and its
     * storage can be reused.  Consumers use it only for lifetime hygiene. */
    lib_u64 source_identity;
    kvm_event_type type;
    union {
        struct {
            lib_u16 scan_code;
            /* A kvm-base-defined key identity. Zero means absent. */
            kvm_key key;
            /* Platform-neutral physical-key facts. */
            lib_u32 flags;
            /* Current Ctrl/Alt/Shift state for the generic matcher. */
            lib_u8 modifiers;
            lib_bool pressed;
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
            lib_bool relative;
        } mouse;
        struct {
            lib_u8 identifier[KVM_HOTKEY_IDENTIFIER_CAPACITY];
        } hotkey;
    } data;
} kvm_input_event;

typedef lib_bool (*kvm_input_sink)(void *context, const kvm_input_event *event);

static inline void kvm_input_event_set_source(kvm_input_event *event,
    const void *source, lib_u64 source_identity)
{
    if (event != LIB_NULL) {
        event->source = source;
        event->source_identity = source_identity;
    }
}

#endif
