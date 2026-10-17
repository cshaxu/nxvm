#include "lib/kvm-base/input_interface.h"
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
    event.data.key.flags = (record_flags & BASE_KEY_FLAG_EXTENDED) != 0u ?
        BASE_KEY_FLAG_EXTENDED : 0u;
    event.data.key.modifiers = hotkey_modifiers;
    return sink(context, &event);
}

static lib_bool kvm_keyboard_submit_character(const kvm_hotkey_matcher *held_keys,
    void *context, kvm_input_sink sink, lib_u32 scalar)
{
    static const base_key modifier_keys[] = { BASE_KEY_CONTROL, BASE_KEY_ALT, BASE_KEY_SHIFT };
    static const lib_u16 modifier_scans[] = { 0x1du, 0x38u, 0x2au };
    kvm_input_event keys[4] = { 0 };
    lib_bool owned[4] = { 0 };
    lib_u16 scan;
    base_key_input mapped;
    lib_u32 key, i, count = 0u;
    lib_u8 modifiers, active = 0u;

    if (scalar == 0u || scalar > 0x10ffffu ||
        (scalar >= 0xd800u && scalar <= 0xdfffu)) return LIB_FALSE;
    if (!base_keyboard_map_scalar(scalar, &mapped)) {
        kvm_input_event event = { .type = KVM_EVENT_TEXT };
        event.data.text.scalar = scalar;
        return sink != LIB_NULL && sink(context, &event);
    }
    scan = mapped.scan_code;
    key = mapped.key;
    modifiers = mapped.modifiers;
    if (sink == LIB_NULL) return LIB_FALSE;
    /* Snapshot before any delivery can grow the shared ledger. This local
     * chord owns only the keys it adds; it is not a second held-key table. */
    for (i = 0u; i < 4u; ++i) {
        base_key identity = i < 3u ? modifier_keys[i] : key;
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
            active |= keys[i].data.key.key == BASE_KEY_CONTROL ? BASE_KEY_MODIFIER_CONTROL :
                keys[i].data.key.key == BASE_KEY_ALT ? BASE_KEY_MODIFIER_ALT : BASE_KEY_MODIFIER_SHIFT;
        }
        keys[i].data.key.pressed = LIB_TRUE;
        keys[i].data.key.modifiers = active;
        if (!sink(context, &keys[i])) return LIB_FALSE;
    }
    while (count != 0u) {
        --count;
        if (!owned[count]) continue;
        keys[count].data.key.pressed = LIB_FALSE;
        if (keys[count].data.key.key == BASE_KEY_CONTROL) active &= (lib_u8)~BASE_KEY_MODIFIER_CONTROL;
        if (keys[count].data.key.key == BASE_KEY_ALT) active &= (lib_u8)~BASE_KEY_MODIFIER_ALT;
        if (keys[count].data.key.key == BASE_KEY_SHIFT) active &= (lib_u8)~BASE_KEY_MODIFIER_SHIFT;
        keys[count].data.key.modifiers = active;
        if (!sink(context, &keys[count])) return LIB_FALSE;
    }
    return LIB_TRUE;
}


lib_bool kvm_keyboard_submit_input(const kvm_hotkey_matcher *held_keys,
    void *context, kvm_input_sink sink, const base_key_input *input)
{
    lib_u32 count;
    if (input == LIB_NULL || sink == LIB_NULL) return LIB_FALSE;
    if (input->key != 0u) {
        count = input->pressed && input->repeat_count != 0u ? input->repeat_count : 1u;
        while (count-- != 0u)
            if (!kvm_keyboard_emit(context, sink, input->scan_code, input->key,
                    input->extended ? BASE_KEY_FLAG_EXTENDED : 0u,
                    input->modifiers, input->pressed)) return LIB_FALSE;
    } else if (input->scalar != 0u) {
        count = input->repeat_count != 0u ? input->repeat_count : 1u;
        while (count-- != 0u)
            if (!kvm_keyboard_submit_character(held_keys, context, sink, input->scalar))
                return LIB_FALSE;
    }
    return LIB_TRUE;
}

lib_i32 kvm_keyboard_submit_record(base_keyboard_decoder *state,
    const kvm_hotkey_matcher *held_keys, void *context,
    kvm_input_sink sink, const base_keyboard_record *record)
{
    base_key_input input;
    lib_i32 result;
    if (sink == LIB_NULL) return BASE_KEYBOARD_REJECTED;
    result = base_keyboard_decode(state, record, &input);
    if (result != BASE_KEYBOARD_ACCEPTED) return result;
    return kvm_keyboard_submit_input(held_keys, context, sink, &input) ?
        BASE_KEYBOARD_ACCEPTED : BASE_KEYBOARD_REJECTED;
}
