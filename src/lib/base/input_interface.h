#ifndef BASE_INPUT_INTERFACE_H
#define BASE_INPUT_INTERFACE_H

#include "lib/types/types_interface.h"

typedef lib_u32 base_key;

enum {
    /* Neutral key identities. Printable keys use their ASCII scalar. These
     * values describe input only; product hotkey interpretation remains an
     * application concern. */
    BASE_KEY_F1 = 0x00010001u,
    BASE_KEY_F2 = 0x00010002u,
    BASE_KEY_F3 = 0x00010003u,
    BASE_KEY_F4 = 0x00010004u,
    BASE_KEY_F5 = 0x00010005u,
    BASE_KEY_F6 = 0x00010006u,
    BASE_KEY_F7 = 0x00010007u,
    BASE_KEY_F8 = 0x00010008u,
    BASE_KEY_F9 = 0x00010009u,
    BASE_KEY_F10 = 0x0001000au,
    BASE_KEY_F11 = 0x0001000bu,
    BASE_KEY_F12 = 0x0001000cu,
    BASE_KEY_ENTER = 0x00020001u,
    BASE_KEY_BACKSPACE = 0x00020002u,
    BASE_KEY_UP = 0x00020003u,
    BASE_KEY_DOWN = 0x00020004u,
    BASE_KEY_LEFT = 0x00020005u,
    BASE_KEY_RIGHT = 0x00020006u,
    BASE_KEY_HOME = 0x00020007u,
    BASE_KEY_END = 0x00020008u,
    BASE_KEY_PAGE_UP = 0x00020009u,
    BASE_KEY_PAGE_DOWN = 0x0002000au,
    BASE_KEY_INSERT = 0x0002000bu,
    BASE_KEY_DELETE = 0x0002000cu,
    BASE_KEY_ESCAPE = 0x00030001u,
    BASE_KEY_TAB = 0x00030002u,
    BASE_KEY_SHIFT = 0x00030003u,
    BASE_KEY_CONTROL = 0x00030004u,
    BASE_KEY_ALT = 0x00030005u,
    BASE_KEY_CAPS_LOCK = 0x00030006u,
    BASE_KEY_NUM_LOCK = 0x00030007u,
    BASE_KEY_SCROLL_LOCK = 0x00030008u,
    BASE_KEY_PAUSE = 0x00030009u,
    BASE_KEY_PRINT_SCREEN = 0x0003000au,
    BASE_KEY_LEFT_WINDOWS = 0x0003000bu,
    BASE_KEY_RIGHT_WINDOWS = 0x0003000cu,
    BASE_KEY_MENU = 0x0003000du,
    BASE_KEY_KEYPAD_0 = 0x00040000u,
    BASE_KEY_KEYPAD_1,
    BASE_KEY_KEYPAD_2,
    BASE_KEY_KEYPAD_3,
    BASE_KEY_KEYPAD_4,
    BASE_KEY_KEYPAD_5,
    BASE_KEY_KEYPAD_6,
    BASE_KEY_KEYPAD_7,
    BASE_KEY_KEYPAD_8,
    BASE_KEY_KEYPAD_9,
    BASE_KEY_KEYPAD_MULTIPLY,
    BASE_KEY_KEYPAD_ADD,
    BASE_KEY_KEYPAD_SUBTRACT,
    BASE_KEY_KEYPAD_DECIMAL,
    BASE_KEY_KEYPAD_DIVIDE,
    BASE_KEY_F13 = 0x0005000du,
    BASE_KEY_F14,
    BASE_KEY_F15,
    BASE_KEY_F16,
    BASE_KEY_F17,
    BASE_KEY_F18,
    BASE_KEY_F19,
    BASE_KEY_F20,
    BASE_KEY_F21,
    BASE_KEY_F22,
    BASE_KEY_F23,
    BASE_KEY_F24,
    BASE_MOUSE_BUTTON_LEFT = 0x01u,
    BASE_MOUSE_BUTTON_RIGHT = 0x02u,
    BASE_MOUSE_BUTTON_MIDDLE = 0x04u
};

enum {
    BASE_KEY_FLAG_EXTENDED = 0x01u,
    BASE_KEY_MODIFIER_CONTROL = 0x01u,
    BASE_KEY_MODIFIER_ALT = 0x02u,
    BASE_KEY_MODIFIER_SHIFT = 0x04u
};

/* Copied neutral input. Physical identity is the PC set-1 make code without
 * its break bit; EXTENDED denotes the E0 prefix. Zero means unavailable.
 * Pause uses make code 0x45 without E0 and BASE_KEY_PAUSE (no E1 byte stream);
 * Print Screen uses 0x37 with E0. Modifiers retain left/right physical codes.
 * modifiers uses BASE_KEY_MODIFIER_* bits. A logical key and scalar are
 * alternatives: scalar is a complete Unicode
 * scalar, never a UTF-16 code unit. Zero key/scalar means no delivery.
 * Repeat zero means one; releases occur once. */
typedef struct base_key_input {
    base_key key;
    lib_u32 scalar;
    lib_u16 scan_code;
    lib_u8 modifiers;
    lib_bool extended;
    lib_bool pressed;
    lib_u16 repeat_count;
} base_key_input;

#endif
