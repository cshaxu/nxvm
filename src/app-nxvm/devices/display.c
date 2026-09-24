#include "lib/types/types_interface.h"

#include "app-nxvm/devices/display_interface.h"

struct core_machine_display_provider_slot {
    void *mode_context;
    core_machine_display_provider mode_provider;
    void *snapshot_context;
    core_machine_display_snapshot_provider snapshot_provider;
    lib_i32 frozen;
};

lib_status core_machine_display_provider_slot_create(
    core_machine_display_provider_slot **out_slot)
{
    core_machine_display_provider_slot *slot;

    if (out_slot == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_slot = LIB_NULL;
    slot = (core_machine_display_provider_slot *)lib_allocate_zero(1u, sizeof(*slot));
    if (slot == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    *out_slot = slot;
    return LIB_STATUS_OK;
}

void core_machine_display_provider_slot_bind(
    core_machine_display_provider_slot *slot, void *mode_context,
    core_machine_display_provider mode_provider, void *snapshot_context,
    core_machine_display_snapshot_provider snapshot_provider)
{
    if (slot == LIB_NULL || slot->frozen) return;
    slot->mode_context = mode_context;
    slot->mode_provider = mode_provider;
    slot->snapshot_context = snapshot_context;
    slot->snapshot_provider = snapshot_provider;
}

void core_machine_display_provider_slot_freeze(
    core_machine_display_provider_slot *slot)
{
    if (slot != LIB_NULL) slot->frozen = 1;
}

void core_machine_display_provider_slot_destroy(
    core_machine_display_provider_slot *slot)
{
    lib_release(slot);
}

lib_i32 core_machine_display_capture_snapshot_from(
    const core_machine_display_provider_slot *slot,
    core_machine_display_snapshot *out_snapshot)
{
    if (slot == LIB_NULL || slot->snapshot_provider == LIB_NULL || out_snapshot == LIB_NULL) {
        return 0;
    }
    return slot->snapshot_provider(slot->snapshot_context, out_snapshot);
}
