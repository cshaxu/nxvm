#include "lib/base/base.h"
#include "lib/ux/actions.h"

void ux_actions_initialize(ux_action_registry *registry)
{
    if (registry != LIB_NULL) memset(registry, 0, sizeof(*registry));
}

lib_status ux_actions_register(ux_action_registry *registry,
    lib_u32 key, lib_u8 modifiers, ux_action action)
{
    lib_u32 index;

    if (registry == LIB_NULL || action == UX_ACTION_NONE)
        return LIB_STATUS_INVALID_ARGUMENT;
    for (index = 0u; index < registry->count; ++index) {
        if (registry->entries[index].key == key &&
            registry->entries[index].modifiers == modifiers)
            return LIB_STATUS_INVALID_STATE;
    }
    if (registry->count >= UX_ACTION_CAPACITY) return LIB_STATUS_INVALID_STATE;
    registry->entries[registry->count++] = (ux_action_chord) {
        key, modifiers, action
    };
    return LIB_STATUS_OK;
}

ux_action ux_actions_match(const ux_action_registry *registry,
    lib_u32 key, lib_u8 modifiers)
{
    lib_u32 index;

    if (registry == LIB_NULL) return UX_ACTION_NONE;
    for (index = 0u; index < registry->count; ++index) {
        const ux_action_chord *entry = &registry->entries[index];
        if (entry->key == key && entry->modifiers == modifiers)
            return entry->action;
    }
    return UX_ACTION_NONE;
}
