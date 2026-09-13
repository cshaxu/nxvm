#ifndef UI_INPUT_H
#define UI_INPUT_H

#include "lib/ui-base/input_interface.h"

/* Selected platform decoding; state and event delivery stay in input.c. */
lib_bool ui_keyboard_platform_transition(lib_u16 scan, lib_u16 raw_key,
    lib_u16 *out_scan, lib_u32 *out_key);
lib_bool ui_keyboard_platform_map_scalar(lib_u32 scalar,
    lib_u16 *out_raw_key, lib_u8 *out_modifiers);

#endif
