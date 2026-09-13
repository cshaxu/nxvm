#include "lib/kvm-base/hotkey_interface.h"

static lib_u8 kvm_hotkey_modifier_bit(kvm_key key)
{
    if (key == KVM_HOTKEY_KEY_CONTROL) return KVM_HOTKEY_MODIFIER_CONTROL;
    if (key == KVM_HOTKEY_KEY_ALT) return KVM_HOTKEY_MODIFIER_ALT;
    if (key == KVM_HOTKEY_KEY_SHIFT) return KVM_HOTKEY_MODIFIER_SHIFT;
    return 0u;
}

static const kvm_hotkey_registration *kvm_hotkey_registry_match(
    const kvm_hotkey_registry *registry, kvm_key key, lib_u8 modifiers)
{
    lib_u32 index;
    if (registry == LIB_NULL) return LIB_NULL;
    for (index = 0u; index < registry->count; ++index) {
        const kvm_hotkey_registration *entry = &registry->entries[index];
        if (entry->key == key && entry->modifiers == modifiers) return entry;
    }
    return LIB_NULL;
}

static lib_bool kvm_hotkey_registry_has_modifier(const kvm_hotkey_registry *registry,
    lib_u8 modifier)
{
    lib_u32 index;
    if (registry == LIB_NULL || modifier == 0u) return LIB_FALSE;
    for (index = 0u; index < registry->count; ++index) {
        if ((registry->entries[index].modifiers & modifier) != 0u)
            return LIB_TRUE;
    }
    return LIB_FALSE;
}

static lib_bool kvm_hotkey_same_key(const kvm_input_event *a,
    const kvm_input_event *b)
{
    if ((a->data.key.flags & KVM_KEY_FLAG_EXTENDED) !=
        (b->data.key.flags & KVM_KEY_FLAG_EXTENDED)) return LIB_FALSE;
    if (a->data.key.scan_code != 0u || b->data.key.scan_code != 0u)
        return a->data.key.scan_code == b->data.key.scan_code;
    return a->data.key.key == b->data.key.key;
}

static int kvm_hotkey_flush_pending(kvm_hotkey_matcher *matcher,
    kvm_input_sink sink, void *context)
{
    lib_size i;
    for (i = 0u; i < matcher->held_count; ++i) {
        kvm_hotkey_held_key *key = &matcher->held[i];
        if (key->state != KVM_HOTKEY_PENDING) continue;
        if (key->allow_replay && !sink(context, &key->make)) return 0;
        key->state = KVM_HOTKEY_DELIVERED;
    }
    return 1;
}

static int kvm_hotkey_transition(kvm_hotkey_matcher *matcher,
    const kvm_input_event *event, kvm_input_sink sink, void *context,
    lib_bool allow_replay)
{
    lib_size i, index;
    kvm_hotkey_held_key *key;
    const kvm_hotkey_registration *matched;
    kvm_input_event held_transition;
    lib_bool eligible = LIB_TRUE;
    lib_u8 modifier;

    for (index = 0u; index < matcher->held_count; ++index)
        if (kvm_hotkey_same_key(&matcher->held[index].make, event)) break;
    if (index != matcher->held_count) {
        /* Layout/lock changes must not change an already held key's lifetime. */
        held_transition = *event;
        held_transition.data.key.key = matcher->held[index].make.data.key.key;
        event = &held_transition;
    }
    modifier = kvm_hotkey_modifier_bit(event->data.key.key);
    if (!event->data.key.pressed) {
        if (index == matcher->held_count)
            return kvm_hotkey_flush_pending(matcher, sink, context) && sink(context, event);
        if (matcher->held[index].state != KVM_HOTKEY_CONSUMED &&
            (!kvm_hotkey_flush_pending(matcher, sink, context) || !sink(context, event)))
            return 0;
        --matcher->held_count;
        lib_memory_move(&matcher->held[index], &matcher->held[index + 1u],
            (matcher->held_count - index) * sizeof(*matcher->held));
        return 1;
    }
    if (index == matcher->held_count) {
        if (matcher->held_count == matcher->held_capacity) {
            lib_size capacity = matcher->held_capacity ? matcher->held_capacity * 2u : 8u;
            kvm_hotkey_held_key *held;
            if (capacity < matcher->held_capacity ||
                capacity > LIB_SIZE_MAX / sizeof(*held)) return 0;
            held = lib_reallocate(matcher->held, capacity * sizeof(*held));
            if (held == LIB_NULL) return 0;
            matcher->held = held;
            matcher->held_capacity = capacity;
        }
        matcher->held[matcher->held_count++] = (kvm_hotkey_held_key) {
            *event, KVM_HOTKEY_PENDING, allow_replay };
    }
    key = &matcher->held[index];
    if (key->state == KVM_HOTKEY_DELIVERED)
        return kvm_hotkey_flush_pending(matcher, sink, context) && sink(context, event);

    for (i = 0u; i < matcher->held_count; ++i)
        if (matcher->held[i].state == KVM_HOTKEY_DELIVERED &&
            (kvm_hotkey_modifier_bit(matcher->held[i].make.data.key.key) &
                event->data.key.modifiers) != 0u) eligible = LIB_FALSE;
    matched = eligible ? kvm_hotkey_registry_match(&matcher->registry,
        event->data.key.key, event->data.key.modifiers) : LIB_NULL;
    if (matched != LIB_NULL) {
        kvm_input_event hotkey = *event;
        for (i = 0u; i < matcher->held_count; ++i)
            if (matcher->held[i].state == KVM_HOTKEY_PENDING)
                matcher->held[i].state = KVM_HOTKEY_CONSUMED;
        hotkey.type = KVM_EVENT_HOTKEY;
        lib_memory_copy(hotkey.data.hotkey.identifier, matched->identifier,
            sizeof(hotkey.data.hotkey.identifier));
        return sink(context, &hotkey);
    }
    if (key->state == KVM_HOTKEY_CONSUMED) return 1;
    if (modifier != 0u && kvm_hotkey_registry_has_modifier(&matcher->registry, modifier))
        return 1;
    /* The new ordinary make is already pending in insertion order. */
    return kvm_hotkey_flush_pending(matcher, sink, context);
}

