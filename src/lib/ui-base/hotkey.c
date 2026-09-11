#include "lib/ui-base/hotkey_interface.h"

#include <string.h>

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

static int ui_hotkey_flush_pending(ui_hotkey_matcher *matcher,
    ui_input_sink sink, void *context)
{
    lib_u32 index;
    for (index = 0u; index < matcher->pending_count; ++index) {
        if (sink == LIB_NULL || !sink(context, &matcher->pending[index])) return 0;
    }
    matcher->pending_count = 0u;
    return 1;
}

static lib_bool ui_hotkey_is_suppressed(const ui_hotkey_matcher *matcher,
    const ui_input_event *event)
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

static void ui_hotkey_suppress_chord(ui_hotkey_matcher *matcher,
    const ui_input_event *trigger)
{
    lib_u32 index;
    matcher->suppressed_count = 0u;
    for (index = 0u; index < matcher->pending_count; ++index) {
        matcher->suppressed_keys[matcher->suppressed_count++] =
            (ui_hotkey_suppressed_key) {
                matcher->pending[index].data.key.key,
                matcher->pending[index].data.key.scan_code };
    }
    matcher->suppressed_keys[matcher->suppressed_count++] =
        (ui_hotkey_suppressed_key) { trigger->data.key.key,
            trigger->data.key.scan_code };
    matcher->pending_count = 0u;
}

void ui_hotkey_registry_initialize(ui_hotkey_registry *registry)
{
    if (registry != LIB_NULL) memset(registry, 0, sizeof(*registry));
}

lib_status ui_hotkey_registry_register(ui_hotkey_registry *registry,
    ui_key key, lib_u8 modifiers, const char *identifier)
{
    lib_u32 index;
    const char *end;

    if (registry == LIB_NULL || key == 0u || identifier == LIB_NULL ||
        (end = memchr(identifier, '\0', UI_HOTKEY_IDENTIFIER_CAPACITY)) == LIB_NULL)
        return LIB_STATUS_INVALID_ARGUMENT;
    for (index = 0u; index < registry->count; ++index) {
        if (registry->entries[index].key == key &&
            registry->entries[index].modifiers == modifiers)
            return LIB_STATUS_INVALID_STATE;
    }
    if (registry->count == UI_HOTKEY_CAPACITY) return LIB_STATUS_LIMIT_EXCEEDED;
    registry->entries[registry->count] = (ui_hotkey_registration) { key, modifiers,
        { 0 } };
    memcpy(registry->entries[registry->count].identifier, identifier,
        (lib_size)(end - identifier) + 1u);
    ++registry->count;
    return LIB_STATUS_OK;
}

void ui_hotkey_matcher_initialize(ui_hotkey_matcher *matcher,
    const ui_hotkey_registry *registry)
{
    if (matcher == LIB_NULL) return;
    memset(matcher, 0, sizeof(*matcher));
    if (registry != LIB_NULL) matcher->registry = *registry;
}

int ui_hotkey_matcher_submit(ui_hotkey_matcher *matcher,
    const ui_input_event *event, ui_input_sink sink, void *context)
{
    const ui_hotkey_registration *matched;
    lib_u8 modifier;

    if (matcher == LIB_NULL || event == LIB_NULL || sink == LIB_NULL) return 0;
    if (event->type != UI_EVENT_KEY) {
        return ui_hotkey_flush_pending(matcher, sink, context) && sink(context, event);
    }
    if (event->data.key.pressed == 0u && ui_hotkey_is_suppressed(matcher,
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
    if (event->data.key.pressed != 0u && (matched = ui_hotkey_registry_match(
            &matcher->registry, event->data.key.key,
            event->data.key.modifiers)) != LIB_NULL) {
        ui_input_event hotkey = *event;
        ui_hotkey_suppress_chord(matcher, event);
        hotkey.type = UI_EVENT_HOTKEY;
        memcpy(hotkey.data.hotkey.identifier, matched->identifier,
            sizeof(hotkey.data.hotkey.identifier));
        return sink(context, &hotkey);
    }
    modifier = ui_hotkey_modifier_bit(event->data.key.key);
    if (event->data.key.pressed != 0u && modifier != 0u &&
        ui_hotkey_registry_has_modifier(&matcher->registry, modifier)) {
        if (matcher->pending_count == UI_HOTKEY_PENDING_CAPACITY &&
            !ui_hotkey_flush_pending(matcher, sink, context)) return 0;
        matcher->pending[matcher->pending_count++] = *event;
        return 1;
    }
    return ui_hotkey_flush_pending(matcher, sink, context) && sink(context, event);
}

void ui_hotkey_matcher_discard(ui_hotkey_matcher *matcher)
{
    if (matcher == LIB_NULL) return;
    matcher->pending_count = 0u;
    matcher->suppressed_count = 0u;
}
