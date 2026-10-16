#include "lib/types/types_interface.h"
#include "x86-nxvm/board-base/machine_board_interface.h"

#include "x86-nxvm/board-base/display_interface.h"

#include "x86-nxvm/machine/display.h"
#include "x86-nxvm/machine/frame_interface.h"

#include "lib/base/clock_interface.h"

#include "x86-nxvm/machine/machine_private.h"

#define X86_NXVM_MACHINE_DISPLAY_CADENCE_MILLISECONDS 16u

static lib_bool x86_nxvm_machine_display_publish_is_due(x86_nxvm_machine *machine, lib_bool force)
{
    lib_u64 now;

    if (machine == LIB_NULL || base_clock_milliseconds(&now) != LIB_STATUS_OK) {
        return LIB_TRUE;
    }
    if (!force && now >= machine->last_display_publish_milliseconds &&
        now - machine->last_display_publish_milliseconds <
            X86_NXVM_MACHINE_DISPLAY_CADENCE_MILLISECONDS) {
        return LIB_FALSE;
    }
    machine->last_display_publish_milliseconds = now;
    return LIB_TRUE;
}

static lib_status x86_nxvm_machine_capture_display_snapshot(void *context,
    x86_video_snapshot *out_snapshot)
{
    x86_nxvm_machine *session = (x86_nxvm_machine *)context;

    if (session == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    return x86_nxvm_board_capture_display_snapshot(session->board, out_snapshot);
}

lib_status x86_nxvm_machine_publish_display(x86_nxvm_machine *machine,
    lib_bool force)
{
    x86_video_snapshot_observation observation;
    lib_bool buffer_changed;
    lib_bool cursor_changed;
    x86_video_snapshot snapshot;
    lib_status status;

    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (!x86_nxvm_machine_display_publish_is_due(machine, force)) return LIB_STATUS_OK;
    if (!force && x86_nxvm_board_observe_display_snapshot(machine->board,
            machine->display_snapshot_generation_valid,
            machine->display_snapshot_generation, &observation) == LIB_STATUS_OK &&
        !observation.capture_required) {
        return LIB_STATUS_OK;
    }
    lib_memory_set(&observation, 0, sizeof(observation));
    status = x86_nxvm_board_display_capture_snapshot_from(machine->display_provider, &snapshot);
    if (status == LIB_STATUS_UNSUPPORTED) return LIB_STATUS_OK;
    if (status != LIB_STATUS_OK) return status;
    buffer_changed = snapshot.buffer_changed;
    cursor_changed = snapshot.cursor_changed;
    if (!force && !buffer_changed && !cursor_changed) return LIB_STATUS_OK;
    status = x86_nxvm_machine_frame_from_display(&snapshot, machine->display_generation + 1u,
        &machine->latest_frame);
    if (status != LIB_STATUS_OK) return status;
    machine->display_frame_pending = LIB_TRUE;
    machine->display_kind = snapshot.kind;
    ++machine->display_generation;
    if (x86_nxvm_board_observe_display_snapshot(machine->board,
            LIB_FALSE, 0u, &observation) == LIB_STATUS_OK &&
        observation.generation_reliable) {
        machine->display_snapshot_generation = observation.generation;
        machine->display_snapshot_generation_valid = LIB_TRUE;
    } else {
        machine->display_snapshot_generation_valid = LIB_FALSE;
    }
    return LIB_STATUS_OK;
}

void x86_nxvm_machine_bind_display(x86_nxvm_machine *machine)
{
    if (machine == LIB_NULL) return;
    x86_nxvm_board_display_provider_slot_bind(machine->display_provider,
        machine, x86_nxvm_machine_capture_display_snapshot);
}
