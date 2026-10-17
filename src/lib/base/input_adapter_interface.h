#ifndef BASE_INPUT_ADAPTER_INTERFACE_H
#define BASE_INPUT_ADAPTER_INTERFACE_H

#include "lib/types/input_values.h"

/* Internal Lib native-adapter support, not a portable application input API.
 * Only selected native producers interpret these copied native key/scan/UTF-16
 * fields. Public Console/KVM consumers receive lib_key_input instead. */
typedef struct base_keyboard_decoder {
    lib_u16 pending_high_surrogate;
    lib_u16 pending_repeat_count;
} base_keyboard_decoder;

enum { BASE_KEYBOARD_REJECTED = 0, BASE_KEYBOARD_ACCEPTED = 1, BASE_KEYBOARD_UNMAPPED = 2 };
typedef enum base_keyboard_record_kind {
    BASE_KEYBOARD_TRANSITION,
    BASE_KEYBOARD_CHARACTER,
    BASE_KEYBOARD_COMBINED
} base_keyboard_record_kind;

typedef struct base_keyboard_record {
    base_keyboard_record_kind kind;
    lib_u16 scan;
    lib_u16 key;
    lib_u16 utf16;
    lib_u8 flags;
    lib_u8 modifiers;
    lib_bool pressed;
    lib_u16 repeat_count;
} base_keyboard_record;

/* One zero-initialized decoder per native source. Reset on source handoff.
 * ACCEPTED with zero key/scalar buffers an incomplete prefix or ignores a
 * non-text break. Native repetitions are expanded only after decoding. */
lib_i32 base_keyboard_decode(base_keyboard_decoder *state,
    const base_keyboard_record *record, lib_key_input *output);

/* Host layout query with neutral results; false permits Unicode TEXT fallback. */
lib_bool base_keyboard_map_scalar(lib_u32 scalar, lib_key_input *output);

#endif
