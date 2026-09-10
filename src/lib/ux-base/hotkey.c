#include "lib/ux-base/hotkey.h"

static lib_u8 ux_hotkey_modifier_bit(ux_key key)
{
    if (key == UX_HOTKEY_KEY_CONTROL) return UX_HOTKEY_MODIFIER_CONTROL;
    if (key == UX_HOTKEY_KEY_ALT) return UX_HOTKEY_MODIFIER_ALT;
    if (key == UX_HOTKEY_KEY_SHIFT) return UX_HOTKEY_MODIFIER_SHIFT;
    return 0u;
}

static const ux_hotkey_registration *ux_hotkey_registry_match(
    const ux_hotkey_registry *registry, ux_key key, lib_u8 modifiers)
{
    lib_u32 index;
    if (registry == LIB_NULL) return LIB_NULL;
    for (index = 0u; index < registry->count; ++index) {
        const ux_hotkey_registration *entry = &registry->entries[index];
        if (entry->key == key && entry->modifiers == modifiers) return entry;
    }
    return LIB_NULL;
}

static lib_bool ux_hotkey_registry_has_modifier(const ux_hotkey_registry *registry,
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

static int ux_hotkey_flush_pending(ux_hotkey_matcher *matcher,
    ux_input_sink sink, void *context)
{
    lib_u32 index;
    for (index = 0u; index < matcher->pending_count; ++index) {
        if (sink == LIB_NULL || !sink(context, &matcher->pending[index])) return 0;
    }
    matcher->pending_count = 0u;
    return 1;
}

static lib_bool ux_hotkey_is_suppressed(const ux_hotkey_matcher *matcher,
    const ux_input_event *event)
{
    lib_u32 index;
    if (matcher == LIB_NULL || event == LIB_NULL) return LIB_FALSE;
    for (index = 0u; index < matcher->suppressed_count; ++index)
        if (matcher->suppressed_keys[index].key ==
                event->data.key.key &&
            matcher->suppressed_keys[index].scan_code ==
                event->data.key.scan_code) return LIB_TRUE;
    return LIB_FALSE;
}

static void ux_hotkey_suppress_chord(ux_hotkey_matcher *matcher,
    const ux_input_event *trigger)
{
    lib_u32 index;
    matcher->suppressed_count = 0u;
    for (index = 0u; index < matcher->pending_count; ++index) {
        matcher->suppressed_keys[matcher->suppressed_count++] =
            (ux_hotkey_suppressed_key) {
                matcher->pending[index].data.key.key,
                matcher->pending[index].data.key.scan_code };
    }
    matcher->suppressed_keys[matcher->suppressed_count++] =
        (ux_hotkey_suppressed_key) { trigger->data.key.key,
            trigger->data.key.scan_code };
    matcher->pending_count = 0u;
}

void ux_hotkey_registry_initialize(ux_hotkey_registry *registry)
{
    if (registry != LIB_NULL) memset(registry, 0, sizeof(*registry));
}

lib_status ux_hotkey_registry_register(ux_hotkey_registry *registry,
    ux_key key, lib_u8 modifiers, const char *identifier)
{
    lib_u32 index;
    const char *end;

    if (registry == LIB_NULL || key == 0u || identifier == LIB_NULL ||
        (end = memchr(identifier, '\0', UX_HOTKEY_IDENTIFIER_CAPACITY)) == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    for (index = 0u; index < registry->count; ++index) {
        if (registry->entries[index].key == key &&
            registry->entries[index].modifiers == modifiers)
            return LIB_STATUS_INVALID_STATE;
    }
    if (registry->count == UX_HOTKEY_CAPACITY) return LIB_STATUS_LIMIT_EXCEEDED;
    registry->entries[registry->count] = (ux_hotkey_registration) { key, modifiers,
        { 0 } };
    memcpy(registry->entries[registry->count].identifier, identifier,
        (lib_size)(end - identifier) + 1u);
    ++registry->count;
    return LIB_STATUS_OK;
}

void ux_hotkey_matcher_initialize(ux_hotkey_matcher *matcher,
    const ux_hotkey_registry *registry)
{
    if (matcher == LIB_NULL) return;
    memset(matcher, 0, sizeof(*matcher));
    if (registry != LIB_NULL) matcher->registry = *registry;
}

int ux_hotkey_matcher_submit(ux_hotkey_matcher *matcher,
    const ux_input_event *event, ux_input_sink sink, void *context)
{
    const ux_hotkey_registration *matched;
    lib_u8 modifier;

    if (matcher == LIB_NULL || event == LIB_NULL || sink == LIB_NULL) return 0;
    if (event->type != UX_EVENT_KEY) {
        return ux_hotkey_flush_pending(matcher, sink, context) && sink(context, event);
    }
    if (event->data.key.pressed == 0u && ux_hotkey_is_suppressed(matcher,
            event)) {
        lib_u32 index;
        for (index = 0u; index < matcher->suppressed_count; ++index) {
            if (matcher->suppressed_keys[index].key ==
                    event->data.key.key &&
                matcher->suppressed_keys[index].scan_code ==
                    event->data.key.scan_code) {
                matcher->suppressed_keys[index] = matcher->suppressed_keys[
                    matcher->suppressed_count - 1u];
                --matcher->suppressed_count;
                break;
            }
        }
        return 1;
    }
    if (event->data.key.pressed != 0u && (matched = ux_hotkey_registry_match(
            &matcher->registry, event->data.key.key,
            event->data.key.modifiers)) != LIB_NULL) {
        ux_input_event hotkey = *event;
        ux_hotkey_suppress_chord(matcher, event);
        hotkey.type = UX_EVENT_HOTKEY;
        memcpy(hotkey.data.hotkey.identifier, matched->identifier,
            sizeof(hotkey.data.hotkey.identifier));
        return sink(context, &hotkey);
    }
    modifier = ux_hotkey_modifier_bit(event->data.key.key);
    if (event->data.key.pressed != 0u && modifier != 0u &&
        ux_hotkey_registry_has_modifier(&matcher->registry, modifier)) {
        if (matcher->pending_count == UX_HOTKEY_PENDING_CAPACITY &&
            !ux_hotkey_flush_pending(matcher, sink, context)) return 0;
        matcher->pending[matcher->pending_count++] = *event;
        return 1;
    }
    return ux_hotkey_flush_pending(matcher, sink, context) && sink(context, event);
}

void ux_hotkey_matcher_discard(ux_hotkey_matcher *matcher)
{
    if (matcher == LIB_NULL) return;
    matcher->pending_count = 0u;
    matcher->suppressed_count = 0u;
}
