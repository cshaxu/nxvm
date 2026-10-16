#include "x86-nxvm/board-base/display_interface.h"

typedef struct capture_fixture {
    lib_u16 columns;
    lib_status status;
} capture_fixture;

static lib_status capture(void *context, x86_video_snapshot *snapshot)
{
    const capture_fixture *fixture = context;

    if (fixture->status != LIB_STATUS_OK) return fixture->status;
    snapshot->columns = fixture->columns;
    return LIB_STATUS_OK;
}

lib_i32 main(void)
{
    x86_nxvm_board_display_provider_slot *slot = LIB_NULL;
    x86_video_snapshot *snapshot;
    capture_fixture first = {7u, LIB_STATUS_OK};
    capture_fixture replacement = {9u, LIB_STATUS_OK};
    lib_i32 failed = 0;

    if (x86_nxvm_board_display_provider_slot_create(LIB_NULL) !=
            LIB_STATUS_INVALID_ARGUMENT) return 1;
    snapshot = lib_allocate_zero(1u, sizeof(*snapshot));
    if (snapshot == LIB_NULL) return 1;
    if (x86_nxvm_board_display_provider_slot_create(&slot) != LIB_STATUS_OK) {
        lib_release(snapshot);
        return 1;
    }
    failed |= x86_nxvm_board_display_capture_snapshot_from(slot, snapshot) != LIB_STATUS_INVALID_STATE;
    failed |= x86_nxvm_board_display_capture_snapshot_from(LIB_NULL, snapshot) != LIB_STATUS_INVALID_ARGUMENT;
    x86_nxvm_board_display_provider_slot_bind(slot, &first, capture);
    failed |= x86_nxvm_board_display_capture_snapshot_from(slot, snapshot) != LIB_STATUS_OK;
    failed |= snapshot->columns != first.columns;
    failed |= x86_nxvm_board_display_capture_snapshot_from(slot, LIB_NULL) != LIB_STATUS_INVALID_ARGUMENT;
    x86_nxvm_board_display_provider_slot_freeze(slot);
    x86_nxvm_board_display_provider_slot_bind(slot,
        &replacement, capture);
    first.columns = 11u;
    failed |= x86_nxvm_board_display_capture_snapshot_from(slot, snapshot) != LIB_STATUS_OK;
    failed |= snapshot->columns != first.columns;
    {
        static const lib_status failures[] = {
            LIB_STATUS_UNSUPPORTED, LIB_STATUS_IO_ERROR, LIB_STATUS_LIMIT_EXCEEDED
        };

        for (lib_size index = 0u; index < sizeof(failures) / sizeof(failures[0]); ++index) {
            first.status = failures[index];
            failed |= x86_nxvm_board_display_capture_snapshot_from(slot, snapshot) != failures[index] ||
                snapshot->columns != first.columns;
        }
        first.status = LIB_STATUS_OK;
        failed |= x86_nxvm_board_display_capture_snapshot_from(slot, snapshot) != LIB_STATUS_OK;
    }
    x86_nxvm_board_display_provider_slot_destroy(slot);
    x86_nxvm_board_display_provider_slot_destroy(LIB_NULL);
    x86_nxvm_board_display_provider_slot_bind(LIB_NULL, LIB_NULL, LIB_NULL);
    x86_nxvm_board_display_provider_slot_freeze(LIB_NULL);
    lib_release(snapshot);
    return failed;
}
