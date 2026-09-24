#include "lib/kvm-base/input.h"
#include "lib/kvm-base/hotkey_interface.h"

static lib_bool kvm_keyboard_emit(void *context, kvm_input_sink sink,
    lib_u16 scan, lib_u32 key, lib_u8 record_flags,
    lib_u8 hotkey_modifiers, lib_bool pressed)
{
    kvm_input_event event;

    if (sink == LIB_NULL || key == 0u) return LIB_FALSE;
    lib_memory_set(&event, 0, sizeof(event));
    event.type = KVM_EVENT_KEY;
    event.data.key.pressed = pressed;
    event.data.key.key = key;
    event.data.key.scan_code = scan;
    event.data.key.flags = (record_flags & KVM_INPUT_FLAG_EXTENDED) != 0u ?
        KVM_KEY_FLAG_EXTENDED : 0u;
    event.data.key.modifiers = hotkey_modifiers;
    return sink(context, &event);
}

static lib_bool kvm_keyboard_submit_character(const kvm_hotkey_matcher *held_keys,
    void *context, kvm_input_sink sink, lib_u32 scalar)
{
    static const kvm_key modifier_keys[] = { KVM_KEY_CONTROL, KVM_KEY_ALT, KVM_KEY_SHIFT };
    static const lib_u16 modifier_scans[] = { 0x1du, 0x38u, 0x2au };
    kvm_input_event keys[4] = { 0 };
    lib_bool owned[4] = { 0 };
    lib_u16 virtual_key, scan;
    lib_u32 key, i, count = 0u;
    lib_u8 modifiers, active = 0u;

    if (scalar == 0u || scalar > 0x10ffffu ||
        (scalar >= 0xd800u && scalar <= 0xdfffu)) return LIB_FALSE;
    if (!kvm_keyboard_platform_map_scalar(scalar, &virtual_key, &modifiers) ||
        !kvm_keyboard_platform_transition(0u, virtual_key, &scan, &key)) {
        kvm_input_event event = { .type = KVM_EVENT_TEXT };
        event.data.text.scalar = scalar;
        return sink != LIB_NULL && sink(context, &event);
    }
    if (sink == LIB_NULL) return LIB_FALSE;
    /* Snapshot before any delivery can grow the shared ledger. This local
     * chord owns only the keys it adds; it is not a second held-key table. */
    for (i = 0u; i < 4u; ++i) {
        kvm_key identity = i < 3u ? modifier_keys[i] : key;
        const kvm_input_event *held = kvm_hotkey_matcher_held_key(held_keys, identity);
        if (i < 3u) {
            if (held != LIB_NULL) active |= (lib_u8)(1u << i);
            if ((modifiers & (1u << i)) == 0u) continue;
        }
        if (held != LIB_NULL) keys[count] = *held;
        else {
            keys[count].type = KVM_EVENT_KEY;
            keys[count].data.key.key = identity;
            keys[count].data.key.scan_code = i < 3u ? modifier_scans[i] : scan;
            owned[count] = LIB_TRUE;
        }
        ++count;
    }
    for (i = 0u; i < count; ++i) {
        if (i + 1u < count) {
            if (!owned[i]) continue;
            active |= keys[i].data.key.key == KVM_KEY_CONTROL ? KVM_INPUT_MODIFIER_CONTROL :
                keys[i].data.key.key == KVM_KEY_ALT ? KVM_INPUT_MODIFIER_ALT : KVM_INPUT_MODIFIER_SHIFT;
        }
        keys[i].data.key.pressed = LIB_TRUE;
        keys[i].data.key.modifiers = active;
        if (!sink(context, &keys[i])) return LIB_FALSE;
    }
    while (count != 0u) {
        --count;
        if (!owned[count]) continue;
        keys[count].data.key.pressed = LIB_FALSE;
        if (keys[count].data.key.key == KVM_KEY_CONTROL) active &= (lib_u8)~KVM_INPUT_MODIFIER_CONTROL;
        if (keys[count].data.key.key == KVM_KEY_ALT) active &= (lib_u8)~KVM_INPUT_MODIFIER_ALT;
        if (keys[count].data.key.key == KVM_KEY_SHIFT) active &= (lib_u8)~KVM_INPUT_MODIFIER_SHIFT;
        keys[count].data.key.modifiers = active;
        if (!sink(context, &keys[count])) return LIB_FALSE;
    }
    return LIB_TRUE;
}

static lib_bool kvm_keyboard_submit_utf16(kvm_keyboard_normalizer *state,
    const kvm_hotkey_matcher *held_keys, void *context, kvm_input_sink sink,
    lib_u16 code_unit, lib_u16 repeat_count)
{
    lib_u32 scalar = code_unit;
    lib_u16 high, prior_count;

    if (state == LIB_NULL) return LIB_FALSE;
    if (repeat_count == 0u) repeat_count = 1u;
    high = state->pending_high_surrogate;
    prior_count = state->pending_repeat_count;
    state->pending_high_surrogate = 0u;
    state->pending_repeat_count = 0u;
    if (code_unit >= 0xd800u && code_unit <= 0xdbffu) {
        /* A new high replaces a malformed unfinished prefix. */
        state->pending_high_surrogate = code_unit;
        state->pending_repeat_count = repeat_count;
        return LIB_TRUE;
    }
    if (code_unit >= 0xdc00u && code_unit <= 0xdfffu) {
        if (high == 0u || prior_count != repeat_count) return LIB_FALSE;
        scalar = 0x10000u + (((lib_u32)high - 0xd800u) << 10u) +
            ((lib_u32)code_unit - 0xdc00u);
    }
    /* A valid BMP unit survives a malformed prefix. Expand complete scalars,
     * stopping immediately on delivery failure, never retrying a partial batch. */
    while (repeat_count-- != 0u)
        if (!kvm_keyboard_submit_character(held_keys, context, sink, scalar)) return LIB_FALSE;
    return LIB_TRUE;
}

lib_i32 kvm_keyboard_submit_record(kvm_keyboard_normalizer *state,
    const kvm_hotkey_matcher *held_keys, void *context,
    kvm_input_sink sink, const kvm_keyboard_record *record)
{
    lib_u16 scan;
    lib_u32 identity;
    if (state == LIB_NULL || sink == LIB_NULL || record == LIB_NULL) return KVM_KEYBOARD_REJECTED;
    if (record->kind == KVM_KEYBOARD_CHARACTER)
        return kvm_keyboard_submit_utf16(state, held_keys, context, sink, record->utf16, record->repeat_count);
    if (record->kind != KVM_KEYBOARD_TRANSITION &&
        record->kind != KVM_KEYBOARD_COMBINED) return KVM_KEYBOARD_REJECTED;
    if (kvm_keyboard_platform_transition(record->scan, record->key, &scan, &identity)) {
        lib_u32 count = record->pressed && record->repeat_count != 0u ?
            record->repeat_count : 1u;
        state->pending_high_surrogate = 0u;
        state->pending_repeat_count = 0u;
        while (count-- != 0u)
            if (!kvm_keyboard_emit(context, sink, scan, identity,
                    record->flags, record->modifiers, record->pressed)) return KVM_KEYBOARD_REJECTED;
        return KVM_KEYBOARD_ACCEPTED;
    }
    if (record->kind == KVM_KEYBOARD_COMBINED) {
        if (record->pressed && record->utf16 != 0u)
            return kvm_keyboard_submit_utf16(state, held_keys, context, sink, record->utf16, record->repeat_count);
        return KVM_KEYBOARD_ACCEPTED;
    }
    return KVM_KEYBOARD_UNMAPPED;
}
