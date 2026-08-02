#include "core/machine/display.h"
#include "vm/composition_display.h"
#include "vm/platform/display_frame.h"
#include "vm/platform/platform.h"

#include <string.h>

static uint64_t vmCompositionDisplayGeneration;

void vm_composition_publish_display(int force)
{
    vm_platform_display_frame frame;
    uint16_t row;
    uint16_t column;
    int buffer_changed;
    int cursor_changed;

    core_machine_display_snapshot snapshot;

    memset(&snapshot, 0, sizeof(snapshot));
    if (!core_machine_display_capture_snapshot(&snapshot)) return;
    buffer_changed = snapshot.buffer_changed;
    cursor_changed = snapshot.cursor_changed;
    if (!force && !buffer_changed && !cursor_changed) return;

    memset(&frame, 0, sizeof(frame));
    frame.columns = snapshot.columns;
    frame.rows = snapshot.rows;
    if (frame.columns > VM_PLATFORM_DISPLAY_MAX_COLUMNS) {
        frame.columns = VM_PLATFORM_DISPLAY_MAX_COLUMNS;
    }
    if (frame.rows > VM_PLATFORM_DISPLAY_MAX_ROWS) {
        frame.rows = VM_PLATFORM_DISPLAY_MAX_ROWS;
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
            uint16_t index = row * VM_PLATFORM_DISPLAY_MAX_COLUMNS + column;
            frame.characters[index] = snapshot.characters[index];
            frame.attributes[index] = snapshot.attributes[index];
        }
    }
    frame.generation = ++vmCompositionDisplayGeneration;
    vm_platform_display_publish(&frame);
}

static void vmCompositionDisplayModeChanged(void *context)
{
    (void)context;
    vm_composition_publish_display(1);
    platformDisplaySetScreen();
}

void vm_composition_bind_display(void)
{
    core_machine_display_bind(NULL, vmCompositionDisplayModeChanged);
}