void kvm_hotkey_registry_initialize(kvm_hotkey_registry *registry)
{
    if (registry != LIB_NULL) lib_memory_set(registry, 0, sizeof(*registry));
}

lib_status kvm_hotkey_registry_register(kvm_hotkey_registry *registry,
    kvm_key key, lib_u8 modifiers, const char *identifier)
{
    lib_u32 index;
    const char *end;

    if (registry == LIB_NULL || key == 0u || identifier == LIB_NULL ||
        (end = lib_memory_find(identifier, '\0', KVM_HOTKEY_IDENTIFIER_CAPACITY)) == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    for (index = 0u; index < registry->count; ++index) {
        if (registry->entries[index].key == key &&
            registry->entries[index].modifiers == modifiers)
            return LIB_STATUS_INVALID_STATE;
    }
    if (registry->count == KVM_HOTKEY_CAPACITY) return LIB_STATUS_LIMIT_EXCEEDED;
    registry->entries[registry->count] = (kvm_hotkey_registration) { key, modifiers,
        { 0 } };
    lib_memory_copy(registry->entries[registry->count].identifier, identifier,
        (lib_size)(end - identifier) + 1u);
    ++registry->count;
    return LIB_STATUS_OK;
}

void kvm_hotkey_matcher_initialize(kvm_hotkey_matcher *matcher,
    const kvm_hotkey_registry *registry)
{
    if (matcher == LIB_NULL) return;
    lib_memory_set(matcher, 0, sizeof(*matcher));
    if (registry != LIB_NULL) matcher->registry = *registry;
}

int kvm_hotkey_matcher_submit(kvm_hotkey_matcher *matcher,
    const kvm_input_event *event, kvm_input_sink sink, void *context,
    lib_bool allow_replay)
{
    int delivered;
    if (matcher == LIB_NULL || event == LIB_NULL || sink == LIB_NULL ||
        matcher->failed) return 0;
    if (event->type == KVM_EVENT_KEY)
        delivered = kvm_hotkey_transition(matcher, event, sink, context, allow_replay);
    else
        delivered = (event->type != KVM_EVENT_TEXT ||
            kvm_hotkey_flush_pending(matcher, sink, context)) && sink(context, event);
    if (!delivered) matcher->failed = LIB_TRUE;
    return delivered;
}

void kvm_hotkey_matcher_discard(kvm_hotkey_matcher *matcher)
{
    if (matcher == LIB_NULL) return;
    lib_release(matcher->held);
    matcher->held = LIB_NULL;
    matcher->held_count = matcher->held_capacity = 0u;
    matcher->failed = LIB_FALSE;
}

const kvm_input_event *kvm_hotkey_matcher_held_key(
    const kvm_hotkey_matcher *matcher, kvm_key key)
{
    lib_size i;
    if (matcher != LIB_NULL)
        for (i = 0u; i < matcher->held_count; ++i)
            if (matcher->held[i].make.data.key.key == key)
                return &matcher->held[i].make;
    return LIB_NULL;
}
