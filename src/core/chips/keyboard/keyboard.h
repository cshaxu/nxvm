/* Copyright 2012-2026 Neko. */
#ifndef X86_KEYBOARD_H
#define X86_KEYBOARD_H
#include "core/chips/keyboard/keyboard_interface.h"

typedef enum keyboard_parameter {
    KEYBOARD_PARAMETER_NONE,
    KEYBOARD_PARAMETER_LEDS,
    KEYBOARD_PARAMETER_TYPEMATIC,
    KEYBOARD_PARAMETER_SCAN_SET
} keyboard_parameter;

struct x86_keyboard {
    keyboard_parameter pending;
    lib_u8 scan_set;
    lib_u8 leds;
    lib_u8 typematic;
    lib_u8 repeat_byte;
    lib_u8 last_output;
    lib_u8 previous_output;
    lib_bool scanning;
    lib_bool repeat_active;
    lib_bool break_pending;
    lib_bool has_output;
    lib_bool bat_pending;
    lib_bool startup_released;
    lib_u32 nominal_initial_ticks;
    lib_u32 nominal_repeat_ticks;
    lib_u32 initial_ticks;
    lib_u32 repeat_ticks;
    lib_u64 remaining_ticks;
};
#endif
