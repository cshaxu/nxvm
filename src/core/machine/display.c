#include "type.h"

#include "core/machine/display_interface.h"

struct core_machine_display_provider_slot {
    C_VOID *mode_context;
    core_machine_display_provider mode_provider;
    C_VOID *snapshot_context;
    core_machine_display_snapshot_provider snapshot_provider;
    C_INT frozen;
};

type_status core_machine_display_provider_slot_create(
    core_machine_display_provider_slot **out_slot)
{
    core_machine_display_provider_slot *slot;

    if (out_slot == STD_NULL) return TYPE_STATUS_INVALID_ARGUMENT;
    *out_slot = STD_NULL;
    slot = (core_machine_display_provider_slot *)STD_CALLOC(1u, sizeof(*slot));
    if (slot == STD_NULL) return TYPE_STATUS_NO_MEMORY;
    *out_slot = slot;
    return TYPE_STATUS_OK;
}

C_VOID core_machine_display_provider_slot_bind(
    core_machine_display_provider_slot *slot, C_VOID *mode_context,
    core_machine_display_provider mode_provider, C_VOID *snapshot_context,
    core_machine_display_snapshot_provider snapshot_provider)
{
    if (slot == STD_NULL || slot->frozen) return;
    slot->mode_context = mode_context;
    slot->mode_provider = mode_provider;
    slot->snapshot_context = snapshot_context;
    slot->snapshot_provider = snapshot_provider;
}

C_VOID core_machine_display_provider_slot_freeze(
    core_machine_display_provider_slot *slot)
{
    if (slot != STD_NULL) slot->frozen = 1;
}

C_VOID core_machine_display_provider_slot_destroy(
    core_machine_display_provider_slot *slot)
{
    STD_FREE(slot);
}

C_VOID core_machine_display_notify_mode_changed_to(
    const core_machine_display_provider_slot *slot)
{
    if (slot != STD_NULL && slot->mode_provider != STD_NULL) {
        slot->mode_provider(slot->mode_context);
    }
}

C_INT core_machine_display_capture_snapshot_from(
    const core_machine_display_provider_slot *slot,
    core_machine_display_snapshot *out_snapshot)
{
    if (slot == STD_NULL || slot->snapshot_provider == STD_NULL || out_snapshot == STD_NULL) {
        return 0;
    }
    return slot->snapshot_provider(slot->snapshot_context, out_snapshot);
}
