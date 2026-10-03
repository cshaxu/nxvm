#include "x86/ibmpc-common/display_interface.h"

static lib_i32 capture(void *context, x86_video_snapshot *snapshot)
{
    snapshot->columns = *(const lib_u16 *)context;
    return 1;
}

lib_i32 main(void)
{
    core_machine_display_provider_slot *slot = LIB_NULL;
    x86_video_snapshot *snapshot;
    lib_u16 first = 7u;
    lib_u16 replacement = 9u;
    lib_i32 failed = 0;

    if (core_machine_display_provider_slot_create(LIB_NULL) !=
            LIB_STATUS_INVALID_ARGUMENT) return 1;
    snapshot = lib_allocate_zero(1u, sizeof(*snapshot));
    if (snapshot == LIB_NULL) return 1;
    if (core_machine_display_provider_slot_create(&slot) != LIB_STATUS_OK) {
        lib_release(snapshot);
        return 1;
    }
    failed |= core_machine_display_capture_snapshot_from(slot, snapshot) != 0;
    failed |= core_machine_display_capture_snapshot_from(LIB_NULL, snapshot) != 0;
    core_machine_display_provider_slot_bind(slot, &first, capture);
    failed |= !core_machine_display_capture_snapshot_from(slot, snapshot);
    failed |= snapshot->columns != first;
    failed |= core_machine_display_capture_snapshot_from(slot, LIB_NULL) != 0;
    core_machine_display_provider_slot_freeze(slot);
    core_machine_display_provider_slot_bind(slot,
        &replacement, capture);
    first = 11u;
    failed |= !core_machine_display_capture_snapshot_from(slot, snapshot);
    failed |= snapshot->columns != first;
    core_machine_display_provider_slot_destroy(slot);
    core_machine_display_provider_slot_destroy(LIB_NULL);
    core_machine_display_provider_slot_bind(LIB_NULL, LIB_NULL, LIB_NULL);
    core_machine_display_provider_slot_freeze(LIB_NULL);
    lib_release(snapshot);
    return failed;
}
