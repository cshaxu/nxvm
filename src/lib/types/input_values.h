#ifndef LIB_TYPES_INPUT_VALUES_H
#define LIB_TYPES_INPUT_VALUES_H

#include "lib/types/types_interface.h"

typedef lib_u32 lib_key;

enum {
    /* Neutral key identities. Printable keys use their ASCII scalar. These
     * values describe input only; product hotkey interpretation remains an
     * application concern. */
    LIB_KEY_F1 = 0x00010001u,
    LIB_KEY_F2 = 0x00010002u,
    LIB_KEY_F3 = 0x00010003u,
    LIB_KEY_F4 = 0x00010004u,
    LIB_KEY_F5 = 0x00010005u,
    LIB_KEY_F6 = 0x00010006u,
    LIB_KEY_F7 = 0x00010007u,
    LIB_KEY_F8 = 0x00010008u,
    LIB_KEY_F9 = 0x00010009u,
    LIB_KEY_F10 = 0x0001000au,
    LIB_KEY_F11 = 0x0001000bu,
    LIB_KEY_F12 = 0x0001000cu,
    LIB_KEY_ENTER = 0x00020001u,
    LIB_KEY_BACKSPACE = 0x00020002u,
    LIB_KEY_UP = 0x00020003u,
    LIB_KEY_DOWN = 0x00020004u,
    LIB_KEY_LEFT = 0x00020005u,
    LIB_KEY_RIGHT = 0x00020006u,
    LIB_KEY_HOME = 0x00020007u,
    LIB_KEY_END = 0x00020008u,
    LIB_KEY_PAGE_UP = 0x00020009u,
    LIB_KEY_PAGE_DOWN = 0x0002000au,
    LIB_KEY_INSERT = 0x0002000bu,
    LIB_KEY_DELETE = 0x0002000cu,
    LIB_KEY_ESCAPE = 0x00030001u,
    LIB_KEY_TAB = 0x00030002u,
    LIB_KEY_SHIFT = 0x00030003u,
    LIB_KEY_CONTROL = 0x00030004u,
    LIB_KEY_ALT = 0x00030005u,
    LIB_KEY_CAPS_LOCK = 0x00030006u,
    LIB_KEY_NUM_LOCK = 0x00030007u,
    LIB_KEY_SCROLL_LOCK = 0x00030008u,
    LIB_KEY_PAUSE = 0x00030009u,
    LIB_KEY_PRINT_SCREEN = 0x0003000au,
    LIB_KEY_LEFT_WINDOWS = 0x0003000bu,
    LIB_KEY_RIGHT_WINDOWS = 0x0003000cu,
    LIB_KEY_MENU = 0x0003000du,
    LIB_KEY_KEYPAD_0 = 0x00040000u,
    LIB_KEY_KEYPAD_1,
    LIB_KEY_KEYPAD_2,
    LIB_KEY_KEYPAD_3,
    LIB_KEY_KEYPAD_4,
    LIB_KEY_KEYPAD_5,
    LIB_KEY_KEYPAD_6,
    LIB_KEY_KEYPAD_7,
    LIB_KEY_KEYPAD_8,
    LIB_KEY_KEYPAD_9,
    LIB_KEY_KEYPAD_MULTIPLY,
    LIB_KEY_KEYPAD_ADD,
    LIB_KEY_KEYPAD_SUBTRACT,
    LIB_KEY_KEYPAD_DECIMAL,
    LIB_KEY_KEYPAD_DIVIDE,
    LIB_KEY_F13 = 0x0005000du,
    LIB_KEY_F14,
    LIB_KEY_F15,
    LIB_KEY_F16,
    LIB_KEY_F17,
    LIB_KEY_F18,
    LIB_KEY_F19,
    LIB_KEY_F20,
    LIB_KEY_F21,
    LIB_KEY_F22,
    LIB_KEY_F23,
    LIB_KEY_F24,
    LIB_MOUSE_BUTTON_LEFT = 0x01u,
    LIB_MOUSE_BUTTON_RIGHT = 0x02u,
    LIB_MOUSE_BUTTON_MIDDLE = 0x04u
};

enum {
    LIB_KEY_FLAG_EXTENDED = 0x01u,
    LIB_KEY_MODIFIER_CONTROL = 0x01u,
    LIB_KEY_MODIFIER_ALT = 0x02u,
    LIB_KEY_MODIFIER_SHIFT = 0x04u
};

/* Copied neutral input. Physical identity is the PC set-1 make code without
 * its break bit; EXTENDED denotes the E0 prefix. Zero means unavailable.
 * Pause uses make code 0x45 without E0 and LIB_KEY_PAUSE (no E1 byte stream);
 * Print Screen uses 0x37 with E0. Modifiers retain left/right physical codes.
 * modifiers uses LIB_KEY_MODIFIER_* bits. A logical key and scalar are
 * alternatives: scalar is a complete Unicode
 * scalar, never a UTF-16 code unit. Zero key/scalar means no delivery.
 * Repeat zero means one; releases occur once. */
typedef struct lib_key_input {
    lib_key key;
    lib_u32 scalar;
    lib_u16 scan_code;
    lib_u8 modifiers;
    lib_bool extended;
    lib_bool pressed;
    lib_u16 repeat_count;
} lib_key_input;

#endif
