#include "lib/types/types_interface.h"
#include "core/board-base/machine_board_interface.h"

#include "core/board-base/display_interface.h"

#include "core/machine/display.h"
#include "core/machine/frame_interface.h"

#include "lib/base/clock_interface.h"

#include "core/machine/machine_private.h"

#define VM_MACHINE_DISPLAY_CADENCE_MILLISECONDS 16u

static lib_bool vm_machine_display_publish_is_due(vm_machine *machine, lib_bool force)
{
    lib_u64 now;

    if (machine == LIB_NULL || base_clock_milliseconds(&now) != LIB_STATUS_OK) {
        return LIB_TRUE;
    }
    if (!force && now >= machine->last_display_publish_milliseconds &&
        now - machine->last_display_publish_milliseconds <
            VM_MACHINE_DISPLAY_CADENCE_MILLISECONDS) {
        return LIB_FALSE;
    }
    machine->last_display_publish_milliseconds = now;
    return LIB_TRUE;
}

static lib_i32 vm_machine_capture_display_snapshot(void *context,
    x86_video_snapshot *out_snapshot)
{
    vm_machine *session = (vm_machine *)context;

    return session != LIB_NULL && core_machine_capture_display_snapshot(
        session->board, out_snapshot) == LIB_STATUS_OK;
}

lib_status vm_machine_publish_display(vm_machine *machine,
    lib_bool force)
{
    x86_video_snapshot_observation observation;
    lib_bool buffer_changed;
    lib_bool cursor_changed;
    x86_video_snapshot snapshot;
    lib_status status;

    if (machine == LIB_NULL) return LIB_STATUS_INVALID_ARGUMENT;
    if (!vm_machine_display_publish_is_due(machine, force)) return LIB_STATUS_OK;
    if (!force && core_machine_observe_display_snapshot(machine->board,
            machine->display_snapshot_generation_valid,
            machine->display_snapshot_generation, &observation) == LIB_STATUS_OK &&
        !observation.capture_required) {
        return LIB_STATUS_OK;
    }
    lib_memory_set(&observation, 0, sizeof(observation));
    if (!core_machine_display_capture_snapshot_from(machine->display_provider,
        &snapshot)) return LIB_STATUS_IO_ERROR;
    buffer_changed = snapshot.buffer_changed;
    cursor_changed = snapshot.cursor_changed;
    if (!force && !buffer_changed && !cursor_changed) return LIB_STATUS_OK;
    status = vm_machine_frame_from_display(&snapshot, machine->display_generation + 1u,
        &machine->latest_frame);
    if (status != LIB_STATUS_OK) return status;
    machine->latest_frame_valid = LIB_TRUE;
    machine->display_kind = snapshot.kind;
    ++machine->display_generation;
    if (core_machine_observe_display_snapshot(machine->board,
            LIB_FALSE, 0u, &observation) == LIB_STATUS_OK &&
        observation.generation_reliable) {
        machine->display_snapshot_generation = observation.generation;
        machine->display_snapshot_generation_valid = LIB_TRUE;
    } else {
        machine->display_snapshot_generation_valid = LIB_FALSE;
    }
    return LIB_STATUS_OK;
}

void vm_machine_bind_display(vm_machine *machine)
{
    if (machine == LIB_NULL) return;
    core_machine_display_provider_slot_bind(machine->display_provider,
        machine, vm_machine_capture_display_snapshot);
}
