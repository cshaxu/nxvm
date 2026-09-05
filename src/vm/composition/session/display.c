#include "type.h"

#include "core/machine/display_interface.h"

#include "vm/composition/session/display.h"

#include "core/platform/display_frame.h"

#include "vm/platform/platform.h"
#include "vm/platform/virtual_time.h"

#include "vm/composition/session/session_private.h"

#define VM_SESSION_DISPLAY_CADENCE_MILLISECONDS 16u

static C_INT vm_session_display_publish_is_due(vm_session *machine, C_INT force)
{
    type_unsigned_64 now;

    if (machine == STD_NULL || vm_platform_host_milliseconds(&now) != TYPE_STATUS_OK) {
        return TYPE_TRUE;
    }
    if (!force && now >= machine->last_display_publish_milliseconds &&
        now - machine->last_display_publish_milliseconds <
            VM_SESSION_DISPLAY_CADENCE_MILLISECONDS) {
        return TYPE_FALSE;
    }
    machine->last_display_publish_milliseconds = now;
    return TYPE_TRUE;
}

static C_INT vm_session_capture_display_snapshot(C_VOID *context,
    core_machine_display_snapshot *out_snapshot)
{
    vm_session *session = (vm_session *)context;

    return session != STD_NULL && core_machine_capture_display_snapshot(
        session->core_machine, out_snapshot) == TYPE_STATUS_OK;
}

core_machine_display_kind vm_session_publish_display(vm_session *machine,
    C_INT force)
{
    core_platform_display_frame frame;
    core_machine_display_snapshot_observation observation;
    type_unsigned_16 row;
    type_unsigned_16 column;
    C_INT buffer_changed;
    C_INT cursor_changed;

    core_machine_display_snapshot snapshot;

    if (machine == STD_NULL) return CORE_MACHINE_DISPLAY_KIND_TEXT;
    if (!vm_session_display_publish_is_due(machine, force)) return machine->display_kind;
    if (!force && core_machine_observe_display_snapshot(machine->core_machine,
            machine->display_snapshot_generation_valid,
            machine->display_snapshot_generation, &observation) == TYPE_STATUS_OK &&
        !observation.capture_required) {
        return machine->display_kind;
    }
    STD_MEMSET(&observation, 0, sizeof(observation));
    if (!core_machine_display_capture_snapshot_from(machine->display_provider,
        &snapshot)) return machine->display_kind;
    machine->display_kind = snapshot.kind;
    buffer_changed = snapshot.buffer_changed;
    cursor_changed = snapshot.cursor_changed;
    if (!force && !buffer_changed && !cursor_changed) return snapshot.kind;

    STD_MEMSET(&frame, 0, sizeof(frame));
    frame.kind = snapshot.kind != CORE_MACHINE_DISPLAY_KIND_TEXT ?
        CORE_PLATFORM_DISPLAY_KIND_INDEXED_PIXELS : CORE_PLATFORM_DISPLAY_KIND_TEXT;
    frame.buffer_changed = buffer_changed;
    frame.cursor_changed = cursor_changed;
    if (frame.kind == CORE_PLATFORM_DISPLAY_KIND_INDEXED_PIXELS) {
        frame.pixel_width = snapshot.pixel_width;
        frame.pixel_height = snapshot.pixel_height;
        STD_MEMCPY(frame.pixels, snapshot.pixels, sizeof(frame.pixels));
        STD_MEMCPY(frame.palette_rgb, snapshot.palette_rgb,
            sizeof(frame.palette_rgb));
    } else {
        frame.columns = snapshot.columns;
        frame.rows = snapshot.rows;
        if (frame.columns > CORE_PLATFORM_DISPLAY_MAX_COLUMNS) {
            frame.columns = CORE_PLATFORM_DISPLAY_MAX_COLUMNS;
        }
        if (frame.rows > CORE_PLATFORM_DISPLAY_MAX_ROWS) {
            frame.rows = CORE_PLATFORM_DISPLAY_MAX_ROWS;
        }
        frame.cursor_top = snapshot.cursor_top;
        frame.cursor_bottom = snapshot.cursor_bottom;
        frame.cursor_x = snapshot.cursor_x;
        frame.cursor_y = snapshot.cursor_y;
        frame.cursor_visible = snapshot.cursor_visible;
        frame.text_glyphs_present = snapshot.text_glyphs_present;
        STD_MEMCPY(frame.text_glyphs, snapshot.text_glyphs,
            sizeof(frame.text_glyphs));
        for (row = 0u; row < frame.rows; ++row) {
            for (column = 0u; column < frame.columns; ++column) {
                type_unsigned_16 index = row * CORE_PLATFORM_DISPLAY_MAX_COLUMNS + column;
                frame.characters[index] = snapshot.characters[index];
                frame.attributes[index] = snapshot.attributes[index];
            }
        }
    }
    frame.generation = machine->display_generation + 1u;
    if (core_platform_presentation_mailbox_publish(machine->presentation_mailbox,
            &frame) != TYPE_STATUS_OK) return snapshot.kind;
    machine->display_generation = frame.generation;
    if (core_machine_observe_display_snapshot(machine->core_machine,
            TYPE_FALSE, 0u, &observation) == TYPE_STATUS_OK &&
        observation.generation_reliable) {
        machine->display_snapshot_generation = observation.generation;
        machine->display_snapshot_generation_valid = TYPE_TRUE;
    } else {
        machine->display_snapshot_generation_valid = TYPE_FALSE;
    }
    return snapshot.kind;
}

static C_VOID vmCompositionDisplayModeChanged(C_VOID *context)
{
    vm_session *machine = context;

    vm_session_publish_display(machine, 1);
    vm_platform_display_set_screen(machine->platform_run_context);
}

C_VOID vm_session_bind_display(vm_session *machine)
{
    if (machine == STD_NULL) return;
    core_machine_display_provider_slot_bind(machine->display_provider,
        machine, vmCompositionDisplayModeChanged,
        machine, vm_session_capture_display_snapshot);
}
