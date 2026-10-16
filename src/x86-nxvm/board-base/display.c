#include "lib/types/types_interface.h"

#include "x86-nxvm/board-base/display_interface.h"

struct x86_nxvm_board_display_provider_slot {
    void *snapshot_context;
    x86_nxvm_board_display_snapshot_provider snapshot_provider;
    lib_i32 frozen;
};

lib_status x86_nxvm_board_display_provider_slot_create(
    x86_nxvm_board_display_provider_slot **out_slot)
{
    x86_nxvm_board_display_provider_slot *slot;

    if (out_slot == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    *out_slot = LIB_NULL;
    slot = (x86_nxvm_board_display_provider_slot *)lib_allocate_zero(1u, sizeof(*slot));
    if (slot == LIB_NULL) return LIB_STATUS_NO_MEMORY;
    *out_slot = slot;
    return LIB_STATUS_OK;
}

void x86_nxvm_board_display_provider_slot_bind(
    x86_nxvm_board_display_provider_slot *slot, void *snapshot_context,
    x86_nxvm_board_display_snapshot_provider snapshot_provider)
{
    if (slot == LIB_NULL || slot->frozen) return;
    slot->snapshot_context = snapshot_context;
    slot->snapshot_provider = snapshot_provider;
}

void x86_nxvm_board_display_provider_slot_freeze(
    x86_nxvm_board_display_provider_slot *slot)
{
    if (slot != LIB_NULL) slot->frozen = 1;
}

void x86_nxvm_board_display_provider_slot_destroy(
    x86_nxvm_board_display_provider_slot *slot)
{
    lib_release(slot);
}

lib_status x86_nxvm_board_display_capture_snapshot_from(
    const x86_nxvm_board_display_provider_slot *slot,
    x86_video_snapshot *out_snapshot)
{
    if (slot == LIB_NULL || out_snapshot == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (slot->snapshot_provider == LIB_NULL) return LIB_STATUS_INVALID_STATE;
    return slot->snapshot_provider(slot->snapshot_context, out_snapshot);
}
