#include "lib/ui-base/hotkey_interface.h"

static lib_u8 ui_hotkey_modifier_bit(ui_key key)
{
    if (key == UI_HOTKEY_KEY_CONTROL) return UI_HOTKEY_MODIFIER_CONTROL;
    if (key == UI_HOTKEY_KEY_ALT) return UI_HOTKEY_MODIFIER_ALT;
    if (key == UI_HOTKEY_KEY_SHIFT) return UI_HOTKEY_MODIFIER_SHIFT;
    return 0u;
}

static const ui_hotkey_registration *ui_hotkey_registry_match(
    const ui_hotkey_registry *registry, ui_key key, lib_u8 modifiers)
{
    lib_u32 index;
    if (registry == LIB_NULL) return LIB_NULL;
    for (index = 0u; index < registry->count; ++index) {
        const ui_hotkey_registration *entry = &registry->entries[index];
        if (entry->key == key && entry->modifiers == modifiers) return entry;
    }
    return LIB_NULL;
}

static lib_bool ui_hotkey_registry_has_modifier(const ui_hotkey_registry *registry,
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

static lib_bool ui_hotkey_same_key(const ui_input_event *a,
    const ui_input_event *b)
{
    if ((a->data.key.flags & UI_KEY_FLAG_EXTENDED) !=
        (b->data.key.flags & UI_KEY_FLAG_EXTENDED)) return LIB_FALSE;
    if (a->data.key.scan_code != 0u || b->data.key.scan_code != 0u)
        return a->data.key.scan_code == b->data.key.scan_code;
    return a->data.key.key == b->data.key.key;
}

static int ui_hotkey_flush_pending(ui_hotkey_matcher *matcher,
    ui_input_sink sink, void *context)
{
    lib_size i;
    for (i = 0u; i < matcher->held_count; ++i) {
        ui_hotkey_held_key *key = &matcher->held[i];
        if (key->state != UI_HOTKEY_PENDING) continue;
        if (key->allow_replay && !sink(context, &key->make)) return 0;
        key->state = UI_HOTKEY_DELIVERED;
    }
    return 1;
}

static int ui_hotkey_transition(ui_hotkey_matcher *matcher,
    const ui_input_event *event, ui_input_sink sink, void *context,
    lib_bool allow_replay)
{
    lib_size i, index;
    ui_hotkey_held_key *key;
    const ui_hotkey_registration *matched;
    ui_input_event held_transition;
    lib_bool eligible = LIB_TRUE;
    lib_u8 modifier;

    for (index = 0u; index < matcher->held_count; ++index)
        if (ui_hotkey_same_key(&matcher->held[index].make, event)) break;
    if (index != matcher->held_count) {
        /* Layout/lock changes must not change an already held key's lifetime. */
        held_transition = *event;
        held_transition.data.key.key = matcher->held[index].make.data.key.key;
        event = &held_transition;
    }
    modifier = ui_hotkey_modifier_bit(event->data.key.key);
    if (!event->data.key.pressed) {
        if (index == matcher->held_count)
            return ui_hotkey_flush_pending(matcher, sink, context) && sink(context, event);
        if (matcher->held[index].state != UI_HOTKEY_CONSUMED &&
            (!ui_hotkey_flush_pending(matcher, sink, context) || !sink(context, event)))
            return 0;
        --matcher->held_count;
        lib_memory_move(&matcher->held[index], &matcher->held[index + 1u],
            (matcher->held_count - index) * sizeof(*matcher->held));
        return 1;
    }
    if (index == matcher->held_count) {
        if (matcher->held_count == matcher->held_capacity) {
            lib_size capacity = matcher->held_capacity ? matcher->held_capacity * 2u : 8u;
            ui_hotkey_held_key *held;
            if (capacity < matcher->held_capacity ||
                capacity > LIB_SIZE_MAX / sizeof(*held)) return 0;
            held = lib_reallocate(matcher->held, capacity * sizeof(*held));
            if (held == LIB_NULL) return 0;
            matcher->held = held;
            matcher->held_capacity = capacity;
        }
        matcher->held[matcher->held_count++] = (ui_hotkey_held_key) {
            *event, UI_HOTKEY_PENDING, allow_replay };
    }
    key = &matcher->held[index];
    if (key->state == UI_HOTKEY_DELIVERED)
        return ui_hotkey_flush_pending(matcher, sink, context) && sink(context, event);

    for (i = 0u; i < matcher->held_count; ++i)
        if (matcher->held[i].state == UI_HOTKEY_DELIVERED &&
            (ui_hotkey_modifier_bit(matcher->held[i].make.data.key.key) &
                event->data.key.modifiers) != 0u) eligible = LIB_FALSE;
    matched = eligible ? ui_hotkey_registry_match(&matcher->registry,
        event->data.key.key, event->data.key.modifiers) : LIB_NULL;
    if (matched != LIB_NULL) {
        ui_input_event hotkey = *event;
        for (i = 0u; i < matcher->held_count; ++i)
            if (matcher->held[i].state == UI_HOTKEY_PENDING)
                matcher->held[i].state = UI_HOTKEY_CONSUMED;
        hotkey.type = UI_EVENT_HOTKEY;
        lib_memory_copy(hotkey.data.hotkey.identifier, matched->identifier,
            sizeof(hotkey.data.hotkey.identifier));
        return sink(context, &hotkey);
    }
    if (key->state == UI_HOTKEY_CONSUMED) return 1;
    if (modifier != 0u && ui_hotkey_registry_has_modifier(&matcher->registry, modifier))
        return 1;
    /* The new ordinary make is already pending in insertion order. */
    return ui_hotkey_flush_pending(matcher, sink, context);
}

void ui_hotkey_registry_initialize(ui_hotkey_registry *registry)
{
    if (registry != LIB_NULL) lib_memory_set(registry, 0, sizeof(*registry));
}

lib_status ui_hotkey_registry_register(ui_hotkey_registry *registry,
    ui_key key, lib_u8 modifiers, const char *identifier)
{
    lib_u32 index;
    const char *end;

    if (registry == LIB_NULL || key == 0u || identifier == LIB_NULL ||
        (end = lib_memory_find(identifier, '\0', UI_HOTKEY_IDENTIFIER_CAPACITY)) == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    for (index = 0u; index < registry->count; ++index) {
        if (registry->entries[index].key == key &&
            registry->entries[index].modifiers == modifiers)
            return LIB_STATUS_INVALID_STATE;
    }
    if (registry->count == UI_HOTKEY_CAPACITY) return LIB_STATUS_LIMIT_EXCEEDED;
    registry->entries[registry->count] = (ui_hotkey_registration) { key, modifiers,
        { 0 } };
    lib_memory_copy(registry->entries[registry->count].identifier, identifier,
        (lib_size)(end - identifier) + 1u);
    ++registry->count;
    return LIB_STATUS_OK;
}

void ui_hotkey_matcher_initialize(ui_hotkey_matcher *matcher,
    const ui_hotkey_registry *registry)
{
    if (matcher == LIB_NULL) return;
    lib_memory_set(matcher, 0, sizeof(*matcher));
    if (registry != LIB_NULL) matcher->registry = *registry;
}

int ui_hotkey_matcher_submit(ui_hotkey_matcher *matcher,
    const ui_input_event *event, ui_input_sink sink, void *context,
    lib_bool allow_replay)
{
    int delivered;
    if (matcher == LIB_NULL || event == LIB_NULL || sink == LIB_NULL ||
        matcher->failed) return 0;
    if (event->type == UI_EVENT_KEY)
        delivered = ui_hotkey_transition(matcher, event, sink, context, allow_replay);
    else
        delivered = (event->type != UI_EVENT_TEXT ||
            ui_hotkey_flush_pending(matcher, sink, context)) && sink(context, event);
    if (!delivered) matcher->failed = LIB_TRUE;
    return delivered;
}

void ui_hotkey_matcher_discard(ui_hotkey_matcher *matcher)
{
    if (matcher == LIB_NULL) return;
    lib_release(matcher->held);
    matcher->held = LIB_NULL;
    matcher->held_count = matcher->held_capacity = 0u;
    matcher->failed = LIB_FALSE;
}

const ui_input_event *ui_hotkey_matcher_held_key(
    const ui_hotkey_matcher *matcher, ui_key key)
{
    lib_size i;
    if (matcher != LIB_NULL)
        for (i = 0u; i < matcher->held_count; ++i)
            if (matcher->held[i].make.data.key.key == key)
                return &matcher->held[i].make;
    return LIB_NULL;
}
