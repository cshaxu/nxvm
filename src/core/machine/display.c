#include "type.h"

#include "core/machine/display_interface.h"


#include <stddef.h>

C_VOID core_machine_display_provider_slot_initialize(
    core_machine_display_provider_slot *slot)
{
    if (slot != NULL) {
        slot->mode_context = NULL;
        slot->mode_provider = NULL;
        slot->snapshot_context = NULL;
        slot->snapshot_provider = NULL;
        slot->frozen = 0;
    }
}

C_VOID core_machine_display_provider_slot_bind(
    core_machine_display_provider_slot *slot, C_VOID *mode_context,
    core_machine_display_provider mode_provider, C_VOID *snapshot_context,
    core_machine_display_snapshot_provider snapshot_provider)
{
    if (slot == NULL || slot->frozen) return;
    slot->mode_context = mode_context;
    slot->mode_provider = mode_provider;
    slot->snapshot_context = snapshot_context;
    slot->snapshot_provider = snapshot_provider;
}

C_VOID core_machine_display_provider_slot_freeze(
    core_machine_display_provider_slot *slot)
{
    if (slot != NULL) slot->frozen = 1;
}

C_VOID core_machine_display_provider_slot_finalize(
    core_machine_display_provider_slot *slot)
{
    core_machine_display_provider_slot_initialize(slot);
}

C_VOID core_machine_display_notify_mode_changed_to(
    const core_machine_display_provider_slot *slot)
{
    if (slot != NULL && slot->mode_provider != NULL) {
        slot->mode_provider(slot->mode_context);
    }
}

C_INT core_machine_display_capture_snapshot_from(
    const core_machine_display_provider_slot *slot,
    core_machine_display_snapshot *out_snapshot)
{
    if (slot == NULL || slot->snapshot_provider == NULL || out_snapshot == NULL) {
        return 0;
    }
    return slot->snapshot_provider(slot->snapshot_context, out_snapshot);
}
