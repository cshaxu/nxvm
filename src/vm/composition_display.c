#include "core/machine/display_interface.h"
#include "vm/composition_display.h"
#include "core/platform/display_frame.h"
#include "vm/platform/platform.h"
#include "vm/composition_live_machine.h"

#include <string.h>

static uint64_t vmCompositionDisplayGeneration;

void vm_composition_publish_display(vm_composition_live_machine *machine,
    int force)
{
    core_platform_display_frame frame;
    uint16_t row;
    uint16_t column;
    int buffer_changed;
    int cursor_changed;

    core_machine_display_snapshot snapshot;

    if (machine == NULL) return;
    memset(&snapshot, 0, sizeof(snapshot));
    if (!core_machine_display_capture_snapshot_from(machine->display_provider,
        &snapshot)) return;
    buffer_changed = snapshot.buffer_changed;
    cursor_changed = snapshot.cursor_changed;
    if (!force && !buffer_changed && !cursor_changed) return;

    memset(&frame, 0, sizeof(frame));
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
    frame.buffer_changed = buffer_changed;
    frame.cursor_changed = cursor_changed;
    for (row = 0u; row < frame.rows; ++row) {
        for (column = 0u; column < frame.columns; ++column) {
            uint16_t index = row * CORE_PLATFORM_DISPLAY_MAX_COLUMNS + column;
            frame.characters[index] = snapshot.characters[index];
            frame.attributes[index] = snapshot.attributes[index];
        }
    }
    frame.generation = ++vmCompositionDisplayGeneration;
    core_platform_display_publish(&frame);
}

static void vmCompositionDisplayModeChanged(void *context)
{
    vm_composition_publish_display((vm_composition_live_machine *)context, 1);
    platformDisplaySetScreen();
}

void vm_composition_bind_display(vm_composition_live_machine *machine)
{
    if (machine == NULL) return;
    core_machine_display_bind(machine, vmCompositionDisplayModeChanged);
}
