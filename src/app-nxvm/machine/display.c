#include "lib/types/types_interface.h"
#include "type.h"

#include "app-nxvm/devices/display_interface.h"

#include "app-nxvm/machine/display.h"
#include "app-nxvm/machine/frame.h"

#include "app-nxvm/devices/guest_display_frame.h"

#include "lib/base/clock_interface.h"

#include "app-nxvm/machine/machine_private.h"

#define VM_MACHINE_DISPLAY_CADENCE_MILLISECONDS 16u

static C_INT vm_machine_display_publish_is_due(vm_machine *machine, C_INT force)
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

static C_INT vm_machine_capture_display_snapshot(C_VOID *context,
    core_machine_display_snapshot *out_snapshot)
{
    vm_machine *session = (vm_machine *)context;

    return session != LIB_NULL && core_machine_capture_display_snapshot(
        session->core_machine, out_snapshot) == TYPE_STATUS_OK;
}

static C_VOID vm_machine_display_result(vm_machine *machine,
    const core_machine_guest_display_frame *frame)
{
    vm_machine_display_event display = {0};

    if (machine == LIB_NULL || frame == LIB_NULL) return;
    display.graphics = frame->kind == CORE_MACHINE_GUEST_DISPLAY_KIND_INDEXED_PIXELS;
    display.columns = frame->columns;
    display.rows = frame->rows;
    display.text_cell_height = frame->text_cell_height;
    display.cursor_top = frame->cursor_top;
    display.cursor_bottom = frame->cursor_bottom;
    display.cursor_x = frame->cursor_x;
    display.cursor_y = frame->cursor_y;
    display.cursor_visible = frame->cursor_visible;
    display.buffer_changed = frame->buffer_changed;
    display.cursor_changed = frame->cursor_changed;
    display.glyphs_present = frame->text_glyphs_present;
    display.pixel_width = frame->pixel_width;
    display.pixel_height = frame->pixel_height;
    display.generation = frame->generation;
    lib_memory_copy(display.characters, frame->characters, sizeof(display.characters));
    lib_memory_copy(display.attributes, frame->attributes, sizeof(display.attributes));
    lib_memory_copy(display.glyphs, frame->text_glyphs, sizeof(display.glyphs));
    lib_memory_copy(display.pixels, frame->pixels, sizeof(display.pixels));
    lib_memory_copy(display.palette_rgb, frame->palette_rgb, sizeof(display.palette_rgb));
    if (vm_machine_frame_from_display(&display, &machine->latest_frame) ==
        TYPE_STATUS_OK) machine->latest_frame_valid = LIB_TRUE;
}

core_machine_display_kind vm_machine_publish_display(vm_machine *machine,
    C_INT force)
{
    core_machine_guest_display_frame frame;
    core_machine_display_snapshot_observation observation;
    lib_u16 row;
    lib_u16 column;
    C_INT buffer_changed;
    C_INT cursor_changed;

    core_machine_display_snapshot snapshot;

    if (machine == LIB_NULL) return CORE_MACHINE_DISPLAY_KIND_TEXT;
    if (!vm_machine_display_publish_is_due(machine, force)) return machine->display_kind;
    if (!force && core_machine_observe_display_snapshot(machine->core_machine,
            machine->display_snapshot_generation_valid,
            machine->display_snapshot_generation, &observation) == TYPE_STATUS_OK &&
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

    lib_memory_set(&frame, 0, sizeof(frame));
    frame.kind = snapshot.kind != CORE_MACHINE_DISPLAY_KIND_TEXT ?
        CORE_MACHINE_GUEST_DISPLAY_KIND_INDEXED_PIXELS : CORE_MACHINE_GUEST_DISPLAY_KIND_TEXT;
    frame.buffer_changed = buffer_changed;
    frame.cursor_changed = cursor_changed;
    if (frame.kind == CORE_MACHINE_GUEST_DISPLAY_KIND_INDEXED_PIXELS) {
        frame.pixel_width = snapshot.pixel_width;
        frame.pixel_height = snapshot.pixel_height;
        lib_memory_copy(frame.pixels, snapshot.pixels, sizeof(frame.pixels));
        lib_memory_copy(frame.palette_rgb, snapshot.palette_rgb,
            sizeof(frame.palette_rgb));
    } else {
        frame.columns = snapshot.columns;
        frame.rows = snapshot.rows;
        frame.text_cell_height = snapshot.text_cell_height;
        if (frame.columns > CORE_MACHINE_GUEST_DISPLAY_MAX_COLUMNS) {
            frame.columns = CORE_MACHINE_GUEST_DISPLAY_MAX_COLUMNS;
        }
        if (frame.rows > CORE_MACHINE_GUEST_DISPLAY_MAX_ROWS) {
            frame.rows = CORE_MACHINE_GUEST_DISPLAY_MAX_ROWS;
        }
        frame.cursor_top = snapshot.cursor_top;
        frame.cursor_bottom = snapshot.cursor_bottom;
        frame.cursor_x = snapshot.cursor_x;
        frame.cursor_y = snapshot.cursor_y;
        frame.cursor_visible = snapshot.cursor_visible;
        frame.text_glyphs_present = snapshot.text_glyphs_present;
        lib_memory_copy(frame.text_glyphs, snapshot.text_glyphs,
            sizeof(frame.text_glyphs));
        lib_memory_copy(frame.palette_rgb, snapshot.palette_rgb,
            sizeof(frame.palette_rgb));
        for (row = 0u; row < frame.rows; ++row) {
            for (column = 0u; column < frame.columns; ++column) {
                lib_u16 index = row * CORE_MACHINE_GUEST_DISPLAY_MAX_COLUMNS + column;
                frame.characters[index] = snapshot.characters[index];
                frame.attributes[index] = snapshot.attributes[index];
            }
        }
    }
    frame.generation = machine->display_generation + 1u;
    vm_machine_display_result(machine, &frame);
    machine->display_generation = frame.generation;
    if (core_machine_observe_display_snapshot(machine->core_machine,
            LIB_FALSE, 0u, &observation) == TYPE_STATUS_OK &&
        observation.generation_reliable) {
        machine->display_snapshot_generation = observation.generation;
        machine->display_snapshot_generation_valid = LIB_TRUE;
    } else {
        machine->display_snapshot_generation_valid = LIB_FALSE;
    }
    return snapshot.kind;
}

static C_VOID vm_machine_display_mode_changed(C_VOID *context)
{
    vm_machine *machine = context;

    vm_machine_publish_display(machine, 1);
}

C_VOID vm_machine_bind_display(vm_machine *machine)
{
    if (machine == LIB_NULL) return;
    core_machine_display_provider_slot_bind(machine->display_provider,
        machine, vm_machine_display_mode_changed,
        machine, vm_machine_capture_display_snapshot);
}
