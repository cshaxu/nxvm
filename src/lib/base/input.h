#ifndef BASE_INPUT_H
#define BASE_INPUT_H

#include "lib/base/input_adapter_interface.h"

lib_bool base_keyboard_platform_transition(lib_u16 scan, lib_u16 raw_key,
    lib_u16 *out_scan, lib_u32 *out_key);
lib_bool base_keyboard_platform_map_scalar(lib_u32 scalar,
    lib_u16 *out_raw_key, lib_u8 *out_modifiers);

#endif
