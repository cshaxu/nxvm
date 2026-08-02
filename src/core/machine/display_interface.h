/* Product-neutral display-mode notification and text snapshot contracts. */
#ifndef NTVDM64_CORE_MACHINE_DISPLAY_INTERFACE_H
#define NTVDM64_CORE_MACHINE_DISPLAY_INTERFACE_H

#include <stdint.h>

#define CORE_MACHINE_DISPLAY_MAX_COLUMNS 80u
#define CORE_MACHINE_DISPLAY_MAX_ROWS 25u

typedef void (*core_machine_display_provider)(void *context);

typedef struct core_machine_display_snapshot {
    uint16_t columns;
    uint16_t rows;
    uint8_t cursor_top;
    uint8_t cursor_bottom;
    uint8_t cursor_x;
    uint8_t cursor_y;
    int cursor_visible;
    int buffer_changed;
    int cursor_changed;
    uint8_t characters[CORE_MACHINE_DISPLAY_MAX_COLUMNS * CORE_MACHINE_DISPLAY_MAX_ROWS];
    uint8_t attributes[CORE_MACHINE_DISPLAY_MAX_COLUMNS * CORE_MACHINE_DISPLAY_MAX_ROWS];
} core_machine_display_snapshot;

typedef int (*core_machine_display_snapshot_provider)(void *context,
    core_machine_display_snapshot *out_snapshot);

void core_machine_display_bind(void *context,
    core_machine_display_provider provider);
void core_machine_display_notify_mode_changed(void);
void core_machine_display_bind_snapshot_provider(void *context,
    core_machine_display_snapshot_provider provider);
int core_machine_display_capture_snapshot(core_machine_display_snapshot *out_snapshot);

#endif
