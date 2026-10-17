#include "lib/base/input.h"

lib_bool base_keyboard_map_scalar(lib_u32 scalar, base_key_input *output)
{
    lib_u16 raw_key;
    if (output == LIB_NULL) return LIB_FALSE;
    lib_memory_set(output, 0, sizeof(*output));
    return base_keyboard_platform_map_scalar(scalar, &raw_key, &output->modifiers) &&
        base_keyboard_platform_transition(0u, raw_key, &output->scan_code, &output->key);
}

lib_i32 base_keyboard_decode(base_keyboard_decoder *state,
    const base_keyboard_record *record, base_key_input *output)
{
    lib_u16 high, prior_count, count;
    lib_u32 scalar;
    if (state == LIB_NULL || record == LIB_NULL || output == LIB_NULL)
        return BASE_KEYBOARD_REJECTED;
    lib_memory_set(output, 0, sizeof(*output));
    if (record->kind != BASE_KEYBOARD_CHARACTER) {
        if (record->kind != BASE_KEYBOARD_TRANSITION &&
            record->kind != BASE_KEYBOARD_COMBINED) return BASE_KEYBOARD_REJECTED;
        if (base_keyboard_platform_transition(record->scan, record->key,
                &output->scan_code, &output->key)) {
            state->pending_high_surrogate = state->pending_repeat_count = 0u;
            output->modifiers = record->modifiers;
            output->extended = (record->flags & BASE_KEY_FLAG_EXTENDED) != 0u;
            output->pressed = record->pressed;
            output->repeat_count = record->pressed && record->repeat_count != 0u ?
                record->repeat_count : 1u;
            return BASE_KEYBOARD_ACCEPTED;
        }
        output->key = 0u;
        output->scan_code = 0u;
        if (record->kind == BASE_KEYBOARD_TRANSITION) return BASE_KEYBOARD_UNMAPPED;
        if (!record->pressed || record->utf16 == 0u) return BASE_KEYBOARD_ACCEPTED;
    }
    count = record->repeat_count != 0u ? record->repeat_count : 1u;
    high = state->pending_high_surrogate;
    prior_count = state->pending_repeat_count;
    state->pending_high_surrogate = state->pending_repeat_count = 0u;
    scalar = record->utf16;
    if (scalar >= 0xd800u && scalar <= 0xdbffu) {
        state->pending_high_surrogate = (lib_u16)scalar;
        state->pending_repeat_count = count;
        return BASE_KEYBOARD_ACCEPTED;
    }
    if (scalar >= 0xdc00u && scalar <= 0xdfffu) {
        if (high == 0u || count != prior_count) return BASE_KEYBOARD_REJECTED;
        scalar = 0x10000u + (((lib_u32)high - 0xd800u) << 10u) + (scalar - 0xdc00u);
    }
    if (scalar == 0u) return BASE_KEYBOARD_REJECTED;
    output->scalar = scalar;
    output->pressed = LIB_TRUE;
    output->repeat_count = count;
    return BASE_KEYBOARD_ACCEPTED;
}
