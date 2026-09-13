#include "lib/ui-base/input.h"
#include "lib/ui-base/hotkey_interface.h"

static int ui_keyboard_emit(void *context, ui_input_sink sink,
    lib_u16 scan, lib_u32 key, lib_u8 record_flags,
    lib_u8 hotkey_modifiers, int pressed)
{
    ui_input_event event;

    if (sink == LIB_NULL || key == 0u) return 0;
    lib_memory_set(&event, 0, sizeof(event));
    event.type = UI_EVENT_KEY;
    event.data.key.pressed = pressed != 0;
    event.data.key.key = key;
    event.data.key.scan_code = scan;
    event.data.key.flags = (record_flags & UI_INPUT_FLAG_EXTENDED) != 0u ?
        UI_KEY_FLAG_EXTENDED : 0u;
    event.data.key.modifiers = hotkey_modifiers;
    return sink(context, &event);
}

static int ui_keyboard_submit_character(const ui_hotkey_matcher *held_keys,
    void *context, ui_input_sink sink, lib_u32 scalar)
{
    static const ui_key modifier_keys[] = { UI_KEY_CONTROL, UI_KEY_ALT, UI_KEY_SHIFT };
    static const lib_u16 modifier_scans[] = { 0x1du, 0x38u, 0x2au };
    ui_input_event keys[4] = { 0 };
    lib_bool owned[4] = { 0 };
    lib_u16 virtual_key, scan;
    lib_u32 key, i, count = 0u;
    lib_u8 modifiers, active = 0u;

    if (scalar == 0u || scalar > 0x10ffffu ||
        (scalar >= 0xd800u && scalar <= 0xdfffu)) return 0;
    if (!ui_keyboard_platform_map_scalar(scalar, &virtual_key, &modifiers) ||
        !ui_keyboard_platform_transition(0u, virtual_key, &scan, &key)) {
        ui_input_event event = { .type = UI_EVENT_TEXT };
        event.data.text.scalar = scalar;
        return sink != LIB_NULL && sink(context, &event);
    }
    if (sink == LIB_NULL) return 0;
    /* Snapshot before any delivery can grow the shared ledger. This local
     * chord owns only the keys it adds; it is not a second held-key table. */
    for (i = 0u; i < 4u; ++i) {
        ui_key identity = i < 3u ? modifier_keys[i] : key;
        const ui_input_event *held = ui_hotkey_matcher_held_key(held_keys, identity);
        if (i < 3u) {
            if (held != LIB_NULL) active |= (lib_u8)(1u << i);
            if ((modifiers & (1u << i)) == 0u) continue;
        }
        if (held != LIB_NULL) keys[count] = *held;
        else {
            keys[count].type = UI_EVENT_KEY;
            keys[count].data.key.key = identity;
            keys[count].data.key.scan_code = i < 3u ? modifier_scans[i] : scan;
            owned[count] = LIB_TRUE;
        }
        ++count;
    }
    for (i = 0u; i < count; ++i) {
        if (i + 1u < count) {
            if (!owned[i]) continue;
            active |= keys[i].data.key.key == UI_KEY_CONTROL ? UI_INPUT_MODIFIER_CONTROL :
                keys[i].data.key.key == UI_KEY_ALT ? UI_INPUT_MODIFIER_ALT : UI_INPUT_MODIFIER_SHIFT;
        }
        keys[i].data.key.pressed = LIB_TRUE;
        keys[i].data.key.modifiers = active;
        if (!sink(context, &keys[i])) return 0;
    }
    while (count != 0u) {
        --count;
        if (!owned[count]) continue;
        keys[count].data.key.pressed = LIB_FALSE;
        if (keys[count].data.key.key == UI_KEY_CONTROL) active &= (lib_u8)~UI_INPUT_MODIFIER_CONTROL;
        if (keys[count].data.key.key == UI_KEY_ALT) active &= (lib_u8)~UI_INPUT_MODIFIER_ALT;
        if (keys[count].data.key.key == UI_KEY_SHIFT) active &= (lib_u8)~UI_INPUT_MODIFIER_SHIFT;
        keys[count].data.key.modifiers = active;
        if (!sink(context, &keys[count])) return 0;
    }
    return 1;
}

static int ui_keyboard_submit_utf16(ui_keyboard_normalizer *state,
    const ui_hotkey_matcher *held_keys, void *context, ui_input_sink sink,
    lib_u16 code_unit, lib_u16 repeat_count)
{
    lib_u32 scalar = code_unit;
    lib_u16 high, prior_count;

    if (state == LIB_NULL) return 0;
    if (repeat_count == 0u) repeat_count = 1u;
    high = state->pending_high_surrogate;
    prior_count = state->pending_repeat_count;
    state->pending_high_surrogate = 0u;
    state->pending_repeat_count = 0u;
    if (code_unit >= 0xd800u && code_unit <= 0xdbffu) {
        /* A new high replaces a malformed unfinished prefix. */
        state->pending_high_surrogate = code_unit;
        state->pending_repeat_count = repeat_count;
        return 1;
    }
    if (code_unit >= 0xdc00u && code_unit <= 0xdfffu) {
        if (high == 0u || prior_count != repeat_count) return 0;
        scalar = 0x10000u + (((lib_u32)high - 0xd800u) << 10u) +
            ((lib_u32)code_unit - 0xdc00u);
    }
    /* A valid BMP unit survives a malformed prefix. Expand complete scalars,
     * stopping immediately on delivery failure, never retrying a partial batch. */
    while (repeat_count-- != 0u)
        if (!ui_keyboard_submit_character(held_keys, context, sink, scalar)) return 0;
    return 1;
}

int ui_keyboard_submit_record(ui_keyboard_normalizer *state,
    const ui_hotkey_matcher *held_keys, void *context,
    ui_input_sink sink, const ui_keyboard_record *record)
{
    lib_u16 scan;
    lib_u32 identity;
    if (state == LIB_NULL || sink == LIB_NULL || record == LIB_NULL) return UI_KEYBOARD_REJECTED;
    if (record->kind == UI_KEYBOARD_CHARACTER)
        return ui_keyboard_submit_utf16(state, held_keys, context, sink, record->utf16, record->repeat_count);
    if (record->kind != UI_KEYBOARD_TRANSITION &&
        record->kind != UI_KEYBOARD_COMBINED) return UI_KEYBOARD_REJECTED;
    if (ui_keyboard_platform_transition(record->scan, record->key, &scan, &identity)) {
        lib_u32 count = record->pressed && record->repeat_count != 0u ?
            record->repeat_count : 1u;
        state->pending_high_surrogate = 0u;
        state->pending_repeat_count = 0u;
        while (count-- != 0u)
            if (!ui_keyboard_emit(context, sink, scan, identity,
                    record->flags, record->modifiers, record->pressed)) return UI_KEYBOARD_REJECTED;
        return UI_KEYBOARD_ACCEPTED;
    }
    if (record->kind == UI_KEYBOARD_COMBINED) {
        if (record->pressed && record->utf16 != 0u)
            return ui_keyboard_submit_utf16(state, held_keys, context, sink, record->utf16, record->repeat_count);
        return UI_KEYBOARD_ACCEPTED;
    }
    return UI_KEYBOARD_UNMAPPED;
}
