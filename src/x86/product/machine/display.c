#include "lib/types/types_interface.h"
#include "x86/ibmpc-common/machine_board_interface.h"

#include "x86/ibmpc-common/display_interface.h"

#include "x86/product/machine/display.h"
#include "x86/product/machine/frame_interface.h"

#include "lib/base/clock_interface.h"

#include "x86/product/machine/machine_private.h"

#define VM_MACHINE_DISPLAY_CADENCE_MILLISECONDS 16u

static lib_i32 vm_machine_display_publish_is_due(vm_machine *machine, lib_i32 force)
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

x86_video_kind vm_machine_publish_display(vm_machine *machine,
    lib_i32 force)
{
    x86_video_snapshot_observation observation;
    lib_i32 buffer_changed;
    lib_i32 cursor_changed;

    x86_video_snapshot snapshot;

    if (machine == LIB_NULL) return X86_VIDEO_KIND_TEXT;
    if (!vm_machine_display_publish_is_due(machine, force)) return machine->display_kind;
    if (!force && core_machine_observe_display_snapshot(machine->board,
            machine->display_snapshot_generation_valid,
            machine->display_snapshot_generation, &observation) == LIB_STATUS_OK &&
        !observation.capture_required) {
        return machine->display_kind;
    }
    lib_memory_set(&observation, 0, sizeof(observation));
    if (!core_machine_display_capture_snapshot_from(machine->display_provider,
        &snapshot)) return machine->display_kind;
    machine->display_kind = snapshot.kind;
    buffer_changed = snapshot.buffer_changed;
    cursor_changed = snapshot.cursor_changed;
    if (!force && !buffer_changed && !cursor_changed) return snapshot.kind;

    if (snapshot.kind == X86_VIDEO_KIND_TEXT) {
        if (snapshot.columns > X86_VIDEO_MAX_COLUMNS)
            snapshot.columns = X86_VIDEO_MAX_COLUMNS;
        if (snapshot.rows > X86_VIDEO_MAX_ROWS)
            snapshot.rows = X86_VIDEO_MAX_ROWS;
    }
    if (vm_machine_frame_from_display(&snapshot, machine->display_generation + 1u,
            &machine->latest_frame) == LIB_STATUS_OK)
        machine->latest_frame_valid = LIB_TRUE;
    ++machine->display_generation;
    if (core_machine_observe_display_snapshot(machine->board,
            LIB_FALSE, 0u, &observation) == LIB_STATUS_OK &&
        observation.generation_reliable) {
        machine->display_snapshot_generation = observation.generation;
        machine->display_snapshot_generation_valid = LIB_TRUE;
    } else {
        machine->display_snapshot_generation_valid = LIB_FALSE;
    }
    return snapshot.kind;
}

void vm_machine_bind_display(vm_machine *machine)
{
    if (machine == LIB_NULL) return;
    core_machine_display_provider_slot_bind(machine->display_provider,
        machine, vm_machine_capture_display_snapshot);
}
