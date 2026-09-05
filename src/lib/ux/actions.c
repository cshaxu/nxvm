#include "lib/ux/actions.h"

C_VOID ux_actions_initialize(ux_action_registry *registry)
{
    if (registry != STD_NULL) STD_MEMSET(registry, 0, sizeof(*registry));
}

type_status ux_actions_register(ux_action_registry *registry,
    type_unsigned_32 key, type_unsigned_8 modifiers, ux_action action)
{
    if (registry == STD_NULL || action == UX_ACTION_NONE ||
        registry->count >= UX_ACTION_CAPACITY) return TYPE_STATUS_INVALID_ARGUMENT;
    registry->entries[registry->count++] = (ux_action_chord) {
        key, modifiers, action
    };
    return TYPE_STATUS_OK;
}

ux_action ux_actions_match(const ux_action_registry *registry,
    type_unsigned_32 key, type_unsigned_8 modifiers)
{
    type_unsigned_32 index;

    if (registry == STD_NULL) return UX_ACTION_NONE;
    for (index = 0u; index < registry->count; ++index) {
        const ux_action_chord *entry = &registry->entries[index];
        if (entry->key == key && entry->modifiers == modifiers)
            return entry->action;
    }
    return UX_ACTION_NONE;
}
