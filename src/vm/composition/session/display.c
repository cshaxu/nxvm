#include "type.h"

#include "core/machine/display_interface.h"

#include "vm/composition/session/display.h"

#include "core/platform/display_frame.h"

#include "vm/platform/platform.h"

#include "vm/composition/session/session_private.h"

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
    uint16_t row;
    uint16_t column;
    C_INT buffer_changed;
    C_INT cursor_changed;

    core_machine_display_snapshot snapshot;

    if (machine == STD_NULL) return CORE_MACHINE_DISPLAY_KIND_TEXT;
    STD_MEMSET(&snapshot, 0, sizeof(snapshot));
    if (!core_machine_display_capture_snapshot_from(&machine->display_provider,
        &snapshot)) return CORE_MACHINE_DISPLAY_KIND_TEXT;
    buffer_changed = snapshot.buffer_changed;
    cursor_changed = snapshot.cursor_changed;
    if (!force && !buffer_changed && !cursor_changed) return snapshot.kind;

    STD_MEMSET(&frame, 0, sizeof(frame));
    frame.kind = snapshot.kind == CORE_MACHINE_DISPLAY_KIND_CGA_320X200X4 ?
        CORE_PLATFORM_DISPLAY_KIND_INDEXED_PIXELS : CORE_PLATFORM_DISPLAY_KIND_TEXT;
    frame.buffer_changed = buffer_changed;
    frame.cursor_changed = cursor_changed;
    if (frame.kind == CORE_PLATFORM_DISPLAY_KIND_INDEXED_PIXELS) {
        frame.pixel_width = snapshot.pixel_width;
        frame.pixel_height = snapshot.pixel_height;
        STD_MEMCPY(frame.pixels, snapshot.pixels, sizeof(frame.pixels));
        STD_MEMCPY(frame.palette_rgb, snapshot.palette_rgb,
            sizeof(frame.palette_rgb));
        frame.generation = ++machine->display_generation;
        vm_platform_presentation_mailbox_publish(&machine->presentation_mailbox,
                                                  &frame);
        return snapshot.kind;
    }
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
    for (row = 0u; row < frame.rows; ++row) {
        for (column = 0u; column < frame.columns; ++column) {
            uint16_t index = row * CORE_PLATFORM_DISPLAY_MAX_COLUMNS + column;
            frame.characters[index] = snapshot.characters[index];
            frame.attributes[index] = snapshot.attributes[index];
        }
    }
    frame.generation = ++machine->display_generation;
    vm_platform_presentation_mailbox_publish(&machine->presentation_mailbox,
                                              &frame);
    return snapshot.kind;
}

static C_VOID vmCompositionDisplayModeChanged(C_VOID *context)
{
    vm_session *machine = context;

    vm_session_publish_display(machine, 1);
    vm_platform_display_set_screen(&machine->platform_run_context);
}

C_VOID vm_session_bind_display(vm_session *machine)
{
    if (machine == STD_NULL) return;
    core_machine_display_provider_slot_bind(&machine->display_provider,
        machine, vmCompositionDisplayModeChanged,
        machine, vm_session_capture_display_snapshot);
    core_machine_display_provider_slot_freeze(&machine->display_provider);
}
